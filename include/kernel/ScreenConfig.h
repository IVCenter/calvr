/**
 * @file ScreenConfig.h
 */

#ifndef CALVR_SCREEN_CONFIG_H
#define CALVR_SCREEN_CONFIG_H

#include <kernel/Export.h>
#include <kernel/CalVR.h>

#include <osg/Vec3>
#include <osg/Camera>
#include <osg/Matrix>

#include <vector>
#include <string>

namespace cvr
{

class ScreenBase;

/**
 * @brief Parameters to describe a graphics pipe
 */
struct PipeInfo
{
        int server;     ///< x server number
        int screen;     ///< x screen number
};

/**
 * @brief Parameters to describe a window
 */
struct WindowInfo
{
        int width;                  ///< width of window in pixels
        int height;                 ///< height of window in pixels
        int pipeIndex;              ///< index for graphics pipe
        PipeInfo * myPipe;          ///< graphics pipe used for this window
        int left;                   ///< left of window in pixels in x screen coords
        int bottom;                 ///< bottom of window in pixels in x screen coords
        bool decoration;            ///< puts on window boarder with title bar,etc.
        bool supportsResize;        ///< allow window to change size
        bool overrideRedirect;      ///< override os redirects, can be used to force some window size/positions
        bool useCursor;             ///< show mouse cursor in the window
        bool quadBuffer;            ///< true to enable quad buffered stereo
        osg::GraphicsContext * gc;  ///< osg graphics object for this window
};

/**
 * @brief Parameters to describe a channel (section of window)
 */
struct ChannelInfo
{
        float width;            ///< width of channel in pixels
        float height;           ///< height of channel in pixels
        float left;             ///< left of channel in window space
        float bottom;           ///< bottom of channel in window space
        int windowIndex;        ///< index for window holding this channel
        std::string stereoMode; ///< name of stereo mode used for this channel
        int head;               ///< head number to use for viewer position
        WindowInfo * myWindow;  ///< window params for this channel
};

/**
 * @brief Parameters to describe a screen(viewport)
 */
struct ScreenInfo
{
        osg::Vec3 xyz;          ///< center of screen in world space
        float h;                ///< heading rotation
        float p;                ///< pitch rotation
        float r;                ///< roll rotation
        float width;            ///< screen width
        float height;           ///< screen height
        int channelIndex;       ///< index for channel holding this screen
        ChannelInfo * myChannel;///< channel params for this screen
        osg::Matrix transform;  ///< screen space to world space transform (from xyz,h,p,r)
};

/**
 * @brief Reads screen information from config file and creates the needed \c ScreenBase
 *        instances
 */
class CVRKERNEL_EXPORT ScreenConfig
{
    friend class CalVR;
    public:
        ScreenConfig();

        /**
         * @brief Get a static pointer to the instance of this class
         */
        static ScreenConfig * instance();

        /**
         * @brief Create a number of screens and add them to the viewer
         */
        bool init();

        /**
         * @brief Have all screen recompute their view and projection matrices
         */
        void computeViewProj();

        /**
         * Have all screens update the view/proj matrices in their osg::Cameras
         */
        void updateCamera();

        /**
         * @brief Set the opengl clear color for all screens
         */
        void setClearColor(osg::Vec4 color);

        /**
         * @brief Get screen information from the screen using the given osg::Camera
         * @param c camera to search for
         * @param center world space center of found screen
         * @param width width of found screen
         * @param height height of found screen
         */
        void findScreenInfo(osg::Camera * c, osg::Vec3 & center, float & width,
                            float & height);

        /**
         * @brief Find screen number that contains a given camera
         * @return returns -1 if camera is not found
         */
        int findScreenNumber(osg::Camera * c);

        int getNumWindows();

        WindowInfo * getWindowInfo(int window);

        /**
         * @brief Get the number of screens(viewports) created on this node
         */
        int getNumScreens();

        /**
         * @brief Get the screen instance for a given screen number
         * @return returns NULL if number out of range
         */
        ScreenBase * getScreen(int screen);

        /**
         * @brief Get the parameters for a given screen number
         * @return returns NULL if number out of range
         */
        ScreenInfo * getScreenInfo(int screen);

        /**
         * @brief Get the screen info of the screen on the master node with the given number
         * @param screen screen number on the master node
         * @return returns NULL if number out of range
         */
        ScreenInfo * getMasterScreenInfo(int screen);

        /**
         * @brief Set a multiplier to used when generating the screen matrices
         *
         * Mainly used to turn off eye separation
         */
        void setEyeSeparationMultiplier(float mult);

        /**
         * @brief Get the current eye separation multiplier
         */
        float getEyeSeparationMultiplier();

    protected:
        virtual ~ScreenConfig();

        static ScreenConfig * _myPtr;   ///< static self pointer

        bool readPipes();
        bool readWindows();
        bool readChannels();
        bool readScreens();

        bool makeWindows();
        bool makeScreens();

        void syncMasterScreens();

        std::vector<PipeInfo *> _pipeInfoList;      ///< list of pipe params from config file
        std::vector<WindowInfo *> _windowInfoList;  ///< list of window params from config file
        std::vector<ChannelInfo *> _channelInfoList;///< list of channel params from config file
        std::vector<ScreenInfo *> _screenInfoList;  ///< list of screen params from config file

        std::vector<osg::Matrix> _screenTransformList;  ///< list of matrix transform for created screens

        std::vector<ScreenBase *> _screenList;      ///< list of all created screens

        std::vector<PipeInfo *> _masterPipeInfoList;      ///< list of pipe params for master node
        std::vector<WindowInfo *> _masterWindowInfoList;  ///< list of window params for master node
        std::vector<ChannelInfo *> _masterChannelInfoList;///< list of channel params for master node
        std::vector<ScreenInfo *> _masterScreenInfoList;  ///< list of screen params for master node
};

}
#endif
