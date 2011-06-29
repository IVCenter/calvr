#include <collaborative/CollaborativeManager.h>
#include <collaborative/CollaborativeThread.h>
#include <input/TrackingManager.h>
#include <kernel/SceneManager.h>
#include <kernel/ComController.h>

#include <iostream>
#include <cstring>

#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/Geode>

using namespace cvr;

osg::Vec3 makeColor(float f)
{
    if(f < 0)
    {
        f = 0;
    }
    else if(f > 1.0)
    {
        f = 1.0;
    }

    osg::Vec3 color;

    if(f <= 0.33)
    {
        float part = f / 0.33;
        float part2 = 1.0 - part;

        color.x() = part2;
        color.y() = part;
        color.z() = 0;
    }
    else if(f <= 0.66)
    {
        f = f - 0.33;
        float part = f / 0.33;
        float part2 = 1.0 - part;

        color.x() = 0;
        color.y() = part2;
        color.z() = part;
    }
    else if(f <= 1.0)
    {
        f = f - 0.66;
        float part = f / 0.33;
        float part2 = 1.0 - part;

        color.x() = part;
        color.y() = 0;
        color.z() = part2;
    }

    //std::cerr << "Color x: " << color.x() << " y: " << color.y() << " z: " << color.z() << std::endl;

    return color;
}


CollaborativeManager * CollaborativeManager::_myPtr = NULL;

CollaborativeManager::CollaborativeManager()
{
    _socket = NULL;
    _connected = false;
    _mode = UNLOCKED;
    if(ComController::instance()->isMaster())
    {
	_thread = new CollaborativeThread(&_clientInitMap);
    }
    else
    {
	_thread = NULL;
    }
}

CollaborativeManager::~CollaborativeManager()
{
}

CollaborativeManager * CollaborativeManager::instance()
{
    if(!_myPtr)
    {
	_myPtr = new CollaborativeManager();
    }
    return _myPtr;
}

bool CollaborativeManager::init()
{
    _collabRoot = new osg::MatrixTransform();
    SceneManager::instance()->getObjectsRoot()->addChild(_collabRoot.get());
    return true;
}

bool CollaborativeManager::isConnected()
{
    return _connected;
}

