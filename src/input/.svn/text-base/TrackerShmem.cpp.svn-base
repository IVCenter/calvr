#include <input/TrackerShmem.h>

#include <config/ConfigManager.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include <osg/Vec3>

#ifdef __linux__
#include <sys/shm.h>
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define M_PI 3.141592653589793238462643
#endif

#define FEET_TO_MM 304.8

using namespace cvr;

TrackerShmem::TrackerShmem()
{
    _numBodies = 0;

    _controller = NULL;
    _tracker = NULL;
}

TrackerShmem::~TrackerShmem()
{
}

bool TrackerShmem::initBodyTrack()
{
    int shmKey =
            ConfigManager::getInt("Input.CaveLibConfig.TrackerSHMID", 4126);

#ifndef WIN32
    int shmID = shmget(shmKey, sizeof(struct tracker_header), 0);
    if(shmID == -1)
    {
        std::cerr << "Unable to get Shared memory id for key: " << shmKey
                << std::endl;
        return false;
    }

    _tracker = (tracker_header *)shmat(shmID, NULL, 0);
    if(_tracker == (tracker_header *)-1)
    {
        std::cerr << "Unable to attach to memory id: " << shmID << std::endl;
        return false;
    }

#else
    HANDLE shmID;
    std::stringstream cKey;
    cKey << shmKey;
    shmID = OpenFileMapping(FILE_MAP_WRITE, FALSE, cKey.str().c_str());
    if(shmID)
    {
        _tracker = (struct tracker_header *) MapViewOfFile(shmID, FILE_MAP_WRITE, 0, 0, 0);
    }
    else
    {
        std::cerr << "Unable to attach to memory key: " << shmKey << std::endl;
        return false;
    }

#endif

    _numBodies = _tracker->count;
    for(int i = 0; i < _numBodies; i++)
    {
        trackedBody * tb = new trackedBody;
        tb->x = tb->y = tb->z = tb->qx = tb->qy = tb->qz = tb->qw = 0;
        _bodyList.push_back(tb);
    }

    return true;
}

bool TrackerShmem::initButtonTrack()
{
    int shmKey = ConfigManager::getInt("Input.CaveLibConfig.WandSHMID", 4127);

#ifndef WIN32
    int shmID = shmget(shmKey, sizeof(struct control_header), 0);
    if(shmID == -1)
    {
        std::cerr << "Unable to get Shared memory id for key: " << shmKey
                << std::endl;
        return false;
    }

    _controller = (control_header *)shmat(shmID, NULL, 0);
    if(_controller == (control_header *)-1)
    {
        std::cerr << "Unable to attach to memory id: " << shmID << std::endl;
        return false;
    }

#else
    HANDLE shmID;
    std::stringstream cKey;
    cKey << shmKey;
    shmID = OpenFileMapping(FILE_MAP_WRITE, FALSE, cKey.str().c_str());
    if(shmID)
    {
        _controller = (struct control_header *) MapViewOfFile(shmID, FILE_MAP_WRITE, 0, 0, 0);
    }
    else
    {
        std::cerr << "Unable to attach to memory key: " << shmKey << std::endl;
        return false;
    }
#endif

#ifndef WIN32
    _numButtons.push_back(std::min(_controller->but_count,
                                   (uint32_t)CVR_MAX_BUTTONS));
#else
    _numButtons.push_back(min(_controller->but_count,(uint32_t)CVR_MAX_BUTTONS));
#endif

    _buttonMaskList.push_back(0);

    _numVal.push_back(_controller->val_count);
    _valList.push_back(std::vector<float>());
    for(int i = 0; i < _numVal[0]; i++)
    {
        _valList[0].push_back(0.0);
    }

    return true;
}

trackedBody * TrackerShmem::getBody(int station)
{
    if(station < 0 || station >= _numBodies)
    {
        return NULL;
    }

    return _bodyList[station];
}

unsigned int TrackerShmem::getButtonMask(int station)
{
    if(station < 0 || station >= _buttonMaskList.size())
    {
        return 0;
    }
    return _buttonMaskList[station];
}

float TrackerShmem::getValuator(int station, int index)
{
    if(station < 0 || station >= _numVal.size())
    {
        return 0.0;
    }

    if(index < 0 || index >= _numVal[station])
    {
        return 0.0;
    }

    return _valList[station][index];
}

int TrackerShmem::getNumBodies()
{
    return _numBodies;
}

int TrackerShmem::getNumValuators(int station)
{
    if(station >= 0 && station < _numButtons.size())
    {
        return _numVal[station];
    }
    return 0;
}

int TrackerShmem::getNumValuatorStations()
{
    return _numVal.size();
}

int TrackerShmem::getNumButtons(int station)
{
    if(station >= 0 && station < _numButtons.size())
    {
        return _numButtons[station];
    }
    return 0;
}

int TrackerShmem::getNumButtonStations()
{
    return _numButtons.size();
}

void TrackerShmem::update()
{
    if(_buttonMaskList.size())
    {
        for(int j = 0; j < _buttonMaskList.size(); j++)
        {
            int * buttonStart = (int *)(((char *)_controller)
                    + _controller->but_offset);
            _buttonMaskList[j] = 0;
            for(int i = 0; i < _numButtons[j]; i++)
            {
                _buttonMaskList[j] = _buttonMaskList[j] | (buttonStart[i] << i);
            }
        }
    }

    if(_numVal.size())
    {
        for(int j = 0; j < _numVal.size(); j++)
        {
            float * valStart = (float *)(((char *)_controller)
                    + _controller->val_offset);
            for(int i = 0; i < _numVal[j]; i++)
            {
                _valList[j][i] = valStart[i];
            }
        }
    }

    static const float deg2rad = M_PI / 180.0;

    for(int i = 0; i < _numBodies; i++)
    {
        sensor * sen = (sensor*)(((char*)_tracker) + _tracker->offset
                + (_tracker->size * i));
        _bodyList[i]->x = sen->p[0] * FEET_TO_MM;
        _bodyList[i]->y = sen->p[1] * FEET_TO_MM;
        _bodyList[i]->z = sen->p[2] * FEET_TO_MM;
        osg::Quat q = osg::Quat(sen->r[2] * deg2rad, osg::Vec3(0, 0, 1),
                                sen->r[1] * deg2rad, osg::Vec3(1, 0, 0),
                                sen->r[0] * deg2rad, osg::Vec3(0, 1, 0));
        //_bodyList[i]->h = sen->r[0];
        //_bodyList[i]->p = sen->r[1];
        //_bodyList[i]->r = sen->r[2];
        _bodyList[i]->qx = q.x();
        _bodyList[i]->qy = q.y();
        _bodyList[i]->qz = q.z();
        _bodyList[i]->qw = q.w();
    }
}
