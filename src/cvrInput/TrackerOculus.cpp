#include <cvrInput/TrackerOculus.h>

#include <iostream>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

ovrHmd TrackerOculus::_hmd = NULL;

TrackerOculus::TrackerOculus()
{
}

TrackerOculus::~TrackerOculus()
{
}

bool TrackerOculus::init(std::string tag)
{
	ovr_Initialize();

	_hmd = ovrHmd_Create(0);

	if(_hmd)
	{
		ovrHmd_ConfigureTracking(_hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);

		_body.x = 0.0;
		_body.y = 0.0;
		_body.z = 0.0;
		osg::Quat q;
		_body.qx = q.x();
		_body.qy = q.y();
		_body.qz = q.z();
		_body.qw = q.w();
	}
	else
	{
		std::cerr << "No Oculus found." << std::endl;
		ovr_Shutdown();
		return false;
	}

	return true;
}

TrackerBase::TrackedBody * TrackerOculus::getBody(int index)
{
	if(!_hmd || index != 0)
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
	if(_hmd)
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
	if(!_hmd)
	{
		return;
	}

	ovrTrackingState ts = ovrHmd_GetTrackingState(_hmd, ovr_GetTimeInSeconds());

	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
		_body.x = (ts.HeadPose.ThePose.Position.x - ts.LeveledCameraPose.Position.x) * 1000.0;
		_body.y = (ts.HeadPose.ThePose.Position.y - ts.LeveledCameraPose.Position.y) * 1000.0;
		_body.z = (ts.HeadPose.ThePose.Position.z - ts.LeveledCameraPose.Position.z) * 1000.0;

		/*osg::Vec3 pos(_body.x,_body.y,_body.z);
		osg::Matrix m;
		m.makeRotate(M_PI/2.0,osg::Vec3(1,0,0));
		pos = pos * m;
		_body.x = pos.x();
		_body.y = pos.y();
		_body.z = pos.z();*/
		
		//std::cerr << "Pos x: " << _body.x << " y: " << _body.y << " z: " << _body.z << std::endl;
		//std::cerr << "RawPos x: " << ts.HeadPose.ThePose.Position.x << " y: " << ts.HeadPose.ThePose.Position.y << " z: " << ts.HeadPose.ThePose.Position.z << std::endl;
		//std::cerr << "Cam x: " << ts.CameraPose.Position.x * 1000.0 << " y: " << ts.CameraPose.Position.y * 1000.0 << " z: " << ts.CameraPose.Position.z * 1000.0 << std::endl;

		_body.qx = ts.HeadPose.ThePose.Orientation.x;
		_body.qy = ts.HeadPose.ThePose.Orientation.y;
		_body.qz = ts.HeadPose.ThePose.Orientation.z;
		_body.qw = ts.HeadPose.ThePose.Orientation.w;
	}
}