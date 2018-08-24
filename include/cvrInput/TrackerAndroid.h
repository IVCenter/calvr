/**
 * @file TrackerAndroid.h
 */
#ifndef TRACKER_ANDROID_H
#define TRACKER_ANDROID_H

#include <cvrInput/TrackerBase.h>

#include <vector>

#define CVR_NUM_MOUSE_BUTTONS 3

namespace cvr
{

/**
 * @addtogroup input
 * @{
 */

/**
 * @brief Tracker implementation for a mouse connected to the head node
 */
class TrackerAndroid : public cvr::TrackerBase
{
    public:
        TrackerAndroid();
        virtual ~TrackerAndroid();

        virtual bool init(std::string tag);

        virtual TrackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap);

        virtual TrackerType getTrackerType()
        {
            return MOUSE;
        }
        virtual Navigation::NavImplementation getNavImplementation()
        {
            return Navigation::MOUSE_NAV;
        }
        virtual SceneManager::PointerGraphicType getPointerType()
        {
            return SceneManager::NONE;
        }

        virtual bool thread()
        {
            return false;
        }
        virtual bool genDefaultButtonEvents()
        {
            return false;
        }

    protected:
        TrackedBody _mouseBody; ///< information representing the mouse orientation
        unsigned int _mouseButtonMask; ///< button mask for the mouse
        float _mouseValuator; ///< current value of mouse wheel valuator

        bool _handListInit;
        std::vector<std::pair<int,int> > _handButtonList;
        std::vector<bool> _handValidList;

        bool _debug;
};

/**
 * @}
 */

}

#endif
