#ifdef WIN32
#include <GL/glew.h>
#define M_PI 3.141592653589793238462643
#pragma comment(lib, "Opengl32.lib")
#endif

#include <kernel/ScreenMultiViewer.h>
#include <kernel/CVRViewer.h>
#include <kernel/SceneManager.h>
#include <kernel/MultiViewerCullVisitor.h>
#include <input/TrackingManager.h>
#include <config/ConfigManager.h>

#include <osg/Scissor>
#include <osgViewer/Renderer>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/ShapeDrawable>

#include <iostream>
#include <string>
#include <cmath>

//#define FRAGMENT_QUERY

//TODO: add glewInit call for windows

using namespace cvr;

ScreenMultiViewer::ScreenMultiViewer() : ScreenBase()
{
    std::cerr << "Using Multi Viewer Screen" << std::endl;
    _testGeoAdded = false;
    _frameDelay = -5;
}

ScreenMultiViewer::~ScreenMultiViewer()
{
}

void ScreenMultiViewer::init(int mode)
{
    _stereoMode = (osg::DisplaySettings::StereoMode)mode;

    _camera = new osg::Camera();
    CVRViewer::instance()->addSlave(_camera.get(), osg::Matrixd(), osg::Matrixd());
    defaultCameraInit(_camera.get());

    _camera->getOrCreateStateSet()->setMode( GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    _camera->setCullingMode(osgUtil::CullVisitor::VIEW_FRUSTUM_CULLING);

    osg::Scissor * scissor = new osg::Scissor((int)_myInfo->myChannel->left,(int)_myInfo->myChannel->bottom,(int)_myInfo->myChannel->width,(int)_myInfo->myChannel->height);
    _camera->getOrCreateStateSet()->setAttributeAndModes(scissor,osg::StateAttribute::ON);

    PreDrawCallback * pdc = new PreDrawCallback;
    pdc->_screen = this;
    pdc->_init = false;
    pdc->first = true;

#ifdef FRAGMENT_QUERY
    PostDrawCallback * postdc = new PostDrawCallback;
    postdc->_pdc = pdc;
    postdc->_screen = this;
    postdc->first = true;
#endif

    if(_stereoMode == osg::DisplaySettings::HORIZONTAL_INTERLACE)
    {
	pdc->_index = 0;
	pdc->_indexState = PreDrawCallback::TOGGLE;

	osgViewer::Renderer * renderer =
	    dynamic_cast<osgViewer::Renderer*> (_camera->getRenderer());
	if(!renderer)
	{
	    std::cerr << "Error getting renderer pointer." << std::endl;
	}
	else
	{
	    osg::DisplaySettings * ds =
		renderer->getSceneView(0)->getDisplaySettings();
	    ds->setStereo(true);
	    ds->setStereoMode(_stereoMode);
	    StereoIdentCallback * sc = new StereoIdentCallback;
	    renderer->getSceneView(0)->setComputeStereoMatricesCallback(sc);
	    renderer->getSceneView(1)->setComputeStereoMatricesCallback(sc);

	    renderer->getSceneView(0)->setCullVisitorLeft(new MultiViewerCullVisitor());
	    renderer->getSceneView(1)->setCullVisitorLeft(new MultiViewerCullVisitor());
	    renderer->getSceneView(0)->setCullVisitorRight(new MultiViewerCullVisitor());
	    renderer->getSceneView(1)->setCullVisitorRight(new MultiViewerCullVisitor());
	}
    }
    else
    {
	osgViewer::Renderer * renderer =
	    dynamic_cast<osgViewer::Renderer*> (_camera->getRenderer());
	if(!renderer)
	{
	    std::cerr << "Error getting renderer pointer." << std::endl;
	}
	else
	{
	    //std::cerr << "adding MultiViewerCullVisitor" << std::endl;
	    renderer->getSceneView(0)->setCullVisitor(new MultiViewerCullVisitor());
	    renderer->getSceneView(1)->setCullVisitor(new MultiViewerCullVisitor());
	}

	pdc->_index = 0;
	pdc->_indexState = PreDrawCallback::FIXED;
    }
    

    _camera->setPreDrawCallback(pdc);

#ifdef FRAGMENT_QUERY
    _camera->setPostDrawCallback(postdc);
#endif

    std::string shaderdir;

    char * cvrHome = getenv("CALVR_HOME");
    if(cvrHome)
    {
	shaderdir = cvrHome;
	shaderdir = shaderdir + "/";
    }

    shaderdir = shaderdir + "shaders/";

    _vert = osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shaderdir + "multiviewer.vert"));
    _frag = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shaderdir + "multiviewer.frag"));
    _geom = osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile(shaderdir + "multiviewer.geom.7"));

    _program = new osg::Program;
    _program->addShader(_vert);
    _program->addShader(_geom);
    _program->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, 55 );
    _program->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES );
    _program->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP );
    _program->addShader(_frag);

    _camera->getOrCreateStateSet()->setAttribute(_program);

    float hwidth = _myInfo->width / 2.0;
    float hheight = _myInfo->height / 2.0;

    osg::Vec3 screenPoint(-hwidth,0.0,-hheight);

    screenPoint = screenPoint * _myInfo->transform;

    _screenCorner = new osg::Uniform();
    _screenCorner->setName("screenCorner");
    _screenCorner->setType(osg::Uniform::FLOAT_VEC3);
    _screenCorner->set(screenPoint);
    _camera->getOrCreateStateSet()->addUniform(_screenCorner);

    osg::Vec3 upPoint(-hwidth,0.0,hheight);

    upPoint = upPoint * _myInfo->transform;
    upPoint = upPoint - screenPoint;
    upPoint = upPoint / ((float)_myInfo->myChannel->height);

    _upPerPixel = new osg::Uniform();
    _upPerPixel->setName("upPerPixel");
    _upPerPixel->setType(osg::Uniform::FLOAT_VEC3);
    _upPerPixel->set(upPoint);
    _camera->getOrCreateStateSet()->addUniform(_upPerPixel);

    osg::Vec3 rightPoint(hwidth,0.0,-hheight);

    rightPoint = rightPoint * _myInfo->transform;
    rightPoint = rightPoint - screenPoint;
    rightPoint = rightPoint / ((float)_myInfo->myChannel->width);

    _rightPerPixel = new osg::Uniform();
    _rightPerPixel->setName("rightPerPixel");
    _rightPerPixel->setType(osg::Uniform::FLOAT_VEC3);
    _rightPerPixel->set(rightPoint);
    _camera->getOrCreateStateSet()->addUniform(_rightPerPixel);

    _viewer0Pos = new osg::Uniform();
    _viewer0Pos->setName("viewer0Pos");
    _viewer0Pos->setType(osg::Uniform::FLOAT_VEC3);
    _viewer0Pos->set(TrackingManager::instance()->getHeadMat(0).getTrans());
    _camera->getOrCreateStateSet()->addUniform(_viewer0Pos);

    _viewer1Pos = new osg::Uniform();
    _viewer1Pos->setName("viewer1Pos");
    _viewer1Pos->setType(osg::Uniform::FLOAT_VEC3);
    _viewer1Pos->set(TrackingManager::instance()->getHeadMat(1).getTrans());
    _camera->getOrCreateStateSet()->addUniform(_viewer1Pos);

    osg::Vec3 viewerdir(0.0,1.0,0.0);
    viewerdir = viewerdir * TrackingManager::instance()->getHeadMat(0);
    viewerdir = viewerdir - TrackingManager::instance()->getHeadMat(0).getTrans();
    viewerdir.normalize();

    _viewer0Dir = new osg::Uniform();
    _viewer0Dir->setName("viewer0Dir");
    _viewer0Dir->setType(osg::Uniform::FLOAT_VEC3);
    _viewer0Dir->set(viewerdir);
    _camera->getOrCreateStateSet()->addUniform(_viewer0Dir);

    viewerdir = osg::Vec3(0.0,1.0,0.0);
    viewerdir = viewerdir * TrackingManager::instance()->getHeadMat(1);
    viewerdir = viewerdir - TrackingManager::instance()->getHeadMat(1).getTrans();
    viewerdir.normalize();

    _viewer1Dir = new osg::Uniform();
    _viewer1Dir->setName("viewer1Dir");
    _viewer1Dir->setType(osg::Uniform::FLOAT_VEC3);
    _viewer1Dir->set(viewerdir);
    _camera->getOrCreateStateSet()->addUniform(_viewer1Dir);

    _viewer0Dist = new osg::Uniform();
    _viewer0Dist->setName("viewer0Dist");
    _viewer0Dist->setType(osg::Uniform::FLOAT);
    _camera->getOrCreateStateSet()->addUniform(_viewer0Dist);
    
    _viewer1Dist = new osg::Uniform();
    _viewer1Dist->setName("viewer1Dist");
    _viewer1Dist->setType(osg::Uniform::FLOAT);
    _camera->getOrCreateStateSet()->addUniform(_viewer1Dist);

    _viewportWidth = new osg::Uniform();
    _viewportWidth->setName("vwidth");
    _viewportWidth->setType(osg::Uniform::FLOAT);
    _viewportWidth->set(_myInfo->myChannel->width);
    _camera->getOrCreateStateSet()->addUniform(_viewportWidth);

    _viewportHeight = new osg::Uniform();
    _viewportHeight->setName("vheight");
    _viewportHeight->setType(osg::Uniform::FLOAT);
    _viewportHeight->set(_myInfo->myChannel->height);
    _camera->getOrCreateStateSet()->addUniform(_viewportHeight);

    _nearUni = new osg::Uniform();
    _nearUni->setName("near");
    _nearUni->setType(osg::Uniform::FLOAT);
    _nearUni->set((float)_near);
    _camera->getOrCreateStateSet()->addUniform(_nearUni);

    _farUni = new osg::Uniform();
    _farUni->setName("far");
    _farUni->setType(osg::Uniform::FLOAT);
    _farUni->set((float)_far);
    _camera->getOrCreateStateSet()->addUniform(_farUni);

    _maxRatio = new osg::Uniform();
    _maxRatio->setName("maxRatio");
    _maxRatio->setType(osg::Uniform::FLOAT);
    _maxRatio->set(1.0f);
    _camera->getOrCreateStateSet()->addUniform(_maxRatio);

    _minRatio = new osg::Uniform();
    _minRatio->setName("minRatio");
    _minRatio->setType(osg::Uniform::FLOAT);
    _minRatio->set(0.0f);
    _camera->getOrCreateStateSet()->addUniform(_minRatio);
}

