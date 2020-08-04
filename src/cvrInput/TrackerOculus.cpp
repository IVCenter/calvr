#include <cvrInput/TrackerOculus.h>

#include <iostream>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

ovrHmdDesc TrackerOculus::_hmd;
ovrSession TrackerOculus::_session;
bool TrackerOculus::_init = false;

TrackerOculus::TrackerOculus()
{
}

TrackerOculus::~TrackerOculus()
{
	if (_init)
	{
		ovr_Destroy(_session);
		ovr_Shutdown();
	}
}

bool TrackerOculus::init(std::string tag)
{
	ovrResult result = ovr_Initialize(nullptr);
	if (OVR_FAILURE(result))
	{
		std::cerr << "Error: ovr_Initialized" << std::endl;
		return false;
	}

	ovrGraphicsLuid luid;
	result = ovr_Create(&_session, &luid);
	if (OVR_FAILURE(result))
	{
		std::cerr << "Error: ovr_Create" << std::endl;
		ovr_Shutdown();
		return false;
	}

	_hmd = ovr_GetHmdDesc(_session);

	_body.x = 0.0;
	_body.y = 0.0;
	_body.z = 0.0;
	osg::Quat q;
	_body.qx = q.x();
	_body.qy = q.y();
	_body.qz = q.z();
	_body.qw = q.w();

	_init = true;

	return true;
}

TrackerBase::TrackedBody * TrackerOculus::getBody(int index)
{
	if(!_init || index != 0)
	{
		return NULL;
	}
	return &_body;
}

unsigned int TrackerOculus::getButtonMask()
{
	return 0;
}

float TrackerOculus::getValuator(int index)
{
	return 0.0;
}

int TrackerOculus::getNumBodies()
{
	if(_init)
	{
		return 1;
	}
	return 0;
}

int TrackerOculus::getNumValuators()
{
	return 0;
}

int TrackerOculus::getNumButtons()
{
	return 0;
}

void TrackerOculus::update(std::map<int,std::list<InteractionEvent*> > & eventMap)
{
	if(!_init)
	{
		return;
	}

	ovrTrackingState ts = ovr_GetTrackingState(_session, ovr_GetTimeInSeconds(),false);

	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
#if OVR_PRODUCT_VERSION < 1
		_body.x = (ts.HeadPose.ThePose.Position.x - ts.LeveledCameraPose.Position.x) * 1000.0;
		_body.y = (ts.HeadPose.ThePose.Position.y - ts.LeveledCameraPose.Position.y) * 1000.0;
		_body.z = (ts.HeadPose.ThePose.Position.z - ts.LeveledCameraPose.Position.z) * 1000.0;
#else
		if (ovr_GetTrackerCount(_session) > 0)
		{
			ovrTrackerPose tp = ovr_GetTrackerPose(_session, 0);
			_body.x = (ts.HeadPose.ThePose.Position.x - tp.LeveledPose.Position.x) * 1000.0;
			_body.y = (ts.HeadPose.ThePose.Position.y - tp.LeveledPose.Position.y) * 1000.0;
			_body.z = (ts.HeadPose.ThePose.Position.z - tp.LeveledPose.Position.z) * 1000.0;
		}
#endif

		_body.qx = ts.HeadPose.ThePose.Orientation.x;
		_body.qy = ts.HeadPose.ThePose.Orientation.y;
		_body.qz = ts.HeadPose.ThePose.Orientation.z;
		_body.qw = ts.HeadPose.ThePose.Orientation.w;
	}
}