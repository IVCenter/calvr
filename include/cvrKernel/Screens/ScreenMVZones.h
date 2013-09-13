/**
 * @file ScreenMVZones.h
 * @author John Mangan
 */

#ifndef CALVR_SCREEN_MV_ZONES_H
#define CALVR_SCREEN_MV_ZONES_H

#include <vector>
#include <cvrKernel/Export.h>
#include <cvrKernel/Screens/ScreenBase.h>
#include <cvrKernel/Screens/ScreenMVSimulator.h>
#include <osgViewer/Renderer>

namespace cvr
{
typedef void (*setContributionFunc)(osg::Vec3 toZone0, osg::Vec3 orientation0,
        float &contribution0, osg::Vec3 toZone1, osg::Vec3 orientation1,
        float &contribution1);

/**
 * @addtogroup screens
 * @{
 */

/**
 * @brief Creates a stereo screen for multiple viewers using osg stereo modes and position/orientation weighting
 */
class CVRKERNEL_EXPORT ScreenMVZones : public ScreenMVSimulator
{
    public:
        ScreenMVZones();
        virtual ~ScreenMVZones();

        /**
         * @brief Receives callbacks from osg render for view and projection matrices
         */
        struct StereoCallback : public osgUtil::SceneView::ComputeStereoMatricesCallback
        {
                virtual osg::Matrixd
                computeLeftEyeProjection(const osg::Matrixd &) const;
                virtual osg::Matrixd
                computeLeftEyeView(const osg::Matrixd &) const;
                virtual osg::Matrixd
                computeRightEyeProjection(const osg::Matrixd &) const;
                virtual osg::Matrixd
                computeRightEyeView(const osg::Matrixd &) const;
                osg::Matrix _viewLeft; ///< left eye view matrix
                osg::Matrix _viewRight; ///< right eye view matrix
                osg::Matrix _projLeft; ///< left eye projection matrix
                osg::Matrix _projRight; ///< right eye projection matrix
        };

        /**
         * @brief Create stereo screen for viewer
         * @param mode osg stereo mode to use from enum osg::DisplaySettings::StereoMode
         */
        virtual void init(int mode = 0);

        /**
         * @brief Recompute the view and projection matrices for screen
         */
        virtual void computeViewProj();

        /**
         * @brief update osg::Cameras with current view/proj matrices (not used for this screen)
         */
        virtual void updateCamera();

        /**
         * @brief set the opengl clear color for all cameras in this screen
         * @param color  osg Vec4 to use as the clear color
         */
        virtual void setClearColor(osg::Vec4 color);

        /**
         * @brief find if this screen contains this camera
         */
        virtual ScreenInfo * findScreenInfo(osg::Camera * c);

        /**
         * @brief Sets the contribution function that is used.
         * @param funcNum integer representing the contribution function index of the desired function within the setContributionFuncs vector
         * @return true = success, false = out-of-range
         */
        static bool setSetContributionFunc(int funcNum);
        /**
         * @brief Sets whether or not orientation angles use 3d calculations (as opposed to 2d)
         * @param o3d bool stating whether or not to use 3d calculations
         */
        static void setOrientation3d(bool o3d);
        /**
         * @brief Returns whether or not orientation angles use 3d calculations (as opposed to 2d)
         * @return true = 3d, false = 2d
         */
        static bool getOrientation3d();
        /**
         * @brief Sets how many rows of zones the screen has, when auto-adjustment is disabled and multiple-users is enabled.
         * @param rows integer representing the desired quantity of zone rows
         */
        static void setZoneRows(int rows);
        /**
         * @brief Gets how many rows of zones the screen has, when auto-adjustment is disabled and multiple-users is enabled.
         * @return quantity of zone rows
         */
        static int getZoneRows();
        /**
         * @brief Sets how many columns of zones the screen has, when auto-adjustment is disabled and multiple-users is enabled.
         * @param columns integer representing the desired quantity of zone columns
         */
        static void setZoneColumns(int columns);
        /**
         * @brief Gets how many columns of zones the screen has, when auto-adjustment is disabled and multiple-users is enabled.
         * @return quantity of zone columns
         */
        static int getZoneColumns();
        /**
         * @brief Gets the maximum number of zone rows the screen can have.
         * @return maximum quantity of zone rows
         */
        static int getMaxZoneRows();
        /**
         * @brief Gets the maximum number of zone columns the screen can have.
         * @return maximum quantity of zone columns
         */
        static int getMaxZoneColumns();
        /**
         * @brief Sets whether or not the zone quantity auto-adjusts to match the target Frames Per Second (while in multi-user mode)
         * @param adjust bool value representing whether or not the zone quantity should auto-adjust to match the target FPS
         */
        static void setAutoAdjust(bool adjust);
        /**
         * @brief Gets whether or not the zone quantity auto-adjusts to match the target Frames Per Second (while in multi-user mode)
         * @return true = auto-adjust enabled, false = auto-adjust disabled
         */
        static bool getAutoAdjust();
        /**
         * @brief Sets the auto-adjustment's target Frames Per Second
         * @param target float value representing the desired Frames Per Second
         */
        static void setAutoAdjustTarget(float target);
        /**
         * @brief Gets the auto-adjustment's target Frames Per Second
         * @return target Frames Per Second
         */
        static float getAutoAdjustTarget();
        /**
         * @brief Sets the auto-adjustment's acceptable Frames Per Second offset from its target
         * @param offset float value representing the acceptable Frames Per Second offset
         */
        static void setAutoAdjustOffset(float offset);
        /**
         * @brief Gets the auto-adjustment's acceptable Frames Per Second offset from its target
         * @return acceptable FPS offset
         */
        static float getAutoAdjustOffset();
        /**
         * @brief Sets whether or not the Screen is running in multi-user mode
         * @param multipleUsers bool representing whether to run in multi-user mode or single-user mode
         */
        static void setMultipleUsers(bool multipleUsers);
        /**
         * @brief Gets whether or not the Screen is running in multi-user mode
         * @return true = multi-user mode, false = single-user mode
         */
        static bool getMultipleUsers();
        /**
         * @brief Sets whether or not the zones should be colored based on contributions from users
         * @param zoneColoring bool representing whether to run in zone coloring mode or not
         */
        static void setZoneColoring(bool zoneColoring);
        /**
         * @brief Gets whether or not the zones should be colored based on contributions from users
         * @return true = color, false = default (black)
         */
        static bool getZoneColoring();
        /**
         * @brief Sets the contribution variable for setContributionFuncs to use
         * @param var float value to set the contribution variable to
         */
        static void setContributionVar(float var);
        /**
         * @brief Gets the contribution variable for setContributionFuncs to use.
         * @return the contribution variable for determining offsets
         */
        static float getContributionVar();
        /**
         * @brief Sets whether or not to auto-adjust the contribution variable for setContributionFuncs to use
         * @param autoCV bool representing whether the contribution variable shoudl auto-adjust
         */
        static void setAutoContributionVar(bool autoCV);
        /**
         * @brief Gets whether or not the contribution variable is automatically adjusted
         * @return whether or not the contribution variable is toggled for automatic adjustment
         */
        static bool getAutoContributionVar();