void ScreenMultiViewer::computeViewProj()
{
    {
	osg::Vec3d eyePos;

	switch(_stereoMode)
	{
	    case osg::DisplaySettings::LEFT_EYE:
		eyePos = defaultLeftEye(0);
		_viewer0PosLocal[0] = eyePos;
		break;
	    case osg::DisplaySettings::RIGHT_EYE:
		eyePos = defaultRightEye(0);
		_viewer0PosLocal[0] = eyePos;
		break;
	    case osg::DisplaySettings::HORIZONTAL_INTERLACE:
		_viewer0PosLocal[0] = defaultLeftEye(0);
		_viewer0PosLocal[1] = defaultRightEye(0);
		break;
	    default:
		eyePos = eyePos * getCurrentHeadMatrix(0);
		_viewer0PosLocal[0] = eyePos;
		break;
	}

	//_viewer0Pos->set(eyePos);

	switch(_stereoMode)
	{
	    case osg::DisplaySettings::LEFT_EYE:
		eyePos = defaultLeftEye(1);
		_viewer1PosLocal[0] = eyePos;
		break;
	    case osg::DisplaySettings::RIGHT_EYE:
		eyePos = defaultRightEye(1);
		_viewer1PosLocal[0] = eyePos;
		break;
	    case osg::DisplaySettings::HORIZONTAL_INTERLACE:
		_viewer1PosLocal[0] = defaultLeftEye(1);
		_viewer1PosLocal[1] = defaultRightEye(1);
		break;
	    default:
		eyePos = osg::Vec3d(0,0,0) * getCurrentHeadMatrix(1);
		_viewer1PosLocal[0] = eyePos;
		break;
	}

	//_viewer1Pos->set(eyePos);

	osg::Vec3 viewerdir(0.0,1.0,0.0);
	viewerdir = viewerdir * getCurrentHeadMatrix(0);
	viewerdir = viewerdir - getCurrentHeadMatrix(0).getTrans();
	viewerdir.normalize();

	_viewer0Dir->set(viewerdir);

	if(TrackingManager::instance()->getNumHeads() >= 2)
	{
	    viewerdir = osg::Vec3(0.0,1.0,0.0);
	    viewerdir = viewerdir * getCurrentHeadMatrix(1);
	    viewerdir = viewerdir - getCurrentHeadMatrix(1).getTrans();
	    viewerdir.normalize();

	    _viewer1Dir->set(viewerdir);
	}
	else
	{
	    osg::Matrix rot, trans;
	    rot.makeRotate(M_PI / 2.0, osg::Vec3(0,0,1.0));
	    trans.makeTranslate(osg::Vec3(350,500,100));
	    viewerdir = osg::Vec3(0.0,1.0,0.0);
	    viewerdir = viewerdir * rot * getCurrentHeadMatrix(0) * trans;
	    viewerdir = viewerdir - (getCurrentHeadMatrix(0) * trans).getTrans();
	    viewerdir.normalize();
	    _viewer1Dir->set(viewerdir);
	    _viewer1PosLocal[0] = (getCurrentHeadMatrix(0) * trans).getTrans();
	}

    }

    calcScreenMinMaxRatio();

    osg::Vec3 viewer0BoundEyePos[2];
    osg::Vec3 viewer1BoundEyePos[2];

    for(int i = 0; i < 2; i++)
    {
	viewer0BoundEyePos[i] = _viewer0PosLocal[i] * (1.0 - _minRatioLocal[i]) + _viewer1PosLocal[i] * _minRatioLocal[i];
	//if(TrackingManager::instance()->getNumHeads() >= 2)
	//{
	    viewer1BoundEyePos[i] = _viewer0PosLocal[i] * (1.0 - _maxRatioLocal[i]) + _viewer1PosLocal[i] * _maxRatioLocal[i];
	//}
	//else
	//{
	//    viewer1BoundEyePos[i] = viewer0BoundEyePos[i];
	//}

	if(_stereoMode != osg::DisplaySettings::HORIZONTAL_INTERLACE)
	{
	    break;
	}
    }

    for(int i = 0; i < 2; i++)
    {
	computeDefaultViewProj(viewer0BoundEyePos[i],_viewer0View[i],_viewer0Proj[i],_viewer0DistLocal[i],_viewer0Frustum[i],_viewer0ScreenPos[i]);
	computeDefaultViewProj(viewer1BoundEyePos[i],_viewer1View[i],_viewer1Proj[i],_viewer1DistLocal[i],_viewer1Frustum[i],_viewer1ScreenPos[i]);

	if(_stereoMode != osg::DisplaySettings::HORIZONTAL_INTERLACE)
	{
	    break;
	}
    }

    _proj = osg::Matrix::identity();
    _view = osg::Matrix::identity();

    /*for(int i = 0; i < 2; i++)
    {
	//get eye position
	osg::Vec3d eyePos;

	int headNumber;

	if(TrackingManager::instance()->getNumHeads() >= 2)
	{
	    headNumber = i;
	}
	else
	{
	    headNumber = 0;
	}

	switch(_stereoMode)
	{
	    case osg::DisplaySettings::LEFT_EYE:
		eyePos = defaultLeftEye(headNumber);
		break;
	    case osg::DisplaySettings::RIGHT_EYE:
		eyePos = defaultRightEye(headNumber);
		break;
	    case osg::DisplaySettings::HORIZONTAL_INTERLACE:
		eyePos = defaultLeftEye(headNumber);
		break;
	    default:
		eyePos = eyePos * getCurrentHeadMatrix(headNumber);
		break;
	}

	if(!i)
	{
	    computeDefaultViewProj(eyePos,_viewer0View[0],_viewer0Proj[0],_viewer0DistLocal[0],_viewer0Frustum[0],_viewer0ScreenPos[0]);
	}
	else
	{
	    computeDefaultViewProj(eyePos,_viewer1View[0],_viewer1Proj[0],_viewer1DistLocal[0],_viewer1Frustum[0],_viewer1ScreenPos[0]);
	}


	if(_stereoMode == osg::DisplaySettings::HORIZONTAL_INTERLACE)
	{
	    eyePos = defaultRightEye(headNumber);
	    if(!i)
	    {
		computeDefaultViewProj(eyePos,_viewer0View[1],_viewer0Proj[1],_viewer0DistLocal[1],_viewer0Frustum[1],_viewer0ScreenPos[1]);
	    }
	    else
	    {
		computeDefaultViewProj(eyePos,_viewer1View[1],_viewer1Proj[1],_viewer1DistLocal[1],_viewer1Frustum[1],_viewer1ScreenPos[1]);
	    }

	}

	_proj = osg::Matrix::identity();
	_view = osg::Matrix::identity();

    }*/

    float hwidth = _myInfo->width / 2.0;
    float hheight = _myInfo->height / 2.0;

    osg::Vec3 screenTL(-hwidth,0,hheight);
    osg::Vec3 screenTR(hwidth,0,hheight);
    osg::Vec3 screenBL(-hwidth,0,-hheight);
    osg::Vec3 screenBR(hwidth,0,-hheight);

    screenTL = screenTL * _myInfo->transform;
    screenTR = screenTR * _myInfo->transform;
    screenBL = screenBL * _myInfo->transform;
    screenBR = screenBR * _myInfo->transform;


    for(int i = 0; i < 2; i++)
    {
	osg::Vec3 point, normal;
	osg::Vec3 a,b;

	osg::Polytope::PlaneList plistNear;
	osg::Polytope::PlaneList plistFar;

	if(_viewer0ScreenPos[i].y() < _viewer1ScreenPos[i].y())
	{
	    // Near frust Near plane
	    point = _viewer0Frustum[i].nearTL;
	    a = _viewer0Frustum[i].nearTR - point;
	    b = _viewer0Frustum[i].nearBL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));

	    // Far frust Far plane
	    point = _viewer1Frustum[i].farTL;
	    a = _viewer1Frustum[i].farBL - point;
	    b = _viewer1Frustum[i].farTR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));
	}
	else
	{
	    // Near frust Near plane
	    point = _viewer1Frustum[i].nearTL;
	    a = _viewer1Frustum[i].nearTR - point;
	    b = _viewer1Frustum[i].nearBL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));

	    // Far frust Far plane
	    point = _viewer0Frustum[i].farTL;
	    a = _viewer0Frustum[i].farBL - point;
	    b = _viewer0Frustum[i].farTR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));
	}

	// Near frust Far plane
	point = screenTL;
	a = screenBL - point;
	b = screenTR - point;
	normal = a ^ b;
	normal.normalize();
	plistNear.push_back(osg::Plane(normal, point));
	
	// Far frust Near plane
	point = screenTL;
	a = screenTR - point;
	b = screenBL - point;
	normal = a ^ b;
	normal.normalize();
	plistFar.push_back(osg::Plane(normal, point));

	if(_viewer0ScreenPos[i].x() < _viewer1ScreenPos[i].x())
	{
	    // Near frust Left plane
	    point = _viewer0Frustum[i].nearTL;
	    a = _viewer0Frustum[i].nearBL - point;
	    b = screenTL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));
	    
	    // Far frust Right plane
	    point = screenTR;
	    a = _viewer0Frustum[i].farTR - point;
	    b = screenBR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));

	    // Near frust Right plane
	    point = _viewer1Frustum[i].nearTR;
	    a = screenTR - point;
	    b = _viewer1Frustum[i].nearBR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));
	    
	    // Far frust Left plane
	    point = screenTL;
	    a = screenBL - point;
	    b = _viewer1Frustum[i].farTL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));
	}
	else
	{
	    // Near frust Left plane
	    point = _viewer1Frustum[i].nearTL;
	    a = _viewer1Frustum[i].nearBL - point;
	    b = screenTL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));
	    
	    // Far frust Right plane
	    point = screenTR;
	    a = _viewer1Frustum[i].farTR - point;
	    b = screenBR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));

	    // Near frust Right plane
	    point = _viewer0Frustum[i].nearTR;
	    a = screenTR - point;
	    b = _viewer0Frustum[i].nearBR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));
	    
	    // Far frust Left plane
	    point = screenTL;
	    a = screenBL - point;
	    b = _viewer0Frustum[i].farTL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));
	}

	if(_viewer0ScreenPos[i].z() < _viewer1ScreenPos[i].z())
	{
	    // Near frust Bottom plane
	    point = _viewer0Frustum[i].nearBL;
	    a = _viewer0Frustum[i].nearBR - point;
	    b = screenBL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));

	    // Far frust Top plane
	    point = screenTL;
	    a = _viewer0Frustum[i].farTL - point;
	    b = screenTR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));

	    // Near frust Top plane
	    point = _viewer1Frustum[i].nearTL;
	    a = screenTL - point;
	    b = _viewer1Frustum[i].nearTR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));
	    
	    // Far frust Bottom plane
	    point = screenBL;
	    a = screenBR - point;
	    b = _viewer1Frustum[i].farBL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));
	}
	else
	{
	    // Near frust Bottom plane
	    point = _viewer1Frustum[i].nearBL;
	    a = _viewer1Frustum[i].nearBR - point;
	    b = screenBL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));

	    // Far frust Top plane
	    point = screenTL;
	    a = _viewer1Frustum[i].farTL - point;
	    b = screenTR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));

	    // Near frust Top plane
	    point = _viewer0Frustum[i].nearTL;
	    a = screenTL - point;
	    b = _viewer0Frustum[i].nearTR - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistNear.push_back(osg::Plane(normal, point));
	    
	    // Far frust Bottom plane
	    point = screenBL;
	    a = screenBR - point;
	    b = _viewer0Frustum[i].farBL - point;
	    normal = a ^ b;
	    normal.normalize();
	    plistFar.push_back(osg::Plane(normal, point));
	}

	_cullFrustumNear[i].set(plistNear);
	_cullFrustumFar[i].set(plistFar);

	// Fox in socks my day is done sir. Thank you for a lot of fun sir.

	if(_stereoMode != osg::DisplaySettings::HORIZONTAL_INTERLACE)
	{
	    break;
	}
    }

