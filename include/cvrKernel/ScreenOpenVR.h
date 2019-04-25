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

//


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

    protected:

		bool _init;
		OpenVRDevice * vrDevice;

		OpenVRSwapCallback * _swapCallback;

		osg::ref_ptr<osg::Camera> _leftCamera;
		osg::ref_ptr<osg::Camera> _rightCamera;
};

/**
 * @}
 */

}

#endif