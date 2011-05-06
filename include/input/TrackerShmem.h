#ifndef CALVR_TRACKER_SHMEM_H
#define CALVR_TRACKER_SHMEM_H

#include <input/TrackerBase.h>

#include <vector>

#include <osg/Matrix>

#ifdef WIN32
typedef unsigned int uint32_t;
#endif

namespace cvr
{

struct tracker_header
{
        uint32_t version;
        uint32_t count;
        uint32_t offset;
        uint32_t size;
        uint32_t time[2];
        uint32_t command;
};

struct control_header
{
        uint32_t version;
        uint32_t but_offset;
        uint32_t val_offset;
        uint32_t but_count;
        uint32_t val_count;
        uint32_t time[2];
        uint32_t command;
};

struct sensor
{
        float p[3];
        float r[3];
        uint32_t t[2];
        uint32_t calib;
        uint32_t frame;
};

class TrackerShmem : public TrackerBase
{
    public:
        TrackerShmem();
        virtual ~TrackerShmem();

        virtual bool initBodyTrack();
        virtual bool initButtonTrack();

        virtual trackedBody * getBody(int station);
        virtual unsigned int getButtonMask(int station);
        virtual float getValuator(int station, int index);

        virtual int getNumBodies();
        virtual int getNumValuators(int station = 0);
        virtual int getNumValuatorStations();
        virtual int getNumButtons(int station = 0);
        virtual int getNumButtonStations();

        virtual void update();
    protected:
        int _numBodies;
        std::vector<int> _numVal;
        std::vector<int> _numButtons;

        std::vector<trackedBody *> _bodyList;
        std::vector<unsigned int> _buttonMaskList;
        std::vector<std::vector<float> > _valList;

        control_header * _controller;
        tracker_header * _tracker;
};

}

#endif
