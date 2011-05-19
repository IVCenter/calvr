#include <kernel/InteractionManager.h>
#include <input/TrackingManager.h>
#include <menu/MenuManager.h>
#include <kernel/ComController.h>
#include <kernel/Navigation.h>
#include <kernel/CVRViewer.h>
#include <kernel/PluginManager.h>
#include <config/ConfigManager.h>

#include <osg/Matrix>

#include <vector>
#include <iostream>

using namespace cvr;

osg::Matrix cvr::tie2mat(TrackingInteractionEvent * tie)
{
    osg::Matrix m, t;
    t.makeTranslate(tie->xyz[0], tie->xyz[1], tie->xyz[2]);
    m.makeRotate(osg::Quat(tie->rot[0], tie->rot[1], tie->rot[2], tie->rot[3]));
    return m * t;
}

InteractionManager * InteractionManager::_myPtr = NULL;

InteractionManager::InteractionManager()
{
    _lastMouseButtonMask = 0;
    _mouseButtonMask = 0;
    _mouseInfo = NULL;
    _mouseActive = false;

    _mouseEvents = ConfigManager::getBool("Input.MouseEvents",true);
    std::string tsystem = ConfigManager::getEntry("value","Input.TrackingSystem","NONE");
    std::string bsystem = ConfigManager::getEntry("value","Input.ButtonSystem","NONE");

    if(tsystem == "MOUSE" || bsystem == "MOUSE")
    {
	_mouseTracker = true;
    }
    else
    {
	_mouseTracker = false;
    }
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
}

