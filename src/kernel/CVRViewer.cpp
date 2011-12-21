#include <kernel/CVRViewer.h>
#include <kernel/ComController.h>
#include <kernel/ScreenConfig.h>
#include <kernel/InteractionManager.h>
#include <kernel/SceneManager.h>
#include <kernel/NodeMask.h>
#include <kernel/ScreenBase.h>
#include <kernel/CVRCullVisitor.h>
#include <config/ConfigManager.h>

#include <osg/Version>
#include <osgDB/Registry>
#include <osgUtil/Statistics>
#include <osgViewer/Renderer>

#include <vector>
#include <iostream>

using namespace cvr;

CVRViewer * CVRViewer::_myPtr = NULL;

struct finishOperation : public osg::Operation
{
        finishOperation() :
                osg::Operation("finishOperation",true)
        {
        }

        virtual void operator ()(osg::Object* object)
        {
            //std::cerr << "Calling finish." << std::endl;
            glFinish();
        }
};

struct syncOperation : public osg::Operation
{
        syncOperation() :
                osg::Operation("syncOperation",true)
        {
        }

        virtual void operator ()(osg::Object* object)
        {
            //std::cerr << "Calling Sync." << std::endl;
            ComController::instance()->sync();
        }
};

struct vSyncOperation : public osg::Operation
{
        vSyncOperation(bool value) :
                osg::Operation("vSyncOperation",true)
        {
            _val = value;
        }

        virtual void operator ()(osg::Object* object)
        {
            if(_val)
            {
                putenv("__GL_SYNC_TO_VBLANK=1");
            }
            else
            {
                putenv("__GL_SYNC_TO_VBLANK=0");
            }
        }
        bool _val;
};

/*struct sleepOperation : public osg::Operation
 {
 sleepOperation(long time) : osg::Operation("sleepOperation",true)
 {
 _time = time;
 }

 virtual void operator () (osg::Object* object)
 {
 std::cerr << "Thread sleeping." << std::endl;
 struct timespec ts;
 ts.tv_sec = _time;
 ts.tv_nsec = 0;
 nanosleep(&ts,NULL);
 std::cerr << "Done sleeping." << std::endl;

 }

 long _time;
 };*/

struct printOperation : public osg::Operation
{
        printOperation(std::string text) :
                osg::Operation("sleepOperation",true)
        {
            _output = text;
        }

        virtual void operator ()(osg::Object* object)
        {
            std::cerr << _output << std::endl;

        }

        std::string _output;
};

CVRViewer::CVRViewer() :
        osgViewer::Viewer()
{
    std::string threadModel = ConfigManager::getEntry("value","MultiThreaded",
            "SingleThreaded");
    if(threadModel == "CullThreadPerCameraDrawThreadPerContext")
    {
        osg::Referenced::setThreadSafeReferenceCounting(true);
        setThreadingModel(CullThreadPerCameraDrawThreadPerContext);
    }
    else if(threadModel == "CullDrawThreadPerContext")
    {
        osg::Referenced::setThreadSafeReferenceCounting(true);
        setThreadingModel(CullDrawThreadPerContext);
    }
    else if(threadModel == "DrawThreadPerContext")
    {
        osg::Referenced::setThreadSafeReferenceCounting(true);
        setThreadingModel(DrawThreadPerContext);
    }
    else
    {
        setThreadingModel(SingleThreaded);
    }

    _renderOnMaster = ConfigManager::getBool("RenderOnMaster",true);

    if(ComController::instance()->isMaster())
    {
        _programStartTime = osg::Timer::instance()->tick();
        ComController::instance()->sendSlaves(&_programStartTime,
                sizeof(osg::Timer_t));
    }
    else
    {
        ComController::instance()->readMaster(&_programStartTime,
                sizeof(osg::Timer_t));
    }

    std::string cmode = ConfigManager::getEntry("value","CullingMode","CALVR");
    if(cmode == "CALVR")
    {
        _cullMode = CALVR;
    }
    else
    {
        _cullMode = DEFAULT;
    }

    _frameStartTime = _lastFrameStartTime = _programStartTime;

    // TODO: read from config file
    _doubleClickTimeout = 0.4;

    _updateList.push_back(new DefaultUpdate);

    _invertMouseY = false;

    _myPtr = this;
    _activeMasterScreen = -1;
}

CVRViewer::CVRViewer(osg::ArgumentParser& arguments) :
        osgViewer::Viewer(arguments)
{
    _myPtr = this;
    _activeMasterScreen = -1;
}

CVRViewer::CVRViewer(const CVRViewer& viewer, const osg::CopyOp& copyop) :
        osgViewer::Viewer(viewer,copyop)
{
    _myPtr = this;
    _activeMasterScreen = -1;
}

CVRViewer * CVRViewer::instance()
{
    return _myPtr;
}

CVRViewer::~CVRViewer()
{
}

void CVRViewer::updateTraversal()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    for(std::list<UpdateTraversal*>::iterator it = _updateList.begin();
            it != _updateList.end(); it++)
    {
        (*it)->update();
    }
}

void DefaultUpdate::update()
{
    CVRViewer::instance()->defaultUpdateTraversal();
}

void CVRViewer::defaultUpdateTraversal()
{
    //std::cerr << "Default update called." << std::endl;
    if(_done)
        return;

    double beginUpdateTraversal = osg::Timer::instance()->delta_s(_startTick,
            osg::Timer::instance()->tick());

    _updateVisitor->reset();
    _updateVisitor->setFrameStamp(getFrameStamp());
    _updateVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());

