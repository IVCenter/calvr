/**
 * @file TrackerBase.h
 */

#ifndef CALVR_TRACKER_BASE_H
#define CALVR_TRACKER_BASE_H

#include <cvrKernel/Navigation.h>
#include <cvrKernel/SceneManager.h>

#include <osg/Matrix>

#define CVR_MAX_BUTTONS 32

namespace cvr
{

struct InteractionEvent;

/**
 * @brief Virtual base class for a tracker.
 *
 * Handles tracking of 6dof bodies and button updates.
 */
class TrackerBase
{
    public:
        TrackerBase()
        {
        }

        virtual ~TrackerBase()
        {
        }

        /**
         * @brief Internal representation of a tracked body position and rotation
         */
        struct TrackedBody
        {
                float x; ///< position x
                float y; ///< position y
                float z; ///< position z
                float qx; ///< rotation x (quat)
                float qy; ///< rotation y (quat)
                float qz; ///< rotation z (quat)
                float qw; ///< rotation w (quat)
        };

        /**
         * @brief Types for tracking systems
         */
        enum TrackerType
        {
            TRACKER = 0,
            MOUSE,
            INVALID
        };

        /**
         * @brief Initialization funtion for the tracking system
         * @param tag Base path to the tracking systems xml tag
         * @return Returns true if init went ok
         */
        virtual bool init(std::string tag) = 0;

        /**
         * @brief Get the current value of a tracked body with a given station number
         * @param index Index of tracked body
         */
        virtual TrackedBody * getBody(int index) = 0;

        /**
         * @brief Get the mask representing the current button state for a button station
         *
         * Buttons start with least significant bit.  Value of 1 means the button
         * is down
         */
        virtual unsigned int getButtonMask() = 0;

        /**
         * @brief Get the value of a valuator
         * @param index index of valuator in group
         */
        virtual float getValuator(int index) = 0;

        /**
         * @brief Get the number of bodies tracked by this system
         */
        virtual int getNumBodies() = 0;

        /**
         * @brief Get the number of valuators
         */
        virtual int getNumValuators() = 0;

        /**
         * @brief Get the number of buttons
         */
        virtual int getNumButtons() = 0;

        /**
         * @brief Update body/button information 
         * @param eventMap map to add any extra events into, indexed by cvr::InteractionEventType
         */
        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap) = 0;

        /**
         * @brief Get the type for this tracking system
         */
        virtual TrackerType getTrackerType()
        {
            return TRACKER;
        }

        /**
         * @brief Get the navigation type for this tracking system
         */
        virtual Navigation::NavImplementation getNavImplementation()
        {
            return Navigation::TRACKER_NAV;
        }

        /**
         * @brief Get the type for pointer graphic to use for this system
         */
        virtual SceneManager::PointerGraphicType getPointerType()
        {
            return SceneManager::CONE;
        }

        /**
         * @brief Get if this tracking system should be polled in a thread
         */
        virtual bool thread()
        {
            return true;
        }

        /**
         * @brief Get if default button events should be generated for this system
         */
        virtual bool genDefaultButtonEvents()
        {
            return true;
        }
};

}

#endif
