#include <cvrInput/TrackerOmicron.h>
#include <cvrConfig/ConfigManager.h>

#include <cstring>
#include <iostream>

using namespace cvr;

enum MocapJointIndex
{
    MOCAP_HEAD = 1,
    MOCAP_RIGHT_ELBOW = 7,
    MOCAP_RIGHT_HAND = 9,
    MOCAP_LEFT_ELBOW = 13,
    MOCAP_LEFT_HAND = 15
};

TrackerOmicron::TrackerOmicron()
{

}

TrackerOmicron::~TrackerOmicron()
{
}

bool TrackerOmicron::init(std::string tag)
{
    _client = new omicronConnector::OmicronConnectorClient(this);
    _server = ConfigManager::getEntry("value",tag + ".Omicron.Server",
            "127.0.0.1");
    _port = ConfigManager::getInt("value",tag + ".Omicron.Port",27000);
    _client->connect(_server.c_str(),_port);

    _numWands = ConfigManager::getInt("value",tag + ".Omicron.NumWands",0);
    _numPointers = ConfigManager::getInt("value",tag + ".Omicron.NumPointers",
            0);
    _numMocaps = ConfigManager::getInt("value",tag + ".Omicron.NumMocaps",0);

    _numButtons = 0;
    _buttonMask = 0;
    _numValuators = 0;
    for(int i = 0; i < _numValuators; i++)
    {
        _valArray.push_back(0.0);
    }

    _numBodies = _numWands + _numPointers + (3 * _numMocaps);

    for(int i = 0; i < _numWands + _numPointers; i++)
    {
        TrackedBody * tb = new TrackedBody;
        tb->x = tb->y = tb->z = 0.0;
        osg::Quat q;
        tb->qx = q.x();
        tb->qy = q.y();
        tb->qz = q.z();
        tb->qw = q.w();
        _bodyList.push_back(tb);
    }

    return true;
}

TrackerBase::TrackedBody * TrackerOmicron::getBody(int index)
{
    if(index < 0 || index >= _numBodies)
    {
        return NULL;
    }

    if(index < _numWands + _numPointers)
    {
        return _bodyList[index];
    }

    int mocapIndex = index - (_numWands + _numPointers);
    if(mocapIndex / 3 >= _mocapBodyMap.size())
    {
        return NULL;
    }

    std::map<unsigned int,std::vector<TrackedBody*> >::iterator it =
            _mocapBodyMap.begin();
    for(int i = 0; i < mocapIndex / 3; i++)
    {
        ++it;
    }
    return it->second[mocapIndex % 3];
}

unsigned int TrackerOmicron::getButtonMask()
{
    return _buttonMask;
}

float TrackerOmicron::getValuator(int index)
{
    if(index >= 0 && index < _valArray.size())
    {
        return _valArray[index];
    }
    return 0.0f;
}

int TrackerOmicron::getNumBodies()
{
    return _numBodies;
}

int TrackerOmicron::getNumValuators()
{
    return _numValuators;
}

int TrackerOmicron::getNumButtons()
{
    return _numButtons;
}

void TrackerOmicron::update(
        std::map<int,std::list<InteractionEvent*> > & eventMap)
{
    _eventMapPtr = &eventMap;
    _client->poll();
}

