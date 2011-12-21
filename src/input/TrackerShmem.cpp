#include <input/TrackerShmem.h>

#include <config/ConfigManager.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include <osg/Vec3>

#if defined(__linux__) || defined(__APPLE__)
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
    _numButtons = 0;
    _numVal = 0;

    _controller = NULL;
    _tracker = NULL;

    _buttonMask = 0;
}

TrackerShmem::~TrackerShmem()
{
}

bool TrackerShmem::init(std::string tag)
{
    int shmKey = ConfigManager::getInt(tag + ".SHMEM.TrackerID",4126);

#ifndef WIN32
    int shmID = shmget(shmKey,sizeof(struct tracker_header),0);
    if(shmID == -1)
    {
    }
    else
    {
        _tracker = (tracker_header *)shmat(shmID,NULL,0);
        if(_tracker == (tracker_header *)-1)
        {
            _tracker = NULL;
        }
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
        _tracker = NULL;
    }

#endif

    if(_tracker)
    {
        _numBodies = _tracker->count;
        for(int i = 0; i < _numBodies; i++)
        {
            TrackedBody * tb = new TrackedBody;
            tb->x = tb->y = tb->z = tb->qx = tb->qy = tb->qz = tb->qw = 0;
            _bodyList.push_back(tb);
        }
    }

    shmKey = ConfigManager::getInt(tag + ".SHMEM.ControllerID",4127);

#ifndef WIN32
    shmID = shmget(shmKey,sizeof(struct control_header),0);
    if(shmID == -1)
    {
    }
    else
    {
        _controller = (control_header *)shmat(shmID,NULL,0);
        if(_controller == (control_header *)-1)
        {
            _controller = NULL;
        }
    }

#else
    cKey = std::stringstream();
    cKey << shmKey;
    shmID = OpenFileMapping(FILE_MAP_WRITE, FALSE, cKey.str().c_str());
    if(shmID)
    {
        _controller = (struct control_header *) MapViewOfFile(shmID, FILE_MAP_WRITE, 0, 0, 0);
    }
    else
    {
        _controller = NULL;
    }
#endif

    if(_controller)
    {
#ifndef WIN32
        _numButtons = std::min(_controller->but_count,
                (uint32_t)CVR_MAX_BUTTONS);
#else
        _numButtons = min(_controller->but_count,(uint32_t)CVR_MAX_BUTTONS);
#endif

        _numVal = _controller->val_count;
        for(int i = 0; i < _numVal; i++)
        {
            _valList.push_back(0.0);
        }
    }

    _buttonMask = 0;

    return true;
}

TrackerBase::TrackedBody * TrackerShmem::getBody(int index)
{
    if(index < 0 || index >= _numBodies)
    {
        return NULL;
    }

    return _bodyList[index];
}

unsigned int TrackerShmem::getButtonMask()
{
    return _buttonMask;
}

float TrackerShmem::getValuator(int index)
{
    if(index < 0 || index >= _numVal)
    {
        return 0.0;
    }

    return _valList[index];
}

int TrackerShmem::getNumBodies()
{
    return _numBodies;
}

int TrackerShmem::getNumValuators()
{
    return _numVal;
}

int TrackerShmem::getNumButtons()
{
    return _numButtons;
}

void TrackerShmem::update(
        std::map<int,std::list<InteractionEvent*> > & eventMap)
{
    if(_numButtons)
    {
        int * buttonStart = (int *)(((char *)_controller)
                + _controller->but_offset);
        _buttonMask = 0;
        for(int i = 0; i < _numButtons; i++)
        {
            _buttonMask = _buttonMask | (buttonStart[i] << i);
        }
    }

    if(_numVal)
    {
        float * valStart = (float *)(((char *)_controller)
                + _controller->val_offset);
        for(int i = 0; i < _numVal; i++)
        {
            _valList[i] = valStart[i];
        }
    }

    static const float deg2rad = (float)M_PI / 180.0f;

    for(int i = 0; i < _numBodies; i++)
    {
        sensor * sen = (sensor*)(((char*)_tracker) + _tracker->offset
                + (_tracker->size * i));
        _bodyList[i]->x = sen->p[0] * FEET_TO_MM;
        _bodyList[i]->y = sen->p[1] * FEET_TO_MM;
        _bodyList[i]->z = sen->p[2] * FEET_TO_MM;
        osg::Quat q = osg::Quat(sen->r[2] * deg2rad,osg::Vec3(0,0,1),
                sen->r[1] * deg2rad,osg::Vec3(1,0,0),sen->r[0] * deg2rad,
                osg::Vec3(0,1,0));
        _bodyList[i]->qx = q.x();
        _bodyList[i]->qy = q.y();
        _bodyList[i]->qz = q.z();
        _bodyList[i]->qw = q.w();
    }
}
