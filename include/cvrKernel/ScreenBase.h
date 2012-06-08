/**
 * @file ScreenBase.h
 */

#ifndef CALVR_SCREEN_BASE_H
#define CALVR_SCREEN_BASE_H

#include <cvrKernel/ScreenConfig.h>

#include <osgViewer/Viewer>

#include <map>

namespace cvr
{

/**
 * @addtogroup kernel
 * @{
 */
/**
 * @addtogroup screens Screens
 * @{
 */

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
        ScreenInfo * getScreenInfo()
        {
            return _myInfo;
        }

        /**
         * @brief See if a given osg::Camera was created by this screen
         * @return returns NULL if not found
         */
        virtual ScreenInfo * findScreenInfo(osg::Camera * c) = 0;

        /**
         * @brief Allows screens a chance to modify the viewport coords
         * for an intersection
         * @param x x coord in range 0 to width
         * @param y y coord in range 0 to height
         */
        virtual void adjustViewportCoords(int & x, int & y)
        {
            return;
        }

        /**
         * @brief Set near plane value for all screens
         */
        static void setNear(float near)
        {
            _near = near;
        }

        /**
         * @brief Set far plane value for all screens
         */
        static void setFar(float far)
        {
            _far = far;
        }

        /**
         * @brief Get the eye separation
         */
        static double getEyeSeparation()
        {
            return _separation;
        }

        /**
         * @brief Get the current eye separation multiplier
         */
        static double getEyeSeparationMultiplier()
        {
            return _eyeSepMult;
        }

        /**
         * @brief Applies some default settings to the camera
         *
         * Sets up viewport, sets culling mode/masks, etc
         */
        void defaultCameraInit(osg::Camera * cam);

        osg::Matrix & getCurrentHeadMatrix(int head = 0);

        /**
         * @brief Get the left eye position for a given user
         */
        osg::Vec3d defaultLeftEye(int head = 0);

        /**
         * @brief Get the right eye position for a given user
         */
        osg::Vec3d defaultRightEye(int head = 0);

        /**
         * @brief Compute the view and projection matrices for an eye position
         *
         * Uses information in the screen info to do the typical calculation
         */
        void computeDefaultViewProj(osg::Vec3d eyePos, osg::Matrix & view,
                osg::Matrix & proj);

    protected:
        static double _separation; ///< eye separation
        static double _eyeSepMult; ///< eye separation multiplier
        static double _near; ///< near plane
        static double _far; ///< far plane

        ScreenInfo * _myInfo; ///< config information for this screen

        std::map<int,osg::Matrix> _currentHeadMatList;
        bool _headMatListInit;
};

/**
 * @}
 */
/**
 * @}
 */

}

#endif
