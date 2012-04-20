/**
 * @file ScreenFixedViewer.h
 */

#ifndef CALVR_SCREEN_FIXED_VIEWER_H
#define CALVR_SCREEN_FIXED_VIEWER_H

#include <cvrKernel/ScreenBase.h>

namespace cvr
{

/**
 * @brief Creates a screen with the viewer positon set from
 * the config file 
 */
class ScreenFixedViewer : public ScreenBase
{
    public:
        ScreenFixedViewer();
        virtual ~ScreenFixedViewer();

        /**
         * @brief Creates the screen and adds it to the viewer
         * @param mode currently ignored
         */
        virtual void init(int mode = 0);

        /**
         * @brief Recalculates view and projection matrices
         */
        virtual void computeViewProj();

        /**
         * @brief Updates camera with current view/proj matrices
         */
        virtual void updateCamera();

        /**
         * @brief Set opengl clear color for this screen
         */
        virtual void setClearColor(osg::Vec4 color);

        /**
         * @brief See if this screen created the given osg::Camera
         */
        virtual ScreenInfo * findScreenInfo(osg::Camera * c);
    protected:
        osg::ref_ptr<osg::Camera> _camera;  ///< camera created for screen
        osg::Matrix _view;                  ///< view matrix
        osg::Matrix _proj;                  ///< projection matrix
        osg::Matrix _viewerMat; ///< matrix to use for viewer position/orientation
};

}

#endif
