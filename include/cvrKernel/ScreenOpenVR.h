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

		protected:

			static bool _init;
			OpenVRDevice * vrDevice;

			OpenVRSwapCallback * _swapCallback;

			osg::ref_ptr<osg::Camera> _leftCamera;
			osg::ref_ptr<osg::Camera> _rightCamera;
			osg::ref_ptr<osg::Camera> _previewCamera;
	};

	/**
	 * @}
	 */

}

#endif