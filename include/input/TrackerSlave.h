#ifndef CALVR_TRACKER_SLAVE_H
#define CALVR_TRACKER_SLAVE_H

#include <input/TrackerBase.h>

#include <vector>

namespace cvr
{

class TrackerSlave : public TrackerBase
{
    public:
        TrackerSlave(int bodies, int buttons, int vals);
        virtual ~TrackerSlave();

        virtual bool init(std::string tag);

        virtual trackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual void update(std::map<int,std::list<InteractionEvent*> > & eventMap);

        void readValues(trackedBody * tb, unsigned int * buttons, float * vals);
    protected:
        int _numBodies;
        int _numButtons;
        int _numVals;

        trackedBody * _bodyArray;
        unsigned int _buttonMask;
        float * _valArray;
};

}

#endif
