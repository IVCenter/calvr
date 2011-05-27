#include <input/TrackingManager.h>
#include <input/TrackerSlave.h>
#include <input/TrackerShmem.h>
#include <input/TrackerMouse.h>

#ifdef WITH_VRPN
#include <input/TrackerVRPN.h>
#endif

#include <config/ConfigManager.h>

#include <kernel/ComController.h>
#include <kernel/InteractionManager.h>

#include <iostream>
#include <sstream>

#include <osg/Vec3>
#include <osg/Vec4>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#include <WinBase.h>
#include <util/TimeOfDay.h>
#pragma comment(lib, "kernel.lib")
#pragma comment(lib, "config.lib")
#pragma comment(lib, "util.lib")
#else
#include <sys/time.h>
#endif

using namespace cvr;

TrackingManager * TrackingManager::_myPtr = NULL;

TrackingManager::TrackingManager()
{
    _bodyTracker = NULL;
    _buttonTracker = NULL;
    _debugOutput = false;
    _threadQuit = false;
}

TrackingManager::~TrackingManager()
{
}

TrackingManager * TrackingManager::instance()
{
    if(!_myPtr)
    {
        _myPtr = new TrackingManager();
    }
    return _myPtr;
}

bool TrackingManager::init()
{
    _debugOutput = ConfigManager::getBool("Input.TrackingDebug", false);
    std::string bodySystem = ConfigManager::getEntry("value",
                                                     "Input.TrackingSystem",
                                                     "NONE");
    std::string buttonSystem =
                ConfigManager::getEntry("value", "Input.ButtonSystem", "NONE");

    if(bodySystem == "MOUSE" || buttonSystem == "MOUSE")
    {
	bodySystem = "MOUSE";
	buttonSystem = "MOUSE";
	_mouseTracker = true;
    }
    else
    {
	_mouseTracker = false;
    }

    if(!_mouseTracker)
    {
	_threaded = ConfigManager::getBool("Input.Threaded", false);
	_threadFPS = ConfigManager::getFloat("FPS", "Input.Threaded", 60.0);
    }
    else
    {
	_threaded = false;
    }

    TrackingManInit tmi;
    if(ComController::instance()->isMaster())
    {
	if(!_mouseTracker)
	{
	    _numHands = ConfigManager::getInt("Input.NumHands", 1);
	    _numHeads = ConfigManager::getInt("Input.NumHeads", 1);
	}
	else
	{
	    _numHands = 1;
	    _numHeads = 1;
	}

	if(bodySystem == "NONE")
	{
	    _numHands = 0;
	}

	// TODO: I think the handButtonStation and handButtonOffsets are not used anymore, check and remove
        if(_numHands > 0)
        {
	    if(!_mouseTracker)
	    {
		_handStations.push_back(ConfigManager::getInt("Input.HandAddress",
                                                          0));
	    }
	    else
	    {
		_handStations.push_back(0);
	    }

            _handButtonStations.push_back(
                                          ConfigManager::getInt(
                                                                "Input.HandButtonAddress",
                                                                0));
            _handButtonOffsets.push_back(
                                         ConfigManager::getInt(
                                                               "Input.HandButtonOffset",
                                                               0));
        }
        for(int i = 1; i < _numHands; i++)
        {
            std::stringstream ss;
            ss << "Input.Hand" << i + 1 << "Address";
            _handStations.push_back(ConfigManager::getInt(ss.str(), 0));

            std::stringstream ss1;
            ss1 << "Input.Hand" << i + 1 << "ButtonAddress";
            _handButtonStations.push_back(ConfigManager::getInt(ss1.str(), 0));

            std::stringstream ss2;
            ss2 << "Input.Hand" << i + 1 << "ButtonOffset";
            _handButtonOffsets.push_back(ConfigManager::getInt(ss2.str(), 0));
        }

        if(_numHeads > 0)
        {
	    if(!_mouseTracker)
	    {
		_headStations.push_back(ConfigManager::getInt("Input.HeadAddress",
                                                          1));
	    }
	    else
	    {
		_headStations.push_back(-1);
	    }
        }
        for(int i = 1; i < _numHeads; i++)
        {
            std::stringstream ss;
            ss << "Input.Head" << i + 1 << "Address";
            _headStations.push_back(ConfigManager::getInt(ss.str(), 1));
        }

        bool initGood = true;

        if(bodySystem == "SHMEM")
        {
            _bodyTracker = new TrackerShmem();
        }
#ifdef WITH_VRPN
        else if(bodySystem == "VRPN")
        {
            _bodyTracker = new TrackerVRPN();
        }
#endif
	else if(bodySystem == "MOUSE")
	{
	    _bodyTracker = new TrackerMouse();
	}
        else
        {
            _bodyTracker = NULL;
        }

        if(_bodyTracker)
        {
            if(!_bodyTracker->initBodyTrack())
            {
                std::cerr << "Error on body tracker init." << std::endl;
                delete _bodyTracker;
                _bodyTracker = NULL;
                initGood = false;
            }
        }

        if(_bodyTracker && !_bodyTracker->getNumBodies())
        {
            delete _bodyTracker;
            _bodyTracker = NULL;
        }

        if(buttonSystem == bodySystem && _bodyTracker)
        {
            _buttonTracker = _bodyTracker;

        }
        else if(buttonSystem == "SHMEM")
        {
            _buttonTracker = new TrackerShmem();
        }
#ifdef WITH_VRPN
        else if(buttonSystem == "VRPN")
        {
            _buttonTracker = new TrackerVRPN();
        }
#endif
	// should not hit this since mouse should be used for body system
	else if(buttonSystem == "MOUSE")
	{
	    _buttonTracker = new TrackerMouse();
	}
        else
        {
            _buttonTracker = NULL;
        }

        if(_buttonTracker)
        {
            if(!_buttonTracker->initButtonTrack())
            {
                std::cerr << "Error on button tracker init." << std::endl;
                if(_buttonTracker != _bodyTracker)
                {
                    delete _buttonTracker;
                }
                _buttonTracker = NULL;
                initGood = false;
            }
            else if(!_buttonTracker->hasButtons()
                    && !_buttonTracker->hasValuators())
            {
                if(_buttonTracker != _bodyTracker)
                {
                    delete _buttonTracker;
                }
                _buttonTracker = NULL;
            }
        }

        if(_numHands > 0 && _bodyTracker)
        {
            if(bodySystem == "SHMEM"
#ifdef WITH_VRPN
            || bodySystem == "VRPN"
#endif
            )
            {
                _showWand = true;
            }
            else
            {
                _showWand = false;
            }
        }
        else
        {
            _showWand = false;
        }

        for(int i = 0; i < _numHands; i++)
        {
            _threadHandButtonMasks.push_back(0);
            _threadHandMatList.push_back(osg::Matrix());
        }

        for(int i = 0; i < _numHeads; i++)
        {
            _threadHeadMatList.push_back(osg::Matrix());
        }

        tmi.numHands = _numHands;
        tmi.numHeads = _numHeads;
        tmi.showWand = _showWand;
        tmi.totalBodies = _bodyTracker ? _bodyTracker->getNumBodies() : 0;
        tmi.buttonStations
                = _buttonTracker ? _buttonTracker->getNumButtonStations() : 0;
        tmi.valStations
                = _buttonTracker ? _buttonTracker->getNumValuatorStations() : 0;

        if(_debugOutput)
        {
            std::cerr << "Body System: " << bodySystem << std::endl;
            std::cerr << "Num Bodies: "
                    << (_bodyTracker ? _bodyTracker->getNumBodies() : 0)
                    << std::endl;
            std::cerr << "Number of heads: " << _numHeads << std::endl;
            for(int i = 0; i < _numHeads; i++)
            {
                std::cerr << "Head Station: " << _headStations[i] << std::endl;
            }
            std::cerr << "Number of hands: " << _numHands << std::endl;
            for(int i = 0; i < _numHands; i++)
            {
                std::cerr << "Hand Station: " << _handStations[i] << std::endl;
            }
            std::cerr << "Button System: " << buttonSystem << std::endl;
            std::cerr << "Num Button Stations: "
                    << (_buttonTracker ? _buttonTracker->getNumButtonStations()
                            : 0) << std::endl;
            for(int i = 0; i
                    < (_buttonTracker ? _buttonTracker->getNumButtonStations()
                            : 0); i++)
            {
                std::cerr << "Num Buttons: "
                        << _buttonTracker->getNumButtons(i) << std::endl;
            }
            std::cerr << "Num Valuator Stations: "
                    << (_buttonTracker ? _buttonTracker->getNumValuatorStations()
                            : 0) << std::endl;
            for(int i = 0; i
                    < (_buttonTracker ? _buttonTracker->getNumValuatorStations()
                            : 0); i++)
            {
                std::cerr << "Num Valuators: "
                        << (_buttonTracker ? _buttonTracker->getNumValuators(i)
                                : 0) << std::endl;
            }
        }

        ComController::instance()->sendSlaves(&tmi,
                                              sizeof(struct TrackingManInit));

        int * numButtons = new int[tmi.buttonStations];
        for(int i = 0; i < tmi.buttonStations; i++)
        {
            numButtons[i] = _buttonTracker->getNumButtons(i);
        }
        ComController::instance()->sendSlaves(numButtons, tmi.buttonStations
                * sizeof(int));
        delete[] numButtons;

        int * numVals = new int[tmi.valStations];
        for(int i = 0; i < tmi.valStations; i++)
        {
            numVals[i] = _buttonTracker->getNumValuators(i);
        }
        ComController::instance()->sendSlaves(numVals, tmi.valStations
                * sizeof(int));
        delete[] numVals;

        int * handstations = new int[_numHands];
        for(int i = 0; i < _numHands; i++)
        {
            handstations[i] = _handStations[i];
        }
        ComController::instance()->sendSlaves(handstations, _numHands
                * sizeof(int));
        delete[] handstations;

        int * handbuttonstations = new int[_numHands];
        for(int i = 0; i < _numHands; i++)
        {
            handbuttonstations[i] = _handButtonStations[i];
        }
        ComController::instance()->sendSlaves(handbuttonstations, _numHands
                * sizeof(int));
        delete[] handbuttonstations;

        int * handbuttonoffsets = new int[_numHands];
        for(int i = 0; i < _numHands; i++)
        {
            handbuttonoffsets[i] = _handButtonOffsets[i];
        }
        ComController::instance()->sendSlaves(handbuttonoffsets, _numHands
                * sizeof(int));
        delete[] handbuttonoffsets;

        int * headstations = new int[_numHeads];
        for(int i = 0; i < _numHeads; i++)
        {
            headstations[i] = _headStations[i];
        }
        ComController::instance()->sendSlaves(headstations, _numHeads
                * sizeof(int));
        delete[] headstations;
    }
    else
    {
        ComController::instance()->readMaster(&tmi,
                                              sizeof(struct TrackingManInit));

        int * numButtons = new int[tmi.buttonStations];
        ComController::instance()->readMaster(numButtons, tmi.buttonStations
                * sizeof(int));
        int * numVals = new int[tmi.valStations];
        ComController::instance()->readMaster(numVals, tmi.valStations
                * sizeof(int));

        int * handstations = new int[tmi.numHands];
        ComController::instance()->readMaster(handstations, tmi.numHands
                * sizeof(int));
        int * handbuttonstations = new int[tmi.numHands];
        ComController::instance()->readMaster(handbuttonstations, tmi.numHands
                * sizeof(int));
        int * handbuttonoffsets = new int[tmi.numHands];
        ComController::instance()->readMaster(handbuttonoffsets, tmi.numHands
                * sizeof(int));
        int * headstations = new int[tmi.numHeads];
        ComController::instance()->readMaster(headstations, tmi.numHeads
                * sizeof(int));

        _numHeads = tmi.numHeads;
        _numHands = tmi.numHands;
        _showWand = tmi.showWand;
        for(int i = 0; i < _numHands; i++)
        {
            _handStations.push_back(handstations[i]);
            _handButtonStations.push_back(handbuttonstations[i]);
            _handButtonOffsets.push_back(handbuttonoffsets[i]);
        }

        for(int i = 0; i < _numHeads; i++)
        {
            _headStations.push_back(headstations[i]);
        }

        if(tmi.totalBodies)
        {
            _bodyTracker = new TrackerSlave(tmi.totalBodies,
                                            tmi.buttonStations, numButtons,
                                            tmi.valStations, numVals);
            if(_bodyTracker->hasButtons() || _bodyTracker->hasValuators())
            {
                _buttonTracker = _bodyTracker;
            }
            else
            {
                _buttonTracker = NULL;
            }
        }
        else
        {
            _bodyTracker = NULL;
            _buttonTracker = new TrackerSlave(tmi.totalBodies,
                                              tmi.buttonStations, numButtons,
                                              tmi.valStations, numVals);
            if(!_buttonTracker->hasButtons() && !_buttonTracker->hasValuators())
            {
                delete _buttonTracker;
                _buttonTracker = NULL;
            }
        }

        delete[] handstations;
        delete[] handbuttonstations;
        delete[] handbuttonoffsets;
        delete[] headstations;
        delete[] numButtons;
        delete[] numVals;
    }

    _totalButtons = 0;
    _totalValuators = 0;

    if(_buttonTracker)
    {
        for(int i = 0; i < _numHands; i++)
        {
            _handStationFilterMask.push_back(std::vector<unsigned int>());
        }

        for(int i = 0; i < _buttonTracker->getNumButtonStations(); i++)
        {
            _totalButtons += _buttonTracker->getNumButtons(i);
            for(int j = 0; j < _numHands; j++)
            {
                std::stringstream hss, sss;
                hss << "Input.Hand" << j + 1 << "ButtonMask";
                sss << "station" << i + 1;
                std::string mask;
		if(!_mouseTracker)
		{
		     mask = ConfigManager::getEntry(sss.str(), hss.str(), j ? "0x0"
                                : "0xFFFFFFFF");
		}
		else
		{
		    mask = "0xFFFFFFFF";
		}

                char * eptr;
                unsigned long int imask = strtol(mask.c_str(), &eptr, 0);
                _handStationFilterMask[j].push_back((unsigned int)imask);
            }
            _rawButtonMask.push_back(0);
        }
        for(int i = 0; i < _buttonTracker->getNumValuatorStations(); i++)
        {
            _totalValuators += _buttonTracker->getNumValuators(i);
            _valuatorList.push_back(std::vector<float>());
            for(int j = 0; j < _buttonTracker->getNumValuators(i); j++)
            {
                _valuatorList[i].push_back(0.0);
            }
        }

        if(_debugOutput)
        {
            for(int i = 0; i < _numHands; i++)
            {
                std::cerr << "Hand " << i + 1 << ": ";
                for(int j = 0; j < _buttonTracker->getNumButtonStations(); j++)
                {
                    std::cerr << "Station" << j + 1 << "Mask: "
                            << _handStationFilterMask[i][j] << " ";
                }
                std::cerr << std::endl;
            }
        }
    }

    //std::cerr << "Bodies -------------- " << tmi.totalBodies << std::endl;

    for(int i = 0; i < _numHands; i++)
    {
        _handMatList.push_back(osg::Matrix());
        _handButtonMask.push_back(0);
        _lastHandButtonMask.push_back(0);
    }

    for(int i = 0; i < _numHeads; i++)
    {
        _headMatList.push_back(osg::Matrix());
    }

    float vx, vy, vz;
    vx = ConfigManager::getFloat("x", "ViewerPosition", 0);
    vy = ConfigManager::getFloat("y", "ViewerPosition", 0);
    vz = ConfigManager::getFloat("z", "ViewerPosition", 0);

    //_headMat.setTrans(vx,vy,vz);

    _defaultHead = new trackedBody;
    _defaultHead->x = vx;
    _defaultHead->y = vy;
    _defaultHead->z = vz;
    _defaultHead->qx = _defaultHead->qy = _defaultHead->qz = _defaultHead->qw
            = 0.0;

    _defaultHand = new trackedBody;
    _defaultHand->x = _defaultHand->y = _defaultHand->z = _defaultHand->qx
            = _defaultHand->qy = _defaultHand->qz = _defaultHand->qw = 0.0;

    if(_numHands > 0)
    {
        float x, y, z, h, p, r;
        x = ConfigManager::getFloat("x", "Input.HandDevice.Offset", 0.0);
        y = ConfigManager::getFloat("y", "Input.HandDevice.Offset", 0.0);
        z = ConfigManager::getFloat("z", "Input.HandDevice.Offset", 0.0);
        h = ConfigManager::getFloat("h", "Input.HandDevice.Orientation", 0.0);
        p = ConfigManager::getFloat("p", "Input.HandDevice.Orientation", 0.0);
        r = ConfigManager::getFloat("r", "Input.HandDevice.Orientation", 0.0);
        osg::Matrix m;
        m.makeRotate(h * M_PI / 180.0, osg::Vec3(0, 0, 1), p * M_PI / 180.0,
                     osg::Vec3(1, 0, 0), r * M_PI / 180.0, osg::Vec3(0, 1, 0));
        _handTransformsTrans.push_back(osg::Vec3(x, y, z));
        _handTransformsRot.push_back(m);
    }
    for(int i = 1; i < _numHands; i++)
    {
        std::stringstream ss;
        ss << "Input.HandDevice" << i + 1;
        float x, y, z, h, p, r;
        x = ConfigManager::getFloat("x", ss.str() + ".Offset", 0.0);
        y = ConfigManager::getFloat("y", ss.str() + ".Offset", 0.0);
        z = ConfigManager::getFloat("z", ss.str() + ".Offset", 0.0);
        h = ConfigManager::getFloat("h", ss.str() + ".Orientation", 0.0);
        p = ConfigManager::getFloat("p", ss.str() + ".Orientation", 0.0);
        r = ConfigManager::getFloat("r", ss.str() + ".Orientation", 0.0);
        osg::Matrix m;
        m.makeRotate(h * M_PI / 180.0, osg::Vec3(0, 0, 1), p * M_PI / 180.0,
                     osg::Vec3(1, 0, 0), r * M_PI / 180.0, osg::Vec3(0, 1, 0));
        _handTransformsTrans.push_back(osg::Vec3(x, y, z));
        _handTransformsRot.push_back(m);
    }

    float x, y, z, h, p, r;
    x = ConfigManager::getFloat("x", "Input.TrackingSystem.Offset", 0.0);
    y = ConfigManager::getFloat("y", "Input.TrackingSystem.Offset", 0.0);
    z = ConfigManager::getFloat("z", "Input.TrackingSystem.Offset", 0.0);
    h = ConfigManager::getFloat("h", "Input.TrackingSystem.Orientation", 0.0);
    p = ConfigManager::getFloat("p", "Input.TrackingSystem.Orientation", 0.0);
    r = ConfigManager::getFloat("r", "Input.TrackingSystem.Orientation", 0.0);
    osg::Matrix m;
    m.makeRotate(h * M_PI / 180.0, osg::Vec3(0, 0, 1), p * M_PI / 180.0,
                 osg::Vec3(1, 0, 0), r * M_PI / 180.0, osg::Vec3(0, 1, 0));
    m.setTrans(osg::Vec3(x, y, z));
    _systemTransform = m;

    if(_numHeads)
    {
        x = ConfigManager::getFloat("x", "Input.HeadDevice.Offset", 0.0);
        y = ConfigManager::getFloat("y", "Input.HeadDevice.Offset", 0.0);
        z = ConfigManager::getFloat("z", "Input.HeadDevice.Offset", 0.0);
        h = ConfigManager::getFloat("h", "Input.HeadDevice.Orientation", 0.0);
        p = ConfigManager::getFloat("p", "Input.HeadDevice.Orientation", 0.0);
        r = ConfigManager::getFloat("r", "Input.HeadDevice.Orientation", 0.0);

        m.makeRotate(h * M_PI / 180.0, osg::Vec3(0, 0, 1), p * M_PI / 180.0,
                     osg::Vec3(1, 0, 0), r * M_PI / 180.0, osg::Vec3(0, 1, 0));
        _headTransformsTrans.push_back(osg::Vec3(x, y, z));
        _headTransformsRot.push_back(m);
    }

    for(int i = 1; i < _numHeads; i++)
    {
        std::stringstream ss;
        ss << "Input.HeadDevice" << i + 1;
        x = ConfigManager::getFloat("x", ss.str() + ".Offset", 0.0);
        y = ConfigManager::getFloat("y", ss.str() + ".Offset", 0.0);
        z = ConfigManager::getFloat("z", ss.str() + ".Offset", 0.0);
        h = ConfigManager::getFloat("h", ss.str() + ".Orientation", 0.0);
        p = ConfigManager::getFloat("p", ss.str() + ".Orientation", 0.0);
        r = ConfigManager::getFloat("r", ss.str() + ".Orientation", 0.0);

        m.makeRotate(h * M_PI / 180.0, osg::Vec3(0, 0, 1), p * M_PI / 180.0,
                     osg::Vec3(1, 0, 0), r * M_PI / 180.0, osg::Vec3(0, 1, 0));
        _headTransformsTrans.push_back(osg::Vec3(x, y, z));
        _headTransformsRot.push_back(m);
    }

    if(!_bodyTracker || !_bodyTracker->getNumBodies())
    {
        update();
    }

    if(_threaded && ComController::instance()->isMaster())
    {
        genComTrackEvents = new GenComplexTrackingEvents();
        this->start();
    }

    return true;
}

