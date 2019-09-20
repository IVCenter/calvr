#include <cvrKernel/ScreenBase.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/CVRCullVisitor.h>
#include <cvrInput/TrackingManager.h>
#include <cvrKernel/NodeMask.h>
#include <cvrConfig/ConfigManager.h>

#include <osg/Texture2D>
#include <osgViewer/Renderer>

#include <iostream>

using namespace cvr;

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

std::map<osg::Camera*, osg::FrameBufferObject*> ScreenBase::framebuffers = std::map<osg::Camera*, osg::FrameBufferObject*>();

ScreenBase::ScreenBase()
{
}

void ScreenBase::defaultCameraInit(osg::Camera * cam)
{
    cam->setGraphicsContext(_myInfo->myChannel->myWindow->gc);
    cam->setViewport(
            new osg::Viewport(_myInfo->myChannel->left,
                    _myInfo->myChannel->bottom,_myInfo->myChannel->width,
                    _myInfo->myChannel->height));
    GLenum buffer =
            _myInfo->myChannel->myWindow->gc->getTraits()->doubleBuffer ?
		GL_BACK : GL_FRONT;

	cam->setDrawBuffer(buffer);
	cam->setReadBuffer(buffer);


	if (ConfigManager::getBool("UseFrameBuffer", false))
	{
		frameBufferInit(cam, _myInfo->myChannel->width, _myInfo->myChannel->height);
	}

    cam->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

    cam->setCullMask(CULL_MASK);
    cam->setCullMaskLeft(CULL_MASK_LEFT);
    cam->setCullMaskRight(CULL_MASK_RIGHT);

    std::string cmode = ConfigManager::getEntry("value","CullingMode","CALVR");

    osgViewer::Renderer * renderer =
            dynamic_cast<osgViewer::Renderer*>(cam->getRenderer());
    if(!renderer)
    {
        std::cerr << "Error getting renderer pointer." << std::endl;
    }
    else
    {

        renderer->getSceneView(0)->getDisplaySettings()->setSerializeDrawDispatch(
                false);
        renderer->getSceneView(1)->getDisplaySettings()->setSerializeDrawDispatch(
                false);

        if(cmode == "CALVR")
        {
            renderer->getSceneView(0)->setCullVisitor(new CVRCullVisitor());
            renderer->getSceneView(0)->setCullVisitorLeft(new CVRCullVisitor());
            renderer->getSceneView(0)->setCullVisitorRight(
                    new CVRCullVisitor());
            renderer->getSceneView(1)->setCullVisitor(new CVRCullVisitor());
            renderer->getSceneView(1)->setCullVisitorLeft(new CVRCullVisitor());
            renderer->getSceneView(1)->setCullVisitorRight(
                    new CVRCullVisitor());
        }
    }
}

void ScreenBase::frameBufferInit(osg::Camera * cam, int width, int height)
{
	_fbo = new osg::FrameBufferObject();

	osg::Texture2D* db = new osg::Texture2D();
	db->setName("DepthTexture");
	db->setTextureSize(width, height);
	db->setResizeNonPowerOfTwoHint(false);

	db->setSourceFormat(GL_DEPTH_COMPONENT);
	db->setInternalFormat(GL_DEPTH_COMPONENT32);
	db->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
	db->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
	db->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
	db->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	_fbo->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment(db));

	//osg::RenderBuffer* db = new osg::RenderBuffer(width, height, GL_DEPTH_COMPONENT32);
	//db->set
	//_fbo->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment(db));

	osg::Texture2D* cb = new osg::Texture2D();
	cb->setName("ColorTexture");
	cb->setTextureSize(width, height);
	cb->setResizeNonPowerOfTwoHint(false);

	//cb->setSourceFormat(GL_RGBA);
	cb->setInternalFormat(GL_RGBA);
	cb->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
	cb->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
	cb->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
	cb->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	_fbo->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(cb));

	cam->attach(osg::Camera::COLOR_BUFFER0, cb, 0, 0, false, 0, 0);
	cam->attach(osg::Camera::DEPTH_BUFFER, db, 0, 0, false, 0, 0);
	cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	//cam->setRenderOrder(osg::Camera::PRE_RENDER);
	cam->getGraphicsContext()->setSwapCallback(new RTTSwapCallback(_fbo, _myInfo->myChannel->width, _myInfo->myChannel->height));
	//cam->setPreDrawCallback(new RTTPreDrawCallback(_fbo));

	addBuffer(cam, _fbo);
}

