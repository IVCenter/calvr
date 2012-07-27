#ifndef CALVR_TRACKER_GYRO_MOUSE_H
#define CALVR_TRACKER_GYRO_MOUSE_H

#include <cvrInput/TrackerBase.h>

#include <vector>
#include <string>

#include <osg/Matrix>

namespace cvr
{

struct DeviceInfo;

class TrackerGyroMouse : public TrackerBase
{
    public:
	TrackerGyroMouse();
	virtual ~TrackerGyroMouse();

        virtual bool init(std::string tag);

	virtual TrackedBody * getBody(int index);
        virtual unsigned int getButtonMask();
        virtual float getValuator(int index);

        virtual int getNumBodies();
        virtual int getNumValuators();
        virtual int getNumButtons();

        virtual TrackerType getTrackerType()
        {
            return TrackerBase::POINTER;
        }

        virtual Navigation::NavImplementation getNavImplementation()
        {
            return Navigation::POINTER_NAV;
        }

        virtual SceneManager::PointerGraphicType getPointerType()
        {
            return SceneManager::POINTER;
        }

        virtual bool thread()
        {
            return true;
        }

        virtual void update(
                std::map<int,std::list<InteractionEvent*> > & eventMap);

        virtual TrackedButtonInteractionEvent * getNewBaseEvent(int body);
    protected:
	int _numBodies; ///< number of tracked bodies
        int _numVal; ///< number of valuators
        int _numButtons; ///< number of buttons

        int _hand;
        bool _withWheel;

        float _posX;
        float _posY;
        float _sensitivity;

        std::vector<TrackedBody *> _bodyList; ///< list of body info
        std::vector<osg::Matrix> _matList;
        unsigned int _buttonMask; ///< current button mask
        std::vector<float> _valList; ///< list of valuator values
        std::vector<float> _lastValList;

        struct DeviceInfo;

        DeviceInfo * _device;
};

}

#endif