#if 0

    // simple test of cull frustum
    osg::Vec3 testpoint(0.0,-100.0,0.0);
    std::cerr << "Test point near: " << _cullFrustumNear[0].contains(testpoint) << " far: " << _cullFrustumFar[0].contains(testpoint) << std::endl;
    osg::Polytope::PlaneList pl;
    pl = _cullFrustumNear[0].getPlaneList();
    for(int i = 0; i < pl.size(); i++)
    {
	std::cerr << "Near Dist: " << pl[i].distance(testpoint) << std::endl;
    }

    pl = _cullFrustumFar[0].getPlaneList();
    for(int i = 0; i < pl.size(); i++)
    {
	std::cerr << "Far Dist: " << pl[i].distance(testpoint) << std::endl;
    }

#endif

    osgViewer::Renderer * renderer =
            dynamic_cast<osgViewer::Renderer*> (_camera->getRenderer());
    if(!renderer)
    {
        std::cerr << "Error getting renderer pointer." << std::endl;
    }
    else
    {
	for(int i = 0; i < 2; i++)
	{
	    if(_stereoMode == osg::DisplaySettings::HORIZONTAL_INTERLACE)
	    {
		MultiViewerCullVisitor * mvcv = dynamic_cast<MultiViewerCullVisitor*>(renderer->getSceneView(i)->getCullVisitorLeft());
		if(mvcv)
		{
		    mvcv->setFrustums(_cullFrustumNear[0],_cullFrustumFar[0]);
		}
		mvcv = dynamic_cast<MultiViewerCullVisitor*>(renderer->getSceneView(i)->getCullVisitorRight());
		if(mvcv)
		{
		    mvcv->setFrustums(_cullFrustumNear[1],_cullFrustumFar[1]);
		}
	    }
	    else
	    {
		MultiViewerCullVisitor * mvcv = dynamic_cast<MultiViewerCullVisitor*>(renderer->getSceneView(i)->getCullVisitor());
		if(mvcv)
		{
		    mvcv->setFrustums(_cullFrustumNear[0],_cullFrustumFar[0]);
		}
		else
		{
		    std::cerr << "Error getting mvcv." << std::endl;
		}
	    }
	}
    }

    //algtest();
}

