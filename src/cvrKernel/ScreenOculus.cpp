#include <cvrKernel/ScreenOculus.h>
#include <cvrKernel/CVRStatsHandler.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/NodeMask.h>
#include <cvrInput/TrackingManager.h>
#include <cvrInput/TrackerBase.h>
#include <cvrInput/TrackerOculus.h>

#include <osgViewer/Renderer>
#include <osg/CullFace>

#include <iostream>

#include <OVR_CAPI_GL.h>

#ifdef WIN32
#pragma comment(lib, "Winmm.lib")
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

ScreenOculus::ScreenOculus() :
        ScreenBase()
{
	_init = false;
	_frameNumber = 0;
}

ScreenOculus::~ScreenOculus()
{
	if (_init && !TrackerOculus::isInit())
	{
		ovr_Destroy(_session);
		ovr_Shutdown();
	}
}

void ScreenOculus::init(int mode)
{
	if(!TrackerOculus::isInit())
	{
		ovrResult result = ovr_Initialize(nullptr);
		if (OVR_FAILURE(result))
		{
			std::cerr << "Error: ovr_Initialized" << std::endl;
			return;
		}

		
		ovrGraphicsLuid luid;
		result = ovr_Create(&_session, &luid);
		if (OVR_FAILURE(result))
		{
			std::cerr << "Error: ovr_Create" << std::endl;
			ovr_Shutdown();
			return;
		}

		_hmd = ovr_GetHmdDesc(_session);
	}
	else
	{
		_session = TrackerOculus::getSession();
		_hmd = TrackerOculus::getHMD();
	}

	CVRViewer::instance()->getStatsHandler()->setCameraClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ovrSizei recommenedleftSize = ovr_GetFovTextureSize(_session, ovrEye_Left,_hmd.DefaultEyeFov[0], 1.0f);

	ovrRecti vp;
	vp.Pos.x = 0;
	vp.Pos.y = 0;
	vp.Size.w = recommenedleftSize.w;
	vp.Size.h = recommenedleftSize.h;

	// dummy texture
	_leftTexture = new osg::Texture2D();
	_leftTexture->setTextureSize(1,1);
	_leftTexture->setInternalFormat(GL_RGBA);
	_leftTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
	_leftTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
	_leftTexture->setResizeNonPowerOfTwoHint(false);
	_leftTexture->setUseHardwareMipMapGeneration(false);

	_leftDepthTexture = new osg::Texture2D();
	_leftDepthTexture->setTextureSize(recommenedleftSize.w,recommenedleftSize.h);
	_leftDepthTexture->setInternalFormat(GL_DEPTH_COMPONENT);
	_leftDepthTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
	_leftDepthTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
	_leftDepthTexture->setResizeNonPowerOfTwoHint(false);
	_leftDepthTexture->setUseHardwareMipMapGeneration(false);

	ovrSizei recommenedrightSize = ovr_GetFovTextureSize(_session, ovrEye_Right,_hmd.DefaultEyeFov[1], 1.0f);

	// dummy texture
	_rightTexture = new osg::Texture2D();
	_rightTexture->setTextureSize(1,1);
	_rightTexture->setInternalFormat(GL_RGBA);
	_rightTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
	_rightTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
	_rightTexture->setResizeNonPowerOfTwoHint(false);
	_rightTexture->setUseHardwareMipMapGeneration(false);

	_rightDepthTexture = new osg::Texture2D();
	_rightDepthTexture->setTextureSize(recommenedrightSize.w,recommenedrightSize.h);
	_rightDepthTexture->setInternalFormat(GL_DEPTH_COMPONENT);
	_rightDepthTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
	_rightDepthTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
	_rightDepthTexture->setResizeNonPowerOfTwoHint(false);
	_rightDepthTexture->setUseHardwareMipMapGeneration(false);

	_cameraLeft = new osg::Camera();
	_cameraRight = new osg::Camera();

	osg::CullFace * cf = new osg::CullFace(osg::CullFace::BACK);
	osg::StateSet * stateset = _cameraLeft->getOrCreateStateSet();
	stateset->setAttributeAndModes(cf, osg::StateAttribute::ON);

	osg::DisplaySettings * ds = new osg::DisplaySettings();
	_cameraLeft->setDisplaySettings(ds);

	CVRViewer::instance()->addSlave(_cameraLeft.get(),osg::Matrixd(),
        osg::Matrixd());
	defaultCameraInit(_cameraLeft.get());
	_cameraLeft->setViewport(new osg::Viewport(0,0,recommenedleftSize.w,recommenedleftSize.h));
	_cameraLeft->setCullMask(CULL_MASK_LEFT);

	osgViewer::Renderer * renderer =
        dynamic_cast<osgViewer::Renderer*>(_cameraLeft->getRenderer());
	if(!renderer)
	{
		std::cerr << "Error getting renderer pointer." << std::endl;
	}
	else
	{
		osg::DisplaySettings * ds =
               renderer->getSceneView(0)->getDisplaySettings();
		ds->setStereo(false);
	}
	_cameraLeft->setAllowEventFocus(false);
	_cameraLeft->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_cameraLeft->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	_cameraLeft->attach(osg::Camera::COLOR_BUFFER0,_leftTexture);
	_cameraLeft->attach(osg::Camera::DEPTH_BUFFER,_leftDepthTexture);
	_cameraLeft->setReferenceFrame(osg::Transform::RELATIVE_RF);


	ovrEyeRenderDesc eyeRenderDesc[2];

	eyeRenderDesc[0] = ovr_GetRenderDesc(_session, ovrEye_Left, _hmd.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(_session, ovrEye_Right, _hmd.DefaultEyeFov[1]);
	_hmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeViewOffset;
	_hmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeViewOffset;

	// Initialize our single full screen Fov layer.
	_layer.Header.Type = ovrLayerType_EyeFov;
	_layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
	_layer.ColorTexture[0] = nullptr;
	_layer.ColorTexture[1] = nullptr;
	_layer.Fov[0] = eyeRenderDesc[0].Fov;
	_layer.Fov[1] = eyeRenderDesc[1].Fov;
	_layer.Viewport[0].Pos.x = 0;
	_layer.Viewport[0].Pos.y = 0;
	_layer.Viewport[0].Size = recommenedleftSize;
	_layer.Viewport[1].Pos.x = 0;
	_layer.Viewport[1].Pos.y = 0;
	_layer.Viewport[1].Size = recommenedrightSize;

	OculusPreDrawCallback * pdc = new OculusPreDrawCallback;
	pdc->width = recommenedleftSize.w;
	pdc->height = recommenedleftSize.h;
	pdc->session = &_session;
	pdc->textureSet = NULL;
	pdc->layer = &_layer;
	pdc->eye = ovrEye_Left;

	_cameraLeft->setPreDrawCallback(pdc);

	stateset = _cameraRight->getOrCreateStateSet();
	stateset->setAttributeAndModes(cf, osg::StateAttribute::ON);

	ds = new osg::DisplaySettings();
	_cameraRight->setDisplaySettings(ds);

	CVRViewer::instance()->addSlave(_cameraRight.get(), osg::Matrixd(),
		osg::Matrixd());
	defaultCameraInit(_cameraRight.get());
	_cameraRight->setViewport(new osg::Viewport(0, 0, recommenedrightSize.w, recommenedrightSize.h));
	_cameraRight->setCullMask(CULL_MASK_RIGHT);

	renderer =
		dynamic_cast<osgViewer::Renderer*>(_cameraRight->getRenderer());
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
	_cameraRight->setAllowEventFocus(false);
	_cameraRight->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_cameraRight->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	_cameraRight->attach(osg::Camera::COLOR_BUFFER0, _rightTexture);
	_cameraRight->attach(osg::Camera::DEPTH_BUFFER, _rightDepthTexture);
	_cameraRight->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

	pdc = new OculusPreDrawCallback;
	pdc->width = recommenedleftSize.w;
	pdc->height = recommenedleftSize.h;
	pdc->session = &_session;
	pdc->textureSet = NULL;
	pdc->layer = &_layer;
	pdc->eye = ovrEye_Right;

	_cameraRight->setPreDrawCallback(pdc);

	OculusSwapCallback * osc = new OculusSwapCallback;
	osc->session = &_session;
	osc->layer = &_layer;
	osc->disableVsync = true;
	_swapCallback = osc;

	_myInfo->myChannel->myWindow->gc->setSwapCallback(osc);

	_init = true;
}

void ScreenOculus::computeViewProj()
{
	if(!_init)
	{
		return;
	}

	OculusFramePoseInfo * pinfo = new OculusFramePoseInfo;

	_frameNumber++;
	pinfo->frameNumber = _frameNumber;

	// get current oculus camera adjustment
	double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, pinfo->frameNumber);
	ovrTrackingState ts = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);

	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
		_cameraPos.x() = ts.LeveledCameraPose.Position.x;
		_cameraPos.y() = ts.LeveledCameraPose.Position.y;
		_cameraPos.z() = ts.LeveledCameraPose.Position.z;
	}

	ovr_CalcEyePoses(ts.HeadPose.ThePose, _hmdToEyeViewOffset, pinfo->RenderPose);

	// create tracked body for estimated eye
	TrackerBase::TrackedBody tb;
	tb.x = (pinfo->RenderPose[ovrEye_Left].Position.x - _cameraPos.x()) * 1000.0;
	tb.y = (pinfo->RenderPose[ovrEye_Left].Position.y - _cameraPos.y()) * 1000.0;
	tb.z = (pinfo->RenderPose[ovrEye_Left].Position.z - _cameraPos.z()) * 1000.0;
	tb.qx = pinfo->RenderPose[ovrEye_Left].Orientation.x;
	tb.qy = pinfo->RenderPose[ovrEye_Left].Orientation.y;
	tb.qz = pinfo->RenderPose[ovrEye_Left].Orientation.z;
	tb.qw = pinfo->RenderPose[ovrEye_Left].Orientation.w;

	// transform using tracking adjustment
	osg::Matrix headXForm = TrackingManager::instance()->getHeadTransformFromTrackedBody(_myInfo->myChannel->head,&tb);

    osg::Vec3d eyeLeft = headXForm.getTrans();

	osg::Quat invViewerRot = headXForm.getRotate();
    invViewerRot = invViewerRot.inverse();

    //make frustum
    float top, bottom, left, right;
	
    top = _near * _layer.Fov[ovrEye_Left].UpTan;
    bottom = -_near * _layer.Fov[ovrEye_Left].DownTan;
    right = _near * _layer.Fov[ovrEye_Left].RightTan;
    left = -_near * _layer.Fov[ovrEye_Left].LeftTan;

    _projLeft.makeFrustum(left,right,bottom,top,_near,_far);

    // move camera to origin
    osg::Matrix cameraTrans;
    cameraTrans.makeTranslate(-eyeLeft);

    //make view
    _viewLeft = cameraTrans * osg::Matrix::rotate(invViewerRot)
            * osg::Matrix::lookAt(osg::Vec3(0,0,0),osg::Vec3(0,1,0),
                    osg::Vec3(0,0,1));

	tb.x = (pinfo->RenderPose[ovrEye_Right].Position.x - _cameraPos.x()) * 1000.0;
	tb.y = (pinfo->RenderPose[ovrEye_Right].Position.y - _cameraPos.y()) * 1000.0;
	tb.z = (pinfo->RenderPose[ovrEye_Right].Position.z - _cameraPos.z()) * 1000.0;
	tb.qx = pinfo->RenderPose[ovrEye_Right].Orientation.x;
	tb.qy = pinfo->RenderPose[ovrEye_Right].Orientation.y;
	tb.qz = pinfo->RenderPose[ovrEye_Right].Orientation.z;
	tb.qw = pinfo->RenderPose[ovrEye_Right].Orientation.w;

	// transform using tracking adjustment
	headXForm = TrackingManager::instance()->getHeadTransformFromTrackedBody(_myInfo->myChannel->head,&tb);

	osg::Vec3d eyeRight = headXForm.getTrans();

	invViewerRot = headXForm.getRotate();
    invViewerRot = invViewerRot.inverse();

    //make frustum
    top = _near * _layer.Fov[ovrEye_Right].UpTan;
    bottom = -_near * _layer.Fov[ovrEye_Right].DownTan;
    right = _near * _layer.Fov[ovrEye_Right].RightTan;
    left = -_near * _layer.Fov[ovrEye_Right].LeftTan;

    _projRight.makeFrustum(left,right,bottom,top,_near,_far);

    // move camera to origin
    cameraTrans.makeTranslate(-eyeRight);

    //make view
    _viewRight = cameraTrans * osg::Matrix::rotate(invViewerRot)
            * osg::Matrix::lookAt(osg::Vec3(0,0,0),osg::Vec3(0,1,0),
                    osg::Vec3(0,0,1));

	_swapCallback->addFramePose(pinfo);
}