void TrackingManager::update()
{
    if(ComController::instance()->getIsSyncError())
    {
	return;
    }

    _updateLock.lock();

    //std::cerr << "Update Called." << std::endl;
    if(ComController::instance()->isMaster())
    {
        if(!_threaded)
        {
            if(_bodyTracker)
            {
                _bodyTracker->update();
            }
            if(_buttonTracker && _buttonTracker != _bodyTracker)
            {
                _buttonTracker->update();
            }
        }

        if(_bodyTracker && _bodyTracker->getNumBodies() > 0)
        {
            trackedBody * tbsend =
                    new trackedBody[_bodyTracker->getNumBodies()];
            for(int i = 0; i < _bodyTracker->getNumBodies(); i++)
            {
                tbsend[i] = *_bodyTracker->getBody(i);
            }
            ComController::instance()->sendSlaves(
                                                  tbsend,
                                                  _bodyTracker->getNumBodies()
                                                          * sizeof(struct trackedBody));
            delete[] tbsend;
        }

        if(_buttonTracker)
        {
            char * sendarray = new char[(sizeof(unsigned int)
                    * _buttonTracker->getNumButtonStations())
                    + (_totalValuators * sizeof(float))];
            for(int i = 0; i < _buttonTracker->getNumButtonStations(); i++)
            {
                ((unsigned int*)sendarray)[i]
                        = _buttonTracker->getButtonMask(i);
            }
            int index = 0;
            float * valarray = (float *)(sendarray + (sizeof(unsigned int)
                    * _buttonTracker->getNumButtonStations()));
            for(int j = 0; j < _buttonTracker->getNumValuatorStations(); j++)
            {
                for(int i = 0; i < _buttonTracker->getNumValuators(j); i++)
                {
                    valarray[index] = _buttonTracker->getValuator(j, i);
                    index++;
                }
            }
            ComController::instance()->sendSlaves(
                                                  sendarray,
                                                  (sizeof(unsigned int)
                                                          * _buttonTracker->getNumButtonStations())
                                                          + (_totalValuators
                                                                  * sizeof(float)));

            delete[] sendarray;
        }
    }
    else
    {
        trackedBody * tbRecv = NULL;
        unsigned int * buttonRecv = NULL;
        float * valRecv = NULL;
        if(_bodyTracker && _bodyTracker->getNumBodies())
        {
            tbRecv = new trackedBody[_bodyTracker->getNumBodies()];
            ComController::instance()->readMaster(
                                                  tbRecv,
                                                  _bodyTracker->getNumBodies()
                                                          * sizeof(struct trackedBody));
        }

        char * recvarray = NULL;
        if(_buttonTracker)
        {
            recvarray = new char[(sizeof(unsigned int)
                    * _buttonTracker->getNumButtonStations())
                    + (_totalValuators * sizeof(float))];
            ComController::instance()->readMaster(
                                                  recvarray,
                                                  (sizeof(unsigned int)
                                                          * _buttonTracker->getNumButtonStations())
                                                          + (_totalValuators
                                                                  * sizeof(float)));
            buttonRecv = (unsigned int *)recvarray;
            if(_buttonTracker->getNumValuators())
            {
                valRecv = (float *)(recvarray + (sizeof(unsigned int)
                        * _buttonTracker->getNumButtonStations()));
            }
        }

        TrackerSlave * sTracker = NULL;
        if(_bodyTracker)
        {
            sTracker = dynamic_cast<TrackerSlave *> (_bodyTracker);
        }
        else if(_buttonTracker)
        {
            sTracker = dynamic_cast<TrackerSlave *> (_buttonTracker);
        }

        if(sTracker)
        {
            sTracker->readValues(tbRecv, buttonRecv, valRecv);
        }

        if(recvarray)
        {
            delete[] recvarray;
        }

        if(tbRecv)
        {
            delete[] tbRecv;
        }
    }

    trackedBody * tb;
    for(int i = 0; i < _numHeads; i++)
    {
        if(_bodyTracker && _bodyTracker->getBody(_headStations[i]))
        {
            tb = _bodyTracker->getBody(_headStations[i]);
            if(tb)
            {
                osg::Vec3 pos(tb->x, tb->y, tb->z);
                osg::Matrix rot;
                //float deg2rad = M_PI / 180.0;
                //rot.makeRotate(tb->r * deg2rad, osg::Vec3(0,0,1), tb->p * deg2rad, osg::Vec3(1,0,0), tb->h * deg2rad, osg::Vec3(0,1,0));
                rot.makeRotate(osg::Quat(tb->qx, tb->qy, tb->qz, tb->qw));
                // TODO: check translation stuff
                _headMatList[i] = _headTransformsRot[i] * rot
                        * osg::Matrix::translate(pos) * _systemTransform
                        * osg::Matrix::translate(_headTransformsTrans[i]);
            }
        }
        else
        {
            //std::cerr << "Setting Head Position to x: " << _defaultHead->x << " y: " << _defaultHead->y << " z: " << _defaultHead->z << std::endl;
            _headMatList[i].setTrans(_defaultHead->x, _defaultHead->y,
                                     _defaultHead->z);
            //_headMat =  _headTransformRot * osg::Matrix::translate(osg::Vec3(_defaultHead->x,_defaultHead->y,_defaultHead->z)) * _systemTransform * osg::Matrix::translate(_headTransformTrans);
        }
    }

    for(int i = 0; i < _numHands; i++)
    {
        if(_bodyTracker && _bodyTracker->getBody(_handStations[i]))
        {
            tb = _bodyTracker->getBody(_handStations[i]);
        }
        else
        {
            tb = _defaultHand;
        }

        if(tb)
        {
            osg::Vec3 pos(tb->x, tb->y, tb->z);
            osg::Matrix rot;
            //float deg2rad = M_PI / 180.0;
            //rot.makeRotate(tb->r * deg2rad, osg::Vec3(0,0,1), tb->p * deg2rad, osg::Vec3(1,0,0), tb->h * deg2rad, osg::Vec3(0,1,0));
            rot.makeRotate(osg::Quat(tb->qx, tb->qy, tb->qz, tb->qw));
            // TODO: check translation stuff
            _handMatList[i] = _handTransformsRot[i] * rot
                    * osg::Matrix::translate(pos) * _systemTransform
                    * osg::Matrix::translate(_handTransformsTrans[i]);
        }
    }

    for(int i = 0; i < _rawButtonMask.size(); i++)
    {
        _rawButtonMask[i] = _buttonTracker->getButtonMask();
    }

    for(int i = 0; i < _valuatorList.size(); i++)
    {
        for(int j = 0; j < _valuatorList[i].size(); j++)
        {
            _valuatorList[i][j] = _buttonTracker->getValuator(i, j);
        }
    }

    updateHandMask();

    if(!_threaded)
    {
	if(!_mouseTracker)
	{
	    generateButtonEvents();
	}
    }
    else
    {
        flushEvents();
    }

    _updateLock.unlock();
}

