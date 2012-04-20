/**
 * @file TrackerMouse.h
 */
#ifndef TRACKER_MOUSE_H
#define TRACKER_MOUSE_H

#include <cvrInput/TrackerBase.h>

#define CVR_NUM_MOUSE_BUTTONS 3

namespace cvr
{

/**
 * @brief Tracker implementation for a mouse connected to the head node
 */
class TrackerMouse : public cvr::TrackerBase
{
    public:
        TrackerMouse();
        virtual ~TrackerMouse();

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
        float _mouseValuator;
};

}

#endif
