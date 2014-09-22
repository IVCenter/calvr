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

#include <OVR.h>

namespace cvr
{

/**
 * @addtogroup screens
 * @{
 */

struct OculusCameraCallback : public osg::Camera::DrawCallback
{
	virtual void operator() (osg::RenderInfo &renderInfo) const;

	ovrHmd hmd;
	ovrPosef headPose[2];
	ovrFrameTiming frameTiming;
	mutable osg::ref_ptr<osg::Uniform> _leftTWStart;
	mutable osg::ref_ptr<osg::Uniform> _leftTWEnd;
	mutable osg::ref_ptr<osg::Uniform> _rightTWStart;
	mutable osg::ref_ptr<osg::Uniform> _rightTWEnd;
};

struct OculusFrameEndCallback : public PerContextCallback
{
	virtual void perContextCallback(int contextid, PCCType type) const;

	int screenID;
	ovrHmd hmd;
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
		ovrHmd _hmd;
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
		osg::ref_ptr<osg::Camera> _cameraDistLeft;
		osg::ref_ptr<osg::Camera> _cameraDistRight;

		osg::ref_ptr<osg::Shader> _distVert;
		osg::ref_ptr<osg::Shader> _distFrag;
		osg::ref_ptr<osg::Program> _distProg;

		//osg::ref_ptr<osg::Geode> _meshGeode;
		//osg::ref_ptr<osg::Geometry> _meshGeom;

		osg::ref_ptr<osg::Geode> _meshLeftGeode;
		osg::ref_ptr<osg::Geode> _meshRightGeode;
		osg::ref_ptr<osg::Geometry> _meshLeftGeom;
		osg::ref_ptr<osg::Geometry> _meshRightGeom;

		osg::Vec3 _cameraPos;
		osg::ref_ptr<OculusCameraCallback> _cameraCallback;
		OculusFrameEndCallback * _fec;
};

/**
 * @}
 */

}

#endif