void TrackingManager::run()
{
    struct timeval start, end;
    float target = 1.0 / _threadFPS;

    struct timeval printStart, printEnd;
    int readings = 0;
    gettimeofday(&printStart, NULL);
    while(1)
    {
        gettimeofday(&start, NULL);
        _updateLock.lock();

        if(_bodyTracker)
        {
            _bodyTracker->update();
        }
        if(_buttonTracker && _buttonTracker != _bodyTracker)
        {
            _buttonTracker->update();
        }

        updateThreadMats();
        updateThreadHandMask();
        generateThreadButtonEvents();

        _updateLock.unlock();

        _quitLock.lock();
        if(_threadQuit)
        {
            _quitLock.unlock();
            break;
        }
        _quitLock.unlock();
        gettimeofday(&end, NULL);

        float interval = (end.tv_sec - start.tv_sec) + ((end.tv_usec
                - start.tv_usec) / 1000000.0);
        if(interval < target)
        {
#ifndef WIN32
            timespec ts;
            interval = target - interval;
            ts.tv_sec = (int)interval;
            interval -= ((float)((int)interval));
            ts.tv_nsec = (long int)(interval * 1000000000.0 * 0.95);
            nanosleep(&ts, NULL);
#else
			//TODO: do this sub-milisecond
			interval = target - interval;
			DWORD sleeptime = (DWORD)(interval * 1000.0 * 0.95);
			Sleep(sleeptime);
#endif
        }
        readings++;
        gettimeofday(&printEnd, NULL);
        interval = (printEnd.tv_sec - printStart.tv_sec) + ((printEnd.tv_usec
                - printStart.tv_usec) / 1000000.0);
        if(interval > 10.0)
        {
            printStart = printEnd;
            std::cerr << "Tracking FPS: " << ((float)readings) / interval
                    << std::endl;
            readings = 0;
        }
    }
}

