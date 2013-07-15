/**
 * @file ScreenStereo.h
 */

#ifndef CALVR_SCREEN_STEREO_H
#define CALVR_SCREEN_STEREO_H

#include <cvrKernel/Screens/ScreenBase.h>

#include <osg/DisplaySettings>
#include <osgUtil/SceneView>

namespace cvr
{

/**
 * @addtogroup screens
 * @{
 */

/**
 * @brief Creates a stereo screen using osg stereo modes
 */
class ScreenStereo : public ScreenBase
{
    public:
        ScreenStereo();
        virtual ~ScreenStereo();

        /**
         * @brief Receives callbacks from osg render for view and projection matrices
         */
        struct StereoCallback : public osgUtil::SceneView::ComputeStereoMatricesCallback
        {
                virtual osg::Matrixd
                computeLeftEyeProjection(const osg::Matrixd &projection) const;
                virtual osg::Matrixd
                computeLeftEyeView(const osg::Matrixd &view) const;
                virtual osg::Matrixd
                computeRightEyeProjection(const osg::Matrixd &projection) const;
                virtual osg::Matrixd
                computeRightEyeView(const osg::Matrixd &view) const;
                ScreenStereo * screen;
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

        virtual void viewportResized(int left, int bottom, int width, int height);

        /**
         * @brief Get the currently set osg stereo mode
         */
        osg::DisplaySettings::StereoMode getStereoMode()
        {
            return _stereoMode;
        }

        /**
         * @brief Set the osg stereo mode for this screen
         */
        void setStereoMode(osg::DisplaySettings::StereoMode sm);

    protected:
        osg::DisplaySettings::StereoMode _stereoMode; ///< osg stereo mode for this screen
        osg::Matrix _viewLeft; ///< left eye view matrix
        osg::Matrix _viewRight; ///< right eye view matrix
        osg::Matrix _projLeft; ///< left eye projection matrix
        osg::Matrix _projRight; ///< right eye projection matrix

        osg::ref_ptr<osg::Camera> _camera; ///< osg::Camera for this screen
};

/**
 * @}
 */

}

#endif
