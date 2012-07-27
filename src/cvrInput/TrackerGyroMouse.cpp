#include <cvrInput/TrackerGyroMouse.h>
#include <cvrInput/TrackingManager.h>
#include <cvrConfig/ConfigManager.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include <osg/Vec3>

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>

#define FEET_TO_MM 304.8

using namespace cvr;

bool trackingDebugGM;
bool buttonDebugGM;
bool bodyDebugGM;
int debugStationGM;

struct cvr::TrackerGyroMouse::DeviceInfo
{
        std::string name;
        vrpn_Tracker_Remote *tkr;
        vrpn_Button_Remote *btn;
        vrpn_Analog_Remote *ana;
};

void VRPN_CALLBACK handleBodyInfoGM (void *userdata, const vrpn_TRACKERCB t)
{
    std::vector<TrackerBase::TrackedBody *> * tbList = (std::vector<TrackerBase::TrackedBody *> *) userdata;

    if(t.sensor >= tbList->size())
    {
        return;
    }

    static const float m2mm = 1000.0;

    TrackerBase::TrackedBody * tb = tbList->at(t.sensor);
    tb->x = t.pos[0] * m2mm;
    tb->y = t.pos[1] * m2mm;
    tb->z = t.pos[2] * m2mm;
    tb->qx = t.quat[0];
    tb->qy = t.quat[1];
    tb->qz = t.quat[2];
    tb->qw = t.quat[3];

    if(bodyDebugGM && debugStationGM == t.sensor)
    {
        std::cerr << "Tracker station " << t.sensor << ": x: " << tb->x << " y: " << tb->y << " z: " << tb->z << "Quat: " << tb->qx << " " << tb->qy << " " << tb->qz << " " << tb->qw << std::endl;
    }
}

void VRPN_CALLBACK handleButtonGM (void *userdata, const vrpn_BUTTONCB b)
{
    //const char *name = (const char *)userdata;

    //printf("Button %s, number %d was just %s\n",
    //	name, b.button, b.state?"pressed":"released");
    unsigned int mask = 1;
    mask = mask << b.button;

    unsigned int * button = (unsigned int *)userdata;

    if(b.state)
    {
        *button |= mask;
    }
    else
    {
        mask = ~mask;
        *button &= mask;
    }

    if(buttonDebugGM)
    {
        std::cerr << "Button Mask: " << *button << std::endl;
    }
}

void VRPN_CALLBACK handleAnalogGM (void *userdata, const vrpn_ANALOGCB a)
{
    /*int i;
     const char *name = (const char *)userdata;

     printf("Analog %s:\n         %5.2f", name, a.channel[0]);
     for (i = 1; i < a.num_channel; i++) {
     printf(", %5.2f", a.channel[i]);
     }
     printf(" (%d chans)\n", a.num_channel);*/

    std::vector<float> * valList = (std::vector<float> *)userdata;
    for (int i = 0; i < std::min((size_t)a.num_channel, valList->size()); i++)
    {
        valList->at(i) = a.channel[i];
    }

    if(buttonDebugGM)
    {
        std::cerr << "Valuator " << a.num_channel << ": " << a.channel[a.num_channel] << std::endl;
    }
}

TrackerGyroMouse::TrackerGyroMouse()
{
    _numBodies = 0;
    _numVal = 0;
    _numButtons = 0;

    _hand = -1;
    _posX = 0.5;
    _posY = 0.5;
    _sensitivity = -1.0;

    _buttonMask = 0;
    _device = NULL;

    trackingDebugGM = ConfigManager::getBool("Input.TrackingDebug",false);
    if(trackingDebugGM)
    {
        buttonDebugGM = ConfigManager::getBool("button","Input.TrackingDebug",
                false,NULL);
        bodyDebugGM = ConfigManager::getBool("body","Input.TrackingDebug",false,
                NULL);
        if(bodyDebugGM)
        {
            debugStationGM = ConfigManager::getInt("station",
                    "Input.TrackingDebug",0);
        }
    }
    else
    {
        buttonDebugGM = false;
        bodyDebugGM = false;
    }
}

TrackerGyroMouse::~TrackerGyroMouse()
{
    if(_device)
    {
        if(_device->tkr)
        {
            delete _device->tkr;
        }
        if(_device->btn)
        {
            delete _device->btn;
        }
        if(_device->ana)
        {
            delete _device->ana;
        }
        delete _device;
        _device = NULL;
    }

    for(int i = 0; i < _bodyList.size(); i++)
    {
        delete _bodyList[i];
    }
    _bodyList.clear();
}