void TrackingManager::quitThread()
{
    _quitLock.lock();

    _threadQuit = true;

    _quitLock.unlock();
}

bool TrackingManager::getShowWand()
{
    return _showWand;
}

int TrackingManager::getNumHands()
{
    return _numHands;
}

int TrackingManager::getNumHeads()
{
    return _numHeads;
}

osg::Matrix & TrackingManager::getHandMat(int hand)
{
    if(hand < 0 || hand >= _handMatList.size())
    {
        static osg::Matrix m;
        return m;
    }
    return _handMatList[hand];
}

osg::Matrix & TrackingManager::getHeadMat(int head)
{
    if(head < 0 || head >= _headMatList.size())
    {
        static osg::Matrix m;
        return m;
    }
    return _headMatList[head];
}

int TrackingManager::getNumButtonStations()
{
    if(_buttonTracker)
    {
        return _buttonTracker->getNumButtonStations();
    }
    return 0;
}

int TrackingManager::getNumButtons(int station)
{
    if(_buttonTracker)
    {
        return _buttonTracker->getNumButtons(station);
    }
    return 0;
}

unsigned int TrackingManager::getRawButtonMask(int station)
{
    /*if(_buttonTracker)
     {
     return _buttonTracker->getButtonMask(station);
     }
     return 0;*/
    if(station >= 0 && station < _rawButtonMask.size())
    {
        return _rawButtonMask[station];
    }
    return 0;
}