void ScreenMultiViewer::updateCamera()
{
    //std::cerr << "Frame" << std::endl;
    if(!_testGeoAdded && _frameDelay > 0)
    {
	/*osg::Sphere * sphere = new osg::Sphere(osg::Vec3(0,0,0),100);
	osg::ShapeDrawable * sd = new osg::ShapeDrawable(sphere);
	osg::Geode * geode = new osg::Geode();
	geode->addDrawable(sd);
	SceneManager::instance()->getObjectsRoot()->addChild(geode);*/
		
	//addTestGeometry();
	_testGeoAdded = true;
    }
    else if(!_testGeoAdded)
    {
	_frameDelay++;
    }

    _camera->setViewMatrix(_view);
    _camera->setProjectionMatrix(_proj);

    /*osg::Vec3d eyePos;

    switch(_stereoMode)
    {
	case osg::DisplaySettings::LEFT_EYE:
	    eyePos = defaultLeftEye(0);
	    _viewer0PosLocal[0] = eyePos;
	    break;
	case osg::DisplaySettings::RIGHT_EYE:
	    eyePos = defaultRightEye(0);
	    _viewer0PosLocal[0] = eyePos;
	    break;
	case osg::DisplaySettings::HORIZONTAL_INTERLACE:
	    _viewer0PosLocal[0] = defaultLeftEye(0);
	    _viewer0PosLocal[1] = defaultRightEye(0);
	    break;
	default:
	    eyePos = eyePos * getCurrentHeadMatrix(0);
	    _viewer0PosLocal[0] = eyePos;
	    break;
    }

    //_viewer0Pos->set(eyePos);

    switch(_stereoMode)
    {
	case osg::DisplaySettings::LEFT_EYE:
	    eyePos = defaultLeftEye(1);
	    _viewer1PosLocal[0] = eyePos;
	    break;
	case osg::DisplaySettings::RIGHT_EYE:
	    eyePos = defaultRightEye(1);
	    _viewer1PosLocal[0] = eyePos;
	    break;
	case osg::DisplaySettings::HORIZONTAL_INTERLACE:
	    _viewer1PosLocal[0] = defaultLeftEye(1);
	    _viewer1PosLocal[1] = defaultRightEye(1);
	    break;
	default:
	    eyePos = osg::Vec3d(0,0,0) * getCurrentHeadMatrix(1);
	    _viewer1PosLocal[0] = eyePos;
	    break;
    }

    //_viewer1Pos->set(eyePos);

    osg::Vec3 viewerdir(0.0,1.0,0.0);
    viewerdir = viewerdir * getCurrentHeadMatrix(0);
    viewerdir = viewerdir - getCurrentHeadMatrix(0).getTrans();
    viewerdir.normalize();

    _viewer0Dir->set(viewerdir);

    if(TrackingManager::instance()->getNumHeads() >= 2)
    {
	viewerdir = osg::Vec3(0.0,1.0,0.0);
	viewerdir = viewerdir * getCurrentHeadMatrix(1);
	viewerdir = viewerdir - getCurrentHeadMatrix(1).getTrans();
	viewerdir.normalize();

	_viewer1Dir->set(viewerdir);
    }
    else
    {
	_viewer1Dir->set(viewerdir);
    }*/
}

