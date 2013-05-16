#include <cvrInput/TrackingManager.h>
#include <cvrInput/TrackerSlave.h>
#include <cvrInput/TrackerShmem.h>
#include <cvrInput/TrackerMouse.h>

#ifdef WITH_OMICRON
#include <cvrInput/TrackerOmicron.h>
#endif

#ifdef WITH_VRPN
#include <cvrInput/TrackerVRPN.h>
#include <cvrInput/TrackerGyroMouse.h>
#endif

#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/ComController.h>
#include <cvrKernel/InteractionManager.h>

#include <iostream>
#include <sstream>

#include <osg/Vec3>
#include <osg/Vec4>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#include <cvrUtil/TimeOfDay.h>
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
        bool initOK;
};

TrackingManager * TrackingManager::_myPtr = NULL;

TrackingManager::TrackingManager()
{
    _debugOutput = false;
    _threadQuit = false;
    genComTrackEvents = NULL;
}

TrackingManager::~TrackingManager()
{
    if(ComController::instance()->isMaster() && isThreaded())
    {
        quitThread();
        join();
    }

    if(genComTrackEvents)
    {
        delete genComTrackEvents;
    }

    for(int i = 0; i < _systemInfo.size(); i++)
    {
        delete _systemInfo[i];
    }

    for(int i = 0; i < _systems.size(); i++)
    {
        if(_systems[i])
        {
            delete _systems[i];
        }
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
    _debugOutput = ComController::instance()->isMaster() ? ConfigManager::getBool("Input.TrackingDebug",false) : false;
    _updateHeadTracking = !ConfigManager::getBool("Freeze",false);

    int system = 0;
    bool found = false;

    std::stringstream sss;
    sss << "Input.TrackingSystem" << system;
    ConfigManager::getEntry("value",sss.str(),"",&found);

    std::string configStr = sss.str();
    while(found)
    {
        TrackingSystemInfo * tsi = new TrackingSystemInfo;
        tsi->numBodies = ConfigManager::getInt("value",configStr + ".NumBodies",
                0);
        tsi->numButtons = ConfigManager::getInt("value",
                configStr + ".NumButtons",0);
        tsi->numVal = ConfigManager::getInt("value",configStr + ".NumValuators",
                0);

        float x, y, z, h, p, r;
        x = ConfigManager::getFloat("x",configStr + ".Offset",0.0);
        y = ConfigManager::getFloat("y",configStr + ".Offset",0.0);
        z = ConfigManager::getFloat("z",configStr + ".Offset",0.0);
        h = ConfigManager::getFloat("h",configStr + ".Orientation",0.0);
        p = ConfigManager::getFloat("p",configStr + ".Orientation",0.0);
        r = ConfigManager::getFloat("r",configStr + ".Orientation",0.0);
        osg::Matrix m;
        m.makeRotate(r * M_PI / 180.0,osg::Vec3(0,1,0),p * M_PI / 180.0,
                osg::Vec3(1,0,0),h * M_PI / 180.0,osg::Vec3(0,0,1));
        m.setTrans(osg::Vec3(x,y,z));
        tsi->systemTransform = m;

        for(int i = 0; i < tsi->numBodies; i++)
        {
            std::stringstream bodyss;
            bodyss << ".Body" << i;
            x = ConfigManager::getFloat("x",
                    configStr + bodyss.str() + ".Offset",0.0);
            y = ConfigManager::getFloat("y",
                    configStr + bodyss.str() + ".Offset",0.0);
            z = ConfigManager::getFloat("z",
                    configStr + bodyss.str() + ".Offset",0.0);
            h = ConfigManager::getFloat("h",
                    configStr + bodyss.str() + ".Orientation",0.0);
            p = ConfigManager::getFloat("p",
                    configStr + bodyss.str() + ".Orientation",0.0);
            r = ConfigManager::getFloat("r",
                    configStr + bodyss.str() + ".Orientation",0.0);
            m.makeRotate(r * M_PI / 180.0,osg::Vec3(0,1,0),p * M_PI / 180.0,
                    osg::Vec3(1,0,0),h * M_PI / 180.0,osg::Vec3(0,0,1));
            tsi->bodyTranslations.push_back(osg::Vec3(x,y,z));
            tsi->bodyRotations.push_back(m);
        }

        TrackingSystemInit trackInit;
        TrackerBase * tracker;
        if(ComController::instance()->isMaster())
        {
            std::string systemName = ConfigManager::getEntry("value",configStr,
                    "NONE");
            if(systemName == "SHMEM")
            {
                tracker = new TrackerShmem();
            }
#ifdef WITH_OMICRON
	    else if(systemName == "OMICRON")
	    {
		tracker = new TrackerOmicron();
	    }
#endif
#ifdef WITH_VRPN
            else if(systemName == "VRPN")
            {
                tracker = new TrackerVRPN();
            }
	    else if(systemName == "GYROMOUSE")
            {
                tracker = new TrackerGyroMouse();
            }
#endif
            else if(systemName == "MOUSE")
            {
                tracker = new TrackerMouse();
            }
            else
            {
                std::cerr << "TrackingManager Error: Unknown system: "
                        << systemName << std::endl;
                tracker = NULL;
            }

            if(tracker && tracker->init(configStr))
            {
                trackInit.thread = tracker->thread();
                trackInit.nav = tracker->getNavImplementation();
                trackInit.type = tracker->getTrackerType();
                trackInit.defaultPointerType = tracker->getPointerType();
                trackInit.genDefaultButtonEvents =
                        tracker->genDefaultButtonEvents();
                trackInit.initOK = true;
            }
            else
            {
                trackInit.thread = false;
                trackInit.nav = Navigation::NONE_NAV;
                trackInit.type = TrackerBase::TRACKER;
                trackInit.defaultPointerType = SceneManager::NONE;
                trackInit.genDefaultButtonEvents = false;
                trackInit.initOK = false;
                if(tracker)
                {
                    delete tracker;
                    tracker = NULL;
                }
            }

            ComController::instance()->sendSlaves(&trackInit,
                    sizeof(struct TrackingSystemInit));
        }
        else
        {
            ComController::instance()->readMaster(&trackInit,
                    sizeof(struct TrackingSystemInit));
            if(trackInit.initOK)
            {
                tracker = new TrackerSlave(tsi->numBodies,tsi->numButtons,
                        tsi->numVal);
            }
            else
            {
                tracker = NULL;
            }

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
        _threadFPS = ConfigManager::getFloat("FPS","Input.Threaded",60.0);
    }

    _numHands = ConfigManager::getInt("Input.NumHands",1);
    _numHeads = ConfigManager::getInt("Input.NumHeads",1);
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

        _handAddress.push_back(
                std::pair<int,int>(
                        ConfigManager::getInt("system",
                                handss.str() + ".Address",0),
                        ConfigManager::getInt("body",handss.str() + ".Address",
                                0)));
        _threadHandButtonMasks.push_back(0);
        _threadLastHandButtonMask.push_back(0);
        _threadHandMatList.push_back(osg::Matrix());
        _handMatList.push_back(osg::Matrix());
        _handButtonMask.push_back(0);
        _lastHandButtonMask.push_back(0);
	_handToHeadMap.push_back(ConfigManager::getInt("value",handss.str() + ".Head",0));

        _genHandDefaultButtonEvents.push_back(std::vector<bool>());
        _handStationFilterMask.push_back(std::vector<unsigned int>());

        std::string maskTag = handss.str() + ".ButtonMask";
        for(int j = 0; j < _systems.size(); j++)
        {
            std::stringstream maskss;
            maskss << "system" << j;
            std::string mask;
            mask = ConfigManager::getEntry(maskss.str(),maskTag,"0x0");

            char * eptr;
            unsigned long int imask = strtol(mask.c_str(),&eptr,0);
            _handStationFilterMask[i].push_back((unsigned int)imask);
        }

	bool valFound = false;
	SceneManager::PointerGraphicType pgt = SceneManager::NONE;
	std::string pgtstr = ConfigManager::getEntry("value",handss.str() + ".PointerType","NONE",&valFound);
	if(valFound)
	{
	    if(pgtstr == "CONE")
	    {
		pgt = SceneManager::CONE;
	    }
	}
	else
	{
	    if(_handAddress[i].first >= 0
		    && _handAddress[i].first < _systemInfo.size())
	    {
		if(_handAddress[i].second >= 0
			&& _handAddress[i].second
			< _systemInfo[_handAddress[i].first]->numBodies)
		{
		    pgt = _systemInfo[_handAddress[i].first]->defaultPointerType;
		}
	    }
	}
	_handGraphicType.push_back(pgt);

	valFound = false;
	Navigation::NavImplementation ni = Navigation::NONE_NAV;
	std::string nistr = ConfigManager::getEntry("value",handss.str() + ".NavType","NONE",&valFound);
	if(valFound)
	{
	    if(nistr == "MOUSE")
	    {
		ni = Navigation::MOUSE_NAV;
	    }
	    else if(nistr == "MOUSEKEYBOARD")
	    {
		ni = Navigation::MOUSEKEYBOARD_NAV;
	    }
	    else if(nistr == "TRACKER")
	    {
		ni = Navigation::TRACKER_NAV;
	    }
	}
	else
	{
	    if(_handAddress[i].first < 0
		    || _handAddress[i].first >= _systemInfo.size()
		    || _handAddress[i].second < 0
		    || _handAddress[i].second
		    >= _systemInfo[_handAddress[i].first]->numBodies)
	    {
		ni = Navigation::NONE_NAV;
	    }

            if(_handAddress[i].first >= 0 && _handAddress[i].first < _systemInfo.size() && _systemInfo[_handAddress[i].first])
            {
	        ni = _systemInfo[_handAddress[i].first]->navImp;
            }
            else
            {
                ni = Navigation::NONE_NAV;
            }
	}
	_handNavImplementation.push_back(ni);

	_handGenPositionEvents.push_back(ConfigManager::getBool("value",handss.str() + ".GenPositionEvents",false));

	_eventValuatorAddress.push_back(std::vector<std::pair<int,int> >());
        _eventValuatorType.push_back(std::vector<ValuatorType>());
        _eventValuators.push_back(std::vector<float>());
	_eventValuatorTime.push_back(std::vector<double>());
        _lastEventValuators.push_back(std::map<int,float>());
	_numEventValuators.push_back(0);

	valFound = false;

	do
	{
	    std::stringstream valss;
            valss << handss.str() << ".Valuator" << _numEventValuators[i];
            int system, number;
            system = ConfigManager::getInt("system",valss.str(),0,&valFound);

	    if(valFound)
	    {
		_numEventValuators[i]++;
		number = ConfigManager::getInt("number",valss.str(),0);
		_eventValuatorAddress[i].push_back(
			std::pair<int,int>(system,number));
		std::string type = ConfigManager::getEntry("type",valss.str(),
			"NON_ZERO");
		if(type == "CHANGE")
		{
		    _eventValuatorType[i].push_back(CHANGE);
		}
		else
		{
		    _eventValuatorType[i].push_back(NON_ZERO);
		}

		_eventValuators[i].push_back(0.0);
		_eventValuatorTime[i].push_back(0.0);
	    }
	}
	while(valFound);
    }

    setGenHandDefaultButtonEvents();

    for(int i = 0; i < _numHeads; i++)
    {
        std::stringstream headss;
        headss << "Input.Head" << i << "Address";

        _headAddress.push_back(
                std::pair<int,int>(
                        ConfigManager::getInt("system",headss.str(),0),
                        ConfigManager::getInt("body",headss.str(),0)));
        //_threadHeadMatList.push_back(osg::Matrix());
        _headMatList.push_back(osg::Matrix());
        _lastUpdatedHeadMatList.push_back(osg::Matrix());
	_headToHandsMap.push_back(std::vector<int>());
	for(int j = 0; j < _handToHeadMap.size(); j++)
	{
	    if(i == _handToHeadMap[j])
	    {
		_headToHandsMap[i].push_back(j);
	    }
	}
    }

    if(!_numHeads)
    {
        _headMatList.push_back(osg::Matrix());
        _lastUpdatedHeadMatList.push_back(osg::Matrix());
	_headToHandsMap.push_back(std::vector<int>());
	for(int j = 0; j < _handToHeadMap.size(); j++)
	{
	    if(!_handToHeadMap[j])
	    {
		_headToHandsMap[0].push_back(j);
	    }
	}
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
        vx = ConfigManager::getFloat("x","ViewerPosition",0);
        vy = ConfigManager::getFloat("y","ViewerPosition",0);
        vz = ConfigManager::getFloat("z","ViewerPosition",0);

        vh = ConfigManager::getFloat("h","ViewerPosition",0);
        vp = ConfigManager::getFloat("p","ViewerPosition",0);
        vr = ConfigManager::getFloat("r","ViewerPosition",0);

        vTrans.makeTranslate(vx,vy,vz);
        vRot.makeRotate(vr,osg::Vec3(0,1,0),vp,osg::Vec3(1,0,0),vh,
                osg::Vec3(0,0,1));

        _headMatList[0] = vRot * vTrans;
        _lastUpdatedHeadMatList[0] = _headMatList[0];

    }

    for(int i = 1; i < _numHeads; i++)
    {
        std::stringstream vTag;
        vTag << "Viewer" << i + 1 << "Position";

        vx = ConfigManager::getFloat("x",vTag.str(),0);
        vy = ConfigManager::getFloat("y",vTag.str(),0);
        vz = ConfigManager::getFloat("z",vTag.str(),0);

        vh = ConfigManager::getFloat("h",vTag.str(),0);
        vp = ConfigManager::getFloat("p",vTag.str(),0);
        vr = ConfigManager::getFloat("r",vTag.str(),0);

        vTrans.makeTranslate(vx,vy,vz);
        vRot.makeRotate(vr,osg::Vec3(0,1,0),vp,osg::Vec3(1,0,0),vh,
                osg::Vec3(0,0,1));

        _headMatList[i] = vRot * vTrans;
        _lastUpdatedHeadMatList[i] = _headMatList[i];
    }

    for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
    {
        _eventMap[i] = std::list<InteractionEvent*>();
    }

    setHandButtonMaps();

    if(_threaded && ComController::instance()->isMaster())
    {
        genComTrackEvents = new GenComplexTrackingEvents();
        this->start();
    }

    if(_debugOutput)
    {
	printInitDebug();
    }

    return true;
}

void TrackingManager::update()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }
    
    double startTime, endTime;

    osg::Stats * stats;
    stats = CVRViewer::instance()->getViewerStats();
    if(stats && !stats->collectStats("CalVRStats"))
    {
	stats = NULL;
    }

    if(stats)
    {
	startTime = osg::Timer::instance()->delta_s(CVRViewer::instance()->getStartTick(), osg::Timer::instance()->tick());
    }

    _updateLock.lock();

    int totalData = _totalBodies * sizeof(struct TrackerBase::TrackedBody)
            + _systemInfo.size() * sizeof(unsigned int)
            + _totalValuators * sizeof(float);
    char * data = NULL;
    if(totalData)
    {
        data = new char[totalData];
    }

    //std::cerr << "Update Called." << std::endl;
    if(ComController::instance()->isMaster())
    {
        static TrackerBase::TrackedBody * zeroBody = NULL;
        if(!zeroBody)
        {
            zeroBody = new TrackerBase::TrackedBody;
            zeroBody->x = 0;
            zeroBody->y = 0;
            zeroBody->z = 0;
            osg::Quat q;
            zeroBody->qx = q.x();
            zeroBody->qy = q.y();
            zeroBody->qz = q.z();
            zeroBody->qw = q.w();
        }

        TrackerBase::TrackedBody * tbptr = (TrackerBase::TrackedBody*)data;
        unsigned int * buttonptr = (unsigned int *)(data
                + (_totalBodies * sizeof(struct TrackerBase::TrackedBody)));
        float * valptr = (float *)(data
                + (_totalBodies * sizeof(struct TrackerBase::TrackedBody)
                        + _systemInfo.size() * sizeof(unsigned int)));
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
                    TrackerBase::TrackedBody * tb = _systems[i]->getBody(j);
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
        TrackerBase::TrackedBody * tbptr = (TrackerBase::TrackedBody*)data;
        unsigned int * buttonptr = (unsigned int *)(data
                + (_totalBodies * sizeof(struct TrackerBase::TrackedBody)));
        float * valptr = (float *)(data
                + (_totalBodies * sizeof(struct TrackerBase::TrackedBody)
                        + _systemInfo.size() * sizeof(unsigned int)));

        for(int i = 0; i < _systems.size(); i++)
        {
            TrackerSlave * sTracker = NULL;
            sTracker = dynamic_cast<TrackerSlave*>(_systems[i]);
            if(sTracker)
            {
                sTracker->readValues(tbptr,buttonptr,valptr);
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

    TrackerBase::TrackedBody * tb;
    for(int i = 0; i < _numHeads; i++)
    {
        if(_headAddress[i].first < 0
                || _headAddress[i].first >= _systems.size())
        {
            continue;
        }

        if(_systems[_headAddress[i].first]
                && _systems[_headAddress[i].first]->getBody(
                        _headAddress[i].second))
        {
            tb = _systems[_headAddress[i].first]->getBody(
                    _headAddress[i].second);
            if(tb)
            {
                osg::Vec3 pos(tb->x,tb->y,tb->z);
                osg::Matrix rot;
                rot.makeRotate(osg::Quat(tb->qx,tb->qy,tb->qz,tb->qw));
                _headMatList[i] =
                        _systemInfo[_headAddress[i].first]->bodyRotations[_headAddress[i].second]
                                * rot * osg::Matrix::translate(pos)
                                * _systemInfo[_headAddress[i].first]->systemTransform
                                * osg::Matrix::translate(
                                        _systemInfo[_headAddress[i].first]->bodyTranslations[_headAddress[i].second]);

                if(_updateHeadTracking)
                {
                    _lastUpdatedHeadMatList[i] = _headMatList[i];
                }
            }
        }
    }

    for(int i = 0; i < _numHands; i++)
    {
        if(_handAddress[i].first < 0
                || _handAddress[i].first >= _systems.size())
        {
            continue;
        }

        if(_systems[_handAddress[i].first]
                && _systems[_handAddress[i].first]->getBody(
                        _handAddress[i].second))
        {
            tb = _systems[_handAddress[i].first]->getBody(
                    _handAddress[i].second);
            if(tb)
            {
                osg::Vec3 pos(tb->x,tb->y,tb->z);
                osg::Matrix rot;
                rot.makeRotate(osg::Quat(tb->qx,tb->qy,tb->qz,tb->qw));
                _handMatList[i] =
                        _systemInfo[_handAddress[i].first]->bodyRotations[_handAddress[i].second]
                                * rot * osg::Matrix::translate(pos)
                                * _systemInfo[_handAddress[i].first]->systemTransform
                                * osg::Matrix::translate(
                                        _systemInfo[_handAddress[i].first]->bodyTranslations[_handAddress[i].second]);
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
    generateValuatorEvents();
    flushEvents();

    // merge threaded and non-threaded hand masks
    for(int i = 0; i < _handButtonMask.size(); i++)
    {
        _handButtonMask[i] |= _threadHandButtonMasks[i];
    }

    _updateLock.unlock();

    if(stats)
    {
        endTime = osg::Timer::instance()->delta_s(CVRViewer::instance()->getStartTick(), osg::Timer::instance()->tick());
        stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "Tracking begin time", startTime);
        stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "Tracking end time", endTime);
        stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "Tracking time taken", endTime-startTime);
    }
}

void TrackingManager::run()
{
    struct timeval start, end;
    float target = 1.0 / _threadFPS;

    struct timeval printStart, printEnd;
    int readings = 0;
    gettimeofday(&printStart,NULL);
    while(1)
    {
        gettimeofday(&start,NULL);
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
        generateThreadValuatorEvents();
	generateThreadPositionEvents();

        _updateLock.unlock();

        _quitLock.lock();
        if(_threadQuit)
        {
            _quitLock.unlock();
            break;
        }
        _quitLock.unlock();
        gettimeofday(&end,NULL);

        float interval = (end.tv_sec - start.tv_sec)
                + ((end.tv_usec - start.tv_usec) / 1000000.0);
        if(interval < target)
        {
#ifndef WIN32
            timespec ts;
            interval = target - interval;
            ts.tv_sec = (int)interval;
            interval -= ((float)((int)interval));
            ts.tv_nsec = (long int)(interval * 1000000000.0 * 0.95);
            nanosleep(&ts,NULL);
#else
            //TODO: do this sub-milisecond
            interval = target - interval;
            DWORD sleeptime = (DWORD)(interval * 1000.0 * 0.95);
            Sleep(sleeptime);
#endif
        }
        readings++;

        //TODO: add to debug
        gettimeofday(&printEnd,NULL);
        interval = (printEnd.tv_sec - printStart.tv_sec)
                + ((printEnd.tv_usec - printStart.tv_usec) / 1000000.0);
        if(interval > 10.0)
        {
            printStart = printEnd;
            //std::cerr << "Tracking FPS: " << ((float)readings) / interval << std::endl;
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

SceneManager::PointerGraphicType TrackingManager::getPointerGraphicType(
        int hand)
{
    if(hand >= 0 && hand < _handGraphicType.size())
    {
	return _handGraphicType[hand];
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

int TrackingManager::getHeadForHand(int hand)
{
    if(hand >= 0 && hand < _handToHeadMap.size())
    {
	return _handToHeadMap[hand];
    }

    return 0;
}

const std::vector<int> & TrackingManager::getHandsForHead(int head)
{
    static std::vector<int> dummyReturn;

    if(head >= 0 && head < _headToHandsMap.size())
    {
	return _headToHandsMap[head];
    }

    return dummyReturn;
}

int TrackingManager::getNumTrackingSystems()
{
    return _systems.size();
}

TrackerBase * TrackingManager::getTrackingSystem(int system)
{
    if(system < 0 || system >= _systems.size())
    {
	return NULL;
    }

    return _systems[system];
}

void TrackingManager::getHandAddress(int hand, int & system, int & body)
{
    if(hand < 0 || hand >= getNumHands())
    {
	system = body = -1;
    }
    else
    {
	system = _handAddress[hand].first;
	body = _handAddress[hand].second;
    }
}

void TrackingManager::getHeadAddress(int head, int & system, int & body)
{
    if(head < 0 || head >= getNumHeads())
    {
	system = body = -1;
    }
    else
    {
	system = _headAddress[head].first;
	body = _headAddress[head].second;
    }
}

TrackerBase::TrackerType TrackingManager::getHandTrackerType(int hand)
{
    if(hand < 0 || hand >= _numHands || _handAddress[hand].first < 0
            || _handAddress[hand].first >= _systemInfo.size()
            || _handAddress[hand].second < 0
            || _handAddress[hand].second
                    >= _systemInfo[_handAddress[hand].first]->numBodies)
    {
        return TrackerBase::INVALID;
    }

    return _systemInfo[_handAddress[hand].first]->trackerType;
}

Navigation::NavImplementation TrackingManager::getHandNavType(int hand)
{
    if(hand >= 0 && hand < _handNavImplementation.size())
    {
	return _handNavImplementation[hand];
    }

    return Navigation::NONE_NAV;
}

int TrackingManager::getNumBodies(int system)
{
    if(system >= 0 && system < _systems.size() && _systems[system])
    {
        return _systemInfo[system]->numBodies;
    }
    return 0;
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

unsigned int TrackingManager::getButtonFilter(int hand, int system)
{
    if(hand >= 0 && hand < _handStationFilterMask.size() && system >= 0 && system < _handStationFilterMask[hand].size())
    {
	return _handStationFilterMask[hand][system];
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
    if(system >= 0 && system < _valuatorList.size() && index >= 0
            && index < _valuatorList[system].size())
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

osg::Matrix TrackingManager::getHandTransformFromTrackedBody(int hand, TrackerBase::TrackedBody * body)
{
    if(hand < 0 || hand >= getNumHands() || !body)
    {
	return osg::Matrix();
    }

    osg::Vec3 pos(body->x,body->y,body->z);
    osg::Matrix rot;
    rot.makeRotate(osg::Quat(body->qx,body->qy,body->qz,body->qw));
    osg::Matrix transform =
	_systemInfo[_handAddress[hand].first]->bodyRotations[_handAddress[hand].second]
	* rot * osg::Matrix::translate(pos)
	* _systemInfo[_handAddress[hand].first]->systemTransform
	* osg::Matrix::translate(
		_systemInfo[_handAddress[hand].first]->bodyTranslations[_handAddress[hand].second]);

    return transform;
}

osg::Matrix TrackingManager::getHeadTransformFromTrackedBody(int head, TrackerBase::TrackedBody * body)
{
    if(head < 0 || head >= getNumHeads() || !body)
    {
	return osg::Matrix();
    }

    osg::Vec3 pos(body->x,body->y,body->z);
    osg::Matrix rot;
    rot.makeRotate(osg::Quat(body->qx,body->qy,body->qz,body->qw));
    osg::Matrix transform =
	_systemInfo[_headAddress[head].first]->bodyRotations[_headAddress[head].second]
	* rot * osg::Matrix::translate(pos)
	* _systemInfo[_headAddress[head].first]->systemTransform
	* osg::Matrix::translate(
		_systemInfo[_headAddress[head].first]->bodyTranslations[_headAddress[head].second]);

    return transform;
}

void TrackingManager::getHandButtonFromSystemButton(int system, int systemButton, int & hand, int & handButton)
{
    for(std::map<int,std::vector<std::pair<int,int> > >::iterator it = _handButtonAddressMap.begin(); it != _handButtonAddressMap.end(); it++)
    {
	for(int i = 0; i < it->second.size(); i++)
	{
	    if(it->second[i].first == system && it->second[i].second == systemButton)
	    {
		hand = it->first;
		handButton = _handButtonMapping[it->first][i];
		return;
	    }
	}
    }
    hand = -1;
    handButton = -1;
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
                    if(!_systemInfo[j]->thread)
                    {
                        unsigned int value =
                                (getRawButtonMask(j) & stationMask) ? 1 : 0;
                        value = value << handMaskOffset;
                        _handButtonMask[i] |= value;
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
                    if(_systems[j] && _systemInfo[j]->thread)
                    {
                        unsigned int value =
                                (_systems[j]->getButtonMask() & stationMask) ?
                                        1 : 0;
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
    if(ComController::instance()->isMaster())
    {
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

		    int system, body;
		    system = _handAddress[j].first;
		    body = _handAddress[j].second;

		    TrackedButtonInteractionEvent * buttonEvent = NULL;

		    if(system >= 0 && system < _systems.size() && _systems[system])
		    {
			buttonEvent = _systems[system]->getNewBaseEvent(body);
		    }

		    if(!buttonEvent)
		    {
			buttonEvent = new TrackedButtonInteractionEvent();
		    }

                    if((_lastHandButtonMask[j] & bit) && (newMask & bit))
                    {
                        buttonEvent->setInteraction(BUTTON_DRAG);
                    }
                    else if(_lastHandButtonMask[j] & bit)
                    {
                        buttonEvent->setInteraction(BUTTON_UP);
                    }
                    else
                    {
                        buttonEvent->setInteraction(BUTTON_DOWN);
                    }
                    buttonEvent->setButton(_handButtonMapping[j][i]);
                    // set current pointer info
                    buttonEvent->setTransform(_handMatList[j]);
                    buttonEvent->setHand(j);
		    _eventMap[buttonEvent->getEventType()].push_back(buttonEvent);
                }
                bit = bit << 1;
            }
            _lastHandButtonMask[j] = newMask;
        }
    }
}

void TrackingManager::generateThreadButtonEvents()
{
    TrackedButtonInteractionEvent * buttonEvent;
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
            if((_threadLastHandButtonMask[j] & bit) != (newMask & bit))
	    {
		buttonEvent = NULL;

		int system, body;
		system = _handAddress[j].first;
		body = _handAddress[j].second;

		if(system >= 0 && system < _systems.size() && _systems[system])
		{
		    buttonEvent = _systems[system]->getNewBaseEvent(body);
		}

		if(!buttonEvent)
		{
		    buttonEvent = new TrackedButtonInteractionEvent();
		}

		//std::cerr << "last mask " << _lastButtonMask << " new mask " << newMask << std::endl;
		if(_threadLastHandButtonMask[j] & bit)
		{
		    buttonEvent->setInteraction(BUTTON_UP);
		}
		else
		{
		    buttonEvent->setInteraction(BUTTON_DOWN);
		}
		buttonEvent->setButton(_handButtonMapping[j][i]);
		// set current pointer info
		if(getIsHandThreaded(j))
		{
		    buttonEvent->setTransform(_threadHandMatList[j]);
		}
		else
		{
		    buttonEvent->setTransform(_handMatList[j]);
		}
		buttonEvent->setHand(j);
		//_threadEvents.push((InteractionEvent*)buttonEvent);
		genComTrackEvents->processEvent(buttonEvent);
	    }
            else if((_threadLastHandButtonMask[j] & bit) && (newMask & bit))
	    {
		buttonEvent = NULL;

		int system, body;
		system = _handAddress[j].first;
		body = _handAddress[j].second;

		if(system >= 0 && system < _systems.size() && _systems[system])
		{
		    buttonEvent = _systems[system]->getNewBaseEvent(body);
		}

		if(!buttonEvent)
		{
		    buttonEvent = new TrackedButtonInteractionEvent();
		}

		buttonEvent->setInteraction(BUTTON_DRAG);
		buttonEvent->setButton(_handButtonMapping[j][i]);
		// set current pointer info
		if(getIsHandThreaded(j))
		{
		    buttonEvent->setTransform(_threadHandMatList[j]);
		}
		else
		{
		    buttonEvent->setTransform(_handMatList[j]);
		}
		buttonEvent->setHand(j);
		//_threadEvents.push((InteractionEvent*)buttonEvent);
		genComTrackEvents->processEvent(buttonEvent);
	    }
            bit = bit << 1;
        }
        _threadLastHandButtonMask[j] = newMask;
    }
}

void TrackingManager::generateValuatorEvents()
{
    for(int j = 0; j < _numHands; j++)
    {
        for(int i = 0; i < _numEventValuators[j]; i++)
        {
            float value;
            if(_eventValuatorAddress[j][i].first >= 0
                    && _eventValuatorAddress[j][i].first < _systems.size())
            {
                if(_systemInfo[_eventValuatorAddress[j][i].first]->thread)
                {
                    continue;
                }

                if(_eventValuatorAddress[j][i].second >= 0
                        && _eventValuatorAddress[j][i].second
                                < _valuatorList[_eventValuatorAddress[j][i].first].size())
                {
                    value =
                            _valuatorList[_eventValuatorAddress[j][i].first][_eventValuatorAddress[j][i].second];
                }
                else
                {
                    value = 0.0;
                }
            }
            else
            {
                value = 0.0;
            }

            _eventValuators[j][i] = value;

            if(_eventValuatorType[j][i] == NON_ZERO)
            {
                if(fabs(_eventValuators[j][i]) > 0.05)
                {
		    // attempt to output 60 events per second to be consistent with the threaded version
		    static double timeSlice = 1.0 / 60.0;
		    _eventValuatorTime[j][i] += CVRViewer::instance()->getLastFrameDuration();

		    while(_eventValuatorTime[j][i] > timeSlice)
		    {
			ValuatorInteractionEvent * vie =
			    new ValuatorInteractionEvent();
			vie->setInteraction(VALUATOR);
			vie->setValuator(i);
			vie->setHand(j);
			vie->setValue(_eventValuators[j][i]);
			InteractionManager::instance()->addEvent(
				(InteractionEvent*)vie);
			_eventValuatorTime[j][i] -= timeSlice;
		    }
                }
		else
		{
		    _eventValuatorTime[j][i] = 0.0;
		}
            }
            else if(_eventValuatorType[j][i] == CHANGE)
            {
                if(_lastEventValuators[j].find(i)
                        == _lastEventValuators[j].end())
                {
                    _lastEventValuators[j][i] = value;
                }
                if(_eventValuators[j][i] != _lastEventValuators[j][i])
                {
                    ValuatorInteractionEvent * vie =
                            new ValuatorInteractionEvent();
		    vie->setInteraction(VALUATOR);
                    vie->setValuator(i);
                    vie->setHand(j);
                    vie->setValue(_eventValuators[j][i]);
                    InteractionManager::instance()->addEvent(
                            (InteractionEvent*)vie);
                }

                _lastEventValuators[j][i] = _eventValuators[j][i];
            }
        }
    }
}

void TrackingManager::generateThreadValuatorEvents()
{
    for(int j = 0; j < _numHands; j++)
    {
        for(int i = 0; i < _numEventValuators[j]; i++)
        {
            float value;
            if(_eventValuatorAddress[j][i].first >= 0
                    && _eventValuatorAddress[j][i].first < _systems.size())
            {
                if(!_systemInfo[_eventValuatorAddress[j][i].first]->thread)
                {
                    continue;
                }

                if(_eventValuatorAddress[j][i].second >= 0
                        && _eventValuatorAddress[j][i].second
                                < _systemInfo[_eventValuatorAddress[j][i].first]->numVal)
                {
                    if(_systems[_eventValuatorAddress[j][i].first])
                    {
                        value =
                                _systems[_eventValuatorAddress[j][i].first]->getValuator(
                                        _eventValuatorAddress[j][i].second);
                    }
                    else
                    {
                        value = 0.0;
                    }
                }
                else
                {
                    value = 0.0;
                }
            }
            else
            {
                value = 0.0;
            }

            _eventValuators[j][i] = value;

            if(_eventValuatorType[j][i] == NON_ZERO)
            {
                if(fabs(_eventValuators[j][i]) > 0.05)
                {
                    ValuatorInteractionEvent * vie =
                            new ValuatorInteractionEvent();
		    vie->setInteraction(VALUATOR);
                    vie->setValuator(i);
                    vie->setHand(j);
                    vie->setValue(_eventValuators[j][i]);
                    _eventMap[vie->getEventType()].push_back(vie);
                }
            }
            else if(_eventValuatorType[j][i] == CHANGE)
            {
                if(_lastEventValuators[j].find(i)
                        == _lastEventValuators[j].end())
                {
                    _lastEventValuators[j][i] = value;
                }
                if(_eventValuators[j][i] != _lastEventValuators[j][i])
                {
                    ValuatorInteractionEvent * vie =
                            new ValuatorInteractionEvent();
		    vie->setInteraction(VALUATOR);
                    vie->setValuator(i);
                    vie->setHand(j);
                    vie->setValue(_eventValuators[j][i]);
                    _eventMap[vie->getEventType()].push_back(vie);
                }

                _lastEventValuators[j][i] = _eventValuators[j][i];
            }
        }
    }
}

void TrackingManager::generatePositionEvents()
{
    
}

void TrackingManager::generateThreadPositionEvents()
{
    for(int i = 0; i < _numHands; i++)
    {
	if(!_handGenPositionEvents[i])
	{
	    continue;
	}
	if(_handAddress[i].first >= 0
		&& _handAddress[i].first < _systems.size())
	{
	    if(!_systemInfo[_handAddress[i].first]->thread)
	    {
		continue;
	    }
	}
	else
	{
	    continue;
	}

	PositionInteractionEvent * pie = new PositionInteractionEvent();
	pie->setInteraction(cvr::MOVE);
	pie->setPosition(_threadHandMatList[i].getTrans());
	pie->setHand(i);
	_eventMap[pie->getEventType()].push_back(pie);
    }
}

void TrackingManager::updateThreadMats()
{
    TrackerBase::TrackedBody * tb;
    /*for(int i = 0; i < _numHeads; i++)
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
     }*/

    for(int i = 0; i < _numHands; i++)
    {
        if(_systems[_handAddress[i].first]
                && _systems[_handAddress[i].first]->getBody(
                        _handAddress[i].second))
        {
            if(!_systemInfo[_handAddress[i].first]->thread)
            {
                continue;
            }

            tb = _systems[_handAddress[i].first]->getBody(
                    _handAddress[i].second);
            if(tb)
            {
                osg::Vec3 pos(tb->x,tb->y,tb->z);
                osg::Matrix rot;
                rot.makeRotate(osg::Quat(tb->qx,tb->qy,tb->qz,tb->qw));
                _threadHandMatList[i] =
                        _systemInfo[_handAddress[i].first]->bodyRotations[_handAddress[i].second]
                                * rot * osg::Matrix::translate(pos)
                                * _systemInfo[_handAddress[i].first]->systemTransform
                                * osg::Matrix::translate(
                                        _systemInfo[_handAddress[i].first]->bodyTranslations[_handAddress[i].second]);
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
        ComController::instance()->sendSlaves(numEvents,
                NUM_INTER_EVENT_TYPES * sizeof(int));
    }
    else
    {
        ComController::instance()->readMaster(numEvents,
                NUM_INTER_EVENT_TYPES * sizeof(int));
    }

    int eventsDataSize = 0;

    for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
    {
        if(numEvents[i])
        {
            eventsDataSize += numEvents[i]
                    * getEventSize((InteractionEventType)i);
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
        char * eventptr = data;
        for(int i = 0; i < NUM_INTER_EVENT_TYPES; i++)
        {
            for(std::list<InteractionEvent*>::iterator it =
                    _eventMap[i].begin(); it != _eventMap[i].end(); it++)
            {
                storeEvent(*it,eventptr);
                eventptr += getEventSize((InteractionEventType)i);
                InteractionManager::instance()->addEvent(*it);
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
            for(int j = 0; j < numEvents[i]; j++)
            {
                InteractionEvent * ie = loadEventWithType(
                        (InteractionEvent *)eventptr,(InteractionEventType)i);
                if(ie)
                {
                    InteractionManager::instance()->addEvent(ie);
                }
                eventptr += getEventSize((InteractionEventType)i);
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
                    _genHandDefaultButtonEvents[i].push_back(
                            _systemInfo[j]->genDefaultButtonEvents);

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

void TrackingManager::setHandButtonMaps()
{
    for(int i = 0; i < _systemInfo.size(); i++)
    {
	 unsigned int mask = 1;
	 for(int j = 0; j < _systemInfo[i]->numButtons; j++)
	 {
	     for(int k = 0; k < _numHands; k++)
	     {
		 if(_handStationFilterMask[k][i] & mask)
		 {
		     _handButtonAddressMap[k].push_back(std::pair<int,int>(i,j));
		     break;
		 }
	     }
	     mask = mask << 1;
	 }
    }

    for(int i = 0; i < _numHands; i++)
    {
	_handButtonMapping.push_back(std::map<int,int>());
    }

    for(std::map<int,std::vector<std::pair<int,int> > >::iterator it = _handButtonAddressMap.begin(); it != _handButtonAddressMap.end(); it++)
    {
	for(int i = 0; i < it->second.size(); i++)
	{
	    _handButtonMapping[it->first][i] = i;
	}
    }

    for(int i = 0; i < _numHands; i++)
    {
	bool orderFound = false;
	std::stringstream ss;
	ss << "Input.Hand" << i << ".ButtonMapping";
	std::string order = ConfigManager::getEntry("value",ss.str(),"",&orderFound);

	if(!orderFound)
	{
	    continue;
	}

	if(_handButtonMapping[i].size())
	{
	    std::vector<int> orderList;
	    size_t pos = order.find_first_not_of(',');
	    while(pos != std::string::npos)
	    {
		size_t cpos = order.find_first_of(',',pos);
		if(cpos == std::string::npos)
		{
		    orderList.push_back(atoi(order.substr(pos,order.length()-pos).c_str()));
		    pos = cpos;
		}
		else
		{
		    orderList.push_back(atoi(order.substr(pos,cpos-pos).c_str()));
		    pos = order.find_first_not_of(',',cpos);
		}
	    }

	    for(int j = 0; j < orderList.size() && j < _handButtonMapping[i].size(); j++)
	    {
		_handButtonMapping[i][j] = orderList[j];
	    }
	}
    }
}

bool TrackingManager::getIsHandThreaded(int hand)
{
    if(_handAddress[hand].first >= 0
            && _handAddress[hand].first < _systemInfo.size())
    {
        return _systemInfo[_handAddress[hand].first]->thread;
    }
    return false;
}

void TrackingManager::printInitDebug()
{
    std::cerr << "-----Tracking Init Info-----" << std::endl;

    std::cerr << "Num Systems: " << _systems.size() << std::endl;
    
    for(int i = 0; i < _systems.size(); i++)
    {
	std::cerr << "System: " << i << std::endl;
	if(_systems[i])
	{
	    std::cerr << "Num Bodies - config: " << _systemInfo[i]->numBodies << " system: " << _systems[i]->getNumBodies() << std::endl;
	    std::cerr << "Num Buttons - config: " << _systemInfo[i]->numButtons << " system: " << _systems[i]->getNumButtons() << std::endl;
	    std::cerr << "Num Valuators - config: " << _systemInfo[i]->numVal << " system: " << _systems[i]->getNumValuators() << std::endl;
	}
	else
	{
	    std::cerr << "System failed init." << std::endl;
	}
	std::cerr << std::endl;
    }

    std::cerr << "Num Heads: " << _numHeads << std::endl;

    for(int i = 0; i < _numHeads; i++)
    {
	std::cerr << "Head " << i << " Address: system: " << _headAddress[i].first << " body: " << _headAddress[i].second << std::endl;
    }

    std::map<int,std::vector<std::pair<int,int> > > handButtonAddressMap;
    for(int i = 0; i < _systemInfo.size(); i++)
    {
	 unsigned int mask = 1;
	 for(int j = 0; j < _systemInfo[i]->numButtons; j++)
	 {
	     for(int k = 0; k < _numHands; k++)
	     {
		 if(_handStationFilterMask[k][i] & mask)
		 {
		     handButtonAddressMap[k].push_back(std::pair<int,int>(i,j));
		     break;
		 }
	     }
	     mask = mask << 1;
	 }
    }

    std::cerr << std::endl << "Num Hands: " << _numHands << std::endl;

    for(int i = 0; i < _numHands; i++)
    {
	std::cerr << "Hand " << i << std::endl;
	std::cerr << "Address: system: " << _handAddress[i].first << " body: " << _handAddress[i].second << std::endl;
	std::cerr << "Num Buttons:" << handButtonAddressMap[i].size() << std::endl;

	for(int j = 0; j < handButtonAddressMap[i].size(); j++)
	{
	    std::cerr << "Button " << j << " - system: " << handButtonAddressMap[i][j].first << " button: " << handButtonAddressMap[i][j].second << std::endl;
	}

	std::cerr << "Num Valuators: " << _eventValuators[i].size() << std::endl;
	for(int j = 0; j < _eventValuators[i].size(); j++)
	{
	    std::cerr << "Valuator " << i << " system: " << _eventValuatorAddress[i][j].first << " index: " << _eventValuatorAddress[i][j].second << std::endl;
	}
	std::cerr << std::endl;
    }


    std::cerr << "-----End Tracking Init Info-----" << std::endl;
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

void GenComplexTrackingEvents::processEvent(TrackedButtonInteractionEvent * tie)
{
    if(tie->getInteraction() == BUTTON_DOWN)
    {
        if(!_lastClick[tie->getHand()][tie->getButton()])
        {
            struct timeval * tv = new struct timeval;
            gettimeofday(tv,NULL);
            _lastClick[tie->getHand()][tie->getButton()] = tv;

            _doubleClicked[tie->getHand()][tie->getButton()] = false;
        }
        else
        {

            timeval now;
            gettimeofday(&now,NULL);

            float interval =
                    (now.tv_sec
                            - _lastClick[tie->getHand()][tie->getButton()]->tv_sec)
                            + ((now.tv_usec
                                    - _lastClick[tie->getHand()][tie->getButton()]->tv_usec)
                                    / 1000000.0);

            if(interval < _doubleClickTimeout
                    && !_doubleClicked[tie->getHand()][tie->getButton()])
            {
                tie->setInteraction(BUTTON_DOUBLE_CLICK);
                _doubleClicked[tie->getHand()][tie->getButton()] = true;
            }
            else
            {
                *_lastClick[tie->getHand()][tie->getButton()] = now;
                _doubleClicked[tie->getHand()][tie->getButton()] = false;
            }
        }

        TrackingManager::instance()->_eventMap[tie->getEventType()].push_back(
                tie);
    }
    else
    {
        TrackingManager::instance()->_eventMap[tie->getEventType()].push_back(
                tie);
    }
}
