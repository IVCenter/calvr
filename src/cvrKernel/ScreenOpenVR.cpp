#include <cvrKernel/ScreenOpenVR.h>
#include <cvrKernel/CVRStatsHandler.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/NodeMask.h>
#include <cvrKernel/SceneManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrInput/TrackerBase.h>
#include <cvrInput/TrackerOpenVR.h>
#include <cvrConfig/ConfigManager.h>

#include <osgViewer/Renderer>
#include <osg/CullFace>
#include <osg/Texture>
#include <osg/Version>

#include <iostream>

#ifdef WIN32
#pragma comment(lib, "Winmm.lib")
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

bool ScreenOpenVR::_init = false;

ScreenOpenVR::ScreenOpenVR() :
        ScreenBase()
{
	_init = false;
}

ScreenOpenVR::~ScreenOpenVR()
{
}

void ScreenOpenVR::init(int mode)
{
	//Start up openvr - initialize system and compositor
	if(TrackerOpenVR::isInit())
	{
		vrDevice = TrackerOpenVR::getDevice();
		vrDevice->setNearClip(_near);
		vrDevice->setFarClip(_far);
		vrDevice->calculateProjectionMatrices();

		if (!vrDevice->hmdInitialized()) {
			return;
		}
	}
	else
	{
		vrDevice = new OpenVRDevice(_near, _far, 1000.0f, 1);

		if (!vrDevice->hmdInitialized()) {
			return;
		}
		vrDevice->init();
	}


	if (osgViewer::GraphicsWindow* win = dynamic_cast<osgViewer::GraphicsWindow*>(_myInfo->myChannel->myWindow->gc))
	{
		// Run wglSwapIntervalEXT(0) to force VSync Off
		win->setSyncToVBlank(false);
	}

	uint32_t renderWidth = 0;
	uint32_t renderHeight = 0;
	vrDevice->vrSystem()->GetRecommendedRenderTargetSize(&renderWidth, &renderHeight);

	osg::CullFace * cf = new osg::CullFace(osg::CullFace::BACK);
	osg::StateSet * stateset;

	_leftCamera = new osg::Camera();
	CVRViewer::instance()->addSlave(_leftCamera.get(), osg::Matrixd(), osg::Matrixd());
	defaultCameraInit(_leftCamera.get());
	_leftCamera->setAllowEventFocus(false);
	_leftCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_leftCamera->setReferenceFrame(osg::Transform::RELATIVE_RF);
	_leftCamera->setViewport(new osg::Viewport(0, 0, renderWidth, renderHeight));
	_leftCamera->setCullMask(CULL_MASK_LEFT);
	stateset = _leftCamera->getOrCreateStateSet();
	stateset->setAttributeAndModes(cf, osg::StateAttribute::ON);

	_leftCamera->setInitialDrawCallback(new SOVRInitialDrawCallback(vrDevice, OpenVRDevice::Eye::LEFT));

	osgViewer::Renderer * renderer =
		dynamic_cast<osgViewer::Renderer*>(_leftCamera->getRenderer());
	if (!renderer)
	{
		std::cerr << "Error getting renderer pointer." << std::endl;
	}
	else
	{
		osg::DisplaySettings * ds =
			renderer->getSceneView(0)->getDisplaySettings();
		ds->setStereo(false);
	}



	_rightCamera = new osg::Camera();
	//_rightCamera = vrDevice->createRTTCamera(OpenVRDevice::RIGHT, osg::Camera::ABSOLUTE_RF, osg::Vec4(0, 0, 0, 0), _myInfo->myChannel->myWindow->gc);
	CVRViewer::instance()->addSlave(_rightCamera.get(), osg::Matrixd(), osg::Matrixd());
	defaultCameraInit(_rightCamera.get());

	_rightCamera->setAllowEventFocus(false);
	_rightCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_rightCamera->setReferenceFrame(osg::Transform::RELATIVE_RF);
	_rightCamera->setViewport(new osg::Viewport(0, 0, renderWidth, renderHeight));
	_rightCamera->setCullMask(CULL_MASK_RIGHT);
	stateset = _rightCamera->getOrCreateStateSet();
	stateset->setAttributeAndModes(cf, osg::StateAttribute::ON);


	_rightCamera->setInitialDrawCallback(new SOVRInitialDrawCallback(vrDevice, OpenVRDevice::Eye::RIGHT));

	renderer =
		dynamic_cast<osgViewer::Renderer*>(_rightCamera->getRenderer());
	if (!renderer)
	{
		std::cerr << "Error getting renderer pointer." << std::endl;
	}
	else
	{
		osg::DisplaySettings * ds =
			renderer->getSceneView(0)->getDisplaySettings();
		ds->setStereo(false);
	}


	_swapCallback = new OpenVRSwapCallback(vrDevice);
	_myInfo->myChannel->myWindow->gc->setSwapCallback(_swapCallback);


	/*
	_previewCamera = new osg::Camera();

	osg::DisplaySettings * ds = new osg::DisplaySettings();
	_previewCamera->setDisplaySettings(ds);
	_previewCamera->setRenderOrder(osg::Camera::POST_RENDER);
	_previewCamera->setReferenceFrame(osg::Transform::RELATIVE_RF);
	_previewCamera->setCullingActive(false);

	defaultCameraInit(_previewCamera.get());

	
	osg::ref_ptr<osg::Vec2Array> quadArray = new osg::Vec2Array();
	quadArray->push_back(osg::Vec2(-100.0, 100.0));
	quadArray->push_back(osg::Vec2(-100.0, -100.0));
	quadArray->push_back(osg::Vec2(100.0, 100.0));
	quadArray->push_back(osg::Vec2(100.0, -100.0));

	osg::ref_ptr<osg::Vec2Array> texArray = new osg::Vec2Array();
	texArray->push_back(osg::Vec2(0, 0));
	texArray->push_back(osg::Vec2(0, 1));
	texArray->push_back(osg::Vec2(1, 0));
	texArray->push_back(osg::Vec2(1, 1));

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
	geom->setVertexArray(quadArray);
	geom->setTexCoordArray(0, texArray);
	geom->setUseDisplayList(false);
	geom->setUseVertexBufferObjects(true);
	geom->setCullingActive(false);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, 4));

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->setCullingActive(false);
	geode->addDrawable(geom);
	_previewCamera->addChild(geode);
	

	stateset = _previewCamera->getOrCreateStateSet();
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	*/

	_init = true;
}