bool CollaborativeManager::connect(std::string host, int port)
{
    bool res = true;
    int id;
    int numUsers;
    ClientInitInfo * ciiList;
    if(ComController::instance()->isMaster())
    {
	if(_thread->isRunning())
	{
	    disconnect();
	}

	if(_socket)
	{
	    delete _socket;
	    _socket = NULL;
	}

	_socket = new CVRSocket(CONNECT, host, port);

	_socket->setBlocking(false);
	_socket->setNoDelay(true);

	if(_socket->connect(5))
	{
	    std::cerr << "Connected to Collaborative Server: " << host << " Port: " << port << std::endl;
	    if(!_socket->recv(&id,sizeof(int)))
	    {
		res = false;
	    }
	    std::cerr << "My id is " << id << std::endl;

	    /*char hostname[255];
	    gethostname(hostname, 254);

	    int length = strlen(hostname)+1;
	    std::cerr << "Sending length: " << length << std::endl;
	    if(!_socket->send(&length,sizeof(int),MSG_NOSIGNAL))
	    {
		res = false;
	    }
	    std::cerr << "Sending hostname: " << hostname << std::endl;
	    if(!_socket->send(hostname,length,MSG_NOSIGNAL))
	    {
		res = false;
	    }*/

	    ClientInitInfo cii;
	    gethostname(cii.name, 254);
	    cii.numHeads = TrackingManager::instance()->getNumHeads();
	    cii.numHands = TrackingManager::instance()->getNumHands();

	    _socket->setBlocking(true);

	    std::cerr << "Sending hostname: " << cii.name << " NumHeads: " << cii.numHeads << " NumHands: " << cii.numHands << std::endl;

	    if(!_socket->send(&cii,sizeof(struct ClientInitInfo),MSG_NOSIGNAL))
	    {
		res = false;
	    }

	    ServerInitInfo sii;
	    if(!_socket->recv(&sii,sizeof(struct ServerInitInfo)))
	    {
		res = false;
	    }

	    std::cerr << "There are " << sii.numUsers << " users connected to collab server." << std::endl;
	    numUsers = sii.numUsers;

	    _clientInitMap.clear();

	    if(res && sii.numUsers)
	    {
		std::cerr << "Getting cii from " << sii.numUsers << " users." << std::endl;
		ciiList = new ClientInitInfo[sii.numUsers];
		if(!_socket->recv(ciiList,sizeof(struct ClientInitInfo)*sii.numUsers))
		{
		    res = false;
		}
		std::cerr << "cii list value name: " << ciiList[0].name << std::endl;
	    }

	    _thread->init(_socket, id);
	    _thread->start();

            //startUpdate();
	}
	else
	{
	    std::cerr << "Unable to connect to Collaborative Server: " << host << " Port: " << port << std::endl;
	    res = false;
	}
	ComController::instance()->sendSlaves(&res,sizeof(bool));
    }
    else
    {
	ComController::instance()->readMaster(&res,sizeof(bool));
    }

    if(res)
    {
	if(ComController::instance()->isMaster())
	{
	    ComController::instance()->sendSlaves(&id,sizeof(int));
	    ComController::instance()->sendSlaves(&numUsers,sizeof(int));
	    if(numUsers)
	    {
		ComController::instance()->sendSlaves(ciiList,sizeof(struct ClientInitInfo)*numUsers);
	    }
	}
	else
	{
	    ComController::instance()->readMaster(&id,sizeof(int));
	    ComController::instance()->readMaster(&numUsers,sizeof(int));
	    if(numUsers)
	    {
		ciiList = new ClientInitInfo[numUsers];
		ComController::instance()->readMaster(ciiList,sizeof(struct ClientInitInfo)*numUsers);
	    }
	}
	_id = id;

	std::cerr << "Init with " << numUsers << " users." << std::endl;

	for(int i = 0; i < numUsers; i++)
	{
	    _clientInitMap[ciiList[i].id] = ciiList[i];
	    std::cerr << "Adding other user with id: " << ciiList[i].id << " name: " << ciiList[i].name << " numHeads: " << ciiList[i].numHeads << " numHands: " << ciiList[i].numHands << std::endl;
	    BodyUpdate bu;
	    bu.pos[0] = 0;
	    bu.pos[1] = 0;
	    bu.pos[2] = 0;
	    bu.rot[0] = 0;
	    bu.rot[1] = 0;
	    bu.rot[2] = 0;
	    bu.rot[3] = 1.0;

	    if(ciiList[i].numHeads)
	    {
		_headBodyMap[ciiList[i].id] = std::vector<BodyUpdate>();
		_collabHeads[ciiList[i].id] = std::vector<osg::ref_ptr<osg::MatrixTransform> >();
		for(int j = 0; j < ciiList[i].numHeads; j++)
		{
		    _headBodyMap[ciiList[i].id].push_back(bu);
		    _collabHeads[ciiList[i].id].push_back(new osg::MatrixTransform());
		    _collabHeads[ciiList[i].id][j]->addChild(makeHead(ciiList[i].id));
		}
	    }

	    if(ciiList[i].numHands)
	    {
		_handBodyMap[ciiList[i].id] = std::vector<BodyUpdate>();
		_collabHands[ciiList[i].id] = std::vector<osg::ref_ptr<osg::MatrixTransform> >();
		for(int j = 0; j < ciiList[i].numHands; j++)
		{
		    _handBodyMap[ciiList[i].id].push_back(bu);
		    _collabHands[ciiList[i].id].push_back(new osg::MatrixTransform());
		    _collabHands[ciiList[i].id][j]->addChild(makeHand(ciiList[i].id));
		}
	    }
	}
	if(ComController::instance()->isMaster())
	{
	    startUpdate();
	}
    }

    _connected = res;
    return res;
}

void CollaborativeManager::disconnect()
{
    if(ComController::instance()->isMaster())
    {
	std::cerr << "Disconnect from collaborative server." << std::endl;
	if(_thread)
	{
	    if(_thread->isRunning())
	    {
		_thread->quit();
	    }
	    while(_thread->isRunning())
	    {
	    }
	}

	if(_socket)
	{
	    delete _socket;
	    _socket = NULL;
	}
    }

    _clientMap.clear();
    _collabRoot->removeChildren(0,_collabRoot->getNumChildren());
    _connected = false;
}

