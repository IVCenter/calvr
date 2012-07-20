/**
 * @file TrackeriOmnicron.h
 */
#ifndef CALVR_TRACKER_OMICRON_H
#define CALVR_TRACKER_OMICRON_H

#include <cvrInput/TrackerBase.h>

#include <connector/omicronConnectorClient.h>

#include <vector>
#include <map>

namespace cvr
{

/**
 * @addtogroup input
 * @{
 */

/**
 * @brief Tracker that runs on the slave nodes
 */
class TrackerOmicron : public TrackerBase, public omicronConnector::IOmicronConnectorClientListener
{
    public:
        TrackerOmicron();
        virtual ~TrackerOmicron();

        virtual bool init(std::string tag);

        virtual TrackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual bool thread()
        {
            return true;
        }

        virtual TrackerType getTrackerType()
        {
            return TRACKER;
        }

        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap);

        virtual void onEvent(const omicronConnector::EventData& e);
    protected:
        omicronConnector::OmicronConnectorClient * _client;

        int _numBodies; ///< number of bodies
        int _numButtons; ///< number of buttons
        int _numVals; ///< number of valuators

        std::map<unsigned int,std::vector<TrackedBody*> > _bodyMap;

        unsigned int _buttonMask; ///< current button mask
        float * _valArray; ///< array of valuator values
};

/**
 * @}
 */

}

#endif