void ScreenOculus::updateCamera()
{
    _cameraLeft->setViewMatrix(_viewLeft);
	_cameraLeft->setProjectionMatrix(_projLeft);

	_cameraRight->setViewMatrix(_viewRight);
	_cameraRight->setProjectionMatrix(_projRight);
}

void ScreenOculus::setClearColor(osg::Vec4 color)
{
    _cameraLeft->setClearColor(color);
	_cameraRight->setClearColor(color);
}

ScreenInfo * ScreenOculus::findScreenInfo(osg::Camera * c)
{
    if(c == _cameraLeft.get() || c == _cameraRight.get())
    {
        return _myInfo;
    }
    return NULL;
}

void ScreenOculus::adjustViewportCoords(int & x, int & y)
{
    if(x > (_myInfo->myChannel->width / 2.0))
    {
        x = (int)(((float)x) - (_myInfo->myChannel->width / 2.0));
    }
    x *= 2;

    return;
}

void OculusPreDrawCallback::operator()(osg::RenderInfo& renderInfo) const
{
	if (!textureSet)
	{
		std::cerr << "Init texture set" << std::endl;
		osg::State * state = renderInfo.getState();

		osgViewer::Renderer * renderer = dynamic_cast<osgViewer::Renderer*>(renderInfo.getCurrentCamera()->getRenderer());
		if (renderer != NULL) 
		{
			osgUtil::SceneView* sceneView = renderer->getSceneView(0);
			if (sceneView != NULL) 
			{
				osgUtil::RenderStage* renderStage = sceneView->getRenderStage();
				if (renderStage != NULL) 
				{
					osg::FrameBufferObject* fbobj = renderStage->getFrameBufferObject();
					fbo = fbobj->getHandle(state->getContextID());
					initTextureSet();
				}
			}
		}
	}

	if (textureSet)
	{
		textureSet->CurrentIndex = (textureSet->CurrentIndex + 1) % textureSet->TextureCount;
		const osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(renderInfo.getState()->getContextID(), true);
		ovrGLTexture* tex = reinterpret_cast<ovrGLTexture*>(&textureSet->Textures[textureSet->CurrentIndex]);
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex->OGL.TexId, 0);
	}
}

