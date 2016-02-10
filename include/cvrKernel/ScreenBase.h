/**
 * @file ScreenBase.h
 */

#ifndef CALVR_SCREEN_BASE_H
#define CALVR_SCREEN_BASE_H

#include <cvrKernel/Export.h>
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
class CVRKERNEL_EXPORT ScreenBase
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
         * @brief Called on a master node screen if the viewport has been 
         * resized
         * @param left new viewport left value
         * @param bottom new viewport bottom value
         * @param width new viewport width value
         * @param height new viewport height value
         */
        virtual void viewportResized(int left, int bottom, int width,
                int height)
        {
            _myInfo->myChannel->left = (float)left;
            _myInfo->myChannel->bottom = (float)bottom;
            _myInfo->myChannel->width = (float)width;
            _myInfo->myChannel->height = (float)height;
            return;
        }

        /**
         * @brief Get near plane value for all screens
         */
        static double getNear()
        {
            return _near;
        }

        /**
         * @brief Set near plane value for all screens
         */
		static void setNear(double near);

        /**
         * @brief Get far plane value for all screens
         */
        static double getFar()
        {
            return _far;
        }

        /**
         * @brief Set far plane value for all screens
         */
		static void setFar(double far);

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
         * @brief Get if omni stereo rendering mode is active
         */
        static bool getOmniStereoActive()
        {
            return _omniStereo;
        }

        /**
         * @brief Set if omni stereo rendering mode is active
         */
        static void setOmniStereoActive(bool active)
        {
            _omniStereo = active;
        }

        /**
         * @brief Applies some default settings to the camera
         *
         * Sets up viewport, sets culling mode/masks, etc
         */
        void defaultCameraInit(osg::Camera * cam);

        /**
         * @brief Get the head orientation matrix for a given head id
         */
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

        static bool _omniStereo; ///< is omni stereo mode active

        ScreenInfo * _myInfo; ///< config information for this screen
};

/**
 * @}
 */
/**
 * @}
 */

}

#endif
