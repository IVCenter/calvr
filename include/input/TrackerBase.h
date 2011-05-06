/**
 * @file TrackerBase.h
 */

#ifndef CALVR_TRACKER_BASE_H
#define CALVR_TRACKER_BASE_H

#include <osg/Matrix>

#define CVR_MAX_BUTTONS 32

namespace cvr
{


/**
 * @brief Internal representation of a tracked body position and rotation
 */
struct trackedBody
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
         * @brief Init function called if this system is to track 6dof bodies
         */
        virtual bool initBodyTrack() = 0;

        /**
         * @brief Init function called if this system manages buttons
         */
        virtual bool initButtonTrack() = 0;

        /**
         * @brief Get the current value of a tracked body with a given station number
         * @param station Index of tracked body
         */
        virtual trackedBody * getBody(int station) = 0;

        /**
         * @brief Get the mask representing the current button state for a button station
         *
         * Buttons start with least significant bit.  Value of 1 means the button
         * is down
         */
        virtual unsigned int getButtonMask(int station = 0) = 0;

        /**
         * @brief Get the value of a valuator
         * @param station valuator group number
         * @param index index of valuator in group
         */
        virtual float getValuator(int station, int index) = 0;

        /**
         * @brief Get the number of bodies tracked by this system
         */
        virtual int getNumBodies() = 0;

        /**
         * @brief Get the number of valuators in a station
         */
        virtual int getNumValuators(int station = 0) = 0;

        /**
         * @brief Get the number of valuator stations in this tracker
         */
        virtual int getNumValuatorStations() = 0;

        /**
         * @brief Get the number of buttons in a button station
         */
        virtual int getNumButtons(int station = 0) = 0;

        /**
         * @brief Get the number of button stations in this tracker
         */
        virtual int getNumButtonStations() = 0;

        /**
         * @brief Update body/button information 
         */
        virtual void update() = 0;

        /**
         * @brief Check if this tracker has any buttons
         */
        bool hasButtons();

        /**
         * @brief Check if this tracker has any valuators
         */
        bool hasValuators();
};

}

#endif