void CollaborativeManager::startUpdate()
{
    if(!ComController::instance()->isMaster() || !_thread || !_thread->isRunning())
    {
	return;
    }

    struct ClientUpdate cu;

    /*osg::Vec3 vec = TrackingManager::instance()->getHeadMat().getTrans();
    cu.headPos[0] = vec[0];
    cu.headPos[1] = vec[1];
    cu.headPos[2] = vec[2];

    osg::Quat quat = TrackingManager::instance()->getHeadMat().getRotate();
    cu.headRot[0] = quat[0];
    cu.headRot[1] = quat[1];
    cu.headRot[2] = quat[2];
    cu.headRot[3] = quat[3];

    vec = TrackingManager::instance()->getHandMat().getTrans();
    cu.handPos[0] = vec[0];
    cu.handPos[1] = vec[1];
    cu.handPos[2] = vec[2];

    quat = TrackingManager::instance()->getHandMat().getRotate();
    cu.handRot[0] = quat[0];
    cu.handRot[1] = quat[1];
    cu.handRot[2] = quat[2];
    cu.handRot[3] = quat[3];*/

    cu.objScale = SceneManager::instance()->getObjectScale();
    
    osg::Matrix m = SceneManager::instance()->getObjectTransform()->getMatrix();
    for(int i = 0; i < 4; i++)
    {
	for(int j = 0; j < 4; j++)
	{
	    cu.objTrans[(4*i)+j] = m(i,j);
	}
    }

    int numBodies = TrackingManager::instance()->getNumHeads() + TrackingManager::instance()->getNumHands();
    BodyUpdate * bodies = NULL;

    if(numBodies)
    {
	bodies = new BodyUpdate[numBodies];
	int bindex = 0;
	for(int i = 0; i < TrackingManager::instance()->getNumHeads(); i++)
	{
	    osg::Vec3 pos = TrackingManager::instance()->getHeadMat(i).getTrans();
	    osg::Quat rot = TrackingManager::instance()->getHeadMat(i).getRotate();
	    bodies[bindex].pos[0] = pos.x();
	    bodies[bindex].pos[1] = pos.y();
	    bodies[bindex].pos[2] = pos.z();
	    bodies[bindex].rot[0] = rot.x();
	    bodies[bindex].rot[1] = rot.y();
	    bodies[bindex].rot[2] = rot.z();
	    bodies[bindex].rot[3] = rot.w();
	    bindex++;
	}

	for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
	{
	    osg::Vec3 pos = TrackingManager::instance()->getHandMat(i).getTrans();
	    osg::Quat rot = TrackingManager::instance()->getHandMat(i).getRotate();
	    bodies[bindex].pos[0] = pos.x();
	    bodies[bindex].pos[1] = pos.y();
	    bodies[bindex].pos[2] = pos.z();
	    bodies[bindex].rot[0] = rot.x();
	    bodies[bindex].rot[1] = rot.y();
	    bodies[bindex].rot[2] = rot.z();
	    bodies[bindex].rot[3] = rot.w();
	    bindex++;
	}
    }

    //TODO: add message passing
    cu.numMes = 0;

    _thread->startUpdate(cu,numBodies,bodies,cu.numMes,NULL,NULL);
}

