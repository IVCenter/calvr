#include <iostream>
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

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/ViewerEventHandlers>

using namespace std;

int main(int argc, char **argv)
{

    char * cvrDir = getenv("CALVR_HOME");
    if(!cvrDir)
    {
        std::cerr
                << "Error: CALVR_HOME environment variable not set.  Quitting."
                << std::endl;
        return 0;
    }

    cvr::ConfigManager * config = new cvr::ConfigManager();
    if(!config->init())
    {
        std::cerr << "Error loading config file." << std::endl;
        return 0;
    }

    // config class test
    /*std::string attribute;
     std::string path;
     std::string ret;
     bool found;
     while(1)
     {
     std::cerr << "XML Test attribute: ";
     getline(cin, attribute);
     std::cerr << "Path: ";
     getline(cin, path);

     ret = config->getEntry(attribute, path, "", &found);
     if(found)
     {
     std::cerr << "Found entry: " << ret << std::endl;
     }
     else
     {
     std::cerr << "Did not find entry." << std::endl;
     }
     }*/

    osg::ArgumentParser arguments(&argc, argv);

    cvr::ComController * communication = cvr::ComController::instance();
    if(!communication->init(&arguments))
    {
        std::cerr << "Error starting Communication Controller." << std::endl;
        return 0;
    }

    cvr::TrackingManager * tracking = cvr::TrackingManager::instance();
    tracking->init();

    cvr::InteractionManager * interaction = cvr::InteractionManager::instance();
    interaction->init();

    cvr::Navigation * nav = cvr::Navigation::instance();
    nav->init();

    // construct the viewer.
    cvr::CVRViewer * viewer = new cvr::CVRViewer();

    cvr::ScreenConfig * screens = cvr::ScreenConfig::instance();
    if(!screens->init())
    {
        std::cerr << "Error setting up screens." << std::endl;
        return 0;
    }

    cvr::SceneManager * scene = cvr::SceneManager::instance();
    if(!scene->init())
    {
        std::cerr << "Error setting up scene." << std::endl;
    }

    scene->setViewerScene(viewer);
    viewer->setReleaseContextAtEndOfFrameHint(false);
    //viewer.setReleaseContextAtEndOfFrameHint(true);

    cvr::CollaborativeManager * collab = cvr::CollaborativeManager::instance();
    collab->init();

    std::string commandLineFile;

    // TODO: do this better
    if(communication->isMaster())
    {
        if(arguments.argc() > 1)
        {
            commandLineFile = arguments.argv()[1];
        }
    }
    else
    {
        if(arguments.argc() > 7)
        {
            commandLineFile = arguments.argv()[7];
        }
    }

    osgViewer::StatsHandler * stats = new osgViewer::StatsHandler;
    stats->setKeyEventTogglesOnScreenStats((int)'S');
    stats->setKeyEventPrintsOutStats((int)'P');
    viewer->addEventHandler(stats);

    cvr::MenuManager * menu = cvr::MenuManager::instance();
    if(!menu->init())
    {
	std::cerr << "Error setting up menu systems." << std::endl;
    }

    cvr::FileHandler::instance();

    cvr::PluginManager * plugins = cvr::PluginManager::instance();
    plugins->init();

    if(!commandLineFile.empty())
    {
        cvr::FileHandler::instance()->loadFile(commandLineFile);
    }


    if(!viewer->isRealized())
    {
        viewer->realize();
    }

    int frameNum = 0;

    while(!viewer->done())
    {
        //std::cerr << "Frame " << frameNum << std::endl;
        viewer->frameStart();
	viewer->advance(USE_REFERENCE_TIME);
        viewer->eventTraversal();
        tracking->update();
        scene->update();
        menu->update();
        interaction->update();
        nav->update();
        screens->computeViewProj();
        screens->updateCamera();
        plugins->preFrame();
	// should these be after preFrame?
        collab->update();
        viewer->updateTraversal();
        viewer->renderingTraversals();

	if(communication->getIsSyncError())
	{
	    std::cerr << "Sync Error Exit." << std::endl;
	    break;
	}

        plugins->postFrame();

        frameNum++;
    }
    delete plugins;
    delete viewer;
    //delete plugins;

    std::cerr << "Main done." << std::endl;

    return 1;
}
