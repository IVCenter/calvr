/**
 * @file TrackerShmem.h
 */
#ifndef CALVR_TRACKER_SHMEM_H
#define CALVR_TRACKER_SHMEM_H

#include <cvrInput/TrackerBase.h>

#include <vector>

#include <osg/Matrix>

#ifdef WIN32
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

namespace cvr
{

/**
 * @addtogroup input
 * @{
 */

/**
 * @brief Tracker implementation for reading out of a trackd shared memory block
 */
class TrackerShmem : public TrackerBase
{
    public:
        TrackerShmem();
        virtual ~TrackerShmem();

        virtual bool init(std::string tag);

        virtual TrackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap);
    protected:

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

        int _numBodies; ///< number of bodies in block
        int _numVal; ///< number of valuators in block
        int _numButtons; ///< number of buttons in block

        std::vector<TrackedBody *> _bodyList; ///< list of all tracked body info
        unsigned int _buttonMask; ///< current button mask
        std::vector<float> _valList; ///< list of all valuator values

        control_header * _controller; ///< header for trackd controller
        tracker_header * _tracker; ///< header for trackd tracker
};

/**
 * @}
 */

}

#endif
