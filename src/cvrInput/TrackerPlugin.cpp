#include <cvrInput/TrackerPlugin.h>

#include <cvrConfig/ConfigManager.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include <osg/Vec3>

using namespace cvr;
using namespace osg;
using namespace std;

TrackerPlugin::TrackerPlugin()
{
    _numBodies = 0;
    _numButtons = 0;
    _numVal = 0;
    //

    _buttonMask = 0;
}

TrackerPlugin::~TrackerPlugin()
{
}

bool TrackerPlugin::init(std::string tag)
{
    _numBodies = 12;
    for(int i = 0; i < _numBodies; i++)
    {
        TrackedBody * tb = new TrackedBody;
        tb->x = tb->y = tb->z = 0.0;
        osg::Quat q;
        tb->qx = q.x();
        tb->qy = q.y();
        tb->qz = q.z();
        tb->qw = q.w();
        _bodyList.push_back(tb);
        _bodyListMem.push_back(tb);
    }

    _numButtons = 2;

    _numVal = 0;
    for(int i = 0; i < _numVal; i++)
    {
        _valList.push_back(0.0);
        _valListMem.push_back(0.0);
    }

    _buttonMask = 0;
    _buttonMaskMem = 0;

    return true;
}

TrackerBase::TrackedBody * TrackerPlugin::getBody(int index)
{
    if(index < 0 || index >= _numBodies)
    {
        return NULL;
    }

    return _bodyList[index];
}
void TrackerPlugin::setBody(int index, TrackedBody * tb)
{
    if(index < 0 || index >= _numBodies)
    {

    }
    else
    {

        *_bodyListMem[index] = *tb;
    }
}

unsigned int TrackerPlugin::getButtonMask()
{
    return _buttonMask;
}

void TrackerPlugin::setButtonMask(int buttonMask)
{
    _buttonMaskMem = buttonMask;
}
void TrackerPlugin::setButton(int buttonNum, bool state)
{
    unsigned int buttonMask = 1 << buttonNum;
    if(state)
        _buttonMaskMem |= buttonMask;
    else
        _buttonMaskMem &= (~buttonMask);

}
float TrackerPlugin::getValuator(int index)
{
    if(index < 0 || index >= _numVal)
    {
        return 0.0;
    }

    return _valList[index];
}

void TrackerPlugin::setValuator(int index, float val)
{
    _valListMem[index] = val;

}

int TrackerPlugin::getNumBodies()
{
    return _numBodies;
}

int TrackerPlugin::getNumValuators()
{
    return _numVal;
}

int TrackerPlugin::getNumButtons()
{
    return _numButtons;
}

void TrackerPlugin::update(
        std::map<int,std::list<InteractionEvent*> > & eventMap)
{
    if(_numButtons)
    {
        _buttonMask = _buttonMaskMem;
    }

    if(_numVal)
    {
        for(int i = 0; i < _numVal; i++)
        {
            _valList[i] = _valListMem[i];
        }
    }

    for(int i = 0; i < _numBodies; i++)
    {
        _bodyList[i]->x = _bodyListMem[i]->x;
        _bodyList[i]->y = _bodyListMem[i]->y;
        _bodyList[i]->z = _bodyListMem[i]->z;

        _bodyList[i]->qx = _bodyListMem[i]->qx;
        _bodyList[i]->qy = _bodyListMem[i]->qy;
        _bodyList[i]->qz = _bodyListMem[i]->qz;
        _bodyList[i]->qw = _bodyListMem[i]->qw;

    }
}
