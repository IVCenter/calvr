/**
 * @file ScreenMono.h
 */

#ifndef CALVR_SCREEN_MONO_H
#define CALVR_SCREEN_MONO_H

#include <cvrKernel/ScreenBase.h>

namespace cvr
{

/**
 * @addtogroup screens
 * @{
 */

/**
 * @brief Creates a screen for non stereo rendering
 */
class ScreenMono : public ScreenBase
{
    public:
        ScreenMono();
        virtual ~ScreenMono();

        /**
         * @brief defines the eye being rendered
         */
        enum monoType
        {
            CENTER = 0, LEFT, RIGHT
        };

        /**
         * @brief Creates the screen and adds it to the viewer
         * @param mode value interpreted as ScreenMono::monoType, defines eye position
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

        virtual void viewportResized(int left, int bottom, int width,
                int height);
    protected:
        monoType _type;                     ///< defines eye location
        osg::ref_ptr<osg::Camera> _camera;  ///< camera created for screen
        osg::Matrix _view;                  ///< view matrix
        osg::Matrix _proj;                  ///< projection matrix
};

/**
 * @}
 */

}

#endif
