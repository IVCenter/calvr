#include <input/TrackerVRPN.h>

#include <config/ConfigManager.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include <osg/Vec3>

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>

#define FEET_TO_MM 304.8

using namespace cvr;

bool trackingDebug;
bool buttonDebug;
bool bodyDebug;
int debugStation;

struct cvr::DeviceInfo
{
        std::string name;
        vrpn_Tracker_Remote *tkr;
        vrpn_Button_Remote *btn;
        vrpn_Analog_Remote *ana;
};

void VRPN_CALLBACK handleBodyInfo (void *userdata, const vrpn_TRACKERCB t)
{
    /*t_user_callback	*t_data = (t_user_callback *)userdata;

     // Make sure we have a count value for this sensor
     while (t_data->t_counts.size() <= static_cast<unsigned>(t.sensor)) {
     t_data->t_counts.push_back(0);
     }

     // See if we have gotten enough reports from this sensor that we should
     // print this one.  If so, print and reset the count.
     if ( ++t_data->t_counts[t.sensor] >= tracker_stride ) {
     t_data->t_counts[t.sensor] = 0;
     printf("Tracker %s, sensor %d:\n        pos (%5.2f, %5.2f, %5.2f); quat (%5.2f, %5.2f, %5.2f, %5.2f)\n",
     t_data->t_name,
     t.sensor,
     t.pos[0], t.pos[1], t.pos[2],
     t.quat[0], t.quat[1], t.quat[2], t.quat[3]);
     }*/

    std::vector<trackedBody *> * tbList = (std::vector<trackedBody *> *) userdata;

    static const float m2mm = 1000.0;

    trackedBody * tb = tbList->at(t.sensor);
    tb->x = t.pos[0] * m2mm;
    tb->y = t.pos[1] * m2mm;
    tb->z = t.pos[2] * m2mm;
    tb->qx = t.quat[0];
    tb->qy = t.quat[1];
    tb->qz = t.quat[2];
    tb->qw = t.quat[3];

    if(bodyDebug && debugStation == t.sensor)
    {
        std::cerr << "Tracker station " << t.sensor << ": x: " << tb->x << " y: " << tb->y << " z: " << tb->z << "Quat: " << tb->qx << " " << tb->qy << " " << tb->qz << " " << tb->qw << std::endl;
    }
}

void VRPN_CALLBACK handleButton (void *userdata, const vrpn_BUTTONCB b)
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

    if(buttonDebug)
    {
        std::cerr << "Button Mask: " << *button << std::endl;
    }
}

void VRPN_CALLBACK handleAnalog (void *userdata, const vrpn_ANALOGCB a)
{
    /*int i;
     const char *name = (const char *)userdata;

     printf("Analog %s:\n         %5.2f", name, a.channel[0]);
     for (i = 1; i < a.num_channel; i++) {
     printf(", %5.2f", a.channel[i]);
     }
     printf(" (%d chans)\n", a.num_channel);*/

    std::vector<float> * valList = (std::vector<float> *)userdata;
    for (int i = 1; i < std::min((size_t)a.num_channel, valList->size()); i++)
    {
        valList->at(i) = a.channel[i];
    }

    if(buttonDebug)
    {
        std::cerr << "Valuator " << a.num_channel << ": " << a.channel[a.num_channel] << std::endl;
    }
}

TrackerVRPN::TrackerVRPN()
{
    _numBodies = 0;
    _numVal = 0;
    _numButtons = 0;

    _buttonMask = 0;
    _device = NULL;

    trackingDebug = ConfigManager::getBool("Input.TrackingDebug", false);
    if(trackingDebug)
    {
        buttonDebug = ConfigManager::getBool("button", "Input.TrackingDebug",
                                             false, NULL);
        bodyDebug = ConfigManager::getBool("body", "Input.TrackingDebug",
                                           false, NULL);
        if(bodyDebug)
        {
            debugStation = ConfigManager::getInt("station",
                                                 "Input.TrackingDebug", 0);
        }
    }
    else
    {
        buttonDebug = false;
        bodyDebug = false;
    }
}

TrackerVRPN::~TrackerVRPN()
{
}

bool TrackerVRPN::initBodyTrack()
{
    if(!_device)
    {
        _device = new DeviceInfo;
        _device->tkr = NULL;
        _device->btn = NULL;
        _device->ana = NULL;
    }

    _device->name = ConfigManager::getEntry("Input.VRPN.Server");
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

    _numBodies = ConfigManager::getInt("Input.VRPN.NumBodies", 2);
    _device->tkr->register_change_handler(&_bodyList, handleBodyInfo);

    for(int i = 0; i < _numBodies; i++)
    {
        trackedBody * tb = new trackedBody;
        tb->x = tb->y = tb->z = tb->qx = tb->qy = tb->qz = tb->qw = 0.0;
        _bodyList.push_back(tb);
    }

    return true;
}

bool TrackerVRPN::initButtonTrack()
{
    if(!_device)
    {
        _device = new DeviceInfo;
        _device->tkr = NULL;
        _device->btn = NULL;
        _device->ana = NULL;
    }

    _device->name = ConfigManager::getEntry("Input.VRPN.Server");
    if(_device->name.empty())
    {
        std::cerr << "No VRPN server specified." << std::endl;
        return false;
    }

    _device->btn = new vrpn_Button_Remote(_device->name.c_str());
    _device->ana = new vrpn_Analog_Remote(_device->name.c_str());

    if(!_device->btn || !_device->ana)
    {
        std::cerr << "Error setting up VRPN Controller." << std::endl;
        return false;
    }

    _numButtons = ConfigManager::getInt("Input.VRPN.NumButtons", 3);
    _numVal = ConfigManager::getInt("Input.VRPN.NumValuators", 2);

    _device->btn->register_change_handler(&_buttonMask, handleButton);
    _device->ana->register_change_handler(&_valList, handleAnalog);

    for(int i = 0; i < _numVal; i++)
    {
        _valList.push_back(0);
    }

    return true;
}

trackedBody * TrackerVRPN::getBody(int station)
{
    if(station < 0 || station >= _numBodies)
    {
        return NULL;
    }

    return _bodyList[station];
}

unsigned int TrackerVRPN::getButtonMask(int station)
{
    (void)station;
    return _buttonMask;
}

float TrackerVRPN::getValuator(int station, int index)
{
    if(index < 0 || index >= _numVal)
    {
        return 0.0;
    }

    return _valList[index];
}

int TrackerVRPN::getNumBodies()
{
    return _numBodies;
}

int TrackerVRPN::getNumValuators(int station)
{
    return _numVal;
}

int TrackerVRPN::getNumValuatorStations()
{
    return 1;
}

int TrackerVRPN::getNumButtons(int station)
{
    return _numButtons;
}

int TrackerVRPN::getNumButtonStations()
{
    return 1;
}

void TrackerVRPN::update()
{
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
}