void ScreenMultiViewer::setClearColor(osg::Vec4 color)
{
    _camera->setClearColor(color);
}

ScreenInfo * ScreenMultiViewer::findScreenInfo(osg::Camera * c)
{
    if(c == _camera.get())
    {
        return _myInfo;
    }
    return NULL;
}

void ScreenMultiViewer::computeDefaultViewProj(osg::Vec3d eyePos, osg::Matrix & view, osg::Matrix & proj, float & dist, struct FrustumPoints & fp, osg::Vec3 & viewerScreenPos)
{
    //translate screen to origin
    osg::Matrix screenTrans;
    screenTrans.makeTranslate(-_myInfo->xyz);

    //rotate screen to xz
    osg::Matrix screenRot;
    screenRot.makeRotate(-_myInfo->h * M_PI / 180.0, osg::Vec3(0, 0, 1),
			 -_myInfo->p * M_PI / 180.0, osg::Vec3(1, 0, 0),
			 -_myInfo->r * M_PI / 180.0, osg::Vec3(0, 1, 0));

    eyePos = eyePos * screenTrans * screenRot;

    viewerScreenPos = eyePos;

    //make frustum
    float top, bottom, left, right;
    float screenDist = -eyePos.y();
    dist = -screenDist;
    //std::cerr << "Setting viewerDist to: " << dist << std::endl;

    top = _near * (_myInfo->height / 2.0 - eyePos.z()) / screenDist;
    bottom = _near * (-_myInfo->height / 2.0 - eyePos.z()) / screenDist;
    right = _near * (_myInfo->width / 2.0 - eyePos.x()) / screenDist;
    left = _near * (-_myInfo->width / 2.0 - eyePos.x()) / screenDist;

    proj.makeFrustum(left, right, bottom, top, _near, _far);

    osg::Matrix cam2world;
    cam2world.makeTranslate(eyePos);
    cam2world = cam2world * _myInfo->transform;

    fp.nearTL = osg::Vec3(left, _near, top) * cam2world;
    fp.nearTR = osg::Vec3(right, _near, top) * cam2world;
    fp.nearBL = osg::Vec3(left, _near, bottom) * cam2world;
    fp.nearBR = osg::Vec3(right, _near, bottom) * cam2world;

    float ftop, fbottom, fleft, fright;

    ftop = _far * (_myInfo->height / 2.0 - eyePos.z()) / screenDist;
    fbottom = _far * (-_myInfo->height / 2.0 - eyePos.z()) / screenDist;
    fright = _far * (_myInfo->width / 2.0 - eyePos.x()) / screenDist;
    fleft = _far * (-_myInfo->width / 2.0 - eyePos.x()) / screenDist;

    fp.farTL = osg::Vec3(fleft, _far, ftop) * cam2world;
    fp.farTR = osg::Vec3(fright, _far, ftop) * cam2world;
    fp.farBL = osg::Vec3(fleft, _far, fbottom) * cam2world;
    fp.farBR = osg::Vec3(fright, _far, fbottom) * cam2world;

    // move camera to origin
    osg::Matrix cameraTrans;
    cameraTrans.makeTranslate(-eyePos);

    //make view
    view = screenTrans * screenRot * cameraTrans
	* osg::Matrix::lookAt(osg::Vec3(0, 0, 0), osg::Vec3(0, 1, 0),
		osg::Vec3(0, 0, 1));
}

