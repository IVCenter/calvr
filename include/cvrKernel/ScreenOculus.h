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

#include <OVR_Version.h>
#include <OVR_CAPI.h>

#include <list>

namespace cvr
{

/**
 * @addtogroup screens
 * @{
 */

struct OculusPreDrawCallback : public osg::Camera::DrawCallback
{
	virtual void operator()(osg::RenderInfo& renderInfo) const;
	void initTextureSet() const;

	mutable GLuint fbo[2];
	mutable int fboIndex;
	mutable bool twoSV;
	int width;
	int height;
	ovrSession * session;
	int eye;
	mutable ovrLayerEyeFov * layer;
#if OVR_PRODUCT_VERSION < 1
	mutable ovrSwapTextureSet * textureSet;
	mutable ovrTexture * mirrorTex;
	int mWidth, mHeight;
	unsigned int mFormat;
#else
	mutable ovrTextureSwapChain textureChain;
	mutable ovrTextureSwapChainDesc textureChainDesc;
	ovrMirrorTextureDesc mirrorDesc;
	mutable ovrMirrorTexture mirrorTex;
#endif
	osg::ref_ptr<osg::Texture2D> previewTexture;

};

struct OculusFramePoseInfo
{
	int frameNumber;
	ovrPosef RenderPose[2];
};

struct OculusSwapCallback : public osg::GraphicsContext::SwapCallback 
{
	void swapBuffersImplementation(osg::GraphicsContext *gc);

	void addFramePose(OculusFramePoseInfo * pinfo);

	OpenThreads::Mutex mutex;
	std::list<OculusFramePoseInfo*> poseList;
	ovrSession * session;
	ovrLayerEyeFov * layer;
	bool disableVsync;
};

/**
 * @brief Creates an Oculus display screen
 */
class ScreenOculus : public ScreenBase
{
    public:
        ScreenOculus();
        virtual ~ScreenOculus();

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

    protected:
		void initTextures();

		bool _init;
		ovrSession _session;
		ovrHmdDesc _hmd;
		ovrLayerEyeFov _layer;
		ovrVector3f _hmdToEyeViewOffset[2];

		osg::Matrix _viewLeft; ///< left eye view matrix
		osg::Matrix _viewRight; ///< right eye view matrix
		osg::Matrix _projLeft; ///< left eye projection matrix
		osg::Matrix _projRight; ///< right eye projection matrix

		osg::ref_ptr<osg::Texture2D> _leftTexture;
		osg::ref_ptr<osg::Texture2D> _leftDepthTexture;
		osg::ref_ptr<osg::Texture2D> _rightTexture;
		osg::ref_ptr<osg::Texture2D> _rightDepthTexture;

		osg::ref_ptr<osg::Camera> _cameraLeft; ///< osg::Camera for this screen
		osg::ref_ptr<osg::Camera> _cameraRight;

		osg::ref_ptr<osg::Camera> _previewCamera;

		osg::Vec3 _cameraPos;

		OculusSwapCallback * _swapCallback;
		long long _frameNumber;
};

/**
 * @}
 */

}

#endif