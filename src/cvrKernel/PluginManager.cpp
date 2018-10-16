#include <cvrKernel/PluginManager.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrKernel/ComController.h>
#include <cvrConfig/ConfigManager.h>

#include <iostream>
#include <algorithm>
#include <sys/stat.h>

#ifndef WIN32
#include <dlfcn.h>
#include <unistd.h>
#else
#include <Windows.h>
#endif

#if __ANDROID__
#include <cvrUtil/AndroidPreloadPlugins.h>
#endif

using namespace cvr;

PluginManager * PluginManager::_myPtr = NULL;

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        delete _loadedPluginList[i]->ptr;
    }
}

PluginManager * PluginManager::instance()
{
    if(!_myPtr)
    {
        _myPtr = new PluginManager();
    }
    return _myPtr;
}

bool PluginManager::init(AAssetManager*const assetManager){
    std::vector<std::string> plugins;
    ConfigManager::getChildren("Plugin",plugins);

    for(int i = 0; i < plugins.size(); i++)
    {
        if(_pluginMap.find(plugins[i]) != _pluginMap.end())
        {
            continue;
        }
        _pluginMap[plugins[i]] = ConfigManager::getBool(
                std::string("Plugin") + "." + plugins[i], false);
    }

    // default nav buttons
    _pluginMap["MenuBasics"] = ConfigManager::getBool("Plugin.MenuBasics",true);

    for(std::map<std::string,bool>::iterator it = _pluginMap.begin();
        it != _pluginMap.end(); it++)
    {
        if(it->second)
        {
            CVRPlugin * pluginPtr = ClassFactory::getInstance(it->first);
            if(!pluginPtr) {
                it->second = false;
                continue;
            }
            pluginPtr->setAssetManager(assetManager);
            int priority = pluginPtr->getPriority();
            PluginInfo * pi = new PluginInfo;
            pi->priority = priority;
            pi->name = it->first;
            pi->ptr = pluginPtr;
            pi->path = "";

            _loadedPluginList.push_back(pi);
        }
    }

    std::sort(_loadedPluginList.begin(),_loadedPluginList.end(),PrioritySort());

    for(std::vector<PluginInfo *>::iterator it = _loadedPluginList.begin();
        it != _loadedPluginList.end();)
    {
        if(!(*it)->ptr->init())
        {
            _pluginMap[(*it)->name] = false;
            delete (*it);
            it = _loadedPluginList.erase(it);
            continue;
        }
        it++;
    }
    return true;
}

bool PluginManager::init()
{
    std::string pluginsHome = CalVR::instance()->getPluginsHomeDir();

    size_t position = 0;

    while(position < pluginsHome.size())
    {
        size_t lastPosition = position;
#ifndef WIN32
        position = pluginsHome.find_first_of(':',position);
#else
		position = pluginsHome.find_first_of(';', position);
#endif
        if(position == std::string::npos)
        {
            size_t length = pluginsHome.size() - lastPosition;
            if(length)
            {
                _pluginLibDirs.push_back(
                        pluginsHome.substr(lastPosition,length));
                break;
            }
        }
        else
        {
            size_t length = position - lastPosition;
            if(length)
            {
                _pluginLibDirs.push_back(
                        pluginsHome.substr(lastPosition,length));
            }
            position++;
        }
    }

    for(int i = 0; i < _pluginLibDirs.size(); ++i)
    {
#ifdef WIN32
        _pluginLibDirs[i] += "/bin/calvr-plugins/";
#else
        _pluginLibDirs[i] += "/lib/calvr-plugins/";
#endif
    }

    std::vector<std::string> plugins;
    ConfigManager::getChildren("Plugin",plugins);

    for(int i = 0; i < plugins.size(); i++)
    {
        if(_pluginMap.find(plugins[i]) != _pluginMap.end())
        {
            continue;
        }
        _pluginMap[plugins[i]] = ConfigManager::getBool(
                std::string("Plugin") + "." + plugins[i],false);
    }

    // default nav buttons
    _pluginMap["MenuBasics"] = ConfigManager::getBool("Plugin.MenuBasics",true);

    //std::cerr << "Plugins: " << std::endl;
    for(std::map<std::string,bool>::iterator it = _pluginMap.begin();
            it != _pluginMap.end(); it++)
    {
        //std::cerr << it->first << " " << it->second << std::endl;
        if(it->second)
        {
            it->second = loadPlugin(it->first);
        }
    }

    std::sort(_loadedPluginList.begin(),_loadedPluginList.end(),PrioritySort());

    std::cerr << "Loaded Plugins: " << std::endl;
    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        std::cerr << _loadedPluginList[i]->name << " Priority: "
                << _loadedPluginList[i]->priority << std::endl;
    }

    for(std::vector<PluginInfo *>::iterator it = _loadedPluginList.begin();
            it != _loadedPluginList.end();)
    {
        if(!(*it)->ptr->init())
        {
            _pluginMap[(*it)->name] = false;
            delete (*it);
            it = _loadedPluginList.erase(it);
            continue;
        }
        it++;
    }

    return true;
}