bool TrackerGyroMouse::init(std::string tag)
{
    if(!_device)
    {
        _device = new DeviceInfo;
        _device->tkr = NULL;
        _device->btn = NULL;
        _device->ana = NULL;
    }

    _numBodies = 1;
    _numButtons = 3;
    _withWheel = ConfigManager::getBool("value",tag + ".GyroMouse.WithWheel",false);
    _numVal = _withWheel ? 3 : 2;

    _device->name = ConfigManager::getEntry(tag + ".GyroMouse.Server");
    if(_device->name.empty())
    {
        std::cerr << "No VRPN server specified." << std::endl;
        return false;
    }

    _device->tkr = new vrpn_Tracker_Remote(_device->name.c_str());

    if(!_device->tkr)
    {
        std::cerr << "Error setting up VRPN Tracking." << std::endl;
        return false;
    }

    _device->tkr->register_change_handler(&_bodyList,handleBodyInfoGM);

    for(int i = 0; i < _numBodies; i++)
    {
        TrackedBody * tb = new TrackedBody;
        tb->x = tb->y = tb->z = tb->qx = tb->qy = tb->qz = tb->qw = 0.0;
        _bodyList.push_back(tb);
	_matList.push_back(osg::Matrix());
    }

    _device->btn = new vrpn_Button_Remote(_device->name.c_str());
    _device->ana = new vrpn_Analog_Remote(_device->name.c_str());

    if(!_device->btn || !_device->ana)
    {
        std::cerr << "Error setting up VRPN Controller." << std::endl;
        return false;
    }

    _device->btn->register_change_handler(&_buttonMask,handleButtonGM);
    _device->ana->register_change_handler(&_valList,handleAnalogGM);

    for(int i = 0; i < _numVal; i++)
    {
        _valList.push_back(0);
	_lastValList.push_back(0);
    }

    return true;
}

TrackerBase::TrackedBody * TrackerGyroMouse::getBody(int index)
{
    if(index < 0 || index >= _numBodies)
    {
        return NULL;
    }

    return _bodyList[index];
}

unsigned int TrackerGyroMouse::getButtonMask()
{
    return _buttonMask;
}

float TrackerGyroMouse::getValuator(int index)
{
    if(index < 0 || index >= _numVal)
    {
        return 0.0;
    }

    return _valList[index];
}

int TrackerGyroMouse::getNumBodies()
{
    return _numBodies;
}

int TrackerGyroMouse::getNumValuators()
{
    return _numVal;
}

int TrackerGyroMouse::getNumButtons()
{
    return _numButtons;
}

