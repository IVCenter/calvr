#include <cvrKernel/InteractionManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrInput/TrackerMouse.h>
#include <cvrMenu/MenuManager.h>
#include <cvrKernel/ComController.h>
#include <cvrKernel/Navigation.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/SceneManager.h>
#include <cvrKernel/ScreenConfig.h>
#include <cvrKernel/ScreenBase.h>
#include <cvrKernel/PluginManager.h>
#include <cvrConfig/ConfigManager.h>

#include <osg/Matrix>

#include <vector>
#include <iostream>

using namespace cvr;

InteractionManager * InteractionManager::_myPtr = NULL;

InteractionManager::InteractionManager()
{
    _lastMouseButtonMask = 0;
    _mouseButtonMask = 0;
    _mouseActive = false;

    _mouseX = 0;
    _mouseY = 0;

    _mouseWheelTimeout = ConfigManager::getDouble("value",
            "Input.MouseWheelTimeout",0.05);
    _mouseWheelTime = 0;
    _mouseWheel = 0;

    _dragEventTime = 0;

    _eventDebug = ConfigManager::getBool("value","EventDebug",false,NULL);
}

InteractionManager::~InteractionManager()
{
}

InteractionManager * InteractionManager::instance()
{
    if(!_myPtr)
    {
        _myPtr = new InteractionManager();
    }
    return _myPtr;
}

bool InteractionManager::init()
{
    return true;
}

void InteractionManager::update()
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

    handleEvents();

    if(stats)
    {
        endTime = osg::Timer::instance()->delta_s(
                CVRViewer::instance()->getStartTick(),
                osg::Timer::instance()->tick());
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "Interaction begin time", startTime);
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "Interaction end time", endTime);
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "Interaction time taken", endTime - startTime);
    }
}

void InteractionManager::handleEvents()
{
    _queueLock.lock();

    while(_eventQueue.size())
    {
        handleEvent(_eventQueue.front());
        delete _eventQueue.front();
        _eventQueue.pop();
    }

    _queueLock.unlock();

    while(_mouseQueue.size())
    {
        delete _mouseQueue.front();
        _mouseQueue.pop();
    }
}

void InteractionManager::handleEvent(InteractionEvent * event)
{
    if(!event)
    {
        return;
    }

    if(_eventDebug)
    {
        std::cerr << std::endl;
        std::cerr << "Event: " << event->getEventName() << std::endl;
        event->printValues();
    }

    if(MenuManager::instance()->processEvent(event))
    {
        return;
    }

    if(SceneManager::instance()->processEvent(event))
    {
        return;
    }

    if(PluginManager::instance()->processEvent(event))
    {
        return;
    }

    if(CVRViewer::instance()->processEvent(event))
    {
        return;
    }

    //TODO:ANDROID MAY NOT NEED NAVIGATION?
    Navigation::instance()->processEvent(event);
}

void InteractionManager::addEvent(InteractionEvent * event)
{
    _queueLock.lock();

    _eventQueue.push(event);

    _queueLock.unlock();
}

void InteractionManager::setMouse(int x, int y)
{
    ScreenInfo * si = ScreenConfig::instance()->getMasterScreenInfo(
            CVRViewer::instance()->getActiveMasterScreen());

    if(!si)
    {
        return;
    }

    _mouseActive = true;
    _mouseX = x;
    _mouseY = y;

    float screenX, screenY;
    float frac;

    frac = ((float)_mouseX) / si->myChannel->width;
    frac = frac - 0.5;
    screenX = frac * si->width;

    frac = ((float)_mouseY) / si->myChannel->height;
    frac = frac - 0.5;
    screenY = frac * si->height;

    osg::Vec3 screenPoint(screenX,0,screenY);
    // get Point in world space
    screenPoint = screenPoint * si->transform;
    // head mounted displays are relative to the head orientation
    if(si->myChannel->stereoMode == "HMD" || si->myChannel->stereoMode == "OCULUS")
    {
        screenPoint = screenPoint
                * TrackingManager::instance()->getHeadMat(si->myChannel->head);
    }

    osg::Vec3 eyePoint;

    if(si->myChannel->stereoMode == "LEFT")
    {
        eyePoint = osg::Vec3(
                -ScreenBase::getEyeSeparation()
                        * ScreenBase::getEyeSeparationMultiplier() / 2.0,0,0);
    }
    else if(si->myChannel->stereoMode == "RIGHT")
    {
        eyePoint = osg::Vec3(
                -ScreenBase::getEyeSeparation()
                        * ScreenBase::getEyeSeparationMultiplier() / 2.0,0,0);
    }

    eyePoint = eyePoint
            * TrackingManager::instance()->getHeadMat(si->myChannel->head);

    osg::Vec3 v = screenPoint - eyePoint;
    v.normalize();

    _mouseMat.makeRotate(osg::Vec3(0,1,0),v);
    _mouseMat = _mouseMat * osg::Matrix::translate(eyePoint);
}

void InteractionManager::setMouseButtonMask(unsigned int mask)
{
    _mouseButtonMask = mask;
}

unsigned int InteractionManager::getMouseButtonMask()
{
    return _mouseButtonMask;
}

