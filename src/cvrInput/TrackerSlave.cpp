#include <cvrInput/TrackerSlave.h>

#include <cstring>

using namespace cvr;

TrackerSlave::TrackerSlave(int bodies, int buttons, int vals)
{
    _numBodies = bodies;
    _numButtons = buttons;
    _numVals = vals;

    _buttonMask = 0;

    _bodyArray = _numBodies ? new TrackedBody[_numBodies] : NULL;
    _valArray = _numVals ? new float[_numVals] : NULL;
}

TrackerSlave::~TrackerSlave()
{
    if(_bodyArray)
    {
        delete[] _bodyArray;
    }
    if(_valArray)
    {
        delete[] _valArray;
    }
}

bool TrackerSlave::init(std::string tag)
{
    return true;
}

TrackerBase::TrackedBody * TrackerSlave::getBody(int index)
{
    if(index < 0 || index >= _numBodies)
    {
        return NULL;
    }

    return _bodyArray + index;
}

unsigned int TrackerSlave::getButtonMask()
{
    return _buttonMask;
}

float TrackerSlave::getValuator(int index)
{
    if(index < 0 || index >= _numVals)
    {
        return 0.0;
    }

    return _valArray[index];
}

int TrackerSlave::getNumBodies()
{
    return _numBodies;
}

int TrackerSlave::getNumValuators()
{
    return _numVals;
}

int TrackerSlave::getNumButtons()
{
    return _numButtons;
}

void TrackerSlave::update(
        std::map<int,std::list<InteractionEvent*> > & eventMap)
{
}

void TrackerSlave::readValues(TrackedBody * tb, unsigned int * buttons,
        float * vals)
{
    if(tb)
    {
        memcpy(_bodyArray,tb,_numBodies * sizeof(struct TrackedBody));
    }

    _buttonMask = *buttons;

    if(vals)
    {
        memcpy(_valArray,vals,_numVals * sizeof(float));
    }
}
