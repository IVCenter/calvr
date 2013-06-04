#include <cvrKernel/CalVR.h>

#include <cvrConfig/ConfigManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrMenu/MenuManager.h>
#include <cvrCollaborative/CollaborativeManager.h>
#include <cvrKernel/ScreenConfig.h>
#include <cvrKernel/ComController.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/SceneManager.h>
#include <cvrKernel/FileHandler.h>
#include <cvrKernel/PluginManager.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrKernel/Navigation.h>
#include <cvrKernel/ThreadedLoader.h>
#include <cvrKernel/CVRStatsHandler.h>

#include <osgViewer/ViewerEventHandlers>

#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <Winsock2.h>
#include <stdlib.h>
#pragma comment(lib, "wsock32.lib")
#endif

using namespace cvr;

CalVR * CalVR::_myPtr = NULL;

CalVR::CalVR()
{
    _initStatus = INIT_OK;

    _config = NULL;
    _communication = NULL;
    _tracking = NULL;
    _interaction = NULL;
    _navigation = NULL;
    _viewer = NULL;
    _screens = NULL;
    _scene = NULL;
    _collaborative = NULL;
    _menu = NULL;
    _file = NULL;
    _plugins = NULL;
    _threadedLoader = NULL;
    _myPtr = this;
}

CalVR::~CalVR()
{
    if(_plugins)
    {
        delete _plugins;
    }
    if(_file)
    {
        delete _file;
    }
    if(_menu)
    {
        delete _menu;
    }
    if(_threadedLoader)
    {
        delete _threadedLoader;
    }
    if(_collaborative)
    {
        delete _collaborative;
    }
    if(_scene)
    {
        delete _scene;
    }
    if(_screens)
    {
        delete _screens;
    }
    if(_viewer)
    {
        delete _viewer;
    }
    if(_navigation)
    {
        delete _navigation;
    }
    if(_interaction)
    {
        delete _interaction;
    }
    if(_tracking)
    {
        delete _tracking;
    }
    if(_communication)
    {
        delete _communication;
    }
    if(_config)
    {
        delete _config;
    }
}

CalVR * CalVR::instance()
{
    return _myPtr;
}

bool CalVR::init(osg::ArgumentParser & args, std::string home)
{
    _homeDir = home;
    
    if(!setupDirectories())
    {
	std::cerr << "Error: Failure to find needed directory paths." << std::endl;
	return false;
    }
    
    if(!args.read("--host-name",_hostName))
    {
	char * chostname = getenv("CALVR_HOST_NAME");
	if(chostname)
	{
	    _hostName = chostname;
	}
	else
	{
	    char hostname[512];
	    gethostname(hostname,511);
	    _hostName = hostname;
	}
    }

    std::cerr << "HostName: " << _hostName << std::endl;

    _config = new cvr::ConfigManager();
    if(!_config->init())
    {
        std::cerr << "Error loading config file." << std::endl;
        return false;
    }

    _communication = cvr::ComController::instance();
    if(!_communication->init(&args))
    {
        std::cerr << "Error starting Communication Controller." << std::endl;
        return false;
    }

    // distribute files listed on command line
    std::vector<std::string> fileList;
    int files, size;
    if(_communication->isMaster())
    {
        for(int i = 1; i < args.argc(); i++)
        {
            fileList.push_back(args.argv()[i]);
        }
        files = fileList.size();
        _communication->sendSlaves(&files,sizeof(int));
        for(int i = 0; i < files; i++)
        {
            size = fileList[i].length() + 1;
            _communication->sendSlaves(&size,sizeof(int));
            _communication->sendSlaves((void*)fileList[i].c_str(),size);
        }
    }
    else
    {
        _communication->readMaster(&files,sizeof(int));
        char * temp;
        for(int i = 0; i < files; i++)
        {
            _communication->readMaster(&size,sizeof(int));
            temp = new char[size];
            _communication->readMaster(temp,size);
            fileList.push_back(temp);
            delete[] temp;
        }
    }

    _tracking = cvr::TrackingManager::instance();
    _tracking->init();

    _interaction = cvr::InteractionManager::instance();
    _interaction->init();

    _navigation = cvr::Navigation::instance();
    _navigation->init();

    // construct the viewer.
    _viewer = new cvr::CVRViewer();

    _screens = cvr::ScreenConfig::instance();
    if(!_screens->init())
    {
        std::cerr << "Error setting up screens." << std::endl;
	_initStatus = SCREEN_INIT_ERROR;
	syncClusterInitStatus();
        return false;
    }

    _scene = cvr::SceneManager::instance();
    if(!_scene->init())
    {
        std::cerr << "Error setting up scene." << std::endl;
	_initStatus = SCENE_INIT_ERROR;
	syncClusterInitStatus();
        return false;
    }

    _scene->setViewerScene(_viewer);
    //TODO: set this value based on threading model and window pipe mapping
    _viewer->setReleaseContextAtEndOfFrameHint(false);
    //_viewer.setReleaseContextAtEndOfFrameHint(true);

    _collaborative = cvr::CollaborativeManager::instance();
    _collaborative->init();

    _threadedLoader = cvr::ThreadedLoader::instance();

    _menu = cvr::MenuManager::instance();
    if(!_menu->init())
    {
        std::cerr << "Error setting up menu systems." << std::endl;
	_initStatus = MENU_INIT_ERROR;
	syncClusterInitStatus();
        return false;
    }

    _file = cvr::FileHandler::instance();

    _plugins = cvr::PluginManager::instance();
    _plugins->init();

    for(int i = 0; i < fileList.size(); i++)
    {
        cvr::FileHandler::instance()->loadFile(fileList[i]);
    }

    return syncClusterInitStatus();
}