unsigned int TrackingManager::getHandButtonMask(int hand)
{
    if(hand >= 0 && hand < _handButtonMask.size())
    {
        return _handButtonMask[hand];
    }
    return 0;
}

int TrackingManager::getNumValuatorStations()
{
    return _valuatorList.size();
}

int TrackingManager::getNumValuators(int station)
{
    if(station >= 0 && station < _valuatorList.size())
    {
        return _valuatorList[station].size();
    }
    return 0;
}

float TrackingManager::getValuator(int station, int index)
{
    if(station >= 0 && station < _valuatorList.size() && index >= 0 && index
            < _valuatorList[station].size())
    {
        return _valuatorList[station][index];
    }
    return 0.0;
}

bool TrackingManager::getUsingMouseTracker()
{
    return _mouseTracker;
}

void TrackingManager::updateHandMask()
{
    if(!_buttonTracker)
    {
        return;
    }

    for(int i = 0; i < _numHands; i++)
    {
        _handButtonMask[i] = 0;
        int handMaskOffset = 0;
        for(int j = 0; j < _handStationFilterMask[i].size(); j++)
        {
            unsigned int stationMask = 1;
            for(int k = 0; k < CVR_MAX_BUTTONS; k++)
            {
                if(_handStationFilterMask[i][j] & stationMask)
                {
                    unsigned int value =
                            (getRawButtonMask(j) & stationMask) ? 1 : 0;
                    value = value << handMaskOffset;
                    _handButtonMask[i] |= value;

                    handMaskOffset++;
                    if(handMaskOffset == CVR_MAX_BUTTONS)
                    {
                        break;
                    }
                }
                stationMask = stationMask << 1;
            }

            if(handMaskOffset == CVR_MAX_BUTTONS)
            {
                break;
            }
        }
    }
}