void ScreenMultiViewer::algtest()
{
    /*osg::Vec4 point0,point1,point2;
    point0 = osg::Vec4(10,500,0,1.0);
    point2 = osg::Vec4(-10,250,2000,1.0);
    point1 = osg::Vec4(2000,-500,0,1.0);
    float fragx = 0.2;
    float fragy = 0.2;

    osg::Vec4 nearpoint;
    nearpoint.x() = (fragx) * 100.0;
    nearpoint.y() = (fragy) * 100.0;
    nearpoint.w() = 100.0;

    nearpoint = nearpoint * osg::Matrix::inverse(_viewer0Proj);

    osg::Matrix invproj = osg::Matrix::inverse(_viewer0Proj);
    std::cerr << "Inv Proj Matrix:" << std::endl;
    for(int i = 0; i < 4; i++)
    {
	for(int j = 0; j < 4; j++)
	{
	    std::cerr << invproj(j,i) << " ";
	}
	std::cerr << std::endl;
    }

    std::cerr << "Invproj near x: " << nearpoint.x() << " y: " << nearpoint.y() << " z: " << nearpoint.z() << " w: " << nearpoint.w() << std::endl;

    nearpoint.w() = 1.0;

    nearpoint = nearpoint * osg::Matrix::inverse(_viewer0View);

    osg::Matrix invview = osg::Matrix::inverse(_viewer0View);
    std::cerr << "Inv View Matrix:" << std::endl;
    for(int i = 0; i < 4; i++)
    {
	for(int j = 0; j < 4; j++)
	{
	    std::cerr << invview(j,i) << " ";
	}
	std::cerr << std::endl;
    }

    std::cerr << "Invview near x: " << nearpoint.x() << " y: " << nearpoint.y() << " z: " << nearpoint.z() << " w: " << nearpoint.w() << std::endl;

    osg::Vec4 farpoint;
    farpoint.x() = (fragx) * 10000.0;
    farpoint.y() = (fragy) * 10000.0;
    farpoint.w() = 10000.0;

    farpoint = farpoint * osg::Matrix::inverse(_viewer0Proj);

    std::cerr << "Invproj far x: " << farpoint.x() << " y: " << farpoint.y() << " z: " << farpoint.z() << " w: " << farpoint.w() << std::endl;

    farpoint.w() = 1.0;

    farpoint = farpoint * osg::Matrix::inverse(_viewer0View);

    std::cerr << "Invview far x: " << farpoint.x() << " y: " << farpoint.y() << " z: " << farpoint.z() << " w: " << farpoint.w() << std::endl;

    osg::Matrix solve;

    osg::Vec4 point;

    point = farpoint - nearpoint;

    solve(0,0) = point.x();
    solve(0,1) = point.y();
    solve(0,2) = point.z();

    point = point1 - point0;

    solve(1,0) = point.x();
    solve(1,1) = point.y();
    solve(1,2) = point.z();

    point = point2 - point0;
    
    solve(2,0) = point.x();
    solve(2,1) = point.y();
    solve(2,2) = point.z();

    solve = osg::Matrix::inverse(solve);

    point = farpoint - point0;

    point = point * solve;

    std::cerr << "Solution point x: " << point.x() << " y: " << point.y() << " z: " << point.z() << std::endl;

    osg::Vec4 planepoint = farpoint + (nearpoint - farpoint) * point.x();

    std::cerr << "Plane point x: " << planepoint.x() << " y: " << planepoint.y() << " z: " << planepoint.z() << std::endl;
*/

    /*osg::Vec4 realnear = osg::Vec4(33.8048, -2857, 25.3503, 1.0);
    
    realnear = realnear * _viewer0View;

    std::cerr << "realnear post view x: " << realnear.x() << " y: " << realnear.y() << " z: " << realnear.z() << " w: " << realnear.w() << std::endl;

    realnear = realnear * _viewer0Proj;

    std::cerr << "realnear post proj x: " << realnear.x() << " y: " << realnear.y() << " z: " << realnear.z() << " w: " << realnear.w() << std::endl;*/
}

void ScreenMultiViewer::addTestGeometry()
{
    static bool geoadded = false;
    if(geoadded)
    {
	return;
    }
    geoadded = true;
    osg::Geometry * geo = new osg::Geometry();
    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->push_back(osg::Vec3(75,1500,-75));
    verts->push_back(osg::Vec3(0,1500,75));
    verts->push_back(osg::Vec3(-75,1500,-75));
    verts->push_back(osg::Vec3(500,1500,0));
    verts->push_back(osg::Vec3(650,1500,0));
    verts->push_back(osg::Vec3(500,1500,75));
    verts->push_back(osg::Vec3(-500,1500,75));
    verts->push_back(osg::Vec3(-575,1500,75));
    verts->push_back(osg::Vec3(-575,0,-75));

    geo->setVertexArray(verts);

    osg::DrawElementsUInt * ele =
            new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);

    ele->push_back(0);
    ele->push_back(1);
    ele->push_back(2);

    ele->push_back(3);
    ele->push_back(4);
    ele->push_back(5);

    ele->push_back(6);
    ele->push_back(7);
    ele->push_back(8);

    geo->addPrimitiveSet(ele);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    osg::TemplateIndexArray<unsigned int,osg::Array::UIntArrayType,4,4>
            *colorIndexArray;
    colorIndexArray = new osg::TemplateIndexArray<unsigned int,
            osg::Array::UIntArrayType,4,4>;
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);

    geo->setColorArray(colors);
    geo->setColorIndices(colorIndexArray);
    geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    /*osg::Vec3Array* verts = new osg::Vec3Array();
    verts->push_back(osg::Vec3(10000,10000,10000));
    verts->push_back(osg::Vec3(10000,10000,-10000));
    verts->push_back(osg::Vec3(-10000,10000,-10000));
    verts->push_back(osg::Vec3(-10000,10000,10000));
    verts->push_back(osg::Vec3(-10000,-10000,10000));
    verts->push_back(osg::Vec3(-10000,-10000,-10000));
    verts->push_back(osg::Vec3(10000,-10000,-10000));
    verts->push_back(osg::Vec3(10000,-10000,10000));

    geo->setVertexArray(verts);

    osg::DrawElementsUInt * ele =
            new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);

    ele->push_back(0);
    ele->push_back(1);
    ele->push_back(2);
    ele->push_back(3);

    ele->push_back(2);
    ele->push_back(3);
    ele->push_back(4);
    ele->push_back(5);

    ele->push_back(4);
    ele->push_back(5);
    ele->push_back(6);
    ele->push_back(7);

    ele->push_back(6);
    ele->push_back(7);
    ele->push_back(0);
    ele->push_back(1);

    ele->push_back(0);
    ele->push_back(3);
    ele->push_back(4);
    ele->push_back(7);

    ele->push_back(1);
    ele->push_back(2);
    ele->push_back(5);
    ele->push_back(6);

    geo->addPrimitiveSet(ele);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    osg::TemplateIndexArray<unsigned int,osg::Array::UIntArrayType,4,4>
            *colorIndexArray;
    colorIndexArray = new osg::TemplateIndexArray<unsigned int,
            osg::Array::UIntArrayType,4,4>;
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);

    geo->setColorArray(colors);
    geo->setColorIndices(colorIndexArray);
    geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);*/

    osg::Geode * geode = new osg::Geode();
    geode->addDrawable(geo);

    std::cerr << "Adding test geometry." << std::endl;
    SceneManager::instance()->getObjectsRoot()->addChild(geode);
}

