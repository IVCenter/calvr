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
//#include <WinBase.h>
#include <util/TimeOfDay.h>
#else
#include <sys/time.h>
#endif

using namespace cvr;

struct TrackingSystemInit
{
    Navigation::NavImplementation nav;
    TrackerBase::TrackerType type;
    SceneManager::PointerGraphicType defaultPointerType;
    bool thread;
    bool genDefaultButtonEvents;
};

TrackingManager * TrackingManager::_myPtr = NULL;

TrackingManager::TrackingManager()
{
    _debugOutput = false;
    _threadQuit = false;
    _currentEvents = NULL;
}

TrackingManager::~TrackingManager()
{
    if(ComController::instance()->isMaster() && isThreaded())
    {
	quitThread();
	join();
    }
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
    _updateHeadTracking = !ConfigManager::getBool("Freeze", false);

    int system = 0;
    bool found = false;

    std::stringstream sss;
    sss << "Input.TrackingSystem" << system;
    ConfigManager::getEntry("value",sss.str(),"",&found);

    std::string configStr = sss.str();
    while(found)
    {
	TrackingSystemInfo * tsi = new TrackingSystemInfo;
	tsi->numBodies = ConfigManager::getInt("value",configStr + ".NumBodies",0);
	tsi->numButtons = ConfigManager::getInt("value",configStr + ".NumButtons",0);
	tsi->numVal = ConfigManager::getInt("value",configStr + ".NumValuators",0);

	float x, y, z, h, p, r;
	x = ConfigManager::getFloat("x", configStr + ".Offset", 0.0);
	y = ConfigManager::getFloat("y", configStr + ".Offset", 0.0);
	z = ConfigManager::getFloat("z", configStr + ".Offset", 0.0);
	h = ConfigManager::getFloat("h", configStr + ".Orientation", 0.0);
	p = ConfigManager::getFloat("p", configStr + ".Orientation", 0.0);
	r = ConfigManager::getFloat("r", configStr + ".Orientation", 0.0);
	osg::Matrix m;
	m.makeRotate(r * M_PI / 180.0, osg::Vec3(0, 1, 0), p * M_PI / 180.0,
		osg::Vec3(1, 0, 0), h * M_PI / 180.0, osg::Vec3(0, 0, 1));
	m.setTrans(osg::Vec3(x, y, z));
	tsi->systemTransform = m;

	for(int i = 0; i < tsi->numBodies; i++)
	{
	    std::stringstream bodyss;
	    bodyss << ".Body" << i;
	    x = ConfigManager::getFloat("x", configStr + bodyss.str() + ".Offset", 0.0);
	    y = ConfigManager::getFloat("y", configStr + bodyss.str() + ".Offset", 0.0);
	    z = ConfigManager::getFloat("z", configStr + bodyss.str() + ".Offset", 0.0);
	    h = ConfigManager::getFloat("h", configStr + bodyss.str() + ".Orientation", 0.0);
	    p = ConfigManager::getFloat("p", configStr + bodyss.str() + ".Orientation", 0.0);
	    r = ConfigManager::getFloat("r", configStr + bodyss.str() + ".Orientation", 0.0);
	    m.makeRotate(r * M_PI / 180.0, osg::Vec3(0, 1, 0), p * M_PI / 180.0,
		    osg::Vec3(1, 0, 0), h * M_PI / 180.0, osg::Vec3(0, 0, 1));
	    tsi->bodyTranslations.push_back(osg::Vec3(x, y, z));
	    tsi->bodyRotations.push_back(m);
	}

	TrackingSystemInit trackInit;
	TrackerBase * tracker;
	if(ComController::instance()->isMaster())
	{
	    std::string systemName = ConfigManager::getEntry("value",configStr,"NONE");
	    if(systemName == "SHMEM")
	    {
		tracker = new TrackerShmem();
	    }
#ifdef WITH_VRPN
	    else if(systemName == "VRPN")
	    {
		tracker = new TrackerVRPN();
	    }
#endif
	    else if(systemName == "MOUSE")
	    {
		tracker = new TrackerMouse();
	    }
	    else
	    {
		std::cerr << "TrackingManager Error: Unknown system: " << systemName << std::endl;
		tracker = NULL;
	    }

	    if(tracker && tracker->init(configStr))
	    {
		trackInit.thread = tracker->thread();
		trackInit.nav = tracker->getNavImplementation();
		trackInit.type = tracker->getTrackerType();
		trackInit.defaultPointerType = tracker->getPointerType();
		trackInit.genDefaultButtonEvents = tracker->genDefaultButtonEvents();
	    }
	    else
	    {
		trackInit.thread = false;
		trackInit.nav = Navigation::NONE_NAV;
		trackInit.type = TrackerBase::TRACKER;
		trackInit.defaultPointerType = SceneManager::NONE;
		trackInit.genDefaultButtonEvents = false;
	    }

	    ComController::instance()->sendSlaves(&trackInit, sizeof(struct TrackingSystemInit));
	}
	else
	{
	    tracker = new TrackerSlave(tsi->numBodies, tsi->numButtons, tsi->numVal);
	    ComController::instance()->readMaster(&trackInit, sizeof(struct TrackingSystemInit));
	}

	tsi->navImp = trackInit.nav;
	tsi->trackerType = trackInit.type;
	tsi->defaultPointerType = trackInit.defaultPointerType;
	tsi->thread = trackInit.thread;
	tsi->genDefaultButtonEvents = trackInit.genDefaultButtonEvents;

	_systems.push_back(tracker);
	_systemInfo.push_back(tsi);

	system++;
	std::stringstream sss2;
	sss2 << "Input.TrackingSystem" << system;
	ConfigManager::getEntry("value",sss2.str(),"",&found);
	configStr = sss2.str();
    }

    _threaded = false;
    for(int i = 0; i < _systemInfo.size(); i++)
    {
	if(_systemInfo[i]->thread)
	{
	    _threaded = true;
	}
    }

    if(_threaded)
    {
	_threadFPS = ConfigManager::getFloat("FPS", "Input.Threaded", 60.0);
    }

    _numHands = ConfigManager::getInt("Input.NumHands", 1);
    _numHeads = ConfigManager::getInt("Input.NumHeads", 1);
    if(_numHands < 0)
    {
	_numHands = 0;
    }
    if(_numHeads < 0)
    {
	_numHeads = 0;
    }

    for(int i = 0; i < _numHands; i++)
    {
	std::stringstream handss;
	handss << "Input.Hand" << i;

	_handAddress.push_back(std::pair<int,int>(ConfigManager::getInt("system",handss.str(),0),
	                                          ConfigManager::getInt("body",handss.str(),0)));
	_threadHandButtonMasks.push_back(0);
        _threadHandMatList.push_back(osg::Matrix());
	_handMatList.push_back(osg::Matrix());
        _handButtonMask.push_back(0);
        _lastHandButtonMask.push_back(0);

	_genHandDefaultButtonEvents.push_back(std::vector<bool>());
	_handStationFilterMask.push_back(std::vector<unsigned int>());

	std::string maskTag = handss.str() + ".ButtonMask";
	for(int j = 0; j < _systems.size(); j++)
	{
	    std::stringstream maskss;
	    maskss << "system" << j;
	    std::string mask;
	    mask = ConfigManager::getEntry(maskss.str(), maskTag, i ? "0x0" : "0xFFFFFFFF");

	    char * eptr;
	    unsigned long int imask = strtol(mask.c_str(), &eptr, 0);
	    _handStationFilterMask[i].push_back((unsigned int)imask);
	}
    }

    setGenHandDefaultButtonEvents();

    for(int i = 0; i < _numHeads; i++)
    {
	std::stringstream headss;
	headss << "Input.Head" << i;

	_headAddress.push_back(std::pair<int,int>(ConfigManager::getInt("system",headss.str(),0),
	                                          ConfigManager::getInt("body",headss.str(),0)));
	_threadHeadMatList.push_back(osg::Matrix());
	_headMatList.push_back(osg::Matrix());
	_lastUpdatedHeadMatList.push_back(osg::Matrix());
    }

    if(!_numHeads)
    {
	_headMatList.push_back(osg::Matrix());
	_lastUpdatedHeadMatList.push_back(osg::Matrix());
    }

    _totalBodies = 0;
    _totalButtons = 0;
    _totalValuators = 0;

    for(int i = 0; i < _systems.size(); i++)
    {
	_valuatorList.push_back(std::vector<float>());
	_totalBodies += _systemInfo[i]->numBodies;
	_totalButtons += _systemInfo[i]->numButtons;
	_totalValuators += _systemInfo[i]->numVal;
	for(int j = 0; j < _systemInfo[i]->numVal; j++)
	{
	    _valuatorList[i].push_back(0.0);
	}
	_rawButtonMask.push_back(0);
    }

    float vx, vy, vz, vh, vp, vr;
    osg::Matrix vTrans, vRot;
    //if(_numHeads)
    {
	vx = ConfigManager::getFloat("x", "ViewerPosition", 0);
	vy = ConfigManager::getFloat("y", "ViewerPosition", 0);
	vz = ConfigManager::getFloat("z", "ViewerPosition", 0);

	vh = ConfigManager::getFloat("h", "ViewerPosition", 0);
	vp = ConfigManager::getFloat("p", "ViewerPosition", 0);
	vr = ConfigManager::getFloat("r", "ViewerPosition", 0);

	vTrans.makeTranslate(vx, vy, vz);
	vRot.makeRotate(vr,osg::Vec3(0,1,0),vp,osg::Vec3(1,0,0),vh,osg::Vec3(0,0,1));

	_headMatList[0] = vRot * vTrans;
	_lastUpdatedHeadMatList[0] = _headMatList[0];

    }

    for(int i = 1; i < _numHeads; i++)
    {
	std::stringstream vTag;
	vTag << "Viewer" << i+1 << "Position";

	vx = ConfigManager::getFloat("x", vTag.str(), 0);
	vy = ConfigManager::getFloat("y", vTag.str(), 0);
	vz = ConfigManager::getFloat("z", vTag.str(), 0);

	vh = ConfigManager::getFloat("h", vTag.str(), 0);
	vp = ConfigManager::getFloat("p", vTag.str(), 0);
	vr = ConfigManager::getFloat("r", vTag.str(), 0);

	vTrans.makeTranslate(vx, vy, vz);
	vRot.makeRotate(vr,osg::Vec3(0,1,0),vp,osg::Vec3(1,0,0),vh,osg::Vec3(0,0,1));

	_headMatList[i] = vRot * vTrans;
	_lastUpdatedHeadMatList[i] = _headMatList[i];
    }

    /*if(!_bodyTracker || !_bodyTracker->getNumBodies())
    {
        update();
    }*/

    for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
    {
	_eventMap[i] = std::list<InteractionEvent*>();
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

    //TODO: zero size check
    int totalData = _totalBodies * sizeof(struct trackedBody) + _systemInfo.size() * sizeof(unsigned int) + _totalValuators * sizeof(float);
    char * data = new char[totalData];

    //std::cerr << "Update Called." << std::endl;
    if(ComController::instance()->isMaster())
    {
	static trackedBody * zeroBody = NULL;
	if(!zeroBody)
	{
	    zeroBody = new trackedBody;
	    zeroBody->x = 0;
	    zeroBody->y = 0;
	    zeroBody->z = 0;
	    osg::Quat q;
	    zeroBody->qx = q.x();
	    zeroBody->qy = q.y();
	    zeroBody->qz = q.z();
	    zeroBody->qw = q.w();
	}

	trackedBody * tbptr = (trackedBody*)data;
	unsigned int * buttonptr = (unsigned int *)(data + (_totalBodies * sizeof(struct trackedBody)));
	float * valptr = (float *)(data + (_totalBodies * sizeof(struct trackedBody) + _systemInfo.size() * sizeof(unsigned int)));
	for(int i = 0; i < _systems.size(); i++)
	{
	    if(_systems[i])
	    {
		if(!_systemInfo[i]->thread)
		{
		    _systems[i]->update(_eventMap);
		}
		for(int j = 0; j < _systemInfo[i]->numBodies; j++)
		{
		    trackedBody * tb = _systems[i]->getBody(j);
		    if(tb)
		    {
			*tbptr = *tb;
		    }
		    else
		    {
			*tbptr = *zeroBody;
		    }
		    tbptr++;
		}

		*buttonptr = _systems[i]->getButtonMask();
		buttonptr++;

		for(int j = 0; j < _systemInfo[i]->numVal; j++)
		{
		    *valptr = _systems[i]->getValuator(j);
		    valptr++;
		}
	    }
	    else
	    {
		for(int j = 0; j < _systemInfo[i]->numBodies; j++)
		{
		    *tbptr = *zeroBody;
		    tbptr++;
		}
		
		*buttonptr = 0;
		buttonptr++;

		for(int j = 0; j < _systemInfo[i]->numVal; j++)
		{
		    *valptr = 0.0;
		    valptr++;
		}
	    }
        }
	ComController::instance()->sendSlaves(data,totalData);
    }
    else
    {
	ComController::instance()->readMaster(data,totalData);
	trackedBody * tbptr = (trackedBody*)data;
	unsigned int * buttonptr = (unsigned int *)(data + (_totalBodies * sizeof(struct trackedBody)));
	float * valptr = (float *)(data + (_totalBodies * sizeof(struct trackedBody) + _systemInfo.size() * sizeof(unsigned int)));

	for(int i = 0; i < _systems.size(); i++)
	{
	    TrackerSlave * sTracker = NULL;
	    sTracker = dynamic_cast<TrackerSlave*>(_systems[i]);
	    if(sTracker)
	    {
		sTracker->readValues(tbptr, buttonptr, valptr);
	    }
	    tbptr += _systemInfo[i]->numBodies;
	    buttonptr++;
	    valptr += _systemInfo[i]->numVal;
	}
    }

    if(data)
    {
	delete[] data;
    }

    trackedBody * tb;
    for(int i = 0; i < _numHeads; i++)
    {
        if(_systems[_headAddress[i].first] && _systems[_headAddress[i].first]->getBody(_headAddress[i].second))
        {
            tb = _systems[_headAddress[i].first]->getBody(_headAddress[i].second);
            if(tb)
            {
                osg::Vec3 pos(tb->x, tb->y, tb->z);
                osg::Matrix rot;
                rot.makeRotate(osg::Quat(tb->qx, tb->qy, tb->qz, tb->qw));
                _headMatList[i] = _systemInfo[_headAddress[i].first]->bodyRotations[_headAddress[i].second] * rot
                        * osg::Matrix::translate(pos) * _systemInfo[_headAddress[i].first]->systemTransform
                        * osg::Matrix::translate(_systemInfo[_headAddress[i].first]->bodyTranslations[_headAddress[i].second]);
			
		if(_updateHeadTracking)
		{
		    _lastUpdatedHeadMatList[i] = _headMatList[i];
		}
            }
        }
    }

    for(int i = 0; i < _numHands; i++)
    {
	if(_systems[_handAddress[i].first] && _systems[_handAddress[i].first]->getBody(_handAddress[i].second))
        {
            tb = _systems[_handAddress[i].first]->getBody(_handAddress[i].second);
            if(tb)
            {
                osg::Vec3 pos(tb->x, tb->y, tb->z);
                osg::Matrix rot;
                rot.makeRotate(osg::Quat(tb->qx, tb->qy, tb->qz, tb->qw));
                _handMatList[i] = _systemInfo[_handAddress[i].first]->bodyRotations[_handAddress[i].second] * rot
                        * osg::Matrix::translate(pos) * _systemInfo[_handAddress[i].first]->systemTransform
                        * osg::Matrix::translate(_systemInfo[_handAddress[i].first]->bodyTranslations[_handAddress[i].second]);
            }
        }
    }

    for(int i = 0; i < _rawButtonMask.size(); i++)
    {
	if(_systems[i])
	{
	    _rawButtonMask[i] = _systems[i]->getButtonMask();
	}
	else
	{
	    _rawButtonMask[i] = 0;
	}
    }

    for(int i = 0; i < _valuatorList.size(); i++)
    {
        for(int j = 0; j < _valuatorList[i].size(); j++)
        {
	    if(_systems[i])
	    {
		_valuatorList[i][j] = _systems[i]->getValuator(j);
	    }
	    else
	    {
		_valuatorList[i][j] = 0;
	    }
        }
    }

    updateHandMask();

    generateButtonEvents();
    flushEvents();

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

	for(int i = 0; i < _systems.size(); i++)
	{
	    if(_systems[i] && _systemInfo[i]->thread)
	    {
		_systems[i]->update(_eventMap);
	    }
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

	//TODO: add to debug
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

SceneManager::PointerGraphicType TrackingManager::getPointerGraphicType(int hand)
{
    if(hand >= 0 && hand < _numHands)
    {
	if(_handAddress[hand].first >= 0 && _handAddress[hand].first < _systemInfo.size())
	{
	    if(_handAddress[hand].second >= 0 && _handAddress[hand].second < _systemInfo[_handAddress[hand].first]->numBodies)
	    {
		return _systemInfo[_handAddress[hand].first]->defaultPointerType;
	    }
	}
    }

    return SceneManager::NONE;
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
    if(!_numHeads && !head)
    {
	return _lastUpdatedHeadMatList[head];
    }

    if(head < 0 || head >= _lastUpdatedHeadMatList.size())
    {
        static osg::Matrix m;
        return m;
    }
    return _lastUpdatedHeadMatList[head];
}

osg::Matrix & TrackingManager::getUnfrozenHeadMat(int head)
{
    if(!_numHeads && !head)
    {
	return _headMatList[head];
    }

    if(head < 0 || head >= _headMatList.size())
    {
        static osg::Matrix m;
        return m;
    }
    return _headMatList[head];
}

int TrackingManager::getNumTrackingSystems()
{
    return _systems.size();
}

TrackerBase::TrackerType TrackingManager::getHandTrackerType(int hand)
{
    if(hand < 0 || hand >= _numHands || _handAddress[hand].first < 0 || _handAddress[hand].first >= _systemInfo.size() || _handAddress[hand].second < 0 || _handAddress[hand].second >= _systemInfo[_handAddress[hand].first]->numBodies)
    {
	return TrackerBase::INVALID;
    }

    return _systemInfo[_handAddress[hand].first]->trackerType;
}

int TrackingManager::getNumButtons(int system)
{
    if(system >= 0 && system < _systems.size() && _systems[system])
    {
	return _systemInfo[system]->numButtons;
    }
    return 0;
}

unsigned int TrackingManager::getRawButtonMask(int system)
{
    if(system >= 0 && system < _rawButtonMask.size())
    {
        return _rawButtonMask[system];
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

int TrackingManager::getNumValuators(int system)
{
    if(system >= 0 && system < _valuatorList.size())
    {
        return _valuatorList[system].size();
    }
    return 0;
}

float TrackingManager::getValuator(int system, int index)
{
    if(system >= 0 && system < _valuatorList.size() && index >= 0 && index
            < _valuatorList[system].size())
    {
        return _valuatorList[system][index];
    }
    return 0.0;
}

void TrackingManager::setUpdateHeadTracking(bool b)
{
    _updateHeadTracking = b;
}

bool TrackingManager::getUpdateHeadTracking()
{
    return _updateHeadTracking;
}

void TrackingManager::cleanupCurrentEvents()
{
    /*if(_currentEvents)
    {
	delete[] _currentEvents;
	_currentEvents = NULL;
    }*/
}

void TrackingManager::updateHandMask()
{
    for(int i = 0; i < _numHands; i++)
    {
        _handButtonMask[i] = 0;
        int handMaskOffset = 0;
        for(int j = 0; j < _handStationFilterMask[i].size(); j++)
        {
            unsigned int stationMask = 1;
            for(int k = 0; k < _systemInfo[j]->numButtons; k++)
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
    for(int i = 0; i < _numHands; i++)
    {
        _threadHandButtonMasks[i] = 0;
        int handMaskOffset = 0;
        for(int j = 0; j < _systems.size(); j++)
        {
            unsigned int stationMask = 1;
            for(int k = 0; k < _systemInfo[j]->numButtons; k++)
            {
                if(_handStationFilterMask[i][j] & stationMask)
                {
		    if(_systems[j])
		    {
			unsigned int value = (_systems[j]->getButtonMask()
				& stationMask) ? 1 : 0;
			value = value << handMaskOffset;
			_threadHandButtonMasks[i] |= value;
		    }

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
    int numEvents;
    if(ComController::instance()->isMaster())
    {
        std::vector<TrackingInteractionEvent> eventList;
        for(int j = 0; j < _numHands; j++)
        {
            unsigned int bit = 1;
            unsigned int newMask = getHandButtonMask(j);
            //std::cerr << "ButtonMask: " << newMask << std::endl;
            for(int i = 0; i < _genHandDefaultButtonEvents[j].size(); i++)
            {
		if(!_genHandDefaultButtonEvents[j][i])
		{
		    bit = bit << 1;
		    continue;
		}
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
            _currentEvents = new TrackingInteractionEvent[numEvents];
            for(int i = 0; i < numEvents; i++)
            {
                _currentEvents[i] = eventList[i];
            }
            ComController::instance()->sendSlaves(_currentEvents, numEvents
                    * sizeof(TrackingInteractionEvent));
        }
    }
    else
    {
        ComController::instance()->readMaster(&numEvents, sizeof(int));
        if(numEvents)
        {
            _currentEvents = new TrackingInteractionEvent[numEvents];
            ComController::instance()->readMaster(_currentEvents, numEvents
                    * sizeof(TrackingInteractionEvent));
        }
    }

    TrackingInteractionEvent * ie;
    for(int i = 0; i < numEvents; i++)
    {
        ie = new TrackingInteractionEvent;
        *ie = _currentEvents[i];
        InteractionManager::instance()->addEvent(ie);
    }

    if(_currentEvents)
    {
        delete[] _currentEvents;
        _currentEvents = NULL;
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
        for(int i = 0; i < _genHandDefaultButtonEvents[j].size(); i++)
        {
	    if(!_genHandDefaultButtonEvents[j][i])
	    {
		bit = bit << 1;
		continue;
	    }
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
	if(_systems[_headAddress[i].first] && _systems[_headAddress[i].first]->getBody(_headAddress[i].second))
        {
            tb = _systems[_headAddress[i].first]->getBody(_headAddress[i].second);
            if(tb)
            {
                osg::Vec3 pos(tb->x, tb->y, tb->z);
                osg::Matrix rot;
                rot.makeRotate(osg::Quat(tb->qx, tb->qy, tb->qz, tb->qw));
                _threadHeadMatList[i] = _systemInfo[_headAddress[i].first]->bodyRotations[_headAddress[i].second] * rot
                        * osg::Matrix::translate(pos) * _systemInfo[_headAddress[i].first]->systemTransform
                        * osg::Matrix::translate(_systemInfo[_headAddress[i].first]->bodyTranslations[_headAddress[i].second]);
            }
        }
    }

    for(int i = 0; i < _numHands; i++)
    {
	if(_systems[_handAddress[i].first] && _systems[_handAddress[i].first]->getBody(_handAddress[i].second))
        {
            tb = _systems[_handAddress[i].first]->getBody(_handAddress[i].second);
            if(tb)
            {
                osg::Vec3 pos(tb->x, tb->y, tb->z);
                osg::Matrix rot;
                rot.makeRotate(osg::Quat(tb->qx, tb->qy, tb->qz, tb->qw));
                _threadHandMatList[i] = _systemInfo[_handAddress[i].first]->bodyRotations[_handAddress[i].second] * rot
                        * osg::Matrix::translate(pos) * _systemInfo[_handAddress[i].first]->systemTransform
                        * osg::Matrix::translate(_systemInfo[_handAddress[i].first]->bodyTranslations[_handAddress[i].second]);
            }
        }
    }
}

void TrackingManager::flushEvents()
{
    int * numEvents = new int[NUM_INTER_EVENT_TYPES];
    if(ComController::instance()->isMaster())
    {
	for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
	{
	    numEvents[i] = _eventMap[i].size();
	}
	ComController::instance()->sendSlaves(numEvents,NUM_INTER_EVENT_TYPES * sizeof(int));
    }
    else
    {
	ComController::instance()->readMaster(numEvents,NUM_INTER_EVENT_TYPES * sizeof(int));
    }

    int eventsDataSize = 0;

    for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
    {
	//TODO: change this when the new event class structure is in place
	if(i == TRACKING_INTER_EVENT)
	{
	    eventsDataSize += numEvents[i] * sizeof(struct TrackingInteractionEvent);
	}
	else if(i == MOUSE_INTER_EVENT)
	{
	    eventsDataSize += numEvents[i] * sizeof(struct MouseInteractionEvent);
	}
    }

    //std::cerr << "eventsDataSize: " << eventsDataSize << std::endl;
    char * data = NULL;
    if(eventsDataSize > 0)
    {
	data = new char[eventsDataSize];
    }
    if(ComController::instance()->isMaster())
    {
	//TODO: change this when the new event class structure is in place
	char * eventptr = data;
	for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
	{
	    if(i == TRACKING_INTER_EVENT)
	    {
		for(std::list<InteractionEvent*>::iterator it = _eventMap[i].begin(); it != _eventMap[i].end(); it++)
		{
		    *((TrackingInteractionEvent*)eventptr) = *((TrackingInteractionEvent*)(*it));
		    InteractionManager::instance()->addEvent(*it);
		    eventptr += sizeof(struct TrackingInteractionEvent);
		}
	    }
	    else if(i == MOUSE_INTER_EVENT)
	    {
		//std::cerr << "Found " << _eventMap[i].size() << " mouse events." << std::endl;
		for(std::list<InteractionEvent*>::iterator it = _eventMap[i].begin(); it != _eventMap[i].end(); it++)
		{
		    //std::cerr << "Got mouse event." << std::endl;
		    *((MouseInteractionEvent*)eventptr) = *((MouseInteractionEvent*)(*it));
		    InteractionManager::instance()->addEvent(*it);
		    eventptr += sizeof(struct MouseInteractionEvent);
		}
	    }
	    else
	    {
		for(std::list<InteractionEvent*>::iterator it = _eventMap[i].begin(); it != _eventMap[i].end(); it++)
		{
		    delete *it;
		}
	    }
	}
	ComController::instance()->sendSlaves(data,eventsDataSize);
    }
    else
    {
	ComController::instance()->readMaster(data,eventsDataSize);
	char * eventptr = data;
	for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
	{
	    if(i == TRACKING_INTER_EVENT)
	    {
		for(int j = 0; j < numEvents[i]; j++)
		{
		    TrackingInteractionEvent * tie = new TrackingInteractionEvent;
		    *tie = *((TrackingInteractionEvent*)eventptr);
		    InteractionManager::instance()->addEvent(tie);
		    eventptr += sizeof(struct TrackingInteractionEvent);
		}
	    }
	    if(i == MOUSE_INTER_EVENT)
	    {
		for(int j = 0; j < numEvents[i]; j++)
		{
		    MouseInteractionEvent * mie = new MouseInteractionEvent;
		    *mie = *((MouseInteractionEvent*)eventptr);
		    InteractionManager::instance()->addEvent(mie);
		    eventptr += sizeof(struct MouseInteractionEvent);
		}
	    }
	}
    }

    if(ComController::instance()->isMaster())
    {
	for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
	{
	    _eventMap[i].clear();
	}
    }

    if(data)
    {
	delete[] data;
    }

    if(numEvents)
    {
	delete[] numEvents;
    }

    /*int numEvents;
    if(ComController::instance()->isMaster())
    {
        numEvents = _threadEvents.size();
        ComController::instance()->sendSlaves(&numEvents, sizeof(int));
        if(numEvents)
        {
            _currentEvents = new TrackingInteractionEvent[_threadEvents.size()];
            int index = 0;
            while(_threadEvents.size())
            {
                _currentEvents[index]
                        = *((TrackingInteractionEvent*)_threadEvents.front());
                delete _threadEvents.front();
                _threadEvents.pop();
                index++;
            }
            ComController::instance()->sendSlaves(_currentEvents, numEvents
                    * sizeof(struct TrackingInteractionEvent));
        }
    }
    else
    {
        ComController::instance()->readMaster(&numEvents, sizeof(int));
        if(numEvents)
        {
            _currentEvents = new TrackingInteractionEvent[numEvents];
            ComController::instance()->readMaster(_currentEvents, numEvents
                    * sizeof(struct TrackingInteractionEvent));
        }
    }

    TrackingInteractionEvent * ie;
    for(int i = 0; i < numEvents; i++)
    {
        ie = new TrackingInteractionEvent;
        *ie = _currentEvents[i];
        InteractionManager::instance()->addEvent(ie);
    }

    if(_currentEvents)
    {
        delete[] _currentEvents;
        _currentEvents = NULL;
    }*/
}

void TrackingManager::setGenHandDefaultButtonEvents()
{
    for(int i = 0; i < _numHands; i++)
    {
        int handMaskOffset = 0;
        for(int j = 0; j < _handStationFilterMask[i].size(); j++)
        {
            unsigned int stationMask = 1;
            for(int k = 0; k < _systemInfo[j]->numButtons; k++)
            {
                if(_handStationFilterMask[i][j] & stationMask)
                {
		    _genHandDefaultButtonEvents[i].push_back(_systemInfo[j]->genDefaultButtonEvents);

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

        TrackingManager::instance()->_eventMap[(int)TRACKING_INTER_EVENT].push_back(
                                                        (InteractionEvent *)tie);
    }
    else
    {
        TrackingManager::instance()->_eventMap[(int)TRACKING_INTER_EVENT].push_back((InteractionEvent *)tie);
    }
}