void CalVR::run()
{
    if(!_viewer->isRealized())
    {
        _viewer->realize();
    }

    _screens->syncMasterScreens();

    int frameNum = 0;

    while(!_viewer->done())
    {
        //std::cerr << "Frame " << frameNum << std::endl;
        _viewer->frameStart();
        _viewer->advance(USE_REFERENCE_TIME);
        _viewer->eventTraversal();
        _tracking->update();
        _scene->update();
        _menu->update();
        _interaction->update();
        _navigation->update();
        _scene->postEventUpdate();
        _screens->computeViewProj();
        _screens->updateCamera();
        _collaborative->update();
        _threadedLoader->update();
        _plugins->preFrame();
        _viewer->updateTraversal();
        _viewer->renderingTraversals();

        if(_communication->getIsSyncError())
        {
            std::cerr << "Sync Error Exit." << std::endl;
            break;
        }

        _plugins->postFrame();

        frameNum++;
    }
}

bool CalVR::setupDirectories()
{
    char * env;

    env = getenv("CALVR_CONFIG_DIR");
    if(env)
    {
	_configDir = env;
    }
    else
    {
	// look for default config dir
	struct stat sb;
	std::string path = _homeDir + "/config";
	std::string testFile = path + "/config.xml";

	if(stat(testFile.c_str(), &sb) == -1)
	{
	    path = _homeDir + "/share/calvr/config";
	    testFile = path + "/config.xml";

	    if(stat(testFile.c_str(), &sb) == -1)
	    {
		std::cerr << "Error: No valid config directory found.  Checked: " << _homeDir << "/config , " << _homeDir << "/share/calvr/config" << std::endl;
		std::cerr << "Correct or manually set $CALVR_CONFIG_DIR." << std::endl;
		return false;
	    }
	    else
	    {
		_configDir = path;
	    }
	}
	else
	{
	    _configDir = path;
	}
    }

    std::cerr << "Config Directory: " << _configDir << std::endl;

    env = getenv("CALVR_RESOURCE_DIR");
    if(env)
    {
	_resourceDir = env;
    }
    else
    {
	struct stat sb;
	std::string path = _homeDir;
	std::string testFile = path + "/icons/arrow-left.rgb";

	if(stat(testFile.c_str(), &sb) == -1)
	{
	    path = _homeDir + "/share/calvr";
	    testFile = path + "/icons/arrow-left.rgb";

	    if(stat(testFile.c_str(), &sb) == -1)
	    {
		std::cerr << "Error: No calvr resource directory found. Checked: " << _homeDir << " , " << path << std::endl;
		std::cerr << "Correct or manually set $CALVR_RESOURCE_DIR." << std::endl;
		return false;
	    }
	    else
	    {
		_resourceDir = path;
	    }
	}
	else
	{
	    _resourceDir = path;
	}
    }

    std::cerr << "Resource Directory: " << _resourceDir << std::endl;

    env = getenv("CALVR_PLUGINS_HOME");
    if(env)
    {
	_pluginsHomeDir = env;
    }
    else
    {
	_pluginsHomeDir = _homeDir;
    }

    std::cerr << "Plugins Home Directory: " << _pluginsHomeDir << std::endl;

    return true;
}

bool CalVR::syncClusterInitStatus()
{
    if(_communication->getIsSyncError())
    {
	return false;
    }

    if(!_communication->getNumSlaves())
    {
	if(_initStatus == INIT_OK)
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }

    bool initOK = (_initStatus == INIT_OK);
    if(_communication->isMaster())
    {
	CVRInitStatus * is = new CVRInitStatus[_communication->getNumSlaves()];
	if(_communication->readSlaves(is,sizeof(CVRInitStatus)))
	{
	    for(int i = 0; i < _communication->getNumSlaves(); ++i)
	    {
		std::stringstream nodeNamess;
		nodeNamess << "Node " << i << ": ";
		switch(is[i])
		{
		    case INIT_OK:
		    {
			break;
		    }
		    case SCREEN_INIT_ERROR:
		    {
			std::cerr << nodeNamess.str() << "Error setting up screens." << std::endl;
			initOK = false;
			break;
		    }
		    case SCENE_INIT_ERROR:
		    {
			std::cerr << nodeNamess.str() << "Error during scene init." << std::endl;
			initOK = false;
			break;
		    }
		    case MENU_INIT_ERROR:
		    {
			std::cerr << nodeNamess.str() << "Error setting up menu." << std::endl;
			initOK = false;
			break;
		    }
		    default:
		    {
			std::cerr << nodeNamess.str() << "Unknown init error." << std::endl;
			initOK = false;
			break;
		    }
		}
	    }
	}
	else
	{
	    std::cerr << "Error reading cluster init status." << std::endl;
	    return false;
	}
	_communication->sendSlaves(&initOK,sizeof(bool));
    }
    else
    {
	if(!_communication->sendMaster(&_initStatus,sizeof(CVRInitStatus)))
	{
	    std::cerr << "Error sending init status to master." << std::endl;
	    return false;
	}
	_communication->readMaster(&initOK,sizeof(bool));
    }

    return initOK;
}
