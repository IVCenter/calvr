#include <cvrInput/TrackerOmicron.h>

#include <cstring>
#include <iostream>

using namespace cvr;

TrackerOmicron::TrackerOmicron()
{

}

TrackerOmicron::~TrackerOmicron()
{
}

bool TrackerOmicron::init(std::string tag)
{
    _client = new omicronConnector::OmicronConnectorClient(this);
    //_client->connect("67.58.41.51",27000);
    //_client->connect("67.58.41.53",27000);
    _client->connect("137.110.119.244",27000);
    return true;
}

TrackerBase::TrackedBody * TrackerOmicron::getBody(int index)
{
    if(index < 0 || (index / 3) >= _bodyMap.size())
    {
	return NULL;
    }
    std::map<unsigned int,std::vector<TrackedBody*> >::iterator it = _bodyMap.begin();
    for(int i = 0; i < index / 3; i++)
    {
	++it;
    }
    return it->second[index % 3];
}

unsigned int TrackerOmicron::getButtonMask()
{
    return 0;
}

float TrackerOmicron::getValuator(int index)
{
    return 0.0f;
}

int TrackerOmicron::getNumBodies()
{
    int size = 0;
    for(std::map<unsigned int,std::vector<TrackedBody*> >::iterator it = _bodyMap.begin(); it != _bodyMap.end(); it++)
    {
	size += it->second.size();
    }
    //return size;
    return 6;
}

int TrackerOmicron::getNumValuators()
{
    return 0;
}

int TrackerOmicron::getNumButtons()
{
    return 0;
}

void TrackerOmicron::update(
        std::map<int,std::list<InteractionEvent*> > & eventMap)
{
    _client->poll();
}

void TrackerOmicron::onEvent(const omicronConnector::EventData& e)
{
    //std::cerr << "Got event." << std::endl;
    switch(e.serviceType)
    {
	case omicron::EventBase::ServiceTypeMocap:
	{
	    //if(e.type == omicron::EventBase::Trace && _bodyMap.find(e.sourceId) == _bodyMap.end())
	    if(_bodyMap.find(e.sourceId) == _bodyMap.end())
	    {
		std::cerr << "Adding bodies for source id: " << e.sourceId << std::endl;
		TrackedBody * tb = new TrackedBody;
		tb->x = tb->y = tb->z = 0.0;
		osg::Quat q;
		tb->qx = q.x();
		tb->qy = q.y();
		tb->qz = q.z();
		tb->qw = q.w();
		_bodyMap[e.sourceId].push_back(tb);
		tb = new TrackedBody;
		tb->x = tb->y = tb->z = 0.0;
		tb->qx = q.x();
		tb->qy = q.y();
		tb->qz = q.z();
		tb->qw = q.w();
		_bodyMap[e.sourceId].push_back(tb);
		tb = new TrackedBody;
		tb->x = tb->y = tb->z = 0.0;
		tb->qx = q.x();
		tb->qy = q.y();
		tb->qz = q.z();
		tb->qw = q.w();
		_bodyMap[e.sourceId].push_back(tb);
	    }

            if(e.type == omicron::EventBase::Untrace && _bodyMap.find(e.sourceId) != _bodyMap.end())
	    {
		std::cerr << "Removing bodies for source id: " << e.sourceId << std::endl;
		for(int i = 0; i < _bodyMap[e.sourceId].size(); i++)
		{
		    delete _bodyMap[e.sourceId][i];
		}
		_bodyMap.erase(e.sourceId);
	    }

	    if(e.type == omicron::EventBase::Update)
	    {
		//std::cerr << "Update event type. id: " << e.sourceId << std::endl;
	    }

            if(e.type == omicron::EventBase::Update && _bodyMap.find(e.sourceId) != _bodyMap.end())
	    {
		//std::cerr << "Body map update" << std::endl;
		//std::cerr << "Pos x: " << e.posx << " y: " << e.posy << " z: " << e.posz << std::endl;
		float * data = (float*)&e.extraData[0];
		//head
		_bodyMap[e.sourceId][0]->x = 1000.0 * data[3];
	        _bodyMap[e.sourceId][0]->y = 1000.0 * data[4];
		_bodyMap[e.sourceId][0]->z = 1000.0 * data[5];
		//std::cerr << "Head x: " << _bodyMap[e.sourceId][0]->x << " y: " << _bodyMap[e.sourceId][0]->y << " z: " << _bodyMap[e.sourceId][0]->z << std::endl;
		//right hand
		_bodyMap[e.sourceId][1]->x = 1000.0 * data[(9*3)+0];
		_bodyMap[e.sourceId][1]->y = 1000.0 * data[(9*3)+1];
		_bodyMap[e.sourceId][1]->z = 1000.0 * data[(9*3)+2];

		osg::Vec3 bpos(1000.0 * data[(9*3)+0],1000.0 * data[(9*3)+1],1000.0 * data[(9*3)+2]);
		osg::Vec3 epos(1000.0 * data[(7*3)+0],1000.0 * data[(7*3)+1],1000.0 * data[(7*3)+2]);
		osg::Vec3 dir = bpos - epos;
		osg::Quat q;
		osg::Vec3 origin(0,0,-1);
		q.makeRotate(origin,dir);
		_bodyMap[e.sourceId][1]->qx = q.x();
		_bodyMap[e.sourceId][1]->qy = q.y();
		_bodyMap[e.sourceId][1]->qz = q.z();
		_bodyMap[e.sourceId][1]->qw = q.w(); 
		//std::cerr << "RightHand x: " << _bodyMap[e.sourceId][1]->x << " y: " << _bodyMap[e.sourceId][1]->y << " z: " << _bodyMap[e.sourceId][1]->z << std::endl;
		//left hand
		_bodyMap[e.sourceId][2]->x = 1000.0 * data[(15*3)+0];
		_bodyMap[e.sourceId][2]->y = 1000.0 * data[(15*3)+1];
		_bodyMap[e.sourceId][2]->z = 1000.0 * data[(15*3)+2];
		bpos = osg::Vec3(1000.0 * data[(15*3)+0],1000.0 * data[(15*3)+1],1000.0 * data[(15*3)+2]);
		epos = osg::Vec3(1000.0 * data[(13*3)+0],1000.0 * data[(13*3)+1],1000.0 * data[(13*3)+2]);
		dir = bpos - epos;
		q.makeRotate(origin,dir);
		_bodyMap[e.sourceId][2]->qx = q.x();
		_bodyMap[e.sourceId][2]->qy = q.y();
		_bodyMap[e.sourceId][2]->qz = q.z();
		_bodyMap[e.sourceId][2]->qw = q.w();
		//std::cerr << "LeftHand x: " << _bodyMap[e.sourceId][2]->x << " y: " << _bodyMap[e.sourceId][2]->y << " z: " << _bodyMap[e.sourceId][2]->z << std::endl;
			
	    }
	    break;
	}
	case omicron::EventBase::ServiceTypePointer:
	{
	    std::cerr << "Pointer event: pos x: " << e.posx << " y: " << e.posy << " z: " << e.posz << std::endl;
	    break;
	}
	default:
	    break;
    }
}
