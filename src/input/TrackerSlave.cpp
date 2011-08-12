#include <input/TrackerSlave.h>

#include <cstring>

using namespace cvr;

TrackerSlave::TrackerSlave(int bodies, int buttonStations, int * buttons,
                           int valStations, int * vals)
{
    _numBodies = bodies;

    for(int i = 0; i < buttonStations; i++)
    {
        _numButtons.push_back(buttons[i]);
        _buttonMaskList.push_back(0);
    }

    _totalvals = 0;
    for(int i = 0; i < valStations; i++)
    {
        _valArrayIndex.push_back(_totalvals);
        _numVals.push_back(vals[i]);
        _totalvals += vals[i];
    }

    _bodyArray = _numBodies ? new trackedBody[_numBodies] : NULL;
    _valArray = _totalvals ? new float[_totalvals] : NULL;
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

bool TrackerSlave::initBodyTrack()
{
    return true;
}

bool TrackerSlave::initButtonTrack()
{
    return true;
}

trackedBody * TrackerSlave::getBody(int station)
{
    if(station < 0 || station >= _numBodies)
    {
        return NULL;
    }

    return _bodyArray + station;
}

unsigned int TrackerSlave::getButtonMask(int station)
{
    if(station >= 0 && station < _buttonMaskList.size())
    {
        return _buttonMaskList[station];
    }
    return 0;
}

float TrackerSlave::getValuator(int station, int index)
{
    if(station < 0 || station >= _numVals.size())
    {
        return 0.0;
    }

    if(index < 0 || index >= _numVals[station])
    {
        return 0.0;
    }

    return _valArray[_valArrayIndex[station] + index];
}

int TrackerSlave::getNumBodies()
{
    return _numBodies;
}

int TrackerSlave::getNumValuators(int station = 0)
{
    if(station < 0 || station >= _numVals.size())
    {
        return 0;
    }
    return _numVals[station];
}

int TrackerSlave::getNumValuatorStations()
{
    return _numVals.size();
}

int TrackerSlave::getNumButtons(int station = 0)
{
    if(station < 0 || station >= _numButtons.size())
    {
        return 0;
    }
    return _numButtons[station];
}

int TrackerSlave::getNumButtonStations()
{
    return _numButtons.size();
}

void TrackerSlave::update()
{
}

void TrackerSlave::readValues(trackedBody * tb, unsigned int * buttons,
                              float * vals)
{
    if(tb)
    {
        memcpy(_bodyArray, tb, _numBodies * sizeof(struct trackedBody));
    }

    for(int i = 0; i < _buttonMaskList.size(); i++)
    {
        _buttonMaskList[i] = buttons[i];
    }

    if(vals)
    {
        memcpy(_valArray, vals, _totalvals * sizeof(float));
    }
}
