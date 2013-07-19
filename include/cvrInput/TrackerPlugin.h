/**
 * @file TrackerPlugin.h
 */
//#ifndef CALVR_TRACKER_SHMEM_H
//#define CALVR_TRACKER_SHMEM_H

#include <cvrInput/TrackerBase.h>

#include <vector>

#include <osg/Matrix>

#include <stdint.h>

namespace cvr
{

/**
 * @addtogroup input
 * @{
 */

/**
 * @brief Tracker implementation for converting plugin input to a Tracker
 */
class TrackerPlugin : public TrackerBase
{
    public:
        TrackerPlugin();
        virtual ~TrackerPlugin();

        virtual bool init(std::string tag);

        virtual TrackedBody * getBody(int index);
        virtual void setBody(int index, TrackedBody * tb);
        virtual unsigned int getButtonMask();
        virtual void setButtonMask(int buttonMask);
        virtual void setButton(int buttonNum, bool state);
        virtual float getValuator(int index);
        virtual void setValuator(int index,float val);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual bool thread()
        {
            return true;
        }
        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap);
    protected:


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
        std::vector<TrackedBody *> _bodyListMem; ///< list of all tracked body info
        unsigned int _buttonMask; ///< current button mask
        unsigned int _buttonMaskMem; ///< current button mask
        std::vector<float> _valList; ///< list of all valuator values
        std::vector<float> _valListMem; ///< list of all valuator values
};

/**
 * @}
 */

}