    protected:
        bool _colorZones; ///< Flags whether or not to color zones via user contribution values
        static bool _zoneColoring; ///< Flags what colorZones shoulds be set to
        static bool _multipleUsers; ///< flags whether the screen is running in multi-user mode or single-user mode
        static bool _orientation3d; ///< flags whether the zone contribution is based upon 3d or 2d vector angles
        static osg::Vec4 _clearColor; ///< The clear color for the screen
        static bool _autoAdjust; ///< flags that the screen should auto-adjust the zone quantity to match the target FPS
        static float _autoAdjustTarget; ///< the auto-adjustment's target FPS
        static float _autoAdjustOffset; ///< the auto-adjustment's acceptable FPS offset
        bool _zonesChanged; ///< determines if the zone quantity has changed since the previous frame
        int _zones; ///< How many zones are being used this frame
        int _zoneRows; ///< How many rows of zones are being used this frame
        int _zoneColumns; ///< How many columns of zones are being used this frame
        static int _setZoneRows; ///< The user defined number of zone rows when autoAdjust is false
        static int _setZoneColumns; ///< The user defined number of zone columns when autoAdjust is false
        static int _maxZoneRows; ///< The maximum number of zone rows the screen can have
        static int _maxZoneColumns; ///< The maximum number of zone columns the screen can have
        osg::DisplaySettings::StereoMode _stereoMode; ///< osg stereo mode for this screen

        std::vector<osg::ref_ptr<osg::Camera> > _camera; ///< osg::Camera for this screen
        std::vector<osg::Vec3> _zoneCenter; ///< center for each zone on this screen, in xy pairs

        std::vector<osg::Matrix *> _projLeftPtr; ///< Vector to hold all zones' left eye projection matrices
        std::vector<osg::Matrix *> _projRightPtr; ///< Vector to hold all zones' right eye projection matrices
        std::vector<osg::Matrix *> _viewLeftPtr; ///< Vector to hold all zones' left eye view matrices
        std::vector<osg::Matrix *> _viewRightPtr; ///< Vector to hold all zones' right eye view matrices

        std::vector<osg::Matrix *> * _projPtr; ///< set to either _projLeftPtr, _projRightPtr, or null to setup project matrix in the StarCAVE
        std::vector<osg::Matrix *> * _viewPtr; ///< set to either _viewLeftPtr, _viewRightPtr, or null to setup project matrix in the StarCAVE

        osg::Quat _invScreenRotation; ///< world to screen space rotation

        /**
         * @brief Creates cameras with initial setup for the screen to use
         */
        void createCameras();
        /**
         * @brief Sets the quantity of zone rows and columns to be used
         */
        void determineZoneQuantity();
        /**
         * @brief Updates the zone centers for each zone that is to be used
         */
        void setupZones();
        /**
         * @brief Updates the camera viewports for each zone's camera that is to be used
         */
        void setupCameras();

        /**
         * @brief Sets the eye locations that will determine camera position (per zone that is to be used)
         * @param eyeLeft empty vector of osg Vec3's to be populated
         * @param eyeRight empty vector of osg Vec3's to be populated
         */
        void setEyeLocations(std::vector<osg::Vec3> &eyeLeft,
                std::vector<osg::Vec3> &eyeRight);
        static setContributionFunc setContribution; ///< Sets which contribution function to use for zone contribution calculations
        static std::vector<setContributionFunc> setContributionFuncs; ///< vector of the allowed zone contribution functions
        static float _contributionVar; ///< Variable that can be used in specific setContributionFuncs. (0 implies automatic)
        static bool _autoContributionVar; ///< determines whether or not to auto-set the _contributionVar
};

/**
 * @}
 */

}

#endif