#if (OPENSCENEGRAPH_MAJOR_VERSION == 2) && (OPENSCENEGRAPH_MINOR_VERSION == 8)
    if(getSceneData())
    {
        _updateVisitor->setImageRequestHandler(_scene->getImagePager());
        getSceneData()->accept(*_updateVisitor);
    }

    if(_scene->getDatabasePager())
    {
        // synchronize changes required by the DatabasePager thread to the scene graph
        _scene->getDatabasePager()->updateSceneGraph(*_frameStamp);
    }

    if(_scene->getImagePager())
    {
        // synchronize changes required by the DatabasePager thread to the scene graph
        _scene->getImagePager()->updateSceneGraph(*_frameStamp);
    }

    if(_updateOperations.valid())
    {
        _updateOperations->runOperations(this);
    }

#elif (OPENSCENEGRAPH_MAJOR_VERSION == 2) && (OPENSCENEGRAPH_MINOR_VERSION == 9) && (OPENSCENEGRAPH_PATCH_VERSION <= 11)
    _scene->updateSceneGraph(*_updateVisitor);

    // if we have a shared state manager prune any unused entries
    if(osgDB::Registry::instance()->getSharedStateManager())
    osgDB::Registry::instance()->getSharedStateManager()->prune();

    // update the Registry object cache.
    osgDB::Registry::instance()->updateTimeStampOfObjectsInCacheWithExternalReferences(
            *getFrameStamp());
    osgDB::Registry::instance()->removeExpiredObjectsInCache(*getFrameStamp());

    if(_updateOperations.valid())
    {
        _updateOperations->runOperations(this);
    }

    if(_incrementalCompileOperation.valid())
    {
        // merge subgraphs that have been compiled by the incremental compiler operation.
        _incrementalCompileOperation->mergeCompiledSubgraphs();
    }

#else
    _scene->updateSceneGraph(*_updateVisitor);

    // if we have a shared state manager prune any unused entries
    if(osgDB::Registry::instance()->getSharedStateManager())
        osgDB::Registry::instance()->getSharedStateManager()->prune();

    // update the Registry object cache.
    osgDB::Registry::instance()->updateTimeStampOfObjectsInCacheWithExternalReferences(
            *getFrameStamp());
    osgDB::Registry::instance()->removeExpiredObjectsInCache(*getFrameStamp());

    if(_updateOperations.valid())
    {
        _updateOperations->runOperations(this);
    }

    if(_incrementalCompileOperation.valid())
    {
        // merge subgraphs that have been compiled by the incremental compiler operation.
        _incrementalCompileOperation->mergeCompiledSubgraphs(getFrameStamp());
    }
#endif

    {
        // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
        // leave that to the scene update traversal.
        osg::NodeVisitor::TraversalMode tm = _updateVisitor->getTraversalMode();
        _updateVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

        if(_camera.valid() && _camera->getUpdateCallback())
            _camera->accept(*_updateVisitor);

        for(unsigned int i = 0; i < getNumSlaves(); ++i)
        {
            osg::Camera* camera = getSlave(i)._camera.get();
            if(camera && camera->getUpdateCallback())
                camera->accept(*_updateVisitor);
        }

        _updateVisitor->setTraversalMode(tm);
    }

    // cvr - section removed, the rest is default osg code
    /*if (_cameraManipulator.valid())
     {
     setFusionDistance( getCameraManipulator()->getFusionDistanceMode(),
     getCameraManipulator()->getFusionDistanceValue() );

     _camera->setViewMatrix(_cameraManipulator->getInverseMatrix());
     }

     updateSlaves();*/

    if(getViewerStats() && getViewerStats()->collectStats("update"))
    {
        double endUpdateTraversal = osg::Timer::instance()->delta_s(_startTick,
                osg::Timer::instance()->tick());

        // update current frames stats
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(),
                "Update traversal begin time",beginUpdateTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(),
                "Update traversal end time",endUpdateTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(),
                "Update traversal time taken",
                endUpdateTraversal - beginUpdateTraversal);
    }
}

