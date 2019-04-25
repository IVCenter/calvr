#include <cvrKernel/ScreenOpenVR.h>
#include <cvrKernel/CVRStatsHandler.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/NodeMask.h>
#include <cvrKernel/SceneManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrInput/TrackerBase.h>
#include <cvrConfig/ConfigManager.h>

#include <osgViewer/Renderer>
#include <osg/CullFace>
#include <osg/Texture>
#include <osg/Version>

#include <openvr.h>
#include <openvrdevice.h>

#include <iostream>

#ifdef WIN32
#pragma comment(lib, "Winmm.lib")
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

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
	vrDevice = new OpenVRDevice(0.01f, 10000000.0f, 1.0f, 4);

	if (!vrDevice->hmdInitialized()) {
		return;
	}

	_swapCallback = new OpenVRSwapCallback(vrDevice);

	_myInfo->myChannel->myWindow->gc->setSwapCallback(_swapCallback);

	_leftCamera = vrDevice->createRTTCamera(OpenVRDevice::LEFT, osg::Camera::ABSOLUTE_RF, osg::Vec4(0, 0, 0, 0), _myInfo->myChannel->myWindow->gc);
	CVRViewer::instance()->addSlave(_leftCamera.get(), osg::Matrixd(), osg::Matrixd());
	defaultCameraInit(_leftCamera.get());

	_rightCamera = vrDevice->createRTTCamera(OpenVRDevice::RIGHT, osg::Camera::ABSOLUTE_RF, osg::Vec4(0, 0, 0, 0), _myInfo->myChannel->myWindow->gc);
	CVRViewer::instance()->addSlave(_rightCamera.get(), osg::Matrixd(), osg::Matrixd());
	defaultCameraInit(_rightCamera.get());

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

	osg::Vec3 position = vrDevice->position();
	osg::Quat orientation = vrDevice->orientation();

	osg::Matrix leftView = vrDevice->viewMatrixLeft();
	leftView.preMultRotate(orientation);
	leftView.setTrans(leftView.getTrans() + position);

	osg::Matrix rightView = vrDevice->viewMatrixRight();
	rightView.preMultRotate(orientation);
	rightView.setTrans(rightView.getTrans() + position);


	_leftCamera->setViewMatrix(leftView);
	_rightCamera->setViewMatrix(rightView);


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

    // if(c == _cameraLeft.get() || c == _cameraRight.get())
    // {
    //     return _myInfo;
    // }
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

