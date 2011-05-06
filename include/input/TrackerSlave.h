#ifndef CALVR_TRACKER_SLAVE_H
#define CALVR_TRACKER_SLAVE_H

#include <input/TrackerBase.h>

#include <vector>

namespace cvr
{

class TrackerSlave : public TrackerBase
{
    public:
        TrackerSlave(int bodies, int buttonStations, int * buttons,
                     int valStations, int * vals);
        ~TrackerSlave();

        virtual bool initBodyTrack();
        virtual bool initButtonTrack();

        virtual trackedBody * getBody(int station);
        virtual unsigned int getButtonMask(int station);
        virtual float getValuator(int station, int index);

        virtual int getNumBodies();
        virtual int getNumValuators(int station);
        virtual int getNumValuatorStations();
        virtual int getNumButtons(int station);
        virtual int getNumButtonStations();

        virtual void update();

        void readValues(trackedBody * tb, unsigned int * buttons, float * vals);
    protected:
        int _numBodies;
        std::vector<int> _numButtons;
        std::vector<int> _numVals;

        trackedBody * _bodyArray;
        std::vector<unsigned int> _buttonMaskList;
        std::vector<int> _valArrayIndex;
        int _totalvals;
        float * _valArray;
};

}

#endif
