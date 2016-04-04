#include <cvrKernel/ScreenOculus.h>
#include <cvrKernel/CVRStatsHandler.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/NodeMask.h>
#include <cvrKernel/SceneManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrInput/TrackerBase.h>
#include <cvrInput/TrackerOculus.h>
#include <cvrConfig/ConfigManager.h>

#include <osgViewer/Renderer>
#include <osg/CullFace>
#include <osg/Texture>
#include <osg/Version>

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
	if (!TrackerOculus::isInit())
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

	bool useMirrorTexture = ConfigManager::getBool("mirror", "Oculus", true, NULL);
	bool setFBSRGB = ConfigManager::getBool("framebufferSRGB", "Oculus", false, NULL);

	if (!useMirrorTexture)
	{
		CVRViewer::instance()->getStatsHandler()->setCameraClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

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
	if (setFBSRGB)
	{
		stateset->setMode(GL_FRAMEBUFFER_SRGB, osg::StateAttribute::ON);
	}

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
	_cameraLeft->setRenderOrder(osg::Camera::PRE_RENDER);


	ovrEyeRenderDesc eyeRenderDesc[2];

	eyeRenderDesc[0] = ovr_GetRenderDesc(_session, ovrEye_Left, _hmd.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(_session, ovrEye_Right, _hmd.DefaultEyeFov[1]);
#if OVR_PRODUCT_VERSION < 1
	_hmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeViewOffset;
	_hmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeViewOffset;
#else
	_hmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeOffset;
	_hmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeOffset;
#endif

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
	pdc->twoSV = false;
	pdc->fboIndex = 0;
	pdc->width = recommenedleftSize.w;
	pdc->height = recommenedleftSize.h;
	pdc->session = &_session;
#if OVR_PRODUCT_VERSION < 1
	pdc->textureSet = NULL;
	pdc->mirrorTex = NULL;
#else
	pdc->textureChain = NULL;
	pdc->mirrorTex = NULL;
#endif
	pdc->layer = &_layer;
	pdc->eye = ovrEye_Left;

	if (useMirrorTexture)
	{
		pdc->previewTexture = _leftTexture;
#if OVR_PRODUCT_VERSION < 1
		pdc->mFormat = GL_RGBA;
		pdc->mWidth = _myInfo->myChannel->width;
		pdc->mHeight = _myInfo->myChannel->height;
#else
		pdc->mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM;
		pdc->mirrorDesc.Width = _myInfo->myChannel->width;
		pdc->mirrorDesc.Height = _myInfo->myChannel->height;
		pdc->mirrorDesc.MiscFlags = ovrTextureMisc_None;
#endif
	}

	_cameraLeft->setPreDrawCallback(pdc);

	stateset = _cameraRight->getOrCreateStateSet();
	stateset->setAttributeAndModes(cf, osg::StateAttribute::ON);
	if (setFBSRGB)
	{
		stateset->setMode(GL_FRAMEBUFFER_SRGB, osg::StateAttribute::ON);
	}

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
	_cameraRight->setRenderOrder(osg::Camera::PRE_RENDER);

	pdc = new OculusPreDrawCallback;
	pdc->twoSV = false;
	pdc->fboIndex = 0;
	pdc->width = recommenedleftSize.w;
	pdc->height = recommenedleftSize.h;
	pdc->session = &_session;
#if OVR_PRODUCT_VERSION < 1
	pdc->textureSet = NULL;
#else
	pdc->textureChain = NULL;
#endif
	pdc->layer = &_layer;
	pdc->eye = ovrEye_Right;

	_cameraRight->setPreDrawCallback(pdc);

	OculusSwapCallback * osc = new OculusSwapCallback;
	osc->session = &_session;
	osc->layer = &_layer;
	osc->disableVsync = true;
	_swapCallback = osc;

	_myInfo->myChannel->myWindow->gc->setSwapCallback(osc);

	_previewCamera = new osg::Camera();

	ds = new osg::DisplaySettings();
	_previewCamera->setDisplaySettings(ds);
	_previewCamera->setRenderOrder(osg::Camera::POST_RENDER);
	_previewCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	_previewCamera->setCullingActive(false);
	
	defaultCameraInit(_previewCamera.get());

	osg::ref_ptr<osg::Vec2Array> quadArray = new osg::Vec2Array();
	quadArray->push_back(osg::Vec2(-1.0,1.0));
	quadArray->push_back(osg::Vec2(-1.0, -1.0));
	quadArray->push_back(osg::Vec2(1.0, 1.0));
	quadArray->push_back(osg::Vec2(1.0, -1.0));

	osg::ref_ptr<osg::Vec2Array> texArray = new osg::Vec2Array();
	texArray->push_back(osg::Vec2(0,0));
	texArray->push_back(osg::Vec2(0, 1));
	texArray->push_back(osg::Vec2(1, 0));
	texArray->push_back(osg::Vec2(1, 1));

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
	geom->setVertexArray(quadArray);
	geom->setTexCoordArray(0,texArray);
	geom->setUseDisplayList(false);
	geom->setUseVertexBufferObjects(true);
	geom->setCullingActive(false);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP,0,4));

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->setCullingActive(false);
	geode->addDrawable(geom);
	_previewCamera->addChild(geode);

	stateset = _previewCamera->getOrCreateStateSet();
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	
	stateset = geode->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0, _leftTexture, osg::StateAttribute::ON);

	_init = true;
}

