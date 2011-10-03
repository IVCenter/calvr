#include <kernel/InteractionManager.h>
#include <input/TrackingManager.h>
#include <input/TrackerMouse.h>
#include <menu/MenuManager.h>
#include <kernel/ComController.h>
#include <kernel/Navigation.h>
#include <kernel/CVRViewer.h>
#include <kernel/SceneManager.h>
#include <kernel/ScreenConfig.h>
#include <kernel/ScreenBase.h>
#include <kernel/PluginManager.h>
#include <config/ConfigManager.h>

#include <osg/Matrix>

#include <vector>
#include <iostream>

using namespace cvr;

InteractionManager * InteractionManager::_myPtr = NULL;

InteractionManager::InteractionManager()
{
    _lastMouseButtonMask = 0;
    _mouseButtonMask = 0;
    //_mouseInfo = NULL;
    _mouseActive = false;

    //_mouseEvents = ConfigManager::getBool("Input.MouseEvents",true);
    //std::string tsystem = ConfigManager::getEntry("value","Input.TrackingSystem","NONE");
    //std::string bsystem = ConfigManager::getEntry("value","Input.ButtonSystem","NONE");

    _mouseX = 0;
    _mouseY = 0;

    /*if(tsystem == "MOUSE" || bsystem == "MOUSE")
    {
	_mouseTracker = true;
    }
    else
    {
	_mouseTracker = false;
    }*/
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
    _mouseHand = -1;
    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
	if(TrackingManager::instance()->getHandTrackerType(i) == TrackerBase::MOUSE)
	{
	    _mouseHand = i;
	    break;
	}
    }
    return true;
}

void InteractionManager::update()
{
    if(ComController::instance()->getIsSyncError())
    {
	return;
    }
    //processMouse();

    handleEvents();
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

    //std::cerr << "Cleaning " << _mouseQueue.size() << " mouse events." << std::endl;
    while(_mouseQueue.size())
    {
	delete _mouseQueue.front();
	_mouseQueue.pop();
    }
}

void InteractionManager::handleEvent(InteractionEvent * event)
{
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
    //std::cerr << "Mouse x: " << x << " y: " << y << std::endl;
    ScreenInfo * si = ScreenConfig::instance()->getMasterScreenInfo(CVRViewer::instance()->getActiveMasterScreen());

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

    //frac = ((float)(si->myChannel->height - _mouseY))
    //       / (si->myChannel->height);
    frac = ((float)_mouseY) / si->myChannel->height;
    frac = frac - 0.5;
    screenY = frac * si->height;

    osg::Vec3 screenPoint(screenX,0,screenY);
    // get Point in world space
    screenPoint = screenPoint * si->transform;
    // head mounted displays are relative to the head orientation
    if(si->myChannel->stereoMode == "HMD")
    {
	screenPoint = screenPoint * TrackingManager::instance()->getHeadMat(si->myChannel->head);
    }

    osg::Vec3 eyePoint;

    if(si->myChannel->stereoMode == "LEFT")
    {
	eyePoint = osg::Vec3(-ScreenBase::getEyeSeparation() * ScreenBase::getEyeSeparationMultiplier() / 2.0, 0,0);
    }
    else if(si->myChannel->stereoMode == "RIGHT")
    {
	eyePoint = osg::Vec3(-ScreenBase::getEyeSeparation() * ScreenBase::getEyeSeparationMultiplier() / 2.0, 0,0);
    }

    eyePoint = eyePoint * TrackingManager::instance()->getHeadMat(si->myChannel->head);

    osg::Vec3 v = screenPoint - eyePoint;
    v.normalize();

    _mouseMat.makeRotate(osg::Vec3(0, 1, 0), v);
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
	    else if (i == 2)
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
	    buttonEvent->setHand(_mouseHand);
	    buttonEvent->setMasterScreenNum(CVRViewer::instance()->getActiveMasterScreen());
	    _mouseQueue.push(buttonEvent);
	}
        bit = bit << 1;
    }
    _lastMouseButtonMask = _mouseButtonMask;
}

void InteractionManager::createMouseDragEvents()
{
    if(!_mouseActive)
    {
        return;
    }

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
	    else if (i == 2)
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
	    dEvent->setHand(_mouseHand);
	    dEvent->setMasterScreenNum(CVRViewer::instance()->getActiveMasterScreen());
	    _mouseQueue.push(dEvent);
	}
        bit = bit << 1;
    }
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
	    else if (i == 2)
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
	    dcEvent->setHand(_mouseHand);
	    dcEvent->setMasterScreenNum(CVRViewer::instance()->getActiveMasterScreen());
	    _mouseQueue.push(dcEvent);

	    _mouseButtonMask |= button;
	    _lastMouseButtonMask |= button;
	    return;
	}
        bit = bit << 1;
    }
}
