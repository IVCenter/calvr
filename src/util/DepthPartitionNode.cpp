/* OpenSceneGraph example, osgdepthpartion.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <util/DepthPartitionNode.h>
#include <osgUtil/CullVisitor>

#include <iostream>

using namespace osg;

#define CURRENT_CLASS DepthPartitionNode

CURRENT_CLASS::CURRENT_CLASS()
{
    _forwardOtherTraversals = true;
    init();
}

CURRENT_CLASS::CURRENT_CLASS(const CURRENT_CLASS& dpn, const osg::CopyOp& copyop)
    : osg::Group(dpn, copyop),
          _active(dpn._active),
          _renderOrder(dpn._renderOrder),
          _clearColorBuffer(dpn._clearColorBuffer),
	  _forwardOtherTraversals(dpn._forwardOtherTraversals)
{
    _numCameras = 0;
}

CURRENT_CLASS::~CURRENT_CLASS() {}

void CURRENT_CLASS::init()
{
    _active = true;
    _numCameras = 0;
    setCullingActive(false);
    _renderOrder = osg::Camera::POST_RENDER;
    _clearColorBuffer = true;
}

void CURRENT_CLASS::setActive(bool active)
{
    if(_active == active) return;
    _active = active;
}

void CURRENT_CLASS::setClearColorBuffer(bool clear)
{
    _clearColorBuffer = clear;

    for(std::map<int,CameraList>::iterator it = _cameraList.begin(); it != _cameraList.end(); it++)
    {
	if(!it->second.empty())
	{
	    if(clear) 
		it->second[0]->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    else  
		it->second[0]->setClearMask(GL_DEPTH_BUFFER_BIT);
	}
    }

    // Update the render order for the first Camera if it exists
    /*if(!_cameraList.empty())
    {
      if(clear) 
        _cameraList[0]->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      else  
        _cameraList[0]->setClearMask(GL_DEPTH_BUFFER_BIT);
    }*/
}

void CURRENT_CLASS::setRenderOrder(osg::Camera::RenderOrder order)
{
    _renderOrder = order;

    for(std::map<int,CameraList>::iterator it = _cameraList.begin(); it != _cameraList.end(); it++)
    {

	// Update the render order for existing Cameras
	unsigned int numCameras = it->second.size();
	for(unsigned int i = 0; i < numCameras; i++)
	{
	    it->second[i]->setRenderOrder(_renderOrder);
	}

    }
}

void CURRENT_CLASS::traverse(osg::NodeVisitor &nv)
{
    //printf("DEPTH ADDRESS %d\n", this);

    // If the scene hasn't been defined then don't do anything
    unsigned int numChildren = _children.size();
    if(numChildren == 0) return;

    // If the visitor is not a cull visitor, pass it directly onto the scene.
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);

    if(!_active)
    {
      // Traverse the graph as usual
      if(cv)
      {
	  Group::traverse(nv);
      }
      else if(_forwardOtherTraversals)
      {
	Group::traverse(nv);
      }
      return;
    }

    if(!cv) 
    { 
      if(_forwardOtherTraversals)
      {
	Group::traverse(nv);
      }
      return; 
    }

    int contextId = cv->getRenderInfo().getContextID();
    if( !_daMap[contextId] )  
    {
	_daMap[contextId] = new DistanceAccumulator();
    }

    //std::cerr << "This: " << this << " context: " << contextId << " DA: " << _daMap[contextId].get() << std::endl;
    // We are in the cull traversal, so first collect information on the
    // current modelview and projection matrices and viewport.
    osg::RefMatrix& modelview = *(cv->getModelViewMatrix());
    osg::RefMatrix& projection = *(cv->getProjectionMatrix());
    osg::Viewport* viewport = cv->getViewport();

    // Prepare for scene traversal.
     _daMap[contextId]->setMatrices(modelview, projection);
     _daMap[contextId]->setNearFarRatio(cv->getNearFarRatio());
     _daMap[contextId]->reset();

    // Step 1: Traverse the children, collecting the near/far distances.
    unsigned int i;
    for(i = 0; i < numChildren; i++)
    {
      _children[i]->accept(*( _daMap[contextId].get()));
    }

    // Step 2: Compute the near and far distances for every Camera that
    // should be used to render the scene.
     _daMap[contextId]->computeCameraPairs();

    // Step 3: Create the Cameras, and add them as children.
    DistanceAccumulator::PairList& camPairs =  _daMap[contextId]->getCameraPairs();
    unsigned int numCameras = camPairs.size(); // Get the number of cameras

    // Create the Cameras, and add them as children.
    if(numCameras > 0)
    {
      osg::Camera *currCam;
      DistanceAccumulator::DistancePair currPair;

      for(i = 0; i < numCameras; i++)
      {
        // Create the camera, and clamp it's projection matrix
        currPair = camPairs[i];  // (near,far) pair for current camera
        currCam = createOrReuseCamera(projection, currPair.first, 
                                      currPair.second, i, contextId);

        // TEMP fix Philip 
        currCam->setLODScale(0.0001);

        // Set the modelview matrix and viewport of the camera
        currCam->setViewMatrix(modelview);
        currCam->setViewport(viewport);

        // Redirect the CullVisitor to the current camera
        currCam->accept(nv);
      }

      // Set the clear color for the first camera
      _cameraList[contextId][0]->setClearColor(cv->getRenderStage()->getClearColor());
    }
}