void ScreenOculus::computeViewProj()
{
	if(!_init)
	{
		return;
	}

	if (_previewCamera->getNumParents() == 0)
	{
		//SceneManager::instance()->getScene()->addChild(_previewCamera);
		_cameraLeft->addChild(_previewCamera);
	}

	OculusFramePoseInfo * pinfo = new OculusFramePoseInfo;

	_frameNumber++;
	pinfo->frameNumber = _frameNumber;

	// get current oculus camera adjustment
	double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, pinfo->frameNumber);
	ovrTrackingState ts = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);

	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
#if OVR_PRODUCT_VERSION < 1
		_cameraPos.x() = ts.LeveledCameraPose.Position.x;
		_cameraPos.y() = ts.LeveledCameraPose.Position.y;
		_cameraPos.z() = ts.LeveledCameraPose.Position.z;
#else
		if (ovr_GetTrackerCount(_session) > 0)
		{
			ovrTrackerPose tp = ovr_GetTrackerPose(_session, 0);
			_cameraPos.x() = tp.LeveledPose.Position.x;
			_cameraPos.y() = tp.LeveledPose.Position.y;
			_cameraPos.z() = tp.LeveledPose.Position.z;
		}
#endif
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
	if (!_init)
	{
		return;
	}

    _cameraLeft->setViewMatrix(_viewLeft);
	_cameraLeft->setProjectionMatrix(_projLeft);

	_cameraRight->setViewMatrix(_viewRight);
	_cameraRight->setProjectionMatrix(_projRight);
}

void ScreenOculus::setClearColor(osg::Vec4 color)
{
	if (!_init)
	{
		return;
	}

    _cameraLeft->setClearColor(color);
	_cameraRight->setClearColor(color);
}

ScreenInfo * ScreenOculus::findScreenInfo(osg::Camera * c)
{
	if (!_init)
	{
		return NULL;
	}

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
	osg::State * state = renderInfo.getState();
#if OVR_PRODUCT_VERSION < 1
	if (!textureSet)
#else
	if (!textureChain)
#endif
	{
		//std::cerr << "Init texture set" << std::endl;	
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
					fbo[0] = fbobj->getHandle(state->getContextID());
					initTextureSet();
				}
			}
		}
	}

	if (!twoSV && dynamic_cast<osgViewer::Renderer*>(renderInfo.getCurrentCamera()->getRenderer())->getSceneView(1)->getRenderStage()->getFrameBufferObject())
	{
		//std::cerr << "valid second sv" << std::endl;
		twoSV = true;
		fbo[1] = dynamic_cast<osgViewer::Renderer*>(renderInfo.getCurrentCamera()->getRenderer())->getSceneView(1)->getRenderStage()->getFrameBufferObject()->getHandle(state->getContextID());
	}

	if (twoSV)
	{
		fboIndex = (fboIndex + 1) % 2;
	}

#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	const osg::GLExtensions* fbo_ext = state->get<osg::GLExtensions>();
#else
	const osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(renderInfo.getState()->getContextID(), true);
#endif

#if OVR_PRODUCT_VERSION < 1
	if (CVRViewer::instance()->done())
	{
		if (textureSet)
		{
			ovr_DestroySwapTextureSet(*session, textureSet);
			textureSet = NULL;
		}
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo[fboIndex]);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);

		if (previewTexture && mirrorTex)
		{
			previewTexture->getTextureObject(state->getContextID())->setAllocated(false);
			previewTexture->getTextureObject(state->getContextID())->_id = 0;
			ovr_DestroyMirrorTexture(*session, mirrorTex);
		}
	}

	if (textureSet)
	{
		textureSet->CurrentIndex = (textureSet->CurrentIndex + 1) % textureSet->TextureCount;
		
		ovrGLTexture* tex = reinterpret_cast<ovrGLTexture*>(&textureSet->Textures[textureSet->CurrentIndex]);
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo[fboIndex]);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex->OGL.TexId, 0);
	}

	if (previewTexture && !mirrorTex)
	{
		if (OVR_FAILURE(ovr_CreateMirrorTextureGL(*session,mFormat,mWidth,mHeight,&mirrorTex)))
		{
			std::cerr << "Error creating mirror texture" << std::endl;
		}
		else
		{
			ovrGLTexture * tex = reinterpret_cast<ovrGLTexture*>(mirrorTex);

			previewTexture->getTextureObject(state->getContextID())->setAllocated(true);
			previewTexture->getTextureObject(state->getContextID())->_id = tex->OGL.TexId;
		}
	}
