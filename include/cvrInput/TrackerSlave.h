/**
 * @file TrackerSlave.h
 */
#ifndef CALVR_TRACKER_SLAVE_H
#define CALVR_TRACKER_SLAVE_H

#include <cvrInput/TrackerBase.h>

#include <vector>

namespace cvr
{

/**
 * @addtogroup input
 * @{
 */

/**
 * @brief Tracker that runs on the slave nodes
 */
class TrackerSlave : public TrackerBase
{
    public:
        TrackerSlave(int bodies, int buttons, int vals);
        virtual ~TrackerSlave();

        virtual bool init(std::string tag);

        virtual TrackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap);

        void readValues(TrackedBody * tb, unsigned int * buttons, float * vals);
    protected:
        int _numBodies; ///< number of bodies
        int _numButtons; ///< number of buttons
        int _numVals; ///< number of valuators

        TrackedBody * _bodyArray; ///< array of tracked body info
        unsigned int _buttonMask; ///< current button mask
        float * _valArray; ///< array of valuator values
};

/**
 * @}
 */

}

#endif