bool ScreenBase::resolveBuffers(osg::Camera* c, osg::FrameBufferObject* resolve_fbo, osg::State* state, GLbitfield buffers)
{
	if (framebuffers.find(c) == framebuffers.end()) {
		return false;
	}

	osg::FrameBufferObject* fbo = framebuffers[c];
	const osg::GLExtensions* fbo_ext = state->get<osg::GLExtensions>();
	//Save current framebuffer state
	GLint drawFBO = 0, readFBO = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &drawFBO);
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &readFBO);

	const osg::Texture* src = fbo->getAttachment(osg::Camera::COLOR_BUFFER0).getTexture();
	const osg::Texture* tgt = resolve_fbo->getAttachment(osg::Camera::COLOR_BUFFER0).getTexture();

	//Blit framebuffer to resolve_fbo
	resolve_fbo->apply(*state, osg::FrameBufferObject::DRAW_FRAMEBUFFER);
	fbo->apply(*state, osg::FrameBufferObject::READ_FRAMEBUFFER);
	fbo_ext->glBlitFramebuffer(0, 0, src->getTextureWidth(), src->getTextureHeight(),
		0, 0, tgt->getTextureWidth(), tgt->getTextureHeight(),
		buffers, GL_NEAREST);

	//Restore prev framebuffer state
	fbo_ext->glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, drawFBO);
	fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, readFBO);

	return true;
}

osg::Matrix & ScreenBase::getCurrentHeadMatrix(int head)
{
    // for ref return, single threaded, ok but not ideal
    static osg::Matrix omniMat;

    if(!_omniStereo)
    {
        return TrackingManager::instance()->getHeadMat(head);
    }
    else
    {
        osg::Vec3d headDir;
        headDir = _myInfo->xyz
                - TrackingManager::instance()->getHeadMat(head).getTrans();
        headDir.normalize();

        osg::Vec3d headingDir = headDir;
        headingDir.z() = 0.0;
        headingDir.normalize();
        osg::Vec3d pitchDir = headDir;
        pitchDir.x() = 0.0;
        pitchDir.normalize();

        omniMat = osg::Matrix::identity();
        // check if pitch is valid
        if(pitchDir.length2() > 0.8)
        {
            omniMat *= osg::Matrix::rotate(osg::Vec3d(0,1.0,0),pitchDir);
        }

        if(headingDir.length2() > 0.8)
        {
            omniMat *= osg::Matrix::rotate(osg::Vec3d(0,1.0,0),headingDir);
        }

        omniMat *= osg::Matrix::translate(
                TrackingManager::instance()->getHeadMat(head).getTrans());

        return omniMat;
    }
}

osg::Vec3d ScreenBase::defaultLeftEye(int head)
{
    return osg::Vec3d(-_separation * _eyeSepMult / 2.0,0.0,0.0)
            * getCurrentHeadMatrix(head);
}

osg::Vec3d ScreenBase::defaultRightEye(int head)
{
    return osg::Vec3d(_separation * _eyeSepMult / 2.0,0.0,0.0)
            * getCurrentHeadMatrix(head);
}

void ScreenBase::computeDefaultViewProj(osg::Vec3d eyePos, osg::Matrix & view,
        osg::Matrix & proj)
{
    //translate screen to origin
    osg::Matrix screenTrans;
    screenTrans.makeTranslate(-_myInfo->xyz);

    //rotate screen to xz
    osg::Matrix screenRot;
    screenRot.makeRotate(-_myInfo->h * M_PI / 180.0,osg::Vec3(0,0,1),
            -_myInfo->p * M_PI / 180.0,osg::Vec3(1,0,0),
            -_myInfo->r * M_PI / 180.0,osg::Vec3(0,1,0));

    eyePos = eyePos * screenTrans * screenRot;

    //make frustum
    float top, bottom, left, right;
    float screenDist = -eyePos.y();

    top = _near * (_myInfo->height / 2.0 - eyePos.z()) / screenDist;
    bottom = _near * (-_myInfo->height / 2.0 - eyePos.z()) / screenDist;
    right = _near * (_myInfo->width / 2.0 - eyePos.x()) / screenDist;
    left = _near * (-_myInfo->width / 2.0 - eyePos.x()) / screenDist;

    proj.makeFrustum(left,right,bottom,top,_near,_far);

    // move camera to origin
    osg::Matrix cameraTrans;
    cameraTrans.makeTranslate(-eyePos);

    //make view
    view = screenTrans * screenRot * cameraTrans
            * osg::Matrix::lookAt(osg::Vec3(0,0,0),osg::Vec3(0,1,0),
                    osg::Vec3(0,0,1));
}

void ScreenBase::setNear(double near)
{
	_near = near;
}

void ScreenBase::setFar(double far)
{
	_far = far;
}


void RTTSwapCallback::swapBuffersImplementation(osg::GraphicsContext* gc)
{
	const osg::GLExtensions* fbo_ext = gc->getState()->get<osg::GLExtensions>();

	fbo_ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

	m_fbo->apply(*gc->getState(), osg::FrameBufferObject::READ_FRAMEBUFFER);
	fbo_ext->glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
	fbo_ext->glBlitFramebuffer(0, 0, m_width, m_height,
		0, 0, gc->getTraits()->width, gc->getTraits()->height,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);
	fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);

	gc->swapBuffersImplementation();
}
