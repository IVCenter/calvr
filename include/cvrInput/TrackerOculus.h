/**
 * @file TrackerOculus.h
 */
#ifndef CALVR_TRACKER_OCULUS_H
#define CALVR_TRACKER_OCULUS_H

#include <cvrInput/TrackerBase.h>

#include <OVR_Version.h>
#include <OVR_CAPI.h>

namespace cvr
{

/**
 * @addtogroup input
 * @{
 */

/**
 * @brief Tracker implementation for Oculus rift
 */
class TrackerOculus : public TrackerBase
{
    public:
        TrackerOculus();
        virtual ~TrackerOculus();

        virtual bool init(std::string tag);

        virtual TrackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap);

		static ovrHmdDesc getHMD()
		{
			return _hmd;
		}
		
		static ovrSession getSession()
		{
			return _session;
		}

		static bool isInit()
		{
			return _init;
		}

    protected:

        TrackedBody _body; ///< head body info

		static ovrHmdDesc _hmd;
		static ovrSession _session;
		static bool _init;
};

/**
 * @}
 */

}

#endif