void OculusPreDrawCallback::initTextureSet() const
{
	if (ovr_CreateSwapTextureSetGL(*session, GL_SRGB8_ALPHA8, width, height, &textureSet) == ovrSuccess)
	{
		std::cerr << "Created texture set" << std::endl;
		layer->ColorTexture[eye] = textureSet;
	}
	else
	{
		std::cerr << "Error creating texture set" << std::endl;
		textureSet = NULL;
		return;
	}
}

void OculusSwapCallback::swapBuffersImplementation(osg::GraphicsContext *gc)
{
	osgViewer::GraphicsWindow * win;
	if (disableVsync && (win = dynamic_cast<osgViewer::GraphicsWindow*>(gc)))
	{
		win->setSyncToVBlank(false);
		disableVsync = false;
	}

	if (layer->ColorTexture[0] != nullptr && layer->ColorTexture[1] != nullptr)
	{
		OculusFramePoseInfo * pinfo = NULL;
		mutex.lock();

		if (poseList.size())
		{
			pinfo = poseList.front();
			poseList.pop_front();
		}

		mutex.unlock();

		if (pinfo)
		{
			layer->RenderPose[0] = pinfo->RenderPose[0];
			layer->RenderPose[1] = pinfo->RenderPose[1];
			ovrLayerHeader* layers = &layer->Header;
			ovrResult result = ovr_SubmitFrame(*session, pinfo->frameNumber, nullptr, &layers, 1);
			if (OVR_FAILURE(result))
			{
				std::cerr << "Error ovr_SubmitFrame" << std::endl;
			}

			delete pinfo;
		}
		else
		{
			std::cerr << "No current rendering pose" << std::endl;
		}
	}

	gc->swapBuffersImplementation();
}

void OculusSwapCallback::addFramePose(OculusFramePoseInfo * pinfo)
{
	mutex.lock();

	poseList.push_back(pinfo);

	mutex.unlock();
}