void PluginManager::preFrame()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    double startTime, endTime;

    osg::Stats * stats;
    stats = CVRViewer::instance()->getViewerStats();
    if(stats && !stats->collectStats("CalVRStats"))
    {
        stats = NULL;
    }

    if(stats)
    {
        startTime = osg::Timer::instance()->delta_s(
                CVRViewer::instance()->getStartTick(),
                osg::Timer::instance()->tick());
    }

    osg::Stats * statsPlugins;
    statsPlugins = CVRViewer::instance()->getViewerStats();
    if(statsPlugins && !statsPlugins->collectStats("CalVRStatsPlugins"))
    {
        statsPlugins = NULL;
    }

    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        double pluginsStartTime, pluginsEndTime;
        if(statsPlugins)
        {
            pluginsStartTime = osg::Timer::instance()->delta_s(
                    CVRViewer::instance()->getStartTick(),
                    osg::Timer::instance()->tick());
        }

        _loadedPluginList[i]->ptr->preFrame();

        if(statsPlugins)
        {
            pluginsEndTime = osg::Timer::instance()->delta_s(
                    CVRViewer::instance()->getStartTick(),
                    osg::Timer::instance()->tick());
            statsPlugins->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    _loadedPluginList[i]->name + " preFrame begin time",
                    pluginsStartTime);
            statsPlugins->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    _loadedPluginList[i]->name + " preFrame end time",
                    pluginsEndTime);
            statsPlugins->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    _loadedPluginList[i]->name + " preFrame time taken",
                    pluginsEndTime - pluginsStartTime);
        }
    }

    if(stats)
    {
        endTime = osg::Timer::instance()->delta_s(
                CVRViewer::instance()->getStartTick(),
                osg::Timer::instance()->tick());
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "PreFrame begin time",startTime);
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "PreFrame end time",endTime);
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "PreFrame time taken",endTime - startTime);
    }
}

void PluginManager::postFrame()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    double startTime, endTime;

    osg::Stats * stats;
    stats = CVRViewer::instance()->getViewerStats();
    if(stats && !stats->collectStats("CalVRStatsAdvanced"))
    {
        stats = NULL;
    }

    if(stats)
    {
        startTime = osg::Timer::instance()->delta_s(
                CVRViewer::instance()->getStartTick(),
                osg::Timer::instance()->tick());
    }

    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        _loadedPluginList[i]->ptr->postFrame();
    }

    if(stats)
    {
        endTime = osg::Timer::instance()->delta_s(
                CVRViewer::instance()->getStartTick(),
                osg::Timer::instance()->tick());
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "PostFrame begin time",startTime);
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "PostFrame end time",endTime);
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "PostFrame time taken",endTime - startTime);
    }
}

bool PluginManager::processEvent(InteractionEvent * event)
{
    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        if(_loadedPluginList[i]->ptr->processEvent(event))
        {
            return true;
        }
    }
    return false;
}

void PluginManager::sendMessageByName(std::string plugin, int type, char * data)
{
    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        if(_loadedPluginList[i]->name == plugin)
        {
            _loadedPluginList[i]->ptr->message(type,data,false);
            break;
        }
    }
}

bool PluginManager::getPluginLoaded(std::string plugin)
{
    if(_pluginMap.find(plugin) == _pluginMap.end())
    {
        return false;
    }

    return _pluginMap[plugin];
}

std::string PluginManager::getPluginName(CVRPlugin * plugin)
{
    for(int i = 0; i < _loadedPluginList.size(); ++i)
    {
        if(_loadedPluginList[i]->ptr == plugin)
        {
            return _loadedPluginList[i]->name;
        }
    }
    return "";
}

std::vector<std::string> PluginManager::getLoadedPluginList()
{
    std::vector<std::string> pluginList;

    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        pluginList.push_back(_loadedPluginList[i]->name);
    }

    return pluginList;
}

std::string PluginManager::getPathOfPlugin(std::string plugin_name)
{
    for(size_t i = 0; i < _loadedPluginList.size(); i++)
    {
        PluginInfo * pi = _loadedPluginList[i];
        if(pi && pi->name == plugin_name)
            return pi->path;
    }

    return "";
}

CVRPlugin * PluginManager::getPlugin(std::string plugin)
{
    CVRPlugin * ptr = NULL;
    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        if(_loadedPluginList[i]->name == plugin)
        {
            ptr = _loadedPluginList[i]->ptr;
            break;
        }
    }
    return ptr;
}

bool PluginManager::loadPlugin(std::string plugin)
{
    CVRPlugin * pluginPtr;
    CVRPlugin * (*func)();

    std::string libPath;

#ifdef WIN32
    libPath = plugin + ".dll";
#elif __APPLE__
    libPath = std::string("lib") + plugin + ".dylib";
#else
    libPath = std::string("lib") + plugin + ".so";
#endif

    for(int i = 0; i < _pluginLibDirs.size(); ++i)
    {
        struct stat sb;
        std::string testPath = _pluginLibDirs[i] + "/" + libPath;
        if(stat(testPath.c_str(),&sb) != -1)
        {
            libPath = testPath;
            break;
        }
    }

#ifndef WIN32
    char * error;
    void * libHandle;

    libHandle = dlopen(libPath.c_str(),RTLD_LAZY);
    if(!libHandle)
    {
        std::cerr << dlerror() << std::endl;
        return false;
    }

    func = (CVRPlugin * (*)())dlsym(libHandle, "createPlugin");if
(    (error = dlerror()) != NULL)
    {
        std::cerr << error << std::endl;
        return false;
    }
#else
    HINSTANCE libHandle;
    libHandle = LoadLibrary(libPath.c_str());
    if(!libHandle)
    {
        std::cerr << "Error: Unable to open DLL: " << libPath << std::endl;
        return false;
    }

    func = (CVRPlugin * (*)()) GetProcAddress(libHandle, "createPlugin");
    if(!func)
    {
        std::cerr << "Error: Unable to find function address in " << libPath << std::endl;
        return false;
    }
#endif

    pluginPtr = (*func)();
    int priority = pluginPtr->getPriority();

    PluginInfo * pi = new PluginInfo;
    pi->priority = priority;
    pi->name = plugin;
    pi->ptr = pluginPtr;
    pi->path = libPath;

    _loadedPluginList.push_back(pi);

    return true;
}
