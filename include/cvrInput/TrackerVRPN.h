/**
 * @file TrackerVRPN.h
 */
#ifndef CALVR_TRACKER_VRPN_H
#define CALVR_TRACKER_VRPN_H

#include <cvrInput/TrackerBase.h>

#include <vector>
#include <string>

#include <osg/Matrix>

namespace cvr
{

struct DeviceInfo;

/**
 * @addtogroup input
 * @{
 */

/**
 * @brief Tracker implementation that reads from a VRPN server
 */
class TrackerVRPN : public TrackerBase
{
    public:
        TrackerVRPN();
        virtual ~TrackerVRPN();

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
        int _numBodies; ///< number of tracked bodies
        int _numVal; ///< number of valuators
        int _numButtons; ///< number of buttons

        std::vector<TrackedBody *> _bodyList; ///< list of body info
        unsigned int _buttonMask; ///< current button mask
        std::vector<float> _valList; ///< list of valuator values

        struct DeviceInfo;

        DeviceInfo * _device;

};

/**
 * @}
 */

}

#endif