void TrackingManager::updateThreadHandMask()
{
    if(!_buttonTracker)
    {
        return;
    }

    for(int i = 0; i < _numHands; i++)
    {
        _threadHandButtonMasks[i] = 0;
        int handMaskOffset = 0;
        for(int j = 0; j < _handStationFilterMask[i].size(); j++)
        {
            unsigned int stationMask = 1;
            for(int k = 0; k < CVR_MAX_BUTTONS; k++)
            {
                if(_handStationFilterMask[i][j] & stationMask)
                {
                    unsigned int value = (_buttonTracker->getButtonMask(j)
                            & stationMask) ? 1 : 0;
                    value = value << handMaskOffset;
                    _threadHandButtonMasks[i] |= value;

                    handMaskOffset++;
                    if(handMaskOffset == CVR_MAX_BUTTONS)
                    {
                        break;
                    }
                }
                stationMask = stationMask << 1;
            }

            if(handMaskOffset == CVR_MAX_BUTTONS)
            {
                break;
            }
        }
    }
}

void TrackingManager::generateButtonEvents()
{
    TrackingInteractionEvent * events = NULL;
    int numEvents;
    if(ComController::instance()->isMaster())
    {
        std::vector<TrackingInteractionEvent> eventList;
        for(int j = 0; j < _numHands; j++)
        {
            unsigned int bit = 1;
            unsigned int newMask = getHandButtonMask(j);
            //std::cerr << "ButtonMask: " << newMask << std::endl;
            for(int i = 0; i < CVR_MAX_BUTTONS; i++)
            {
                //std::cerr << "last mask " << _lastButtonMask << " new mask " << newMask << " bit: " << bit << " " << (newMask & bit) << " " << (_lastButtonMask & bit) << std::endl;
                if(((_lastHandButtonMask[j] & bit) != (newMask & bit))
                        || ((_lastHandButtonMask[j] & bit) && (newMask & bit)))
                {
                    //std::cerr << "last mask " << _lastButtonMask << " new mask " << newMask << std::endl;
                    TrackingInteractionEvent buttonEvent;
                    if((_lastHandButtonMask[j] & bit) && (newMask & bit))
                    {
                        buttonEvent.type = BUTTON_DRAG;
                    }
                    else if(_lastHandButtonMask[j] & bit)
                    {
                        buttonEvent.type = BUTTON_UP;
                    }
                    else
                    {
                        buttonEvent.type = BUTTON_DOWN;
                    }
                    buttonEvent.button = i;
                    // set current pointer info
                    osg::Quat q = _handMatList[j].getRotate();
                    osg::Vec3 pos = _handMatList[j].getTrans();
                    buttonEvent.xyz[0] = pos.x();
                    buttonEvent.xyz[1] = pos.y();
                    buttonEvent.xyz[2] = pos.z();
                    buttonEvent.rot[0] = q.x();
                    buttonEvent.rot[1] = q.y();
                    buttonEvent.rot[2] = q.z();
                    buttonEvent.rot[3] = q.w();
                    buttonEvent.hand = j;
                    eventList.push_back(buttonEvent);
                }
                bit = bit << 1;
            }
            _lastHandButtonMask[j] = newMask;
        }
        numEvents = eventList.size();
        ComController::instance()->sendSlaves(&numEvents, sizeof(int));
        if(numEvents)
        {
            events = new TrackingInteractionEvent[numEvents];
            for(int i = 0; i < numEvents; i++)
            {
                events[i] = eventList[i];
            }
            ComController::instance()->sendSlaves(events, numEvents
                    * sizeof(TrackingInteractionEvent));
        }
    }
    else
    {
        ComController::instance()->readMaster(&numEvents, sizeof(int));
        if(numEvents)
        {
            events = new TrackingInteractionEvent[numEvents];
            ComController::instance()->readMaster(events, numEvents
                    * sizeof(TrackingInteractionEvent));
        }
    }

    for(int i = 0; i < numEvents; i++)
    {
        InteractionManager::instance()->handleEvent(&events[i]);
    }

    if(events)
    {
        delete[] events;
    }
}

