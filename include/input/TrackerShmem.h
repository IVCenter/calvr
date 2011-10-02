#ifndef CALVR_TRACKER_SHMEM_H
#define CALVR_TRACKER_SHMEM_H

#include <input/TrackerBase.h>

#include <vector>

#include <osg/Matrix>

#ifdef WIN32
typedef unsigned int uint32_t;
#else
#include <stdint.h>
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

        virtual bool init(std::string tag);

        virtual trackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual void update(std::map<int,std::list<InteractionEvent*> > & eventMap);
    protected:
        int _numBodies;
        int _numVal;
        int _numButtons;

        std::vector<trackedBody *> _bodyList;
        unsigned int _buttonMask;
        std::vector<float> _valList;

        control_header * _controller;
        tracker_header * _tracker;
};

}

#endif