void CollaborativeManager::update()
{
    if(!_connected || ComController::instance()->getIsSyncError())
    {
	return;
    }

    bool toUpdate[2];
    if(ComController::instance()->isMaster())
    {
	toUpdate[0] = _thread->updateDone();
	toUpdate[1] = _thread->isRunning();	
	ComController::instance()->sendSlaves(toUpdate,sizeof(bool) * 2);
    }
    else
    {
	ComController::instance()->readMaster(toUpdate,sizeof(bool) * 2);
    }

    if(!toUpdate[1])
    {
	disconnect();
	return;
    }

    if(!toUpdate[0])
    {
	return;
    }

    //std::cerr << "Update Ready." << std::endl;

    /*struct ClientUpdate cu;

    osg::Vec3 vec = TrackingManager::instance()->getHeadMat().getTrans();
    cu.headPos[0] = vec[0];
    cu.headPos[1] = vec[2];
    cu.headPos[2] = vec[3];

    osg::Quat quat = TrackingManager::instance()->getHeadMat().getRotate();
    cu.headRot[0] = quat[0];
    cu.headRot[1] = quat[1];
    cu.headRot[2] = quat[2];
    cu.headRot[3] = quat[3];

    vec = TrackingManager::instance()->getHandMat().getTrans();
    cu.handPos[0] = vec[0];
    cu.handPos[1] = vec[2];
    cu.handPos[2] = vec[3];

    quat = TrackingManager::instance()->getHandMat().getRotate();
    cu.handRot[0] = quat[0];
    cu.handRot[1] = quat[1];
    cu.handRot[2] = quat[2];
    cu.handRot[3] = quat[3];

    cu.objScale = SceneManager::instance()->getObjectScale();
    
    osg::Matrix m = SceneManager::instance()->getObjectTransform()->getMatrix();
    for(int i = 0; i < 4; i++)
    {
	for(int j = 0; j < 4; j++)
	{
	    cu.objTrans[(4*i)+j] = m(i,j);
	}
    }

    cu.numMes = 0;
    if(!_socket->send(&cu, sizeof(ClientUpdate),MSG_NOSIGNAL))
    {
	disconnect();
	return;
    }

    struct ServerUpdate su;
    if(!_socket->recv(&su, sizeof(ServerUpdate)))
    {
	disconnect();
	return;
    }
    */

    struct ServerUpdate su;
    struct ClientUpdate * culist = NULL;
    struct BodyUpdate * bodies = NULL;
    struct CollaborativeMessageHeader * cmh = NULL;
    char ** messageData = NULL;

    if(ComController::instance()->isMaster())
    {
	ServerUpdate * sup;
	_thread->getUpdate(sup, culist, bodies, cmh, messageData);
	su = *sup;

	ComController::instance()->sendSlaves(&su,sizeof(struct ServerUpdate));	
    }
    else
    {
	ComController::instance()->readMaster(&su,sizeof(struct ServerUpdate));
    }
    
    if(su.mode == LOCKED)
    {
	_masterID = su.masterID;
	if(_masterID != _id)
	{
	    if(ComController::instance()->isMaster())
	    {
		ComController::instance()->sendSlaves(culist,sizeof(struct ClientUpdate));
		_clientMap[culist[0].numMes] = culist[0];
		int numBodies = _clientInitMap[culist[0].numMes].numHeads + _clientInitMap[culist[0].numMes].numHands;
		ComController::instance()->sendSlaves(bodies,sizeof(struct BodyUpdate)*numBodies);
	    }
	    else
	    {
		struct ClientUpdate cu;
		ComController::instance()->readMaster(&cu,sizeof(struct ClientUpdate));
		_clientMap[cu.numMes] = cu;
		int numBodies = _clientInitMap[culist[0].numMes].numHeads + _clientInitMap[culist[0].numMes].numHands;
		bodies = new BodyUpdate[numBodies];
		ComController::instance()->readMaster(bodies,sizeof(struct BodyUpdate)*numBodies);
	    }

	    int bindex = 0;

	    for(int i = 0; i < _clientInitMap[culist[0].numMes].numHeads; i++)
	    {
		_headBodyMap[culist[0].numMes][i] = bodies[bindex];
		bindex++;
	    }

	    for(int i = 0; i < _clientInitMap[culist[0].numMes].numHands; i++)
	    {
		_handBodyMap[culist[0].numMes][i] = bodies[bindex];
		bindex++;
	    }

	    if(bodies && !ComController::instance()->isMaster())
	    {
		delete[] bodies;
	    }
	}
    }
    else
    {
	_clientMap.clear();

	if(su.numUsers > 1)
	{
	    int numBodies;
	    if(ComController::instance()->isMaster())
	    {
		ComController::instance()->sendSlaves(culist,sizeof(struct ClientUpdate) * (su.numUsers - 1));
		numBodies = 0;

		for(int i = 0; i < su.numUsers; i++)
		{
		    numBodies += _clientInitMap[culist[i].numMes].numHeads + _clientInitMap[culist[i].numMes].numHands;
		}

		if(numBodies)
		{
		    ComController::instance()->sendSlaves(bodies,sizeof(struct BodyUpdate) * numBodies);
		}
	    }
	    else 
	    {
		culist = new struct ClientUpdate[su.numUsers - 1];
		ComController::instance()->readMaster(culist,sizeof(struct ClientUpdate) * (su.numUsers - 1));

		numBodies = 0;

		for(int i = 0; i < su.numUsers; i++)
		{
		    numBodies += _clientInitMap[culist[i].numMes].numHeads + _clientInitMap[culist[i].numMes].numHands;
		}

		if(numBodies)
		{
		    bodies = new BodyUpdate[numBodies];
		    ComController::instance()->readMaster(bodies,sizeof(struct BodyUpdate) * numBodies);
		}
	    }

	    int bindex = 0;

	    for(int i = 0; i < su.numUsers - 1; i++)
	    {
		_clientMap[culist[i].numMes] = culist[i];

		for(int j = 0; j < _clientInitMap[culist[i].numMes].numHeads; j++)
		{
		    _headBodyMap[culist[i].numMes][j] = bodies[bindex];
		    bindex++;
		}

		for(int j = 0; j < _clientInitMap[culist[i].numMes].numHands; j++)
		{
		    _handBodyMap[culist[i].numMes][j] = bodies[bindex];
		    bindex++;
		}
	    }

	    if(!ComController::instance()->isMaster())
	    {
		delete[] culist;
		if(numBodies)
		{
		    delete[] bodies;
		}
	    }

	}
    }

    if(su.numMes)
    {
	if(ComController::instance()->isMaster())
	{
	    ComController::instance()->sendSlaves(cmh,sizeof(struct CollaborativeMessageHeader) * su.numMes);
	    for(int i = 0; i < su.numMes; i++)
	    {
		if(cmh[i].size)
		{
		    ComController::instance()->sendSlaves(messageData[i],cmh[i].size);
		}
	    }
	}
	else
	{
	    cmh = new CollaborativeMessageHeader[su.numMes];
	    ComController::instance()->readMaster(cmh,sizeof(struct CollaborativeMessageHeader) * su.numMes);
	    messageData = new char*[su.numMes];
	    for(int i = 0; i < su.numMes; i++)
	    {
		if(cmh[i].size)
		{
		    messageData[i] = new char[cmh[i].size];
		    ComController::instance()->readMaster(messageData[i],cmh[i].size);
		}
		else
		{
		    messageData[i] = NULL;
		}
	    }
	}

	for(int i = 0; i < su.numMes; i++)
	{
	    processMessage(cmh[i],messageData[i]);
	}

	
	if(!ComController::instance()->isMaster())
	{
	    delete[] cmh;
	    delete[] messageData;
	}
    }

    updateCollabNodes();

    startUpdate();
}

