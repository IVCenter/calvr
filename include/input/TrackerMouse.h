#ifndef TRACKER_MOUSE_H
#define TRACKER_MOUSE_H

#include <input/TrackerBase.h>

#define CVR_NUM_MOUSE_BUTTONS 3

namespace cvr
{

class TrackerMouse : public cvr::TrackerBase
{
    public:
        TrackerMouse();
        virtual ~TrackerMouse();

        virtual bool initBodyTrack();
        virtual bool initButtonTrack();

        virtual trackedBody * getBody(int station);
        virtual unsigned int getButtonMask(int station = 0);
        virtual float getValuator(int station, int index);

        virtual int getNumBodies();
        virtual int getNumValuators(int station = 0);
        virtual int getNumValuatorStations();
        virtual int getNumButtons(int station = 0);
        virtual int getNumButtonStations();

        virtual void update();

    protected:
        trackedBody _mouseBody;
        unsigned int _mouseButtonMask;
};

}

#endif