void TrackerGyroMouse::update(std::map<int,std::list<InteractionEvent*> > & eventMap)
{
    float useSensitivity = 1.0;
    if(_sensitivity < 0.0 && SceneManager::instance()->getTiledWallValid())
    {
	float maxDem = std::max(SceneManager::instance()->getTiledWallWidth(),SceneManager::instance()->getTiledWallHeight());
	if(maxDem > 0.0)
	{
	    _sensitivity = 2000.0 / maxDem;
	    //std::cerr << "MaxDem: " << maxDem << " sen: " << _sensitivity << std::endl;
	}
	else
	{
	    _sensitivity = 1.0;
	}
	useSensitivity = _sensitivity;
    }
    else if(_sensitivity > 0.0)
    {
	useSensitivity = _sensitivity;
    }

    if(_hand < 0)
    {
	int mySystem = -1;
	for(int i = 0; i < TrackingManager::instance()->getNumTrackingSystems(); i++)
	{
	    if(TrackingManager::instance()->getTrackingSystem(i) == this)
	    {
		mySystem = i;
		break;
	    }
	}

	if(mySystem < 0)
	{
	    // should never happen
	    return;
	}

	for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
	{
	    int system, body;
	    TrackingManager::instance()->getHandAddress(i,system,body);
	    if(system == mySystem && body == 0)
	    {
		_hand = i;
		break;
	    }
	}
    }
    
    if(_device)
    {
        if(_device->btn)
        {
            _device->btn->mainloop();
        }
        if(_device->ana)
        {
            _device->ana->mainloop();
        }
        if(_device->tkr)
        {
            _device->tkr->mainloop();
        }
    }

    //std::cerr << "Mouse x: " << _valList[1] << " y: " << _valList[2] << std::endl;
    float xoffset = _withWheel ? _valList[1] : _valList[0];
    float yoffset = _withWheel ? _valList[2] : _valList[1];

    float maxv = 75.0;
    float factor = 20.0;
    float xchange = 0.0;
    float ychange = 0.0;

    bool posChanged = false;

    if(xoffset != 0.0)
    {
	xoffset = std::max(xoffset,-maxv);
	xoffset = std::min(xoffset,maxv);

	xoffset /= maxv;
	//xchange = log(fabs(xoffset)+1.0)/factor;
	xchange = xoffset / factor;
	/*if(xoffset < 0)
	{
	    xchange = -xchange;
	}*/
	_posX += xchange * useSensitivity;
	posChanged = true;
    }

    if(yoffset != 0.0)
    {
	yoffset = std::max(yoffset,-maxv);
	yoffset = std::min(yoffset,maxv);

	yoffset /= maxv;

	float wallRatio = SceneManager::instance()->getTiledWallHeight() / SceneManager::instance()->getTiledWallWidth();
	yoffset /= wallRatio;

	//ychange = log(fabs(yoffset)+1.0)/factor;
	ychange = yoffset / factor;
	/*if(yoffset < 0)
	{
	    ychange = -ychange;
	}*/
	_posY += ychange * useSensitivity;
	posChanged = true;
    }

    if(_hand >= 0 && !TrackingManager::instance()->getHandButtonMask(_hand) && (_posX < 0.0 || _posX > 1.0 || _posY < 0.0 || _posY > 1.0))
    {
	_posY = std::max(_posY,0.0f);
	_posY = std::min(_posY,1.0f);
	_posX = std::max(_posX,0.0f);
	_posX = std::min(_posX,1.0f);
	posChanged = true;
    }

    if(posChanged)
    {
	//update matrix
	//std::cerr << "Pos X: " << _posX << " Y: " << _posY << std::endl;

	// is this always right?
	osg::Vec3 camPos = TrackingManager::instance()->getHeadMat(0).getTrans();
	osg::Vec3 wallPos((_posX - 0.5)*SceneManager::instance()->getTiledWallWidth(),0,(-_posY + 0.5)*SceneManager::instance()->getTiledWallHeight());
	wallPos = wallPos * SceneManager::instance()->getTiledWallTransform();
	osg::Vec3 dir = wallPos - camPos;
	dir.normalize();
	osg::Quat q;
	q.makeRotate(osg::Vec3(0,1.0,0),dir);
	_bodyList[0]->x = camPos.x();
	_bodyList[0]->y = camPos.y();
	_bodyList[0]->z = camPos.z();
	_bodyList[0]->qx = q.x();
	_bodyList[0]->qy = q.y();
	_bodyList[0]->qz = q.z();
	_bodyList[0]->qw = q.w();
    }

    /*osg::Matrix m;
    osg::Vec3 pos;
    osg::Quat rot;
    pos.x() = _bodyList[0]->x;
    pos.y() = _bodyList[0]->y;
    pos.z() = _bodyList[0]->z;
    rot.x() = _bodyList[0]->qx;
    rot.y() = _bodyList[0]->qy;
    rot.z() = _bodyList[0]->qz;
    rot.w() = _bodyList[0]->qw;

    m.makeRotate(rot);
    m.setTrans(pos);

    // generate button events;
    unsigned int mask = 1;
    for(int i = 0; i < _numButtons; i++)
    {
	if((_buttonMask & mask) != (_lastButtonMask & mask))
	{
	    if(_buttonMask & mask)
	    {
		std::cerr << "Button " << i << " down" << std::endl;
		//button down
	    }
	    else
	    {
		std::cerr << "Button " << i << " up" << std::endl;
		//button up
	    }
	}
	mask = mask << 1;
    }
    _lastButtonMask = _buttonMask;*/
}

TrackedButtonInteractionEvent * TrackerGyroMouse::getNewBaseEvent(int body)
{
    PointerInteractionEvent * pie = new PointerInteractionEvent();
    pie->setX(_posX);
    pie->setY(_posY);
    pie->setPointerType(PointerInteractionEvent::GYROMOUSE_POINTER);
    return pie;
}