void ScreenOpenVR::computeViewProj()
{
	vrDevice->updatePose();
}

void ScreenOpenVR::updateCamera()
{
	if (!_init)
	{
		return;
	}

	/*
	if (_previewCamera->getNumParents() == 0) {
		_leftCamera->addChild(_previewCamera);
	}
	*/

	double temp;

	osg::Vec3 position = vrDevice->position();
	osg::Quat orientation = vrDevice->orientation();
	temp = orientation[2];
	orientation[2] = orientation[1];
	orientation[1] = -temp;

	osg::Matrix leftView = vrDevice->viewMatrixLeft();
	leftView.preMultRotate(orientation);
	osg::Vec3d lv = leftView.getTrans() + position;
	temp = lv[2];
	lv[2] = lv[1];
	lv[1] = -temp;
	leftView.setTrans(lv);


	osg::Matrix rightView = vrDevice->viewMatrixRight();
	rightView.preMultRotate(orientation);
	osg::Vec3d rv = rightView.getTrans() + position;
	temp = rv[2];
	rv[2] = rv[1];
	rv[1] = -temp;
	rightView.setTrans(rv);



	_leftCamera->setViewMatrix(leftView * _myInfo->transform);
	_rightCamera->setViewMatrix(rightView * _myInfo->transform);


	_leftCamera->setProjectionMatrix(vrDevice->projectionMatrixLeft());
	_rightCamera->setProjectionMatrix(vrDevice->projectionMatrixRight());
}

void ScreenOpenVR::setClearColor(osg::Vec4 color)
{
	if (!_init)
	{
		return;
	}

	_leftCamera->setClearColor(color);
	_rightCamera->setClearColor(color);
}

ScreenInfo * ScreenOpenVR::findScreenInfo(osg::Camera * c)
{
	if (!_init)
	{
		return NULL;
	}

    if(c == _leftCamera.get() || c == _rightCamera.get())
    {
        return _myInfo;
    }
    return NULL;
}

void ScreenOpenVR::adjustViewportCoords(int & x, int & y)
{
    if(x > (_myInfo->myChannel->width / 2.0))
    {
        x = (int)(((float)x) - (_myInfo->myChannel->width / 2.0));
    }
    x *= 2;

    return;
}

