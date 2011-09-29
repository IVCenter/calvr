#include <kernel/PluginManager.h>
#include <kernel/InteractionManager.h>
#include <kernel/ComController.h>
#include <config/ConfigManager.h>

#include <iostream>
#include <algorithm>

#ifndef WIN32
#include <dlfcn.h>
#else
#include <Windows.h>
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

bool PluginManager::init()
{
    _pluginLibDir = CalVR::instance()->getHomeDir();

#ifdef WIN32
    _pluginLibDir += "/bin/plugins/";
#else
    _pluginLibDir += "/lib/plugins/";
#endif

    std::vector<std::string> plugins;
    ConfigManager::getChildren("Plugin", plugins);

    for(int i = 0; i < plugins.size(); i++)
    {
        if(_pluginMap.find(plugins[i]) != _pluginMap.end())
        {
            continue;
        }
        _pluginMap[plugins[i]] = ConfigManager::getBool(std::string("Plugin")
                + "." + plugins[i], false);
    }

    // default nav buttons
    _pluginMap["MenuBasics"]
            = ConfigManager::getBool("Plugin.MenuBasics", true);

    //std::cerr << "Plugins: " << std::endl;
    for(std::map<std::string,bool>::iterator it = _pluginMap.begin(); it
            != _pluginMap.end(); it++)
    {
        //std::cerr << it->first << " " << it->second << std::endl;
        if(it->second)
        {
            it->second = loadPlugin(it->first);
        }
    }

    std::sort(_loadedPluginList.begin(), _loadedPluginList.end(),
              PrioritySort());

    std::cerr << "Loaded Plugins: " << std::endl;
    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        std::cerr << _loadedPluginList[i]->name << " Priority: "
                << _loadedPluginList[i]->priority << std::endl;
    }

    for(std::vector<PluginInfo *>::iterator it = _loadedPluginList.begin(); it
            != _loadedPluginList.end();)
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

    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        _loadedPluginList[i]->ptr->preFrame();
    }
}

void PluginManager::postFrame()
{
    if(ComController::instance()->getIsSyncError())
    {
	return;
    }

    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
        _loadedPluginList[i]->ptr->postFrame();
    }
}

bool PluginManager::processEvent(InteractionEvent * event)
{
    for(int i = 0; i < _loadedPluginList.size(); i++)
    {
	return _loadedPluginList[i]->ptr->processEvent(event);
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

#ifndef WIN32
    char * error;
    void * libHandle;
#ifdef __APPLE__
    std::string libPath = _pluginLibDir + "lib" + plugin + ".dylib";
#else
    std::string libPath = _pluginLibDir + "lib" + plugin + ".so";
#endif
    libHandle = dlopen(libPath.c_str(), RTLD_LAZY);
    if(!libHandle)
    {
        std::cerr << dlerror() << std::endl;
        return false;
    }

    func = (CVRPlugin * (*)())dlsym(libHandle, "createPlugin");
    if((error = dlerror()) != NULL)
    {
        std::cerr << error << std::endl;
        return false;
    }
#else
    HINSTANCE libHandle;
    std::string libPath = _pluginLibDir + plugin + ".dll";
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

    _loadedPluginList.push_back(pi);

    return true;
}