void TrackingManager::generateThreadButtonEvents()
{
    TrackingInteractionEvent * buttonEvent;
    for(int j = 0; j < _numHands; j++)
    {
        unsigned int bit = 1;
        unsigned int newMask = _threadHandButtonMasks[j];
        //std::cerr << "ButtonMask: " << newMask << std::endl;
        for(int i = 0; i < CVR_MAX_BUTTONS; i++)
        {
            //std::cerr << "last mask " << _lastButtonMask << " new mask " << newMask << " bit: " << bit << " " << (newMask & bit) << " " << (_lastButtonMask & bit) << std::endl;
            if((_lastHandButtonMask[j] & bit) != (newMask & bit))
            {
                buttonEvent = new TrackingInteractionEvent;
                //std::cerr << "last mask " << _lastButtonMask << " new mask " << newMask << std::endl;
                if(_lastHandButtonMask[j] & bit)
                {
                    buttonEvent->type = BUTTON_UP;
                }
                else
                {
                    buttonEvent->type = BUTTON_DOWN;
                }
                buttonEvent->button = i;
                // set current pointer info
                osg::Quat q = _threadHandMatList[j].getRotate();
                osg::Vec3 pos = _threadHandMatList[j].getTrans();
                buttonEvent->xyz[0] = pos.x();
                buttonEvent->xyz[1] = pos.y();
                buttonEvent->xyz[2] = pos.z();
                buttonEvent->rot[0] = q.x();
                buttonEvent->rot[1] = q.y();
                buttonEvent->rot[2] = q.z();
                buttonEvent->rot[3] = q.w();
                buttonEvent->hand = j;
                //_threadEvents.push((InteractionEvent*)buttonEvent);
                genComTrackEvents->processEvent(buttonEvent);
            }
            else if((_lastHandButtonMask[j] & bit) && (newMask & bit))
            {
                buttonEvent = new TrackingInteractionEvent;
                buttonEvent->type = BUTTON_DRAG;
                buttonEvent->button = i;
                // set current pointer info
                osg::Quat q = _threadHandMatList[j].getRotate();
                osg::Vec3 pos = _threadHandMatList[j].getTrans();
                buttonEvent->xyz[0] = pos.x();
                buttonEvent->xyz[1] = pos.y();
                buttonEvent->xyz[2] = pos.z();
                buttonEvent->rot[0] = q.x();
                buttonEvent->rot[1] = q.y();
                buttonEvent->rot[2] = q.z();
                buttonEvent->rot[3] = q.w();
                buttonEvent->hand = j;
                //_threadEvents.push((InteractionEvent*)buttonEvent);
                genComTrackEvents->processEvent(buttonEvent);
            }
            bit = bit << 1;
        }
        _lastHandButtonMask[j] = newMask;
    }
}

