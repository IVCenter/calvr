#include <kernel/CalVR.h>

#include <config/ConfigManager.h>
#include <input/TrackingManager.h>
#include <menu/MenuManager.h>
#include <collaborative/CollaborativeManager.h>
#include <kernel/ScreenConfig.h>
#include <kernel/ComController.h>
#include <kernel/CVRViewer.h>
#include <kernel/SceneManager.h>
#include <kernel/FileHandler.h>
#include <kernel/PluginManager.h>
#include <kernel/InteractionManager.h>
#include <kernel/Navigation.h>
#include <kernel/ThreadedLoader.h>

#include <osgViewer/ViewerEventHandlers>

#include <iostream>

using namespace cvr;

CalVR * CalVR::_myPtr = NULL;

CalVR::CalVR()
{
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
    _home = home;

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
        return false;
    }

    _screens->syncMasterScreens();

    _scene = cvr::SceneManager::instance();
    if(!_scene->init())
    {
        std::cerr << "Error setting up scene." << std::endl;
	return false;
    }

    _scene->setViewerScene(_viewer);
    _viewer->setReleaseContextAtEndOfFrameHint(false);
    //_viewer.setReleaseContextAtEndOfFrameHint(true);

    _collaborative = cvr::CollaborativeManager::instance();
    _collaborative->init();

    _threadedLoader = cvr::ThreadedLoader::instance();

    std::string commandLineFile;

    // TODO: do this better
    if(_communication->isMaster())
    {
        if(args.argc() > 1)
        {
            commandLineFile = args.argv()[1];
        }
    }
    else
    {
        if(args.argc() > 7)
        {
            commandLineFile = args.argv()[7];
        }
    }

    osgViewer::StatsHandler * stats = new osgViewer::StatsHandler;
    stats->setKeyEventTogglesOnScreenStats((int)'S');
    stats->setKeyEventPrintsOutStats((int)'P');
    _viewer->addEventHandler(stats);

    _menu = cvr::MenuManager::instance();
    if(!_menu->init())
    {
	std::cerr << "Error setting up menu systems." << std::endl;
	return false;
    }

    _file = cvr::FileHandler::instance();

    _plugins = cvr::PluginManager::instance();
    _plugins->init();

    if(!commandLineFile.empty())
    {
        cvr::FileHandler::instance()->loadFile(commandLineFile);
    }

    return true;
}

void CalVR::run()
{
    if(!_viewer->isRealized())
    {
        _viewer->realize();
    }

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
