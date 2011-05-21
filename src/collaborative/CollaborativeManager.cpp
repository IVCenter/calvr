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
	_thread = new CollaborativeThread();
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
    bool res = false;
    int id;
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

	    char hostname[255];
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
	    }

	    _socket->setBlocking(true);

	    _thread->init(_socket, id);
	    _thread->start();

            startUpdate();

	    res = true;
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
	}
	else
	{
	    ComController::instance()->readMaster(&id,sizeof(int));
	}
	_id = id;
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

    osg::Vec3 vec = TrackingManager::instance()->getHeadMat().getTrans();
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

    _thread->startUpdate(cu);
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
    struct ClientUpdate * culist;

    if(ComController::instance()->isMaster())
    {
	_thread->getUpdate(su, culist);
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
	    }
	    else
	    {
		struct ClientUpdate cu;
		ComController::instance()->readMaster(&cu,sizeof(struct ClientUpdate));
		_clientMap[cu.numMes] = cu;
	    }
	}
    }
    else
    {
	_clientMap.clear();

	if(ComController::instance()->isMaster())
	{
	    ComController::instance()->sendSlaves(culist,sizeof(struct ClientUpdate) * (su.numUsers - 1));
	}
	else
	{
	    culist = new struct ClientUpdate[su.numUsers - 1];
	    ComController::instance()->readMaster(culist,sizeof(struct ClientUpdate) * (su.numUsers - 1));
	}

	for(int i = 0; i < su.numUsers - 1; i++)
	{
	    _clientMap[culist[i].numMes] = culist[i];
	}

	if(!ComController::instance()->isMaster())
	{
	    delete[] culist;
	}
    }

    // TODO setup message passing
    if(su.numMes)
    {
	// process server messages
    }

    updateCollabNodes();

    startUpdate();
}

void CollaborativeManager::updateCollabNodes()
{
    _collabRoot->removeChildren(0,_collabRoot->getNumChildren());

    if(_mode == UNLOCKED)
    {
	for(int i = _collabHands.size(); i < _clientMap.size(); i++)
	{
	    _collabHands.push_back(new osg::MatrixTransform());
	    _collabHeads.push_back(new osg::MatrixTransform());

	    _collabHeads[i]->addChild(makeHead(i));
	    _collabHands[i]->addChild(makeHand(i));
	}

	int index = 0;
	for(std::map<int, ClientUpdate>::iterator it = _clientMap.begin(); it != _clientMap.end(); it++)
	{
	    osg::Vec3 headpos(it->second.headPos[0],it->second.headPos[1],it->second.headPos[2]);
	    osg::Quat headquat(it->second.headRot[0],it->second.headRot[1],it->second.headRot[2],it->second.headRot[3]);
	    osg::Matrix headmat;
	    headmat.makeRotate(headquat);
	    headmat.setTrans(headpos);

	    osg::Vec3 handpos(it->second.handPos[0],it->second.handPos[1],it->second.handPos[2]);
	    osg::Quat handquat(it->second.handRot[0],it->second.handRot[1],it->second.handRot[2],it->second.handRot[3]);
	    osg::Matrix handmat;
	    handmat.makeRotate(handquat);
	    handmat.setTrans(handpos);

	    osg::Matrix objMatInv;

	    for(int i = 0; i < 4; i++)
	    {
		for(int j = 0; j < 4; j++)
		{
		    objMatInv(i,j) = it->second.objTrans[(4*i)+j];
		}
	    }

	    objMatInv = osg::Matrix::inverse(objMatInv);

	    osg::Matrix objScaleInv;
	    objScaleInv.makeScale(osg::Vec3(1.0 / it->second.objScale,1.0 / it->second.objScale,1.0 / it->second.objScale));

	    _collabHands[index]->setMatrix(handmat * objMatInv * objScaleInv);
	    _collabHeads[index]->setMatrix(headmat * objMatInv * objScaleInv);

	    _collabRoot->addChild(_collabHeads[index].get());
	    _collabRoot->addChild(_collabHands[index].get());

	    index++;
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