void InteractionManager::handleEvent(InteractionEvent * event)
{
    if(MenuManager::instance()->processEvent(event))
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

void InteractionManager::setMouseInfo(MouseInfo & mi)
{
    if(!_mouseInfo)
    {
        _mouseInfo = new MouseInfo;
        _mouseActive = true;
    }
    *_mouseInfo = mi;

    float worldxOffset, worldyOffset;

    float frac = ((float)_mouseInfo->x) / ((float)_mouseInfo->viewportX);
    frac = frac - 0.5;
    worldxOffset = frac * _mouseInfo->screenWidth;

    frac = ((float)(_mouseInfo->viewportY - _mouseInfo->y))
            / ((float)_mouseInfo->viewportY);
    frac = frac - 0.5;
    worldyOffset = frac * _mouseInfo->screenHeight;

    osg::Vec3 mousePos = _mouseInfo->screenCenter + osg::Vec3(worldxOffset, 0,
                                                              worldyOffset);
    osg::Vec3 headPos = TrackingManager::instance()->getHeadMat().getTrans();

    osg::Vec3 v = mousePos - headPos;
    v.normalize();

    _mouseMat.makeRotate(osg::Vec3(0, 1, 0), v);
    _mouseMat = _mouseMat * osg::Matrix::translate(headPos);

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

MouseInfo * InteractionManager::getMouseInfo()
{
    return _mouseInfo;
}

int InteractionManager::getMouseX()
{
    if(_mouseInfo)
    {
        return _mouseInfo->x;
    }
    return 0;
}

int InteractionManager::getMouseY()
{
    if(_mouseInfo)
    {
        return _mouseInfo->y;
    }
    return 0;
}

void InteractionManager::processMouse()
{
    if(!_mouseActive || !_mouseInfo)
    {
        return;
    }

    // TODO: get max mouse buttons from somewhere
    unsigned int bit = 1;
    for(int i = 0; i < 10; i++)
    {
        if((_lastMouseButtonMask & bit) != (_mouseButtonMask & bit))
        {
	    if(_mouseTracker)
	    {
		TrackingInteractionEvent * tie = new TrackingInteractionEvent;
		if(_lastMouseButtonMask & bit)
		{
		    tie->type = BUTTON_UP;
		}
		else
		{
		    tie->type = BUTTON_DOWN;
		}
		tie->button = i;
		tie->hand = 0;
		osg::Vec3 pos = _mouseMat.getTrans();
		osg::Quat rot = _mouseMat.getRotate();
		tie->xyz[0] = pos.x();
		tie->xyz[1] = pos.y();
		tie->xyz[2] = pos.z();
		tie->rot[0] = rot.x();
		tie->rot[1] = rot.y();
		tie->rot[2] = rot.z();
		tie->rot[3] = rot.w();
		addEvent(tie);
	    }
	    else if(_mouseEvents)
	    {
		MouseInteractionEvent * buttonEvent = new MouseInteractionEvent;
		if(_lastMouseButtonMask & bit)
		{
		    buttonEvent->type = MOUSE_BUTTON_UP;
		}
		else
		{
		    buttonEvent->type = MOUSE_BUTTON_DOWN;
		}
		buttonEvent->button = i;
		buttonEvent->x = _mouseInfo->x;
		buttonEvent->y = _mouseInfo->y;
		buttonEvent->transform = _mouseMat;
		addEvent(buttonEvent);
	    }
        }
        bit = bit << 1;
    }
    _lastMouseButtonMask = _mouseButtonMask;
}

void InteractionManager::createMouseDragEvents()
{
    if(!_mouseActive || !_mouseInfo)
    {
        return;
    }

    unsigned int bit = 1;
    for(int i = 0; i < 10; i++)
    {
        if((_mouseButtonMask & bit))
        {
	    if(_mouseTracker)
	    {
		TrackingInteractionEvent * tie = new TrackingInteractionEvent;
		tie->type = BUTTON_DRAG;
		tie->button = i;
		tie->hand = 0;
		osg::Vec3 pos = _mouseMat.getTrans();
		osg::Quat rot = _mouseMat.getRotate();
		tie->xyz[0] = pos.x();
		tie->xyz[1] = pos.y();
		tie->xyz[2] = pos.z();
		tie->rot[0] = rot.x();
		tie->rot[1] = rot.y();
		tie->rot[2] = rot.z();
		tie->rot[3] = rot.w();
		addEvent(tie);
	    }
	    else if(_mouseEvents)
	    {
		MouseInteractionEvent * dEvent = new MouseInteractionEvent;
		dEvent->type = MOUSE_DRAG;
		dEvent->button = i;
		dEvent->x = _mouseInfo->x;
		dEvent->y = _mouseInfo->y;
		dEvent->transform = _mouseMat;
		addEvent(dEvent);
	    }
        }
        bit = bit << 1;
    }
}

void InteractionManager::createMouseDoubleClickEvent(int button)
{
    if(!_mouseActive || !_mouseInfo)
    {
        return;
    }

    unsigned int bit = 1;
    for(int i = 0; i < 10; i++)
    {
        if((button & bit))
        {
	    if(_mouseTracker)
	    {
		TrackingInteractionEvent * tie = new TrackingInteractionEvent;
		tie->type = BUTTON_DOUBLE_CLICK;
		tie->button = i;
		tie->hand = 0;
		osg::Vec3 pos = _mouseMat.getTrans();
		osg::Quat rot = _mouseMat.getRotate();
		tie->xyz[0] = pos.x();
		tie->xyz[1] = pos.y();
		tie->xyz[2] = pos.z();
		tie->rot[0] = rot.x();
		tie->rot[1] = rot.y();
		tie->rot[2] = rot.z();
		tie->rot[3] = rot.w();
		addEvent(tie);
	    }
	    else if(_mouseEvents)
	    {
		MouseInteractionEvent * dcEvent = new MouseInteractionEvent;
		dcEvent->type = MOUSE_DOUBLE_CLICK;
		dcEvent->button = i;
		dcEvent->x = _mouseInfo->x;
		dcEvent->y = _mouseInfo->y;
		dcEvent->transform = _mouseMat;
		addEvent(dcEvent);
	    }
            _mouseButtonMask |= button;
            _lastMouseButtonMask |= button;
            return;
        }
        bit = bit << 1;
    }
}
