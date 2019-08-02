/**
 * @file TrackerOpenVR.h
 */
#ifndef CALVR_TRACKER_OCULUS_H
#define CALVR_TRACKER_OCULUS_H

#include <cvrInput/TrackerBase.h>
#include <cvrKernel/ScreenOpenVR.h>
#include <openvr.h>
#include <openvrdevice.h>


namespace cvr
{

/**
 * @addtogroup input
 * @{
 */

/**
 * @brief Tracker implementation for Oculus rift
 */
class TrackerOpenVR : public TrackerBase
{
    public:
        TrackerOpenVR();
        virtual ~TrackerOpenVR();

        virtual bool init(std::string tag);

        virtual TrackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap);

		static OpenVRDevice* getDevice()
		{
			return _device;
		}
		
		static bool isInit()
		{
			return _init;
		}

    protected:

        std::vector<TrackedBody> _bodies;
		std::vector<float> _valuators;
		unsigned int _buttonMask;

		bool _head;
		int _numControllers;
		int _numTrackers;
		int _numBodies; ///< number of tracked bodies
		int _numVal; ///< number of valuators
		int _numButtons; ///< number of buttons


		static OpenVRDevice * _device;
		static bool _init;
};

/**
 * @}
 */

}

#endif