void ScreenMultiViewer::PreDrawCallback::operator()(osg::RenderInfo & ri) const
{
    _screen->_viewer0Pos->set(_screen->_viewer0PosLocal[_index]);
    _screen->_viewer1Pos->set(_screen->_viewer1PosLocal[_index]);
    _screen->_viewer0Dist->set(_screen->_viewer0DistLocal[_index]);
    _screen->_viewer1Dist->set(_screen->_viewer1DistLocal[_index]);
    _screen->_minRatio->set(_screen->_minRatioLocal[_index]);
    _screen->_maxRatio->set(_screen->_maxRatioLocal[_index]);

    /*osg::Matrix v0v, v1v, v0p, v1p;
    for(int i = 0; i < 16; i++)
    {
	v0v.ptr()[i] = (1.0 - _screen->_minRatioLocal[_index]) * _screen->_viewer0View[_index].ptr()[i] + _screen->_minRatioLocal[_index] * _screen->_viewer1View[_index].ptr()[i];
	v1v.ptr()[i] = (1.0 - _screen->_maxRatioLocal[_index]) * _screen->_viewer0View[_index].ptr()[i] + _screen->_maxRatioLocal[_index] * _screen->_viewer1View[_index].ptr()[i];
	v0p.ptr()[i] = (1.0 - _screen->_minRatioLocal[_index]) * _screen->_viewer0Proj[_index].ptr()[i] + _screen->_minRatioLocal[_index] * _screen->_viewer1Proj[_index].ptr()[i];
	v1p.ptr()[i] = (1.0 - _screen->_maxRatioLocal[_index]) * _screen->_viewer0Proj[_index].ptr()[i] + _screen->_maxRatioLocal[_index] * _screen->_viewer1Proj[_index].ptr()[i];
    }*/

#ifdef FRAGMENT_QUERY
    if(!_init)
    {
	glGenQueries(1,&_query);
	_init = true;
    }

    if(first)
    {
	glBeginQuery(GL_SAMPLES_PASSED,_query);
    }

    if(_screen->_stereoMode == osg::DisplaySettings::HORIZONTAL_INTERLACE)
    {
	first = !first;
    }

#endif

    //std::cerr << "PreDraw." << std::endl;
    glMatrixMode(GL_TEXTURE);

    glActiveTexture(GL_TEXTURE4);
    glLoadMatrix((GLdouble *)_screen->_viewer0View[_index].ptr());

    glActiveTexture(GL_TEXTURE5);
    glLoadMatrix((GLdouble *)_screen->_viewer0Proj[_index].ptr());

    //if(TrackingManager::instance()->getNumHeads() >= 2)
    //{
	glActiveTexture(GL_TEXTURE6);
	glLoadMatrix((GLdouble *)_screen->_viewer1View[_index].ptr());

	glActiveTexture(GL_TEXTURE7);
	glLoadMatrix((GLdouble *)_screen->_viewer1Proj[_index].ptr());
    /*}
    else
    {
	glActiveTexture(GL_TEXTURE6);
	glLoadMatrix((GLdouble *)v0v.ptr());

	glActiveTexture(GL_TEXTURE7);
	glLoadMatrix((GLdouble *)v0p.ptr());
    }*/

    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_MODELVIEW);

    if(_indexState == TOGGLE)
    {
	if(_index)
	{
	    _index = 0;
	}
	else
	{
	    _index = 1;
	}
    }
}

void ScreenMultiViewer::PostDrawCallback::operator()(osg::RenderInfo & ri) const
{
#ifdef FRAGMENT_QUERY

    if(_screen->_stereoMode == osg::DisplaySettings::HORIZONTAL_INTERLACE)
    {
	first = !first;
    }

    if(first)
    {
	GLuint result;
	glEndQuery(GL_SAMPLES_PASSED);
	glGetQueryObjectuiv(_pdc->_query,GL_QUERY_RESULT,&result);
	std::cerr << "Fragments passed: " << result << std::endl; 
    }

#endif
}

