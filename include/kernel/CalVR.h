#ifndef CALVR_MAIN_H
#define CALVR_MAIN_H

#include <osg/ArgumentParser>

#include <string>

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

class CalVR
{
    public:
        CalVR();
        ~CalVR();

        static CalVR * instance();

        bool init(osg::ArgumentParser & args, std::string home);

        void run();

        std::string & getHomeDir() { return _home; }

    protected:
        static CalVR * _myPtr;

        std::string _home;

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
};

}

#endif
