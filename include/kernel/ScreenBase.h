/**
 * @file ScreenBase.h
 */

#ifndef CALVR_SCREEN_BASE_H
#define CALVR_SCREEN_BASE_H

#include <kernel/ScreenConfig.h>

#include <osgViewer/Viewer>

#include <map>

namespace cvr
{

/**
 * @brief Virtual base class for all screen implementations
 */
class ScreenBase
{
    friend class ScreenConfig;
    public:
        ScreenBase();

        virtual ~ScreenBase()
        {
        }

        /**
         * @brief create a screen and add all cameras to the viewer
         * @param mode optional specifier used by some screens, ie \c ScreenStereo used it
         *        to specify the osg stereo mode
         */
        virtual void init(int mode = 0) = 0;

        /**
         * @brief Recompute the view and projection matrices for all this screens cameras
         */
        virtual void computeViewProj() = 0;

        /**
         * @brief Update all screen cameras with current view/proj matrices
         */
        virtual void updateCamera() = 0;

        /**
         * @brief Set opengl clear color for this screen
         */
        virtual void setClearColor(osg::Vec4 color) = 0;

        /**
         * @brief Get the screen params used to create this screen
         */
        ScreenInfo * getScreenInfo() { return _myInfo; }

        /**
         * @brief See if a given osg::Camera was created by this screen
         * @return returns NULL if not found
         */
        virtual ScreenInfo * findScreenInfo(osg::Camera * c) = 0;

        // TODO make these pure virtual, or maybe move values into base class
        static void setNear(float near) { _near = near; }
        static void setFar(float far) { _far = far; }

        void defaultCameraInit(osg::Camera * cam);

        osg::Matrix & getCurrentHeadMatrix(int head = 0);

        osg::Vec3d defaultLeftEye(int head = 0);
        osg::Vec3d defaultRightEye(int head = 0);
        void computeDefaultViewProj(osg::Vec3d eyePos, osg::Matrix & view, osg::Matrix & proj);

    protected:
        static double _separation;
        static double _eyeSepMult;
        static double _near;
        static double _far;

        ScreenInfo * _myInfo;

        std::map<int,osg::Matrix> _currentHeadMatList;
        bool _headMatListInit;
};

}

#endif