void ScreenMultiViewer::calcScreenMinMaxRatio()
{
    float bottomLeft,bottomRight,topLeft,topRight;

    _viewer0Dir->get(_dir0);
    _viewer1Dir->get(_dir1);
    _screenCorner->get(_corner);
    _upPerPixel->get(_upPer);
    _rightPerPixel->get(_rightPer);

    for(int i = 0; i < 2; i++)
    {

	bottomLeft = getRatio(0.5,0.5,i);
	bottomRight = getRatio(((float)_myInfo->myChannel->width) - 0.5, 0.5, i);
	topLeft = getRatio(0.5,((float)_myInfo->myChannel->height) - 0.5, i);
	topRight = getRatio(((float)_myInfo->myChannel->width) - 0.5, ((float)_myInfo->myChannel->height) - 0.5, i);

	//std::cerr << "BL: " << bottomLeft << " BR: " << bottomRight << " TL: " << topLeft << " TR: " << topRight << std::endl;

	float currentRatio;
	float minx, maxx, miny, maxy;
	float currentx, currenty;

	// find max
	if(bottomLeft >= bottomRight && bottomLeft >= topLeft && bottomLeft >= topRight)
	{
	    currentRatio = bottomLeft;
	    minx = 0.5;
	    miny = 0.5; 
	    maxx = ((float)_myInfo->myChannel->width) - 0.5;
	    maxx = maxx / 2.0;
	    maxy = ((float)_myInfo->myChannel->height) - 0.5;
	    maxy = maxy / 2.0;

	    currentx = minx;
	    currenty = miny;
	}
	else if(bottomRight >= bottomLeft && bottomRight >= topLeft && bottomRight >= topRight)
	{
	    minx = (((float)_myInfo->myChannel->width) - 0.5) / 2.0;
	    miny = 0.5; 
	    maxx = ((float)_myInfo->myChannel->width) - 0.5;
	    maxy = (((float)_myInfo->myChannel->height) - 0.5) / 2.0;

	    currentx = minx;
	    currenty = miny;
	    currentRatio = getRatio(currentx, currenty, i);
	}
	else if(topLeft >= bottomLeft && topLeft >= bottomRight && topLeft >= topRight)
	{
	    minx = 0.5;
	    miny = (((float)_myInfo->myChannel->height) - 0.5) / 2.0; 
	    maxx = (((float)_myInfo->myChannel->width) - 0.5) / 2.0;
	    maxy = (((float)_myInfo->myChannel->height) - 0.5);

	    currentx = minx;
	    currenty = miny;
	    currentRatio = getRatio(currentx, currenty, i);
	}
	else // topRight
	{
	    minx = (((float)_myInfo->myChannel->width) - 0.5) / 2.0;
	    miny = (((float)_myInfo->myChannel->height) - 0.5) / 2.0; 
	    maxx = (((float)_myInfo->myChannel->width) - 0.5);
	    maxy = (((float)_myInfo->myChannel->height) - 0.5);

	    currentx = minx;
	    currenty = miny;
	    currentRatio = getRatio(currentx, currenty, i);
	}

	while(maxx - minx > 0.5)
	{
	    currentx = minx + ((maxx - minx) / 2.0);
	    float newRatio = getRatio(currentx, currenty,i);
	    if(newRatio > currentRatio)
	    {
		minx = currentx;
		currentRatio = newRatio;
	    }
	    else
	    {
		maxx = currentx;
	    }
	}

	while(maxy - miny > 0.5)
	{
	    currenty = miny + ((maxy - miny) / 2.0);
	    float newRatio = getRatio(currentx, currenty,i);
	    if(newRatio > currentRatio)
	    {
		miny = currenty;
		currentRatio = newRatio;
	    }
	    else
	    {
		maxy = currenty;
	    }
	}

	_maxRatioLocal[i] = currentRatio;

	// find min
	if(bottomLeft <= bottomRight && bottomLeft <= topLeft && bottomLeft <= topRight)
	{
	    currentRatio = bottomLeft;
	    minx = 0.5;
	    miny = 0.5; 
	    maxx = ((float)_myInfo->myChannel->width) - 0.5;
	    maxx = maxx / 2.0;
	    maxy = ((float)_myInfo->myChannel->height) - 0.5;
	    maxy = maxy / 2.0;

	    currentx = minx;
	    currenty = miny;
	}
	else if(bottomRight <= bottomLeft && bottomRight <= topLeft && bottomRight <= topRight)
	{
	    minx = (((float)_myInfo->myChannel->width) - 0.5) / 2.0;
	    miny = 0.5; 
	    maxx = ((float)_myInfo->myChannel->width) - 0.5;
	    maxy = (((float)_myInfo->myChannel->height) - 0.5) / 2.0;

	    currentx = minx;
	    currenty = miny;
	    currentRatio = getRatio(currentx, currenty, i);
	}
	else if(topLeft <= bottomLeft && topLeft <= bottomRight && topLeft <= topRight)
	{
	    minx = 0.5;
	    miny = (((float)_myInfo->myChannel->height) - 0.5) / 2.0; 
	    maxx = (((float)_myInfo->myChannel->width) - 0.5) / 2.0;
	    maxy = (((float)_myInfo->myChannel->height) - 0.5);

	    currentx = minx;
	    currenty = miny;
	    currentRatio = getRatio(currentx, currenty, i);
	}
	else // topRight
	{
	    minx = (((float)_myInfo->myChannel->width) - 0.5) / 2.0;
	    miny = (((float)_myInfo->myChannel->height) - 0.5) / 2.0; 
	    maxx = (((float)_myInfo->myChannel->width) - 0.5);
	    maxy = (((float)_myInfo->myChannel->height) - 0.5);

	    currentx = minx;
	    currenty = miny;
	    currentRatio = getRatio(currentx, currenty, i);
	}

	while(maxx - minx > 0.1)
	{
	    currentx = minx + ((maxx - minx) / 2.0);
	    float newRatio = getRatio(currentx, currenty,i);
	    if(newRatio < currentRatio)
	    {
		minx = currentx;
		currentRatio = newRatio;
	    }
	    else
	    {
		maxx = currentx;
	    }
	}

	while(maxy - miny > 0.1)
	{
	    currenty = miny + ((maxy - miny) / 2.0);
	    float newRatio = getRatio(currentx, currenty,i);
	    if(newRatio < currentRatio)
	    {
		miny = currenty;
		currentRatio = newRatio;
	    }
	    else
	    {
		maxy = currenty;
	    }
	}

	_minRatioLocal[i] = currentRatio;

#if 0
	//compare result to brute force
	
	float bruteforceMin = 1.0;
	float bruteforceMax = 0.0;
	float bfMinx;
	float bfMiny;
	float bfMaxx;
	float bfMaxy;
	for(float j = 0.5; j < _myInfo->myChannel->width; j = j + 1.0)
	{
	    for(float k = 0.5; k < _myInfo->myChannel->height; k = k + 1.0)
	    {
		float r = getRatio(j,k,i);
		if(r < bruteforceMin)
		{
		    bruteforceMin = r;
		    bfMinx = j;
		    bfMiny = k;
		}
		if(r > bruteforceMax)
		{
		    bruteforceMax = r;
		    bfMaxx = j;
		    bfMaxy = k;
		}
	    }
	}

	std::cerr << "MinRatio: " << _minRatioLocal[i] << " bf: " << bruteforceMin << " MaxRatio: " << _maxRatioLocal[i] << " bf: " << bruteforceMax << std::endl;

#endif


	//std::cerr << "currentx: " << currentx << " currenty: " << currenty << " max ratio: " << maxRatio << " min ratio: " << minRatio << std::endl;
	
	if(_stereoMode != osg::DisplaySettings::HORIZONTAL_INTERLACE)
	{
	    break;
	}
    }
}

float ScreenMultiViewer::getRatio(float x, float y, int eyeNum)
{
    osg::Vec3 pos = _corner + _rightPer * x + _upPer * y;
    osg::Vec3 dir = pos - _viewer0PosLocal[eyeNum];
    dir.normalize();

    osg::Vec2 weight;
    weight.x() = acos(dir * _dir0);

    dir = pos - _viewer1PosLocal[eyeNum];
    dir.normalize();

    weight.y() = acos(dir * _dir1);
    weight.x() = weight.x() * weight.x() * -0.1823784 + weight.x() * -0.095493 + 1.0;
    weight.y() = weight.y() * weight.y() * -0.1823784 + weight.y() * -0.095493 + 1.0;

    weight.x() = std::max(weight.x(),0.0f);
    weight.y() = std::max(weight.y(),0.0f);

    return weight.y() / (weight.x() + weight.y());
}
