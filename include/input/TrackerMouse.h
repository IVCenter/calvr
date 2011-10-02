#ifndef TRACKER_MOUSE_H
#define TRACKER_MOUSE_H

#include <input/TrackerBase.h>

#define CVR_NUM_MOUSE_BUTTONS 3

namespace cvr
{

class TrackerMouse : public cvr::TrackerBase
{
    public:
        TrackerMouse();
        virtual ~TrackerMouse();

        virtual bool init(std::string tag);

        virtual trackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual void update(std::map<int,std::list<InteractionEvent*> > & eventMap);

        virtual TrackerType getTrackerType() { return MOUSE; }
        virtual Navigation::NavImplementation getNavImplementation() { return Navigation::MOUSE_NAV; }
        virtual SceneManager::PointerGraphicType getPointerType() { return SceneManager::NONE; }

        virtual bool thread() { return false; }
        virtual bool genDefaultButtonEvents() { return false; }

    protected:
        trackedBody _mouseBody;
        unsigned int _mouseButtonMask;
};

}

#endif