void CVRViewer::eventTraversal()
{
    if(_done || ComController::instance()->getIsSyncError())
        return;

    double beginEventTraversal = osg::Timer::instance()->delta_s(_startTick,
            osg::Timer::instance()->tick());

    eventInfo ei;
    ei.viewportX = 0;
    ei.viewportY = 0;
    event * events;

    if(ComController::instance()->isMaster())
    {
        std::vector<struct event> eventList;

        Contexts contexts;
        getContexts(contexts);

        for(Contexts::iterator citr = contexts.begin(); citr != contexts.end();
                ++citr)
        {
            event evnt;
            osgViewer::GraphicsWindow* gw =
                    dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
            if(gw)
            {
                gw->checkEvents();

                osgGA::EventQueue::Events gw_events;
                gw->getEventQueue()->takeEvents(gw_events);

                osgGA::EventQueue::Events::iterator itr;
                for(itr = gw_events.begin(); itr != gw_events.end(); ++itr)
                {
                    osgGA::GUIEventAdapter* event = itr->get();

                    float x = event->getX();
                    float y = event->getY();

                    switch(event->getEventType())
                    {
                        case (osgGA::GUIEventAdapter::PUSH):
                        case (osgGA::GUIEventAdapter::RELEASE):
                        case (osgGA::GUIEventAdapter::DOUBLECLICK):
                        case (osgGA::GUIEventAdapter::DRAG):
                        case (osgGA::GUIEventAdapter::MOVE):
                        {
                            if(event->getEventType()
                                    != osgGA::GUIEventAdapter::DRAG
                                    || !getCameraWithFocus())
                            {
                                osg::GraphicsContext::Cameras& cameras =
                                        gw->getCameras();
                                for(osg::GraphicsContext::Cameras::iterator citr =
                                        cameras.begin(); citr != cameras.end();
                                        ++citr)
                                {
                                    osg::Camera* camera = *citr;
                                    if(camera->getView() == this
                                            && camera->getAllowEventFocus()
                                            //&& camera->getRenderTargetImplementation()
                                            //        == osg::Camera::FRAME_BUFFER
                                            )
                                    {
                                        osg::Viewport* viewport =
                                                camera ?
                                                        camera->getViewport() :
                                                        0;
                                        if(viewport && x >= viewport->x()
                                                && y >= viewport->y()
                                                && x
                                                        <= (viewport->x()
                                                                + viewport->width())
                                                && y
                                                        <= (viewport->y()
                                                                + viewport->height()))
                                        {
                                            int screenNum =
                                                    ScreenConfig::instance()->findScreenNumber(
                                                            camera);
                                            if(screenNum != _activeMasterScreen)
                                            {
                                                struct event ams;
                                                ams.eventType =
                                                        UPDATE_ACTIVE_SCREEN;
                                                ams.param1 = screenNum;
                                                eventList.push_back(ams);
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }

                    if(event->getEventType() == osgGA::GUIEventAdapter::PUSH)
                    {
                        if(event->getTime() - _lastClickTime[event->getButton()]
                                < _doubleClickTimeout)
                        {
                            event->setEventType(
                                    osgGA::GUIEventAdapter::DOUBLECLICK);
                            _lastClickTime[event->getButton()] = 0.0;
                        }
                        else
                        {
                            _lastClickTime[event->getButton()] =
                                    event->getTime();
                        }
                    }

                    evnt.eventType = event->getEventType();

                    switch(event->getEventType())
                    {
                        case (osgGA::GUIEventAdapter::PUSH):
                        case (osgGA::GUIEventAdapter::RELEASE):
                            evnt.param1 = event->getButtonMask();
                            eventList.push_back(evnt);
                            break;
                        case (osgGA::GUIEventAdapter::DOUBLECLICK):
                            evnt.param1 = event->getButton();
                            eventList.push_back(evnt);
                            //std::cerr << "Got mouse doubleclick for button " << evnt.param1 << std::endl;
                            break;
                        case (osgGA::GUIEventAdapter::DRAG):
                        case (osgGA::GUIEventAdapter::MOVE):
                        {
                            ScreenInfo * si =
                                    ScreenConfig::instance()->getMasterScreenInfo(
                                            _activeMasterScreen);
                            if(!si)
                            {
                                break;
                            }
                            evnt.param1 = (int)(event->getX()
                                    - si->myChannel->left);
                            evnt.param2 = (int)(event->getY()
                                    - si->myChannel->bottom);

                            //std::cerr << "yparam: " << evnt.param2 << " windowHeight: " << event->getWindowHeight() << std::endl;
                            if(!_invertMouseY)
                            {
                                evnt.param2 = -evnt.param2
                                        + (int)si->myChannel->height;
                            }
                            if(ScreenConfig::instance()->getScreen(
                                    _activeMasterScreen))
                            {
                                ScreenConfig::instance()->getScreen(
                                        _activeMasterScreen)->adjustViewportCoords(
                                        evnt.param1,evnt.param2);
                            }
                            eventList.push_back(evnt);
                            break;
                        }
                        case (osgGA::GUIEventAdapter::KEYDOWN):
                        case (osgGA::GUIEventAdapter::KEYUP):
                            evnt.param1 = event->getKey();
                            evnt.param2 = event->getModKeyMask();
                            eventList.push_back(evnt);
                            break;
                        case (osgGA::GUIEventAdapter::QUIT_APPLICATION):
                        case (osgGA::GUIEventAdapter::CLOSE_WINDOW):
                            eventList.push_back(evnt);
                            break;
                        case (osgGA::GUIEventAdapter::RESIZE):
                        {
                            osg::GraphicsContext::Cameras& cameras =
                                    gw->getCameras();
                            for(osg::GraphicsContext::Cameras::iterator citr =
                                    cameras.begin(); citr != cameras.end();
                                    ++citr)
                            {
                                osg::Camera* camera = *citr;
                                if(camera->getView() == this
                                        && camera->getAllowEventFocus()
                                        //&& camera->getRenderTargetImplementation()
                                        //== osg::Camera::FRAME_BUFFER
                                        )
                                {
                                    osg::Viewport* viewport =
                                            camera ? camera->getViewport() : 0;
                                    if(viewport)
                                    {
                                        //std::cerr << "Cam viewport x: " << viewport->x() << " y: " << viewport->y() << " width: " << viewport->width() << " height: " << viewport->height() << std::endl;
                                        int screenNum =
                                                ScreenConfig::instance()->findScreenNumber(
                                                        camera);
                                        if(screenNum != -1)
                                        {
                                            struct event vpevent;
                                            vpevent.eventType = UPDATE_VIEWPORT;
                                            vpevent.param1 = screenNum;
                                            eventList.push_back(vpevent);

                                            vpevent.param1 = (int)viewport->x();
                                            vpevent.param2 = (int)viewport->y();
                                            eventList.push_back(vpevent);

                                            vpevent.param1 =
                                                    (int)viewport->width();
                                            vpevent.param2 =
                                                    (int)viewport->height();
                                            eventList.push_back(vpevent);
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        }
        ei.numEvents = eventList.size();
        //std::cerr << "found " << ei.numEvents << " events." << std::endl;
        ComController::instance()->sendSlaves(&ei,sizeof(struct eventInfo));
        if(ei.numEvents)
        {
            events = new event[eventList.size()];
            for(int i = 0; i < eventList.size(); i++)
            {
                events[i] = eventList[i];
            }
            ComController::instance()->sendSlaves(events,
                    eventList.size() * sizeof(struct event));
        }
    }
    else
    {
        //std::cerr << "doing event sync." << std::endl;
        ComController::instance()->readMaster(&ei,sizeof(struct eventInfo));
        //std::cerr << "got " << ei.numEvents << " events." << std::endl;
        if(ei.numEvents)
        {
            events = new event[ei.numEvents];
            ComController::instance()->readMaster(events,
                    ei.numEvents * sizeof(struct event));
        }
    }

    _eventQueue->frame(getFrameStamp()->getReferenceTime());

    if((_keyEventSetsDone != 0) || _quitEventSetsDone)
    {
        for(int i = 0; i < ei.numEvents; i++)
        {
            switch(events[i].eventType)
            {
                case (osgGA::GUIEventAdapter::KEYUP):
                    if(_keyEventSetsDone
                            && events[i].param1 == _keyEventSetsDone)
                        _done = true;
                    break;
                case (osgGA::GUIEventAdapter::QUIT_APPLICATION):
                case (osgGA::GUIEventAdapter::CLOSE_WINDOW):
                    if(_quitEventSetsDone)
                        _done = true;
                default:
                    break;
            }
        }
    }

    if(_done)
    {
        if(ei.numEvents)
        {
            delete[] events;
        }
        return;
    }

    static int lastMask = 0;

    for(int i = 0; i < ei.numEvents; i++)
    {
        switch(events[i].eventType)
        {
            case (osgGA::GUIEventAdapter::PUSH):
            case (osgGA::GUIEventAdapter::RELEASE):
            {
                InteractionManager::instance()->setMouseButtonMask(
                        events[i].param1);
                InteractionManager::instance()->processMouse();
                break;
            }
            case (osgGA::GUIEventAdapter::DOUBLECLICK):
            {
                InteractionManager::instance()->createMouseDoubleClickEvent(
                        events[i].param1);
                break;
            }
            case (osgGA::GUIEventAdapter::DRAG):
            {
                if(InteractionManager::instance()->mouseActive())
                {
                    InteractionManager::instance()->setMouse(events[i].param1,
                            events[i].param2);
                    InteractionManager::instance()->createMouseDragEvents();
                }
                break;
            }
            case (osgGA::GUIEventAdapter::MOVE):
            {
                InteractionManager::instance()->setMouse(events[i].param1,
                        events[i].param2);
                break;
            }
            case (osgGA::GUIEventAdapter::KEYDOWN):
            {
                KeyboardInteractionEvent * kie = new KeyboardInteractionEvent();
                kie->setInteraction(KEY_DOWN);
                kie->setKey(events[i].param1);
                kie->setMod(events[i].param2);
                InteractionManager::instance()->addEvent(kie);
                break;
            }
            case (osgGA::GUIEventAdapter::KEYUP):
            {
                KeyboardInteractionEvent * kie = new KeyboardInteractionEvent();
                kie->setInteraction(KEY_UP);
                kie->setKey(events[i].param1);
                kie->setMod(events[i].param2);
                InteractionManager::instance()->addEvent(kie);
                break;
            }
            case (UPDATE_ACTIVE_SCREEN):
                _activeMasterScreen = events[i].param1;
                break;
            case (UPDATE_VIEWPORT):
            {
                struct ScreenInfo * si =
                        ScreenConfig::instance()->getMasterScreenInfo(
                                events[i].param1);
                i++;
                if(i >= ei.numEvents)
                {
                    break;
                }

                if(si)
                {
                    si->myChannel->left = events[i].param1;
                    si->myChannel->bottom = events[i].param2;
                }

                i++;
                if(i >= ei.numEvents)
                {
                    break;
                }

                if(si)
                {
                    si->myChannel->width = events[i].param1;
                    si->myChannel->height = events[i].param2;
                }

                break;
            }
            default:
                break;
        }
    }

    if(ei.numEvents)
    {
        delete[] events;
    }

    if(getViewerStats() && getViewerStats()->collectStats("event"))
    {
        double endEventTraversal = osg::Timer::instance()->delta_s(_startTick,
                osg::Timer::instance()->tick());

        // update current frames stats
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(),
                "Event traversal begin time",beginEventTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(),
                "Event traversal end time",endEventTraversal);
        getViewerStats()->setAttribute(_frameStamp->getFrameNumber(),
                "Event traversal time taken",
                endEventTraversal - beginEventTraversal);
    }

}

void CVRViewer::renderingTraversals()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    //std::cerr << "render start." << std::endl;
    bool _outputMasterCameraLocation = false;
    if(_outputMasterCameraLocation)
    {
        Views views;
        getViews(views);

        for(Views::iterator itr = views.begin(); itr != views.end(); ++itr)
        {
            osgViewer::View* view = *itr;
            if(view)
            {
                const osg::Matrixd& m =
                        view->getCamera()->getInverseViewMatrix();
                //OSG_NOTIFY(osg::NOTICE)<<"View "<<view<<", Master Camera position("<<m.getTrans()<<"), rotation("<<m.getRotate()<<")"<<std::endl;
            }
        }
    }

    // check to see if windows are still valid
    checkWindowStatus();

    double beginRenderingTraversals = elapsedTime();

    osg::FrameStamp* frameStamp = getViewerFrameStamp();

#ifndef WIN32
    if(getViewerStats() && getViewerStats()->collectStats("scene"))
    {
        int frameNumber = frameStamp ? frameStamp->getFrameNumber() : 0;

        Views views;
        getViews(views);
        for(Views::iterator vitr = views.begin(); vitr != views.end(); ++vitr)
        {
            View* view = *vitr;
            osg::Stats* stats = view->getStats();
            osg::Node* sceneRoot = view->getSceneData();
            if(sceneRoot && stats)
            {
                osgUtil::StatsVisitor statsVisitor;
                sceneRoot->accept(statsVisitor);
                statsVisitor.totalUpStats();

                unsigned int unique_primitives = 0;
                osgUtil::Statistics::PrimitiveCountMap::iterator pcmitr;
                for(pcmitr = statsVisitor._uniqueStats.GetPrimitivesBegin();
                        pcmitr != statsVisitor._uniqueStats.GetPrimitivesEnd();
                        ++pcmitr)
                {
                    unique_primitives += pcmitr->second;
                }

                stats->setAttribute(frameNumber,"Number of unique StateSet",
                        static_cast<double>(statsVisitor._statesetSet.size()));
                stats->setAttribute(frameNumber,"Number of unique Group",
                        static_cast<double>(statsVisitor._groupSet.size()));
                stats->setAttribute(frameNumber,"Number of unique Transform",
                        static_cast<double>(statsVisitor._transformSet.size()));
                stats->setAttribute(frameNumber,"Number of unique LOD",
                        static_cast<double>(statsVisitor._lodSet.size()));
                stats->setAttribute(frameNumber,"Number of unique Switch",
                        static_cast<double>(statsVisitor._switchSet.size()));
                stats->setAttribute(frameNumber,"Number of unique Geode",
                        static_cast<double>(statsVisitor._geodeSet.size()));
                stats->setAttribute(frameNumber,"Number of unique Drawable",
                        static_cast<double>(statsVisitor._drawableSet.size()));
                stats->setAttribute(frameNumber,"Number of unique Geometry",
                        static_cast<double>(statsVisitor._geometrySet.size()));
                stats->setAttribute(
                        frameNumber,
                        "Number of unique Vertices",
                        static_cast<double>(statsVisitor._uniqueStats._vertexCount));
                stats->setAttribute(frameNumber,"Number of unique Primitives",
                        static_cast<double>(unique_primitives));

                unsigned int instanced_primitives = 0;
                for(pcmitr = statsVisitor._instancedStats.GetPrimitivesBegin();
                        pcmitr
                                != statsVisitor._instancedStats.GetPrimitivesEnd();
                        ++pcmitr)
                {
                    instanced_primitives += pcmitr->second;
                }

                stats->setAttribute(
                        frameNumber,
                        "Number of instanced Stateset",
                        static_cast<double>(statsVisitor._numInstancedStateSet));
                stats->setAttribute(frameNumber,"Number of instanced Group",
                        static_cast<double>(statsVisitor._numInstancedGroup));
                stats->setAttribute(
                        frameNumber,
                        "Number of instanced Transform",
                        static_cast<double>(statsVisitor._numInstancedTransform));
                stats->setAttribute(frameNumber,"Number of instanced LOD",
                        static_cast<double>(statsVisitor._numInstancedLOD));
                stats->setAttribute(frameNumber,"Number of instanced Switch",
                        static_cast<double>(statsVisitor._numInstancedSwitch));
                stats->setAttribute(frameNumber,"Number of instanced Geode",
                        static_cast<double>(statsVisitor._numInstancedGeode));
                stats->setAttribute(
                        frameNumber,
                        "Number of instanced Drawable",
                        static_cast<double>(statsVisitor._numInstancedDrawable));
                stats->setAttribute(
                        frameNumber,
                        "Number of instanced Geometry",
                        static_cast<double>(statsVisitor._numInstancedGeometry));
                stats->setAttribute(
                        frameNumber,
                        "Number of instanced Vertices",
                        static_cast<double>(statsVisitor._instancedStats._vertexCount));
                stats->setAttribute(frameNumber,
                        "Number of instanced Primitives",
                        static_cast<double>(instanced_primitives));
            }
        }
    }
#endif

    Scenes scenes;
    getScenes(scenes);

    for(Scenes::iterator sitr = scenes.begin(); sitr != scenes.end(); ++sitr)
    {
        osgViewer::Scene* scene = *sitr;
        osgDB::DatabasePager* dp = scene ? scene->getDatabasePager() : 0;
        if(dp)
        {
            dp->signalBeginFrame(frameStamp);
        }

        if(scene->getSceneData())
        {
            // fire off a build of the bounding volumes while we 
            // are still running single threaded.
            scene->getSceneData()->getBound();
        }
    }

    // OSG_NOTIFY(osg::NOTICE)<<std::endl<<"Start frame"<<std::endl;

    Contexts contexts;
    getContexts(contexts);

    Cameras cameras;
    getCameras(cameras);

    Contexts::iterator itr;

    bool doneMakeCurrentInThisThread = false;

    if(_endDynamicDrawBlock.valid())
    {
        _endDynamicDrawBlock->reset();
    }


    if(_cullMode == CALVR)
    {
        CVRCullVisitor::PreCullVisitor precv;
        SceneManager::instance()->getScene()->accept(precv);
    }

    // dispatch the rendering threads
    if(_startRenderingBarrier.valid())
        _startRenderingBarrier->block();

    // reset any double buffer graphics objects
    for(Cameras::iterator camItr = cameras.begin(); camItr != cameras.end();
            ++camItr)
    {
        osg::Camera* camera = *camItr;
        osgViewer::Renderer* renderer =
                dynamic_cast<osgViewer::Renderer*>(camera->getRenderer());
        if(renderer)
        {
            if(!renderer->getGraphicsThreadDoesCull()
                    && !(camera->getCameraThread()))
            {
                if(!ComController::instance()->isMaster() || _renderOnMaster)
                {
                    renderer->cull();
                }
            }
        }
    }

    for(itr = contexts.begin(); itr != contexts.end(); ++itr)
    {
        if(!((*itr)->getGraphicsThread()) && (*itr)->valid())
        {
            doneMakeCurrentInThisThread = true;
            makeCurrent(*itr);
            if(!ComController::instance()->isMaster() || _renderOnMaster)
            {
                (*itr)->runOperations();
            }
            glFinish();
        }
    }

    if(_endRenderingDispatchBarrier.valid())
        _endRenderingDispatchBarrier->block();

    ComController::instance()->sync();

    if(_swapReadyBarrier.valid())
    {
        _swapReadyBarrier->block();
    }

    for(itr = contexts.begin(); itr != contexts.end(); ++itr)
    {
        if(!((*itr)->getGraphicsThread()) && (*itr)->valid())
        {
            doneMakeCurrentInThisThread = true;
            makeCurrent(*itr);
            (*itr)->swapBuffers();
        }
    }

    for(Scenes::iterator sitr = scenes.begin(); sitr != scenes.end(); ++sitr)
    {
        osgViewer::Scene* scene = *sitr;
        osgDB::DatabasePager* dp = scene ? scene->getDatabasePager() : 0;
        if(dp)
        {
            dp->signalEndFrame();
        }
    }

    // wait till the dynamic draw is complete.
    if(_endDynamicDrawBlock.valid())
    {
        _endDynamicDrawBlock->block();
    }

    if(_cullMode == CALVR)
    {
        CVRCullVisitor::PostCullVisitor postcv;
        SceneManager::instance()->getScene()->accept(postcv);
    }

    if(_releaseContextAtEndOfFrameHint && doneMakeCurrentInThisThread)
    {
        releaseContext();
    }

    if(getViewerStats() && getViewerStats()->collectStats("update"))
    {
        double endRenderingTraversals = elapsedTime();

        // update current frames stats
        getViewerStats()->setAttribute(frameStamp->getFrameNumber(),
                "Rendering traversals begin time ",beginRenderingTraversals);
        getViewerStats()->setAttribute(frameStamp->getFrameNumber(),
                "Rendering traversals end time ",endRenderingTraversals);
        getViewerStats()->setAttribute(frameStamp->getFrameNumber(),
                "Rendering traversals time taken",
                endRenderingTraversals - beginRenderingTraversals);
    }

#if !((OPENSCENEGRAPH_MAJOR_VERSION == 2) && (OPENSCENEGRAPH_MINOR_VERSION == 8)) 
    _requestRedraw = false;
#endif
}

void CVRViewer::startThreading()
{
    if(_threadsRunning)
        return;

    //OSG_NOTIFY(osg::INFO)<<"Viewer::startThreading() - starting threading"<<std::endl;

    // release any context held by the main thread.
    releaseContext();

    _threadingModel =
            _threadingModel == AutomaticSelection ?
                    suggestBestThreadingModel() : _threadingModel;

    Contexts contexts;
    getContexts(contexts);

    //OSG_NOTIFY(osg::INFO)<<"Viewer::startThreading() - contexts.size()="<<contexts.size()<<std::endl;

    Cameras cameras;
    getCameras(cameras);

    //std::cerr << "Num cameras: " << cameras.size() << std::endl;

    unsigned int numThreadsOnStartBarrier = 0;
    unsigned int numThreadsOnEndBarrier = 0;

    switch(_threadingModel)
    {
        case (SingleThreaded):
            numThreadsOnStartBarrier = 1;
            numThreadsOnEndBarrier = 1;
            return;
        case (CullDrawThreadPerContext):
            numThreadsOnStartBarrier = contexts.size() + 1;
            numThreadsOnEndBarrier = contexts.size() + 1;
            break;
        case (DrawThreadPerContext):
            numThreadsOnStartBarrier = 1;
            numThreadsOnEndBarrier = contexts.size() + 1;
            break;
        case (CullThreadPerCameraDrawThreadPerContext):
            numThreadsOnStartBarrier = cameras.size() + 1;
            numThreadsOnEndBarrier = contexts.size() + 1;
            break;
        default:
            //OSG_NOTIFY(osg::NOTICE)<<"Error: Threading model not selected"<<std::endl;
            return;
    }

    // using multi-threading so make sure that new objects are allocated with thread safe ref/unref
    osg::Referenced::setThreadSafeReferenceCounting(true);

    Scenes scenes;
    getScenes(scenes);
    for(Scenes::iterator scitr = scenes.begin(); scitr != scenes.end(); ++scitr)
    {
        if((*scitr)->getSceneData())
        {
            //OSG_NOTIFY(osg::INFO)<<"Making scene thread safe"<<std::endl;

            // make sure that existing scene graph objects are allocated with thread safe ref/unref
            (*scitr)->getSceneData()->setThreadSafeRefUnref(true);

            // update the scene graph so that it has enough GL object buffer memory for the graphics contexts that will be using it.
            (*scitr)->getSceneData()->resizeGLObjectBuffers(
                    osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts());
        }
    }

    int numProcessors = OpenThreads::GetNumberOfProcessors();
    bool affinity = numProcessors > 1;

    Contexts::iterator citr;

    unsigned int numViewerDoubleBufferedRenderingOperation = 0;

    bool graphicsThreadsDoesCull = _threadingModel == CullDrawThreadPerContext
            || _threadingModel == SingleThreaded;

    for(Cameras::iterator camItr = cameras.begin(); camItr != cameras.end();
            ++camItr)
    {
        osg::Camera* camera = *camItr;
        osgViewer::Renderer* renderer =
                dynamic_cast<osgViewer::Renderer*>(camera->getRenderer());
        if(renderer)
        {
            renderer->setGraphicsThreadDoesCull(graphicsThreadsDoesCull);
            renderer->setDone(false);
            ++numViewerDoubleBufferedRenderingOperation;
        }
    }

    if(_threadingModel == CullDrawThreadPerContext)
    {
        _startRenderingBarrier = 0;
        _endRenderingDispatchBarrier = 0;
        _endDynamicDrawBlock = 0;
    }
    else if(_threadingModel == DrawThreadPerContext
            || _threadingModel == CullThreadPerCameraDrawThreadPerContext)
    {
        _startRenderingBarrier = 0;
        _endRenderingDispatchBarrier = 0;
        if(!ComController::instance()->isMaster() || _renderOnMaster)
        {
            _endDynamicDrawBlock = new osg::EndOfDynamicDrawBlock(
                    numViewerDoubleBufferedRenderingOperation);
        }
        else
        {
            _endDynamicDrawBlock = 0;
        }

#ifndef OSGUTIL_RENDERBACKEND_USE_REF_PTR
        if(!osg::Referenced::getDeleteHandler())
            osg::Referenced::setDeleteHandler(new osg::DeleteHandler(2));
        else
            osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(2);
#endif
    }

    if(numThreadsOnStartBarrier > 1)
    {
        if(!ComController::instance()->isMaster() || _renderOnMaster)
        {
            _startRenderingBarrier = new osg::BarrierOperation(
                    numThreadsOnStartBarrier,
                    osg::BarrierOperation::NO_OPERATION);
        }
        else
        {
            _startRenderingBarrier = 0;
        }
    }

    if(numThreadsOnEndBarrier > 1)
    {
        _endRenderingDispatchBarrier = new osg::BarrierOperation(
                numThreadsOnEndBarrier,osg::BarrierOperation::NO_OPERATION);
    }

    if(_threadingModel == CullThreadPerCameraDrawThreadPerContext)
    {
        _cullDrawBarrier = new osg::BarrierOperation(
                cameras.size() + contexts.size(),
                osg::BarrierOperation::NO_OPERATION);
    }

    _swapReadyBarrier =
            contexts.empty() ?
                    0 :
                    new osg::BarrierOperation(contexts.size() + 1,
                            osg::BarrierOperation::NO_OPERATION);

    osg::ref_ptr < osg::SwapBuffersOperation > swapOp =
            new osg::SwapBuffersOperation();

    typedef std::map<OpenThreads::Thread*,int> ThreadAffinityMap;
    ThreadAffinityMap threadAffinityMap;

    // IVL
    static bool addSync = true;

    unsigned int processNum = 1;
    for(citr = contexts.begin(); citr != contexts.end(); ++citr, ++processNum)
    {
        osg::GraphicsContext* gc = (*citr);

        if(!gc->isRealized())
        {
            //OSG_NOTIFY(osg::INFO)<<"ViewerBase::startThreading() : Realizng window "<<gc<<std::endl;
            gc->realize();
        }

        gc->getState()->setDynamicObjectRenderingCompletedCallback(
                _endDynamicDrawBlock.get());

        //std::cerr << "Create graphics thread." << std::endl;
        // create the a graphics thread for this context
        gc->createGraphicsThread();

        if(affinity)
        {
            /*int proc;
             proc = (2 * processNum) % (2 * numProcessors);
             if(proc >= numProcessors)
             {
             proc = (proc - numProcessors) + 1;
             }
             gc->getGraphicsThread()->setProcessorAffinity(proc);*/
            gc->getGraphicsThread()->setProcessorAffinity(
                    processNum % numProcessors);
        }
        threadAffinityMap[gc->getGraphicsThread()] = processNum % numProcessors;

        //std::cerr << "Thread Affinity: " << processNum % numProcessors << std::endl;

        // add the startRenderingBarrier
        if((_threadingModel == CullDrawThreadPerContext)
                && _startRenderingBarrier.valid())
            gc->getGraphicsThread()->add(_startRenderingBarrier.get());

        if(_threadingModel == CullThreadPerCameraDrawThreadPerContext
                && _cullDrawBarrier.valid())
        {
            if(!ComController::instance()->isMaster() || _renderOnMaster)
            {
                gc->getGraphicsThread()->add(_cullDrawBarrier.get());
            }
        }

        // add the rendering operation itself.
        if(!ComController::instance()->isMaster() || _renderOnMaster)
        {
            gc->getGraphicsThread()->add(new osg::RunOperations());
        }

        // IVL
        gc->getGraphicsThread()->add(new finishOperation());

        if((_threadingModel == CullDrawThreadPerContext
                || _threadingModel == CullThreadPerCameraDrawThreadPerContext
                || _threadingModel == DrawThreadPerContext)
                && _endRenderingDispatchBarrier.valid())
        {
            // add the endRenderingDispatchBarrier
            gc->getGraphicsThread()->add(_endRenderingDispatchBarrier.get());
        }

        if(_swapReadyBarrier.valid())
            gc->getGraphicsThread()->add(_swapReadyBarrier.get());

        // add the swap buffers
        gc->getGraphicsThread()->add(swapOp.get());
    }

    if(_threadingModel == CullThreadPerCameraDrawThreadPerContext
            && numThreadsOnStartBarrier > 1)
    {
        Cameras::iterator camItr;

        for(camItr = cameras.begin(); camItr != cameras.end();
                ++camItr, ++processNum)
        {
            osg::Camera* camera = *camItr;
            camera->createCameraThread();

            if(affinity)
                camera->getCameraThread()->setProcessorAffinity(
                        processNum % numProcessors);
            threadAffinityMap[camera->getCameraThread()] = processNum
                    % numProcessors;

            osg::GraphicsContext* gc = camera->getGraphicsContext();

            // add the startRenderingBarrier
            if(_startRenderingBarrier.valid())
                camera->getCameraThread()->add(_startRenderingBarrier.get());

            osgViewer::Renderer* renderer =
                    dynamic_cast<osgViewer::Renderer*>(camera->getRenderer());
            renderer->setGraphicsThreadDoesCull(false);
            camera->getCameraThread()->add(renderer);

            if(_cullDrawBarrier.valid())
            {
                camera->getCameraThread()->add(_cullDrawBarrier.get());
            }
        }

        for(camItr = cameras.begin(); camItr != cameras.end(); ++camItr)
        {
            osg::Camera* camera = *camItr;
            if(camera->getCameraThread()
                    && !camera->getCameraThread()->isRunning())
            {
                //OSG_NOTIFY(osg::INFO)<<"  camera->getCameraThread()-> "<<camera->getCameraThread()<<std::endl;
                camera->getCameraThread()->startThread();
            }
        }
    }

#if 0    
    if (affinity)
    {
        OpenThreads::SetProcessorAffinityOfCurrentThread(0);
        if (_scene.valid() && _scene->getDatabasePager())
        {
#if 0        
            _scene->getDatabasePager()->setProcessorAffinity(1);
#else
            _scene->getDatabasePager()->setProcessorAffinity(0);
#endif
        }
    }
#endif

#if 0
    if (affinity)
    {
        for(ThreadAffinityMap::iterator titr = threadAffinityMap.begin();
                titr != threadAffinityMap.end();
                ++titr)
        {
            titr->first->setProcessorAffinity(titr->second);
        }
    }
#endif

    for(citr = contexts.begin(); citr != contexts.end(); ++citr)
    {
        osg::GraphicsContext* gc = (*citr);
        if(gc->getGraphicsThread() && !gc->getGraphicsThread()->isRunning())
        {
            //OSG_NOTIFY(osg::INFO)<<"  gc->getGraphicsThread()->startThread() "<<gc->getGraphicsThread()<<std::endl;
            gc->getGraphicsThread()->startThread();
        }
    }

    _threadsRunning = true;

    //OSG_NOTIFY(osg::INFO)<<"Set up threading"<<std::endl;
}

void CVRViewer::frameStart()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    FrameUpdate frameUp;
    if(ComController::instance()->isMaster())
    {
        frameUp.currentTime = osg::Timer::instance()->tick();
        ComController::instance()->sendSlaves(&frameUp,
                sizeof(struct FrameUpdate));
    }
    else
    {
        ComController::instance()->readMaster(&frameUp,
                sizeof(struct FrameUpdate));
    }

    _lastFrameStartTime = _frameStartTime;
    _frameStartTime = frameUp.currentTime;
}

void CVRViewer::addUpdateTraversalFront(UpdateTraversal * ut)
{
    _updateList.push_front(ut);
}

void CVRViewer::addUpdateTraversalBack(UpdateTraversal * ut)
{
    _updateList.push_back(ut);
}

void CVRViewer::removeUpdateTraversal(UpdateTraversal * ut)
{
    for(std::list<UpdateTraversal *>::iterator it = _updateList.begin();
            it != _updateList.end(); it++)
    {
        if((*it) == ut)
        {
            _updateList.erase(it);
            return;
        }
    }
}

bool CVRViewer::processEvent(InteractionEvent * event)
{
    bool ret;
    KeyboardInteractionEvent * kevent = event->asKeyboardEvent();

    if(!kevent)
    {
        return false;
    }

    osgGA::GUIEventAdapter * ea = new osgGA::GUIEventAdapter();
    if(kevent->getInteraction() == KEY_UP)
    {
        ea->setEventType(osgGA::GUIEventAdapter::KEYUP);
    }
    else
    {
        ea->setEventType(osgGA::GUIEventAdapter::KEYDOWN);
    }

    ea->setKey(kevent->getKey());
    ea->setModKeyMask(kevent->getMod());
    ea->setHandled(false);
    for(EventHandlers::iterator hitr = _eventHandlers.begin();
            hitr != _eventHandlers.end(); ++hitr)
    {
        (*hitr)->handleWithCheckAgainstIgnoreHandledEventsMask(*ea,*this,0,0);
    }
    ret = ea->getHandled();
    ea->unref();

    return ret;
}

bool CVRViewer::getRenderOnMaster()
{
    return _renderOnMaster;
}

double CVRViewer::getLastFrameDuration()
{
    return osg::Timer::instance()->delta_s(_lastFrameStartTime,_frameStartTime);
}

double CVRViewer::getProgramDuration()
{
    return osg::Timer::instance()->delta_s(_programStartTime,_frameStartTime);
}

double CVRViewer::getFrameStartTime()
{
    return (double)(_frameStartTime
            * osg::Timer::instance()->getSecondsPerTick());
}

double CVRViewer::getProgramStartTime()
{
    return (double)(_programStartTime
            * osg::Timer::instance()->getSecondsPerTick());
}
