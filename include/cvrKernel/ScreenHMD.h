/**
 * @file ScreenHMD.h
 */

#ifndef CALVR_SCREEN_HMD_H
#define CALVR_SCREEN_HMD_H

#include <cvrKernel/ScreenBase.h>

#include <osg/DisplaySettings>
#include <osgUtil/SceneView>

namespace cvr
{

/**
 * @brief Creates a head mounted display screen
 */
class ScreenHMD : public ScreenBase
{
    public:
        ScreenHMD();
        virtual ~ScreenHMD();

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
                ScreenHMD * screen;
        };

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
        bool _stereo;
        osg::DisplaySettings::StereoMode _stereoMode; ///< osg stereo mode for this screen
        osg::Matrix _viewLeft; ///< left eye view matrix
        osg::Matrix _viewRight; ///< right eye view matrix
        osg::Matrix _projLeft; ///< left eye projection matrix
        osg::Matrix _projRight; ///< right eye projection matrix

        osg::ref_ptr<osg::Camera> _camera; ///< osg::Camera for this screen
};

}

#endif
