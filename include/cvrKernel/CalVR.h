/**
 * @file CalVR.h
 */

#ifndef CALVR_MAIN_H
#define CALVR_MAIN_H

#include <cvrKernel/Export.h>
#include <cvrKernel/InteractionEvent.h>
#include <osg/ArgumentParser>
#include <osg/Group>
#include <string>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#endif

namespace cvr
{
class ConfigManager;
class ComController;
class TrackingManager;
class InteractionManager;
class Navigation;
class CVRViewer;
class ScreenConfig;
class SceneManager;
class CollaborativeManager;
class MenuManager;
class FileHandler;
class PluginManager;
class ThreadedLoader;
class assetLoader;
class ARCoreManager;

/**
 * @addtogroup kernel cvrKernel
 * @{
 */

/**
 * @brief Main class for the CalVR framework.  Sets up all other core classes 
 * and contains the main run loop.
 */
class CVRKERNEL_EXPORT CalVR
{
    public:
        CalVR();

        virtual ~CalVR();

        /**
         * @brief get static self pointer
         */
        static CalVR * instance();

        /**
         * @brief does main framework init
         */
        bool init(osg::ArgumentParser & args, std::string home);

        /**
         * @brief main run loop, does not return util quit is signaled
         */
        void run();

        /**
         * @brief returns the set home directory for the CalVR install
         */
        const std::string & getHomeDir()
        {
            return _homeDir;
        }

        const std::string & getConfigDir()
        {
            return _configDir;
        }

        const std::string & getResourceDir()
        {
            return _resourceDir;
        }

        const std::string & getPluginsHomeDir()
        {
            return _pluginsHomeDir;
        }

        /**
         * @brief Get the host name for this node
         */
        std::string & getHostName()
        {
            return _hostName;
        }
#ifdef __ANDROID__
        /**
            * @brief Initialization For Android
            */
        bool init(const char* home, AAssetManager *assetManager);

        /**
        * @brief Run Loop, do update per frame
        */
        void frame();
        void onViewChanged(int rot, int width, int height);
        void onPause();
        void onResume(void *env, void *context, void *activity);
        osg::ref_ptr<osg::Group> getSceneRoot();
        void setSceneData(osg::ref_ptr<osg::Group> root);
        void setMouseEvent(cvr::MouseInteractionEvent * mie,
                           int pointer_num, float x, float y);

#endif
    protected:
        static CalVR * _myPtr; ///< static self pointer

        bool setupDirectories();
        bool syncClusterInitStatus();

        enum CVRInitStatus
        {
            INIT_OK = 0, SCREEN_INIT_ERROR, SCENE_INIT_ERROR, MENU_INIT_ERROR
        };

        CVRInitStatus _initStatus;

        std::string _homeDir; ///< CalVR home directory
        std::string _resourceDir;
        std::string _configDir;
        std::string _pluginsHomeDir;
        std::string _hostName; ///< node's host name

        ConfigManager * _config;
        ComController * _communication;
        TrackingManager * _tracking;
        InteractionManager * _interaction;
        Navigation * _navigation;
        CVRViewer * _viewer;
        ScreenConfig * _screens;
        SceneManager * _scene;
        CollaborativeManager * _collaborative;
        MenuManager * _menu;
        FileHandler * _file;
        PluginManager * _plugins;
        ThreadedLoader * _threadedLoader;

        assetLoader * _assetLoader;
        ARCoreManager * _arcore;
};

/**
 * @}
 */

}

#endif
