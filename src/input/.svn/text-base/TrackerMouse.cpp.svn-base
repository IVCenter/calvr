#include <input/TrackerMouse.h>

#include <kernel/InteractionManager.h>

#include <osg/Quat>

using namespace cvr;

TrackerMouse::TrackerMouse() : TrackerBase()
{
}

TrackerMouse::~TrackerMouse()
{
}

bool TrackerMouse::initBodyTrack()
{
    _mouseBody.x = 0;
    _mouseBody.y = 0;
    _mouseBody.z = 0;
    osg::Quat q;
    _mouseBody.qx = q.x();
    _mouseBody.qy = q.y();
    _mouseBody.qz = q.z();
    _mouseBody.qw = q.w();

    return true;
}

bool TrackerMouse::initButtonTrack()
{
    _mouseButtonMask = 0;

    return true;
}

trackedBody * TrackerMouse::getBody(int station)
{
    if(station != 0)
    {
	return NULL;
    }

    return &_mouseBody;
}

unsigned int TrackerMouse::getButtonMask(int station)
{
    if(station != 0)
    {
	return 0;
    }

    return _mouseButtonMask;
}

float TrackerMouse::getValuator(int station, int index)
{
    return 0;
}

int TrackerMouse::getNumBodies()
{
    return 1;
}

int TrackerMouse::getNumValuators(int station)
{
    return 0;
}

int TrackerMouse::getNumValuatorStations()
{
    return 0;
}

int TrackerMouse::getNumButtons(int station)
{
    return CVR_NUM_MOUSE_BUTTONS;
}

int TrackerMouse::getNumButtonStations()
{
    return 1;
}

void TrackerMouse::update()
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
}
