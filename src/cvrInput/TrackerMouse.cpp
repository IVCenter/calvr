#include <cvrInput/TrackerMouse.h>
#include <cvrInput/TrackingManager.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrKernel/ComController.h>
#include <cvrConfig/ConfigManager.h>

#include <osg/Quat>

#include <iostream>

using namespace cvr;

TrackerMouse::TrackerMouse() :
        TrackerBase()
{
}

TrackerMouse::~TrackerMouse()
{
}

bool TrackerMouse::init(std::string tag)
{
    _debug = ComController::instance()->isMaster() ? ConfigManager::getBool("value","Input.TrackingDebug",false,NULL) : false;

    _mouseBody.x = 0;
    _mouseBody.y = 0;
    _mouseBody.z = 0;
    osg::Quat q;
    _mouseBody.qx = q.x();
    _mouseBody.qy = q.y();
    _mouseBody.qz = q.z();
    _mouseBody.qw = q.w();

    _mouseButtonMask = 0;

    _mouseValuator = 0;
    _handListInit = false;

    return true;
}

TrackerBase::TrackedBody * TrackerMouse::getBody(int index)
{
    if(index != 0)
    {
        return NULL;
    }

    return &_mouseBody;
}

unsigned int TrackerMouse::getButtonMask()
{
    return _mouseButtonMask;
}

float TrackerMouse::getValuator(int index)
{
    if(index == 0)
    {
	return _mouseValuator;
    }
    return 0;
}

int TrackerMouse::getNumBodies()
{
    return 1;
}

int TrackerMouse::getNumValuators()
{
    return 1;
}

int TrackerMouse::getNumButtons()
{
    return CVR_NUM_MOUSE_BUTTONS;
}

void TrackerMouse::update(
        std::map<int,std::list<InteractionEvent*> > & eventMap)
{
    if(!_handListInit)
    {
	if(_debug)
	{
	    std::cerr << "TrackerMouse: hand list init:" << std::endl;
	}

	int mySystem = -1;
	for(int i = 0; i < TrackingManager::instance()->getNumTrackingSystems(); i++)
	{
	    if(TrackingManager::instance()->getTrackingSystem(i) == this)
	    {
		mySystem = i;
		break;
	    }
	}
	if(mySystem < 0)
	{
	    std::cerr << "TrackerMouse: Error: Unable to find own system in list" << std::endl;
	}
	else
	{
	    if(_debug)
	    {
		std::cerr << "Mouse system: " << mySystem << std::endl;
		for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
		{
		    std::cerr << "Hand: " << i << " buttonFilter: " << TrackingManager::instance()->getButtonFilter(i,mySystem) << std::endl;
		}
	    }

	    int bhand, bbutton;
	    for(int i = 0; i < CVR_NUM_MOUSE_BUTTONS; i++)
	    {
		TrackingManager::instance()->getHandButtonFromSystemButton(mySystem,i,bhand,bbutton);
		_handButtonList.push_back(std::pair<int,int>(bhand,bbutton));
	    }

	    for(int i = 0; i < _handButtonList.size(); i++)
	    {
		if(_handButtonList[i].first < 0)
		{
		    _handValidList.push_back(false);
		    continue;
		}
		int hsystem, hindex;
		TrackingManager::instance()->getHandAddress(_handButtonList[i].first,hsystem,hindex);
		if(hsystem < 0 || hindex < 0)
		{
		    _handValidList.push_back(false);
		    continue;
		}

		if(hsystem >= 0 && hsystem < TrackingManager::instance()->getNumTrackingSystems() && TrackingManager::instance()->getTrackingSystem(hsystem) && hindex >= 0 && hindex < TrackingManager::instance()->getNumBodies(hsystem))
		{
		    _handValidList.push_back(true);
		}
		else
		{
		    _handValidList.push_back(false);
		}
	    }

	    if(_debug)
	    {
		for(int i = 0; i < _handButtonList.size(); i++)
		{
		    std::cerr << "Button: " << i << " hand: " << _handButtonList[i].first << " handButton: " << _handButtonList[i].second << " valid: " << _handValidList[i] << std::endl;
		}
	    }
	}
	_handListInit = true;
    }

    osg::Matrix m = InteractionManager::instance()->getMouseMat();

    osg::Vec3 pos = m.getTrans();
    osg::Quat rot = m.getRotate();
    _mouseBody.x = pos.x();
    _mouseBody.y = pos.y();
    _mouseBody.z = pos.z();
    _mouseBody.qx = rot.x();
    _mouseBody.qy = rot.y();
    _mouseBody.qz = rot.z();
    _mouseBody.qw = rot.w();

    _mouseButtonMask = InteractionManager::instance()->getMouseButtonMask();

    int mouseWheel = InteractionManager::instance()->getMouseWheel();
    if(mouseWheel > 0)
    {
	_mouseValuator = 1.0;
    }
    else if(mouseWheel < 0)
    {
	_mouseValuator = -1.0;
    }
    else
    {
	_mouseValuator = 0.0;
    }

    if(!_handButtonList.size())
    {
	return;
    }

    std::queue<InteractionEvent *,std::list<InteractionEvent *> > tempQueue;

    //std::cerr << "Mouse queue size: " << InteractionManager::instance()->_mouseQueue.size() << std::endl;
    while(InteractionManager::instance()->_mouseQueue.size())
    {
	MouseInteractionEvent * mie = dynamic_cast<MouseInteractionEvent*>(InteractionManager::instance()->_mouseQueue.front());
	if(!mie)
	{
	    tempQueue.push(InteractionManager::instance()->_mouseQueue.front());
	    InteractionManager::instance()->_mouseQueue.pop();
	    continue;
	}

	InteractionManager::instance()->_mouseQueue.pop();

	if(mie->getButton() < 0 || mie->getButton() >= _handButtonList.size() || !_handValidList[mie->getButton()])
	{
	    tempQueue.push(mie);
	    continue;
	}

	InteractionEvent * event;

	if(TrackingManager::instance()->getHandTrackerType(_handButtonList[mie->getButton()].first) == TrackerBase::MOUSE)
	{
	    mie->setHand(_handButtonList[mie->getButton()].first);
	    mie->setButton(_handButtonList[mie->getButton()].second);
	    event = mie;
	}
	else
	{
	    TrackedButtonInteractionEvent * tie = new TrackedButtonInteractionEvent;
	    tie->setInteraction(mie->getInteraction());
	    tie->setHand(_handButtonList[mie->getButton()].first);
	    tie->setButton(_handButtonList[mie->getButton()].second);

	    int hsystem, hindex;
	    TrackingManager::instance()->getHandAddress(_handButtonList[mie->getButton()].first,hsystem,hindex);
	    if(TrackingManager::instance()->getTrackingSystem(hsystem))
	    {
		TrackerBase::TrackedBody * body = TrackingManager::instance()->getTrackingSystem(hsystem)->getBody(hindex);
		tie->setTransform(TrackingManager::instance()->getHandTransformFromTrackedBody(_handButtonList[mie->getButton()].first,body));
	    }

	    delete mie;
	    event = tie;
	}

        eventMap[event->getEventType()].push_back(event);
    }

    // put unusable events back in the mouse interaction queue so they will be cleaned up later
    while(tempQueue.size())
    {
	InteractionManager::instance()->_mouseQueue.push(tempQueue.front());
	tempQueue.pop();
    }
}
