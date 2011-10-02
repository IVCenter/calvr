#include <input/TrackerMouse.h>
#include <kernel/InteractionManager.h>

#include <osg/Quat>

#include <iostream>

using namespace cvr;

TrackerMouse::TrackerMouse() : TrackerBase()
{
}

TrackerMouse::~TrackerMouse()
{
}

bool TrackerMouse::init(std::string tag)
{
    _mouseBody.x = 0;
    _mouseBody.y = 0;
    _mouseBody.z = 0;
    osg::Quat q;
    _mouseBody.qx = q.x();
    _mouseBody.qy = q.y();
    _mouseBody.qz = q.z();
    _mouseBody.qw = q.w();

    _mouseButtonMask = 0;

    return true;
}

trackedBody * TrackerMouse::getBody(int index)
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
    return 0;
}

int TrackerMouse::getNumBodies()
{
    return 1;
}

int TrackerMouse::getNumValuators()
{
    return 0;
}

int TrackerMouse::getNumButtons()
{
    return CVR_NUM_MOUSE_BUTTONS;
}

void TrackerMouse::update(std::map<int,std::list<InteractionEvent*> > & eventMap)
{
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

    //std::cerr << "Mouse queue size: " << InteractionManager::instance()->_mouseQueue.size() << std::endl;
    while(InteractionManager::instance()->_mouseQueue.size())
    {
	eventMap[InteractionManager::instance()->_mouseQueue.front()->getEventType()].push_back(InteractionManager::instance()->_mouseQueue.front());
	InteractionManager::instance()->_mouseQueue.pop();
    }
}
