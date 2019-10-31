#include <cvrInput/TrackerOpenVR.h>
#include <cvrConfig/ConfigManager.h>
#include <iostream>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

TrackerOpenVR::TrackerOpenVR()
{
	_bodies = std::vector<TrackedBody>();
	_valuators = std::vector<float>();
}

TrackerOpenVR::~TrackerOpenVR()
{
	if (_init)
	{
	}
}

bool TrackerOpenVR::init(std::string tag)
{
	if (!OpenVRDevice::instance())
	{
		_device = new OpenVRDevice(0.1f, 1000.0f, 1000.0f, 4);
	}
	else
	{
		_device = OpenVRDevice::instance();
	}

	if(!_device->hmdInitialized())
	{
		return false;
	}
	_device->init();

	_head = ConfigManager::getBool("value", tag + ".TrackHead",false);
	_numControllers = ConfigManager::getInt("value",tag + ".NumControllers",0);
	_numTrackers = ConfigManager::getInt("value",tag + ".NumTrackers",0);
	//std::cout << head << ", " << _numControllers << ", " << _numTrackers << std::endl;
    _numBodies = _head + _numControllers + _numTrackers;
    _numButtons = _numControllers * 4;
    _numVal = _numControllers * 3;

	_device->numControllers = _numControllers;
	_device->numTrackers = _numTrackers;

	for(int i = 0; i < _numBodies; ++i)
	{
		TrackedBody body;
		body.x = 0.0;
		body.y = 0.0;
		body.z = 0.0;
		osg::Quat q;
		body.qx = q.x();
		body.qy = q.y();
		body.qz = q.z();
		body.qw = q.w();
		_bodies.push_back(body);
	}

	for (int i = 0; i < _numVal; ++i)
	{
		_valuators.push_back(0);
	}

	_init = true;

	return true;
}

TrackerBase::TrackedBody * TrackerOpenVR::getBody(int index)
{
	if(!_init || index < 0 || index >= _numBodies)
	{
		return NULL;
	}
	return &(_bodies[index]);
}

unsigned int TrackerOpenVR::getButtonMask()
{
	return _buttonMask;
}

float TrackerOpenVR::getValuator(int index)
{
	if(!_init || index < 0 || index >= _numVal)
	{
		return 0;
	}
	return _valuators[index];
}

int TrackerOpenVR::getNumBodies()
{
	return _numBodies;
}

int TrackerOpenVR::getNumValuators()
{
	return _numVal;
}

int TrackerOpenVR::getNumButtons()
{
	return _numButtons;
}

void TrackerOpenVR::update(std::map<int,std::list<InteractionEvent*> > & eventMap)
{
	if(!_init)
	{
		return;
	}

	if (!ScreenOpenVR::isInit()) {
		_device->updatePose();
	}
	if (_head)
	{
		TrackedBody* body = &(_bodies[0]);

		//osg::Matrix m = osg::Matrix();
		//m.makeRotate(_device->orientation());
		//m.postMultTranslate(_device->position());
		//m = osg::Matrix::inverse(m); //Need to invert view matrix
		osg::Vec3 pos = _device->position(); // m.getTrans();
		osg::Quat rot = _device->orientation(); // m.getRotate();

		//std::cout << pos.x() << ", " << pos.y() << ", " << pos.z() << std::endl;

		body->x = pos.x();
		body->y = -pos.z();
		body->z = pos.y();
		body->qx = rot.x();
		body->qy = -rot.z();
		body->qz = rot.y();
		body->qw = rot.w();
	}
	if (_numControllers)
	{
		int initial = (int)_head;
		for (int i = 0; i < _numControllers; ++i)
		{
			OpenVRDevice::ControllerData controller = _device->controllers[i];
			TrackedBody* body = &(_bodies[initial + i]);
			body->x = controller.position.x();
			body->y = -controller.position.z();
			body->z = controller.position.y();
			body->qx = controller.rotation.x();
			body->qy = -controller.rotation.z();
			body->qz = controller.rotation.y();
			body->qw = controller.rotation.w();
		}
	}
	if (_numTrackers)
	{
		int initial = (int)_head + _numControllers;
		for (int i = 0; i < _numTrackers; ++i)
		{
			OpenVRDevice::TrackerData tracker = _device->trackers[i];
			TrackedBody* body = &(_bodies[initial + i]);
			body->x = tracker.position.x();
			body->y = -tracker.position.z();
			body->z = tracker.position.y();
			body->qx = tracker.rotation.x();
			body->qy = -tracker.rotation.z();
			body->qz = tracker.rotation.y();
			body->qw = tracker.rotation.w();
		}
	}

	if (_numControllers)
	{
		_device->updateControllerEvents();
		std::vector<bool> bits = std::vector<bool>();
		for (int i = 0; i < _numControllers; ++i)
		{
			OpenVRDevice::ControllerData controller = _device->controllers[i];
			bits.push_back(controller.menuPressed);
			bits.push_back(controller.gripPressed);
			bits.push_back(controller.padPressed);
			bits.push_back(controller.triggerPressed);

			_valuators[i * 3] = controller.trigVal;
			_valuators[i * 3 + 1] = controller.padX;
			_valuators[i * 3 + 2] = controller.padY;
		}

		_buttonMask = 0;
		for (int i = 0; i < bits.size(); ++i)
		{
			_buttonMask = _buttonMask | ((unsigned int)bits[i] << i);
		}

	}
}