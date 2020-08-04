/**
 * @file ScreenInterlacedTopBottom.h
 */

#ifndef CALVR_SCREEN_INTERLACED_TOP_BOTTOM_H
#define CALVR_SCREEN_INTERLACED_TOP_BOTTOM_H

#include <cvrKernel/ScreenBase.h>

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
 * @addtogroup screens
 * @{
 */

/**
 * @brief Creates a screen that renders top/bottom stereo to textures, then uses a 
 *  shader to interlace it
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
                computeRightEyeProjection(const osg::Matrixd &projection) const;
                virtual osg::Matrixd
                computeRightEyeView(const osg::Matrixd &view) const;
                ScreenInterlacedTopBottom * screen;
        };

        /**
         * @brief Osg camera callback that interlaces the right/left eye images using
         *  a shader
         */
        struct InterlaceCallback : public osg::Camera::DrawCallback
        {
                /**
                 * @brief Called by osg camera draw callback
                 */
                virtual void operator()(osg::RenderInfo &renderInfo) const;
                ScreenInterlacedTopBottom * screen; ///< screen for this callback

                mutable std::map<int,bool> _initMap; ///< map of shader initialization per context
                mutable std::map<int,osg::ref_ptr<osg::Program> > _programMap; ///< shader program
                mutable std::map<int,osg::ref_ptr<osg::Geometry> > _geometryMap; ///< screen filling quad
                mutable bool odd; ///< draw to odd or even lines
                mutable bool first; ///< if first to draw, clear color/depth buffers
                static OpenThreads::Mutex _initLock; ///< lock for multithread init ops
                osg::Texture2D * _texture; ///< texture of rendered eye
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

        osg::ref_ptr<osg::Camera> _cameraL; ///< osg::Camera for left eye
        osg::ref_ptr<osg::Camera> _cameraR; ///< osg::Camera for right eye
        osg::ref_ptr<osg::Texture2D> _colorTextureL; ///< texture render target of left eye
        osg::ref_ptr<osg::Texture2D> _colorTextureR; ///< texture render target of right eye

        //osg::Image * image;
};

/**
 * @}
 */

}

#endif