#else
	if (CVRViewer::instance()->done())
	{
		//std::cerr << "Oculus cleanup" << std::endl;
		if (textureChain)
		{
			ovr_DestroyTextureSwapChain(*session, textureChain);
			textureChain = NULL;
		}
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo[fboIndex]);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);

		if (previewTexture && mirrorTex)
		{
			previewTexture->getTextureObject(state->getContextID())->setAllocated(false);
			previewTexture->getTextureObject(state->getContextID())->_id = 0;
			ovr_DestroyMirrorTexture(*session, mirrorTex);
		}

		return;
	}

	if (textureChain)
	{
		int index;
		unsigned int texId;
		//textureSet->CurrentIndex = (textureSet->CurrentIndex + 1) % textureSet->TextureCount;
		if (!OVR_SUCCESS(ovr_GetTextureSwapChainCurrentIndex(*session, textureChain, &index)))
		{
			std::cerr << "Error getting chain buffer" << std::endl;
			return;
		}
		if (!OVR_SUCCESS(ovr_GetTextureSwapChainBufferGL(*session, textureChain, index, &texId)))
		{
			std::cerr << "Error getting chain index" << std::endl;
			return;
		}
		fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo[fboIndex]);
		fbo_ext->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texId, 0);
	}

	if (previewTexture && !mirrorTex)
	{
		if (OVR_FAILURE(ovr_CreateMirrorTextureGL(*session,&mirrorDesc,&mirrorTex)))
		{
			std::cerr << "Error creating mirror texture" << std::endl;
		}
		else
		{
			unsigned int texId;
			if (OVR_SUCCESS(ovr_GetMirrorTextureBufferGL(*session, mirrorTex, &texId)))
			{
				previewTexture->getTextureObject(state->getContextID())->setAllocated(true);
				previewTexture->getTextureObject(state->getContextID())->_id = texId;
			}
			else
			{
				std::cerr << "Error get mirror texture id" << std::endl;
			}
		}
	}
#endif
}

void OculusPreDrawCallback::initTextureSet() const
{
#if OVR_PRODUCT_VERSION < 1
	if (ovr_CreateSwapTextureSetGL(*session, GL_SRGB8_ALPHA8, width, height, &textureSet) == ovrSuccess)
	{
		//std::cerr << "Created texture set" << std::endl;
		layer->ColorTexture[eye] = textureSet;
	}
	else
	{
		std::cerr << "Error creating texture set" << std::endl;
		textureSet = NULL;
		return;
	}
#else
	textureChainDesc.Type = ovrTexture_2D;
	textureChainDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureChainDesc.Width = width;
	textureChainDesc.Height = height;
	textureChainDesc.MipLevels = 1;
	textureChainDesc.ArraySize = 1;
	textureChainDesc.SampleCount = 1;
	textureChainDesc.StaticImage = false;
	textureChainDesc.BindFlags = ovrTextureBind_None;
	textureChainDesc.MiscFlags = ovrTextureMisc_None;

	if (OVR_SUCCESS(ovr_CreateTextureSwapChainGL(*session,&textureChainDesc,&textureChain)))
	{
		layer->ColorTexture[eye] = textureChain;
	}
	else
	{
		std::cerr << "Error creating texture chain" << std::endl;
		ovrErrorInfo ei;
		ovr_GetLastErrorInfo(&ei);
		std::cerr << ei.ErrorString << std::endl;
		textureChain = NULL;
		return;
	}
#endif
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
#if OVR_PRODUCT_VERSION >= 1
		// TODO maybe move to postdraw callback
		if (OVR_FAILURE(ovr_CommitTextureSwapChain(*session, layer->ColorTexture[0])))
		{
			ovrErrorInfo ei;
			ovr_GetLastErrorInfo(&ei);
			std::cerr << "commit fail: " << ei.ErrorString << std::endl;
		}
		if (OVR_FAILURE(ovr_CommitTextureSwapChain(*session, layer->ColorTexture[1])))
		{
			ovrErrorInfo ei;
			ovr_GetLastErrorInfo(&ei);
			std::cerr << "commit fail: " << ei.ErrorString << std::endl;
		}
#endif

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