bool InteractionManager::mouseActive()
{
    return _mouseActive;
}

osg::Matrix & InteractionManager::getMouseMat()
{
    return _mouseMat;
}

int InteractionManager::getMouseX()
{
    return _mouseX;
}

int InteractionManager::getMouseY()
{
    return _mouseY;
}

void InteractionManager::setMouseWheel(int w)
{
    _mouseWheel = w;
    _mouseWheelTime = 0;
}

void InteractionManager::processMouse()
{
    if(!_mouseActive)
    {
        return;
    }

    // TODO: get max mouse buttons from somewhere
    unsigned int bit = 1;
    for(int i = 0; i < 10; i++)
    {
        if((_lastMouseButtonMask & bit) != (_mouseButtonMask & bit))
        {
            MouseInteractionEvent * buttonEvent = new MouseInteractionEvent();
            if(_lastMouseButtonMask & bit)
            {
                buttonEvent->setInteraction(BUTTON_UP);
            }
            else
            {
                buttonEvent->setInteraction(BUTTON_DOWN);
            }

            //TODO: make config file flag
            // makes the right click button 1
            if(i == 1)
            {
                buttonEvent->setButton(2);
            }
            else if(i == 2)
            {
                buttonEvent->setButton(1);
            }
            else
            {
                buttonEvent->setButton(i);
            }
            buttonEvent->setX(_mouseX);
            buttonEvent->setY(_mouseY);
            buttonEvent->setTransform(_mouseMat);
            buttonEvent->setHand(-1);
            buttonEvent->setMasterScreenNum(
                    CVRViewer::instance()->getActiveMasterScreen());
            _mouseQueue.push(buttonEvent);
        }
        bit = bit << 1;
    }
    _lastMouseButtonMask = _mouseButtonMask;
}

void InteractionManager::createMouseDragEvents(bool single)
{
    if(!_mouseActive)
    {
        return;
    }

    int numEvents = 0;

    if(single)
    {
        numEvents = 1;
        _dragEventTime = 0;
    }
    else
    {
        static const double slice = 1.0 / 60.0;
        _dragEventTime += CVRViewer::instance()->getLastFrameDuration();
        numEvents = (int)(_dragEventTime / slice);
        _dragEventTime -= slice * ((double)numEvents);
    }

    for(int j = 0; j < numEvents; j++)
    {
        unsigned int bit = 1;
        for(int i = 0; i < 10; i++)
        {
            if((_mouseButtonMask & bit))
            {
                MouseInteractionEvent * dEvent = new MouseInteractionEvent();
                dEvent->setInteraction(BUTTON_DRAG);
                //TODO: make config file flag
                // makes the right click button 1
                if(i == 1)
                {
                    dEvent->setButton(2);
                }
                else if(i == 2)
                {
                    dEvent->setButton(1);
                }
                else
                {
                    dEvent->setButton(i);
                }
                dEvent->setX(_mouseX);
                dEvent->setY(_mouseY);
                dEvent->setTransform(_mouseMat);
                dEvent->setHand(-1);
                dEvent->setMasterScreenNum(
                        CVRViewer::instance()->getActiveMasterScreen());
                _mouseQueue.push(dEvent);
            }
            bit = bit << 1;
        }
    }
}
void InteractionManager::setMouseInteraction(int button, osg::Vec3f pos){
    _mouseMat.makeTranslate(pos);
    createMouseDoubleClickEvent(button);

}
void InteractionManager::createMouseDoubleClickEvent(int button)
{
    if(!_mouseActive)
    {
        return;
    }

    unsigned int bit = 1;
    for(int i = 0; i < 10; i++)
    {
        if((button & bit))
        {
            MouseInteractionEvent * dcEvent = new MouseInteractionEvent();
            dcEvent->setInteraction(BUTTON_DOUBLE_CLICK);
            //TODO: make config file flag
            // makes the right click button 1
            if(i == 1)
            {
                dcEvent->setButton(2);
            }
            else if(i == 2)
            {
                dcEvent->setButton(1);
            }
            else
            {
                dcEvent->setButton(i);
            }
            dcEvent->setX(_mouseX);
            dcEvent->setY(_mouseY);
            dcEvent->setTransform(_mouseMat);
            dcEvent->setHand(-1);
            dcEvent->setMasterScreenNum(
                    CVRViewer::instance()->getActiveMasterScreen());
//            _mouseQueue.push(dcEvent);
            addEvent(dcEvent);
            _mouseButtonMask |= button;
            _lastMouseButtonMask |= button;
            return;
        }
        bit = bit << 1;
    }
}

void InteractionManager::checkWheelTimeout()
{
    if(_mouseWheel == 0)
    {
        return;
    }

    double timeintv = CVRViewer::instance()->getLastFrameDuration();
    if(timeintv >= _mouseWheelTimeout)
    {
        timeintv = _mouseWheelTimeout - 0.0001;
    }
    _mouseWheelTime += timeintv;
    if(_mouseWheelTime > _mouseWheelTimeout)
    {
        _mouseWheel = 0;
        _mouseWheelTime = 0;
    }
}