bool CURRENT_CLASS::addChild(osg::Node *child)
{
    return insertChild(_children.size(), child);
}

bool CURRENT_CLASS::insertChild(unsigned int index, osg::Node *child)
{
    if(!Group::insertChild(index, child)) return false; // Insert child

    for(std::map<int,CameraList>::iterator it = _cameraList.begin(); it != _cameraList.end(); it++)
    {

	// Insert child into each Camera
	unsigned int totalCameras = it->second.size();
	for(unsigned int i = 0; i < totalCameras; i++)
	{
	    it->second[i]->insertChild(index, child);
	}

    }
    return true;
}


bool CURRENT_CLASS::removeChildren(unsigned int pos, unsigned int numRemove)
{
    if(!Group::removeChildren(pos, numRemove)) return false; // Remove child

    for(std::map<int,CameraList>::iterator it = _cameraList.begin(); it != _cameraList.end(); it++)
    {

	// Remove child from each Camera
	unsigned int totalCameras = it->second.size();
	for(unsigned int i = 0; i < totalCameras; i++)
	{
	    it->second[i]->removeChildren(pos, numRemove);
	}

    }
    return true;
}

bool CURRENT_CLASS::setChild(unsigned int i, osg::Node *node)
{
    if(!Group::setChild(i, node)) return false; // Set child

    for(std::map<int,CameraList>::iterator it = _cameraList.begin(); it != _cameraList.end(); it++)
    {

	// Set child for each Camera
	unsigned int totalCameras = it->second.size();
	for(unsigned int j = 0; j < totalCameras; j++)
	{
	    it->second[j]->setChild(i, node);
	}

    }
    return true;
}

void DepthPartitionNode::setForwardOtherTraversals(bool b)
{
    _forwardOtherTraversals = b;
}

bool DepthPartitionNode::getForwardOtherTraversals()
{
    return _forwardOtherTraversals;
}

osg::Camera* CURRENT_CLASS::createOrReuseCamera(const osg::Matrix& proj, 
                            double znear, double zfar, 
                            const unsigned int &camNum,int context)
{
    if(_cameraList[context].size() <= camNum) _cameraList[context].resize(camNum+1);
    osg::Camera *camera = _cameraList[context][camNum].get();
    
    if(!camera) // Create a new Camera
    {
      camera = new osg::Camera;
      camera->setCullingActive(false);
      camera->setRenderOrder(_renderOrder);
      camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

      // We will compute the near/far planes ourselves
      camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
      camera->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);

      if(camNum == 0 && _clearColorBuffer)
        camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      else
        camera->setClearMask(GL_DEPTH_BUFFER_BIT);

      // Add our children to the new Camera's children
      unsigned int numChildren = _children.size();
      for(unsigned int i = 0; i < numChildren; i++)
      {
        camera->addChild(_children[i].get());
      }

      _cameraList[context][camNum] = camera;
    }

    osg::Matrixd &projection = camera->getProjectionMatrix();
    projection = proj;

    // Slightly inflate the near & far planes to avoid objects at the
    // extremes being clipped out.
    znear *= 0.999;
    zfar *= 1.001;

    // Clamp the projection matrix z values to the range (near, far)
    double epsilon = 1.0e-6;
    if(fabs(projection(0,3)) < epsilon &&
       fabs(projection(1,3)) < epsilon &&
       fabs(projection(2,3)) < epsilon ) // Projection is Orthographic
    {
      epsilon = -1.0/(zfar - znear); // Used as a temp variable
      projection(2,2) = 2.0*epsilon;
      projection(3,2) = (zfar + znear)*epsilon;
    }
    else // Projection is Perspective
    {
      double trans_near = (-znear*projection(2,2) + projection(3,2)) /
                          (-znear*projection(2,3) + projection(3,3));
      double trans_far = (-zfar*projection(2,2) + projection(3,2)) /
                         (-zfar*projection(2,3) + projection(3,3));
      double ratio = fabs(2.0/(trans_near - trans_far));
      double center = -0.5*(trans_near + trans_far);

      projection.postMult(osg::Matrixd(1.0, 0.0, 0.0, 0.0,
                                       0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, ratio, 0.0,
                       0.0, 0.0, center*ratio, 1.0));
    }

    return camera;
}
#undef CURRENT_CLASS