void CollaborativeManager::updateCollabNodes()
{
    _collabRoot->removeChildren(0,_collabRoot->getNumChildren());

    if(_mode == UNLOCKED)
    {
	for(std::map<int, ClientInitInfo>::iterator it = _clientInitMap.begin(); it != _clientInitMap.end(); it++)
	{
	    if(_clientMap.find(it->first) == _clientMap.end())
	    {
		continue;
	    }

	    osg::Matrix objMatInv;

	    for(int i = 0; i < 4; i++)
	    {
		for(int j = 0; j < 4; j++)
		{
		    objMatInv(i,j) = _clientMap[it->first].objTrans[(4*i)+j];
		}
	    }

	    objMatInv = osg::Matrix::inverse(objMatInv);

	    osg::Matrix objScaleInv;
	    objScaleInv.makeScale(osg::Vec3(1.0 / _clientMap[it->first].objScale,1.0 / _clientMap[it->first].objScale,1.0 / _clientMap[it->first].objScale));

	    osg::Matrix m;
	    for(int i = 0; i < it->second.numHeads; i++)
	    {
		BodyUpdate * bu = &_headBodyMap[it->first][i];
		osg::Vec3 pos(bu->pos[0],bu->pos[1],bu->pos[2]);
		osg::Quat quat(bu->rot[0],bu->rot[1],bu->rot[2],bu->rot[3]);
		m.makeRotate(quat);
		m.setTrans(pos);

		/*osg::Vec3 handpos(it->second.handPos[0],it->second.handPos[1],it->second.handPos[2]);
		osg::Quat handquat(it->second.handRot[0],it->second.handRot[1],it->second.handRot[2],it->second.handRot[3]);
		osg::Matrix handmat;
		handmat.makeRotate(handquat);
		handmat.setTrans(handpos);*/

		_collabHeads[it->first][i]->setMatrix(m * objMatInv * objScaleInv);

		_collabRoot->addChild(_collabHeads[it->first][i].get());
	    }

	    for(int i = 0; i < it->second.numHands; i++)
	    {
		BodyUpdate * bu = &_handBodyMap[it->first][i];
		osg::Vec3 pos(bu->pos[0],bu->pos[1],bu->pos[2]);
		osg::Quat quat(bu->rot[0],bu->rot[1],bu->rot[2],bu->rot[3]);
		m.makeRotate(quat);
		m.setTrans(pos);

		/*osg::Vec3 handpos(it->second.handPos[0],it->second.handPos[1],it->second.handPos[2]);
		osg::Quat handquat(it->second.handRot[0],it->second.handRot[1],it->second.handRot[2],it->second.handRot[3]);
		osg::Matrix handmat;
		handmat.makeRotate(handquat);
		handmat.setTrans(handpos);*/

		_collabHands[it->first][i]->setMatrix(m * objMatInv * objScaleInv);

		_collabRoot->addChild(_collabHands[it->first][i].get());
	    }
	}
    }
    else
    {
	if(_masterID != _id)
	{
	    osg::Matrix objMat;
	    for(int i = 0; i < 4; i++)
	    {
		for(int j = 0; j < 4; j++)
		{
		    objMat(i,j) = _clientMap[_masterID].objTrans[(4*i)+j];
		}
	    }

	    SceneManager::instance()->setObjectMatrix(objMat);
	    SceneManager::instance()->setObjectScale(_clientMap[_masterID].objScale);
	}
    }
}

