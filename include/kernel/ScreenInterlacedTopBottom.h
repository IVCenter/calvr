/**
 * @file ScreenInterlacedTopBottom.h
 */

#ifndef CALVR_SCREEN_INTERLACED_TOP_BOTTOM_H
#define CALVR_SCREEN_INTERLACED_TOP_BOTTOM_H

#include <kernel/ScreenBase.h>

#include <osg/DisplaySettings>
#include <osg/Texture2D>
#include <osgUtil/SceneView>
#include <osg/Camera>
#include <osg/Program>
#include <osg/Geometry>
#include <OpenThreads/Mutex>

#include <map>

namespace cvr
{

/**
 * @brief Creates a stereo screen using osg stereo modes
 */
class ScreenInterlacedTopBottom : public ScreenBase
{
    public:
        ScreenInterlacedTopBottom();
        virtual ~ScreenInterlacedTopBottom();

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
                        computeRightEyeProjection(
                                                  const osg::Matrixd &projection) const;
                virtual osg::Matrixd
                        computeRightEyeView(const osg::Matrixd &view) const;
                ScreenInterlacedTopBottom * screen;
        };

        struct InterlaceCallback : public osg::Camera::DrawCallback
        {
            virtual void operator() (osg::RenderInfo &renderInfo) const;
            ScreenInterlacedTopBottom * screen;

            mutable std::map<int,bool> _initMap;
            mutable std::map<int,osg::ref_ptr<osg::Program> > _programMap;
            mutable std::map<int,osg::ref_ptr<osg::Geometry> > _geometryMap;
            mutable bool skip;
            static OpenThreads::Mutex _initLock;
            osg::Texture2D * _texture;
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

    protected:
        osg::DisplaySettings::StereoMode _stereoMode; ///< osg stereo mode for this screen
        osg::Matrix _viewLeft; ///< left eye view matrix
        osg::Matrix _viewRight; ///< right eye view matrix
        osg::Matrix _projLeft; ///< left eye projection matrix
        osg::Matrix _projRight; ///< right eye projection matrix

        osg::ref_ptr<osg::Camera> _camera; ///< osg::Camera for this screen
        osg::ref_ptr<osg::Texture2D> _colorTexture;
        osg::ref_ptr<osg::Texture2D> _depthTexture;
};

}

#endif