void TrackerOmicron::onEvent(const omicronConnector::EventData& e)
{
    //std::cerr << "Got event." << std::endl;
    switch(e.serviceType)
    {
        case omicron::EventBase::ServiceTypeMocap:
        {
            std::map<unsigned int,std::vector<TrackedBody*> >::iterator it =
                    _mocapBodyMap.find(e.sourceId);
            //if(e.type == omicron::EventBase::Trace && _bodyMap.find(e.sourceId) == _bodyMap.end())
            if(it == _mocapBodyMap.end())
            {
                std::cerr << "Adding bodies for source id: " << e.sourceId
                        << std::endl;
                TrackedBody * tb = new TrackedBody;
                tb->x = tb->y = tb->z = 0.0;
                osg::Quat q;
                tb->qx = q.x();
                tb->qy = q.y();
                tb->qz = q.z();
                tb->qw = q.w();
                _mocapBodyMap[e.sourceId].push_back(tb);
                tb = new TrackedBody;
                tb->x = tb->y = tb->z = 0.0;
                tb->qx = q.x();
                tb->qy = q.y();
                tb->qz = q.z();
                tb->qw = q.w();
                _mocapBodyMap[e.sourceId].push_back(tb);
                tb = new TrackedBody;
                tb->x = tb->y = tb->z = 0.0;
                tb->qx = q.x();
                tb->qy = q.y();
                tb->qz = q.z();
                tb->qw = q.w();
                _mocapBodyMap[e.sourceId].push_back(tb);

                it = _mocapBodyMap.find(e.sourceId);
            }

            if(e.type == omicron::EventBase::Untrace
                    && it != _mocapBodyMap.end())
            {
                std::cerr << "Removing bodies for source id: " << e.sourceId
                        << std::endl;
                for(int i = 0; i < _mocapBodyMap[e.sourceId].size(); i++)
                {
                    delete _mocapBodyMap[e.sourceId][i];
                }
                _mocapBodyMap.erase(e.sourceId);
            }

            //if(e.type == omicron::EventBase::Update)
            //{
            //std::cerr << "Update event type. id: " << e.sourceId << std::endl;
            //}

            if(e.type == omicron::EventBase::Update
                    && it != _mocapBodyMap.end())
            {
                //std::cerr << "Body map update" << std::endl;
                //std::cerr << "Pos x: " << e.posx << " y: " << e.posy << " z: " << e.posz << std::endl;
                float * data = (float*)&e.extraData[0];
                //head
                it->second[0]->x = 1000.0 * data[(MOCAP_HEAD * 3) + 0];
                it->second[0]->y = 1000.0 * data[(MOCAP_HEAD * 3) + 1];
                it->second[0]->z = 1000.0 * data[(MOCAP_HEAD * 3) + 2];
                //std::cerr << "Head x: " << _bodyMap[e.sourceId][0]->x << " y: " << _bodyMap[e.sourceId][0]->y << " z: " << _bodyMap[e.sourceId][0]->z << std::endl;
                //left hand
                it->second[1]->x = 1000.0 * data[(MOCAP_LEFT_HAND * 3) + 0];
                it->second[1]->y = 1000.0 * data[(MOCAP_LEFT_HAND * 3) + 1];
                it->second[1]->z = 1000.0 * data[(MOCAP_LEFT_HAND * 3) + 2];

                osg::Vec3 bpos(1000.0 * data[(MOCAP_LEFT_HAND * 3) + 0],
                        1000.0 * data[(MOCAP_LEFT_HAND * 3) + 1],
                        1000.0 * data[(MOCAP_LEFT_HAND * 3) + 2]);
                osg::Vec3 epos(1000.0 * data[(MOCAP_LEFT_ELBOW * 3) + 0],
                        1000.0 * data[(MOCAP_LEFT_ELBOW * 3) + 1],
                        1000.0 * data[(MOCAP_LEFT_ELBOW * 3) + 2]);
                osg::Vec3 dir = bpos - epos;
                osg::Quat q;
                osg::Vec3 origin(0,0,-1);
                q.makeRotate(origin,dir);
                it->second[1]->qx = q.x();
                it->second[1]->qy = q.y();
                it->second[1]->qz = q.z();
                it->second[1]->qw = q.w();
                //std::cerr << "LeftHand x: " << _bodyMap[e.sourceId][1]->x << " y: " << _bodyMap[e.sourceId][1]->y << " z: " << _bodyMap[e.sourceId][1]->z << std::endl;
                //right hand
                it->second[2]->x = 1000.0 * data[(MOCAP_RIGHT_HAND * 3) + 0];
                it->second[2]->y = 1000.0 * data[(MOCAP_RIGHT_HAND * 3) + 1];
                it->second[2]->z = 1000.0 * data[(MOCAP_RIGHT_HAND * 3) + 2];
                bpos = osg::Vec3(1000.0 * data[(MOCAP_RIGHT_HAND * 3) + 0],
                        1000.0 * data[(MOCAP_RIGHT_HAND * 3) + 1],
                        1000.0 * data[(MOCAP_RIGHT_HAND * 3) + 2]);
                epos = osg::Vec3(1000.0 * data[(MOCAP_RIGHT_ELBOW * 3) + 0],
                        1000.0 * data[(MOCAP_RIGHT_ELBOW * 3) + 1],
                        1000.0 * data[(MOCAP_RIGHT_ELBOW * 3) + 2]);
                dir = bpos - epos;
                q.makeRotate(origin,dir);
                it->second[2]->qx = q.x();
                it->second[2]->qy = q.y();
                it->second[2]->qz = q.z();
                it->second[2]->qw = q.w();
                //std::cerr << "RightHand x: " << _bodyMap[e.sourceId][2]->x << " y: " << _bodyMap[e.sourceId][2]->y << " z: " << _bodyMap[e.sourceId][2]->z << std::endl;

            }
            break;
        }
        case omicron::EventBase::ServiceTypePointer:
        {
            //std::cerr << "Pointer event: pos x: " << e.posx << " y: " << e.posy << " z: " << e.posz << std::endl;
            break;
        }
        case omicron::EventBase::ServiceTypeWand:
        {
            if(e.sourceId >= 0 && e.sourceId < _numWands)
            {
                // might need to change units
                _bodyList[e.sourceId]->x = e.posx;
                _bodyList[e.sourceId]->y = e.posy;
                _bodyList[e.sourceId]->z = e.posz;
                _bodyList[e.sourceId]->qx = e.orx;
                _bodyList[e.sourceId]->qy = e.ory;
                _bodyList[e.sourceId]->qz = e.orz;
                _bodyList[e.sourceId]->qw = e.orw;
            }
            break;
        }
        case omicron::EventBase::ServiceTypeKeyboard:
        {
            break;
        }
        default:
            break;
    }
}