osg::Node * CollaborativeManager::makeHand(int num)
{
    osg::Cone * cone = new osg::Cone(osg::Vec3(0,500,0), 10, 2000);
    osg::Quat q = osg::Quat(-M_PI/ 2.0, osg::Vec3(1.0,0,0));
    cone->setRotation(q);
    osg::ShapeDrawable * sd = new osg::ShapeDrawable(cone);
    osg::Geode * geode = new osg::Geode();
    geode->addDrawable(sd);
    return geode;
}

osg::Node * CollaborativeManager::makeHead(int num)
{
    osg::Cone * cone = new osg::Cone(osg::Vec3(0,0,0), 50, 200);
    osg::Quat q = osg::Quat(-M_PI/ 2.0, osg::Vec3(1.0,0,0));
    cone->setRotation(q);
    osg::ShapeDrawable * sd = new osg::ShapeDrawable(cone);
    osg::Geode * geode = new osg::Geode();
    geode->addDrawable(sd);
    return geode;
}

void CollaborativeManager::processMessage(CollaborativeMessageHeader & cmh, char * data)
{
    switch(cmh.type)
    {
	case ADD_CLIENT:
	{
	    std::cerr << "Add Client message." << std::endl;
	    ClientInitInfo * cii = (ClientInitInfo*)data;
	    _clientInitMap[cii->id] = *cii;

	    BodyUpdate bu;
	    bu.pos[0] = 0;
	    bu.pos[1] = 0;
	    bu.pos[2] = 0;
	    bu.rot[0] = 0;
	    bu.rot[1] = 0;
	    bu.rot[2] = 0;
	    bu.rot[3] = 1.0;

	    if(cii->numHeads)
	    {
		_headBodyMap[cii->id] = std::vector<BodyUpdate>();
		_collabHeads[cii->id] = std::vector<osg::ref_ptr<osg::MatrixTransform> >();
		for(int j = 0; j < cii->numHeads; j++)
		{
		    _headBodyMap[cii->id].push_back(bu);
		    _collabHeads[cii->id].push_back(new osg::MatrixTransform());
		    _collabHeads[cii->id][j]->addChild(makeHead(cii->id));
		}
	    }

	    if(cii->numHands)
	    {
		_handBodyMap[cii->id] = std::vector<BodyUpdate>();
		_collabHands[cii->id] = std::vector<osg::ref_ptr<osg::MatrixTransform> >();
		for(int j = 0; j < cii->numHands; j++)
		{
		    _handBodyMap[cii->id].push_back(bu);
		    _collabHands[cii->id].push_back(new osg::MatrixTransform());
		    _collabHands[cii->id][j]->addChild(makeHand(cii->id));
		}
	    }
	    break;
	}
	default:
	    break;
    }

    if(data)
    {
	delete[] data;
    }
}
