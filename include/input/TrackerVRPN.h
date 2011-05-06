#ifndef CALVR_TRACKER_VRPN_H
#define CALVR_TRACKER_VRPN_H

#include <input/TrackerBase.h>

#include <vector>
#include <string>

#include <osg/Matrix>

namespace cvr
{

struct DeviceInfo;

class TrackerVRPN : public TrackerBase
{
    public:
        TrackerVRPN();
        virtual ~TrackerVRPN();

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
        int _numVal;
        int _numButtons;

        std::vector<trackedBody *> _bodyList;
        unsigned int _buttonMask;
        std::vector<float> _valList;

        DeviceInfo * _device;

};

}

#endif
