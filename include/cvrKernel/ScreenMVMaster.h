/**
 * @file ScreenMVMaster.h
 */

#ifndef CALVR_SCREEN_MV_MASTER_H
#define CALVR_SCREEN_MV_MASTER_H

#include <cvrKernel/ScreenMVSimulator.h>

namespace cvr
{

/**
 * @brief Creates a screen for non stereo rendering
 */
class ScreenMVMaster : public ScreenMVSimulator
{
    public:
        ScreenMVMaster();
        virtual ~ScreenMVMaster();

        /**
         * @brief Creates the screen and adds it to the viewer
         * @param mode value is unused
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

        /**
         * @brief Callback for mouse interaction with the viewport
         * @param x Integer value of the x coordinate
         * @param y Integer value of the y coordinate
         */
        virtual void adjustViewportCoords(int &x, int&y);

        /**
         * @brief Set diagram as shown or not
         * @param show Whether or not to show the diagram
         */
        void showDiagram(bool show);
    protected:
        osg::ref_ptr<osg::Camera> _cameraScene;  ///< camera created for scene
        osg::ref_ptr<osg::Camera> _cameraDiagram; ///< camera created for diagram
        osg::Matrix _view;                  ///< view matrix
        osg::Matrix _proj;                  ///< projection matrix

        float viewProjRatio; ///< Ratio used to adjust XY mouse coordinates while viewing the diagram camera.
        /**
         * @brief Instantiates a camera and scene to display a diagram of the users' head positions within the cave. This must be called in the init() call of the master node's screen, if the diagram is to be used
         */
        void setupDiagramCam();
};

}

#endif