void TrackingManager::updateThreadMats()
{
    trackedBody * tb;
    for(int i = 0; i < _numHeads; i++)
    {
        if(_bodyTracker && _bodyTracker->getBody(_headStations[i]))
        {
            tb = _bodyTracker->getBody(_headStations[i]);
            if(tb)
            {
                osg::Vec3 pos(tb->x, tb->y, tb->z);
                osg::Matrix rot;
                //float deg2rad = M_PI / 180.0;
                //rot.makeRotate(tb->r * deg2rad, osg::Vec3(0,0,1), tb->p * deg2rad, osg::Vec3(1,0,0), tb->h * deg2rad, osg::Vec3(0,1,0));
                rot.makeRotate(osg::Quat(tb->qx, tb->qy, tb->qz, tb->qw));
                // TODO: check translation stuff
                _threadHeadMatList[i] = _headTransformsRot[i] * rot
                        * osg::Matrix::translate(pos) * _systemTransform
                        * osg::Matrix::translate(_headTransformsTrans[i]);
            }
        }
        else
        {
            //std::cerr << "Setting Head Position to x: " << _defaultHead->x << " y: " << _defaultHead->y << " z: " << _defaultHead->z << std::endl;
            _threadHeadMatList[i].setTrans(_defaultHead->x, _defaultHead->y,
                                           _defaultHead->z);
            //_headMat =  _headTransformRot * osg::Matrix::translate(osg::Vec3(_defaultHead->x,_defaultHead->y,_defaultHead->z)) * _systemTransform * osg::Matrix::translate(_headTransformTrans);
        }
    }

    for(int i = 0; i < _numHands; i++)
    {
        if(_bodyTracker && _bodyTracker->getBody(_handStations[i]))
        {
            tb = _bodyTracker->getBody(_handStations[i]);
        }
        else
        {
            tb = _defaultHand;
        }

        if(tb)
        {
            osg::Vec3 pos(tb->x, tb->y, tb->z);
            osg::Matrix rot;
            //float deg2rad = M_PI / 180.0;
            //rot.makeRotate(tb->r * deg2rad, osg::Vec3(0,0,1), tb->p * deg2rad, osg::Vec3(1,0,0), tb->h * deg2rad, osg::Vec3(0,1,0));
            rot.makeRotate(osg::Quat(tb->qx, tb->qy, tb->qz, tb->qw));
            // TODO: check translation stuff
            _threadHandMatList[i] = _handTransformsRot[i] * rot
                    * osg::Matrix::translate(pos) * _systemTransform
                    * osg::Matrix::translate(_handTransformsTrans[i]);
        }
    }
}

void TrackingManager::flushEvents()
{
    TrackingInteractionEvent * tie = NULL;
    int numEvents;
    if(ComController::instance()->isMaster())
    {
        numEvents = _threadEvents.size();
        ComController::instance()->sendSlaves(&numEvents, sizeof(int));
        if(numEvents)
        {
            tie = new TrackingInteractionEvent[_threadEvents.size()];
            int index = 0;
            while(_threadEvents.size())
            {
                tie[index]
                        = *((TrackingInteractionEvent*)_threadEvents.front());
                delete _threadEvents.front();
                _threadEvents.pop();
                index++;
            }
            ComController::instance()->sendSlaves(tie, numEvents
                    * sizeof(struct TrackingInteractionEvent));
        }
    }
    else
    {
        ComController::instance()->readMaster(&numEvents, sizeof(int));
        if(numEvents)
        {
            tie = new TrackingInteractionEvent[numEvents];
            ComController::instance()->readMaster(tie, numEvents
                    * sizeof(struct TrackingInteractionEvent));
        }
    }

    for(int i = 0; i < numEvents; i++)
    {
        InteractionManager::instance()->handleEvent(&tie[i]);
    }

    if(tie)
    {
        delete[] tie;
    }
}

GenComplexTrackingEvents::GenComplexTrackingEvents()
{

    _doubleClickTimeout = ConfigManager::getFloat("Input.DoubleClickTimeout",
                                                  0.4);

    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
        _lastClick.push_back(std::map<int,timeval *>());
        _doubleClicked.push_back(std::map<int,bool>());
    }
}

GenComplexTrackingEvents::~GenComplexTrackingEvents()
{
}

void GenComplexTrackingEvents::processEvent(TrackingInteractionEvent * tie)
{
    if(tie->type == BUTTON_DOWN)
    {
        if(!_lastClick[tie->hand][tie->button])
        {
            struct timeval * tv = new struct timeval;
            gettimeofday(tv, NULL);
            _lastClick[tie->hand][tie->button] = tv;

            _doubleClicked[tie->hand][tie->button] = false;
        }
        else
        {

            timeval now;
            gettimeofday(&now, NULL);

            float interval = (now.tv_sec
                    - _lastClick[tie->hand][tie->button]->tv_sec)
                    + ((now.tv_usec
                            - _lastClick[tie->hand][tie->button]->tv_usec)
                            / 1000000.0);

            if(interval < _doubleClickTimeout
                    && !_doubleClicked[tie->hand][tie->button])
            {
                tie->type = BUTTON_DOUBLE_CLICK;
                _doubleClicked[tie->hand][tie->button] = true;
            }
            else
            {
                *_lastClick[tie->hand][tie->button] = now;
                _doubleClicked[tie->hand][tie->button] = false;
            }
        }

        TrackingManager::instance()->_threadEvents.push(
                                                        (InteractionEvent *)tie);
    }
    else
    {
        TrackingManager::instance()->_threadEvents.push(
                                                        (InteractionEvent *)tie);
    }
}
