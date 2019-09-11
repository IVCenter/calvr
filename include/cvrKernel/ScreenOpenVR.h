#ifndef CALVR_SCREEN_OCULUS_H
#define CALVR_SCREEN_OCULUS_H

#include <cvrKernel/ScreenBase.h>
#include <cvrKernel/CVRViewer.h>

#include <osg/DisplaySettings>
#include <osg/Texture2D>
#include <osg/Program>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Camera>
#include <osgUtil/SceneView>
#include <OpenThreads/Mutex>

#include <openvr.h>
#include <openvrdevice.h>

#include <list>

namespace cvr
{

	/**
	 * @addtogroup screens
	 * @{
	 */

	/*
	class SOVRPreDrawCallback : public osg::Camera::DrawCallback
	{

	};

	class SOVRPostDrawCallback : public osg::Camera::DrawCallback
	{

	};
	*/

	class SOVRSwapCallback : public osg::GraphicsContext::SwapCallback
	{
		public:
			explicit SOVRSwapCallback(osg::ref_ptr<osg::FrameBufferObject> left_fbo, osg::ref_ptr<osg::FrameBufferObject> right_fbo, int width, int height, OpenVRMirrorTexture::BlitOptions blit)
				: m_left_fbo(left_fbo), m_right_fbo(right_fbo), m_blit(blit)
			{
				m_resolve_fbo = new osg::FrameBufferObject();
				osg::Texture2D* cb = new osg::Texture2D();
				cb->setName("ColorTexture");
				cb->setTextureSize(width, height);
				cb->setResizeNonPowerOfTwoHint(false);
				cb->setInternalFormat(GL_RGBA);
				cb->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
				cb->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
				cb->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
				cb->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
				m_resolve_fbo->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(cb));
			}
			void swapBuffersImplementation(osg::GraphicsContext* gc);
			int frameIndex() const { return m_frameIndex; }
		private:
			osg::ref_ptr<osg::FrameBufferObject> m_left_fbo;
			osg::ref_ptr<osg::FrameBufferObject> m_right_fbo;
			osg::ref_ptr<osg::FrameBufferObject> m_resolve_fbo;
			int m_frameIndex;
			OpenVRMirrorTexture::BlitOptions m_blit;
	};

	class SOVRInitialDrawCallback : public osg::Camera::DrawCallback
	{
	public:
		SOVRInitialDrawCallback(OpenVRDevice* device, OpenVRDevice::Eye eye)
			: m_device(device)
			, m_eye(eye)
			, textures_created(false)
		{
		}

		virtual void operator()(osg::RenderInfo& renderInfo) const
		{
			/*
			osg::GraphicsOperation* graphicsOperation = renderInfo.getCurrentCamera()->getRenderer();
			osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(graphicsOperation);
			if (renderer != nullptr)
			{
				// Disable normal OSG FBO camera setup because it will undo the MSAA FBO configuration.
				renderer->setCameraRequiresSetUp(false);
			}
			*/

			if (!textures_created && m_eye == OpenVRDevice::Eye::LEFT) {
				m_device->createRenderBuffers(renderInfo.getState());
				textures_created = true;
			}
			renderInfo.getCurrentCamera()->setPreDrawCallback(new OpenVRPreDrawCallback(renderInfo.getCurrentCamera(), m_device->m_textureBuffer[m_eye]));
			renderInfo.getCurrentCamera()->setPostDrawCallback(new OpenVRPostDrawCallback(renderInfo.getCurrentCamera(), m_device->m_textureBuffer[m_eye]));
		}
	protected:
		OpenVRDevice* m_device;
		OpenVRDevice::Eye m_eye;
		mutable bool textures_created;
	};

	/**
	 * @brief Creates an OpenVR display screen
	 */
	class ScreenOpenVR : public ScreenBase
	{
		public:
			ScreenOpenVR();
			virtual ~ScreenOpenVR();

			/**
			 * @brief Create head mounted display screen
			 * @param mode osg stereo mode to use from enum osg::DisplaySettings::StereoMode
			 */
			virtual void init(int mode = 0);

			/**
			 * @brief Recompute the view and projection matrices for screen
			 */
			virtual void computeViewProj();

			/**
			 * @brief update osg::Cameras with current view/proj matrices
			 */
			virtual void updateCamera();

			/**
			 * @brief set the opengl clear color
			 */
			virtual void setClearColor(osg::Vec4 color);

			/**
			 * @brief find if this screen contains this camera
			 */
			virtual ScreenInfo * findScreenInfo(osg::Camera * c);

			virtual void adjustViewportCoords(int & x, int & y);

			static bool isInit()
			{
				return _init;
			}

			virtual void resolveBuffers(osg::Camera* c, osg::FrameBufferObject* resolve_fbo, osg::State* state, GLbitfield buffers = GL_COLOR_BUFFER_BIT)
			{
				//Choose correct framebuffer (left or right) depending on camera
				osg::FrameBufferObject* fbo;
				if (c == _leftCamera)
				{
					fbo = _leftFBO;
				}
				else if (c == _rightCamera)
				{
					fbo = _rightFBO;
				}

				if (!fbo)
				{
					return;
				}
				const osg::GLExtensions* fbo_ext = state->get<osg::GLExtensions>();
				//Save current framebuffer state
				GLint drawFBO = 0, readFBO = 0;
				glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &drawFBO);
				glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &readFBO);

				const osg::Texture2D* src = (osg::Texture2D*)fbo->getAttachment(osg::Camera::COLOR_BUFFER0).getTexture();
				const osg::Texture2D* tgt = (osg::Texture2D*)resolve_fbo->getAttachment(osg::Camera::COLOR_BUFFER0).getTexture();

				//Blit framebuffer to resolve_fbo
				resolve_fbo->apply(*state, osg::FrameBufferObject::DRAW_FRAMEBUFFER);
				fbo->apply(*state, osg::FrameBufferObject::READ_FRAMEBUFFER);
				fbo_ext->glBlitFramebuffer(0, 0, src->getTextureWidth(), src->getTextureHeight(),
					0, 0, tgt->getTextureWidth(), tgt->getTextureHeight(),
					buffers, GL_NEAREST);

				//Restore prev framebuffer state
				fbo_ext->glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, drawFBO);
				fbo_ext->glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, readFBO);
			};


		protected:

			static bool _init;
			OpenVRDevice * vrDevice;

			osg::GraphicsContext::SwapCallback * _swapCallback;

			osg::ref_ptr<osg::Camera> _leftCamera;
			osg::ref_ptr<osg::Camera> _rightCamera;
			osg::ref_ptr<osg::Camera> _previewCamera;

			//uint32_t _renderWidth;
			//uint32_t _renderHeight;

			osg::ref_ptr<osg::FrameBufferObject> _leftFBO;
			osg::ref_ptr<osg::FrameBufferObject> _rightFBO;

	};

	/**
	 * @}
	 */

}

#endif