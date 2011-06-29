#include <collaborative/CollaborativeServer.h>
#include <util/MultiListenSocket.h>
#include <util/CVRSocket.h>

#include <iostream>

using namespace cvr;

CollaborativeServer::CollaborativeServer(int port)
{
    _port = port;
    _masterID = 0;
    _currentMode = UNLOCKED;
}

CollaborativeServer::~CollaborativeServer()
{
}

bool CollaborativeServer::init()
{
    _listenSocket = new cvr::MultiListenSocket(_port);
    if(_listenSocket->setup())
    {
	std::cerr << "Socket setup ok." << std::endl;
	return true;
    }
    else
    {
	std::cerr << "Socket setup failure." << std::endl;
	return false;
    }
}

void CollaborativeServer::run()
{
    int socketid = 0;

    signal(SIGPIPE, SIG_IGN);

    while(1)
    {
	//std::cerr << "Checking for waiting connection." << std::endl;
	cvr::CVRSocket * con;
	if((con = _listenSocket->accept()))
	{
	    std::cerr << "Found connection." << std::endl;
	    con->setNoDelay(true);
	    initConnection(socketid,con);
	    socketid++;
	}
	//sleep(5);
	checkSockets();
    }
}

void CollaborativeServer::initConnection(int id, CVRSocket * sock)
{
    if(!sock->send(&id,sizeof(int)))
    {
	delete sock;
	return;
    }
    /*int namesize;
    if(!sock->recv(&namesize,sizeof(int)))
    {
	delete sock;
	return;
    }
    std::cerr << "Got name size: " << namesize << std::endl;
    char name[255];
    if(!sock->recv(name,namesize))
    {
	delete sock;
	return;
    }*/

    ClientInitInfo cii;
    if(!sock->recv(&cii,sizeof(struct ClientInitInfo)))
    {
	delete sock;
	return;
    }

    std::cerr << "Got name: " << cii.name << " numHands: " << cii.numHands << " numHeads: " << cii.numHeads << std::endl;

    _serverLock.lock();

    ServerInitInfo sii;
    sii.numUsers = _clientInitMap.size();

    ClientInitInfo * activeClients = NULL;

    if(sii.numUsers)
    {
	activeClients = new ClientInitInfo[sii.numUsers];
	int acindex = 0;
	for(std::map<int,struct ClientInitInfo>::iterator it = _clientInitMap.begin(); it != _clientInitMap.end(); it++)
	{
	    activeClients[acindex] = it->second;
	    std::cerr << "Adding client init info to send name: " << activeClients[acindex].name << " id: " << activeClients[acindex].id << " heads: " << activeClients[acindex].numHeads << " hands: " << activeClients[acindex].numHands << std::endl;
	    acindex++;
	}
    }

    // send other users' id and and name size
    /*int index = 0;
    int totalsize = 0;
    int * userinfo = NULL;
    char * usernames = NULL;
    if(sii.numUsers)
    {
	userinfo = new int[sii.numUsers * 2];
	for(std::map<int, std::string>::iterator it = _nameMap.begin(); it != _nameMap.end(); it++)
	{
	    userinfo[index++] = it->first;
	    userinfo[index++] = it->second.size()+1;
	    totalsize += it->second.size()+1;
	}
	usernames = new char[totalsize];
	index = 0;
	for(int i = 0; i < _nameMap.size(); i++)
	{
	    strcpy(usernames + index, _nameMap[userinfo[i*2]].c_str());
	    index += userinfo[(i*2)+1];
	}
    }*/

    _serverLock.unlock();

    if(!sock->send(&sii,sizeof(struct ServerInitInfo)))
    {
	delete sock;
	return;
    }

    if(sii.numUsers)
    {
	/*if(!sock->send(userinfo,sizeof(int)*sii.numUsers*2))
	{
	    delete[] userinfo;
	    delete[] usernames;
	    delete sock;
	    return;
	}

	if(!sock->send(usernames,sizeof(char)*totalsize))
	{
	    delete[] userinfo;
	    delete[] usernames;
	    delete sock;
	    return;
	}

	delete[] userinfo;
	delete[] usernames;*/

	if(!sock->send(activeClients,sizeof(struct ClientInitInfo)*sii.numUsers))
	{
	    delete[] activeClients;
	    delete sock;
	    return;
	}

	delete[] activeClients;
    }

    ClientUpdate * cu = &_clientMap[id];
    memset(cu,0,sizeof(struct ClientUpdate));
    cu->objScale = 1.0;
    cu->objTrans[0] = cu->objTrans[5] = cu->objTrans[10] = cu->objTrans[15] = 1.0;
    cu->numMes = id;

    cii.id = id;
    _clientInitMap[id] = cii;

    _clientHeadList[id] = std::vector<BodyUpdate>();
    _clientHandList[id] = std::vector<BodyUpdate>();

    BodyUpdate bu;
    bu.pos[0] = bu.pos[1] = bu.pos[2] = 0.0;
    bu.rot[0] = bu.rot[1] = bu.rot[2] = 0.0;
    bu.rot[3] = 1.0;

    for(int i = 0; i < cii.numHeads; i++)
    {
	_clientHeadList[id].push_back(bu);
    }

    for(int i = 0; i < cii.numHands; i++)
    {
	_clientHandList[id].push_back(bu);
    }

    if(_threadList.size())
    {

	/*int msize = strlen(cii.name) + 1;
	msize += sizeof(int);
	char * mdata = new char[msize];

	*((int *)mdata) = id;
	strcpy(mdata+sizeof(int), cii.name);*/
	ClientInitInfo * newcii = new ClientInitInfo;
	*newcii = cii;

	CollaborativeMessageHeader cmh;
	cmh.type = ADD_CLIENT;
	strcpy(cmh.target,"Collaborative");
	cmh.size = sizeof(struct ClientInitInfo);

	CollaborativeMessage * cmessage = new CollaborativeMessage(_threadList.size(), cmh, (char*)newcii);

	_messageAddLock.lock();

	for(int i = 0; i < _threadList.size(); i++)
	{
	    _threadList[i]->addMessage(cmessage);
	}

	_messageAddLock.unlock();

    }

    SocketThread * st;
    st = new SocketThread(sock,cii.name,id,this);
    
    _serverLock.lock();

    _threadList.push_back(st);

    _serverLock.unlock();

    st->start();
    std::cerr << "Added socket from host: " << cii.name << std::endl;
}

void CollaborativeServer::checkSockets()
{
    if(!_threadList.size())
    {
	return;
    }

    for(std::vector<SocketThread*>::iterator it = _threadList.begin(); it != _threadList.end();)
    {
	if(!(*it)->isRunning())
	{
	    std::cerr << "Removing client " << (*it)->getName() << std::endl;
	    _serverLock.lock();
	    _clientMap.erase((*it)->getID());
	    _clientInitMap.erase((*it)->getID());
	    if(_currentMode == LOCKED)
	    {
		if(_masterID == (*it)->getID())
		{
		    _currentMode = UNLOCKED;
		    _masterID = 0;
		}
	    }

	    delete (*it);

	    it = _threadList.erase(it);

	    _serverLock.unlock();
	    continue;
	}
	it++;
    }
}

SocketThread::SocketThread(CVRSocket * socket, std::string name, int id, CollaborativeServer * server)
{
    _socket = socket;
    _name = name;
    _id = id;
    _quit = false;
    _server = server;
}

SocketThread::~SocketThread()
{
    if(_socket)
    {
	delete _socket;
    }

    quit();
    join();

    queueCleanup(_messageQueue);
}

void SocketThread::run()
{
    while(1)
    {
	_quitLock.lock();
	if(_quit)
	{
	    return;
	}
	_quitLock.unlock();

	if(!checkSocket())
	{
	    return;
	}
    }
}

void SocketThread::quit()
{
    _quitLock.lock();
    _quit = true;
    _quitLock.unlock();
}

void SocketThread::addMessage(CollaborativeMessage * message)
{
    _messageLock.lock();

    _messageQueue.push(message);

    _messageLock.unlock();
}

std::string SocketThread::getName()
{
    return _name;
}

int SocketThread::getID()
{
    return _id;
}

bool SocketThread::checkSocket()
{
    fd_set socketsetR;
    FD_ZERO(&socketsetR);
    FD_SET((unsigned int)_socket->getSocketFD(),&socketsetR);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500;

    select(_socket->getSocketFD()+1,&socketsetR,NULL,NULL,&tv);

    if(FD_ISSET(_socket->getSocketFD(),&socketsetR))
    {
	return processEvents();
    }

    return true;
}

bool SocketThread::processEvents()
{
    struct ClientUpdate cu;
    if(!_socket->recv(&cu, sizeof(struct ClientUpdate)))
    {
	return false;
    }

    int numTargets = _server->_clientHeadList[_id].size() + _server->_clientHandList[_id].size();
    int index = 0;

    BodyUpdate * bu = NULL;
    if(numTargets)
    {
	bu = new BodyUpdate[numTargets];
	if(!_socket->recv(bu, sizeof(struct BodyUpdate) * numTargets))
	{
	    delete[] bu;
	    return false;
	}
    }

    CollaborativeMessageHeader * cmesh = NULL;
    if(cu.numMes)
    {
	//Process client messages
	cmesh = new CollaborativeMessageHeader[cu.numMes];
	if(!_socket->recv(cmesh, sizeof(struct CollaborativeMessageHeader) * cu.numMes))
	{
	    if(bu)
	    {
		delete[] bu;
	    }
	    delete[] cmesh;
	    return false;
	}

	for(int i = 0; i < cu.numMes; i++)
	{
	    if(!processMessage(cmesh[i]))
	    {
		if(bu)
		{
		    delete[] bu;
		}
		return false;
	    }
	}

	delete[] cmesh;
    }

    cu.numMes = _id;
    struct ServerUpdate su;

    _server->_serverLock.lock();

    su.numUsers = _server->_threadList.size();
    su.mode = _server->_currentMode;
    su.masterID = _server->_masterID;
    struct ClientUpdate * culist = NULL;
    struct BodyUpdate * cubu = NULL;
    int numToSend = 0;
    int numBodyToSend = 0;

    if(_server->_currentMode == LOCKED)
    {
	if(_server->_masterID != _id)
	{
	    numToSend = 1;
	    culist = new struct ClientUpdate[1];
	    culist[0] = _server->_clientMap[_server->_masterID];
	    numBodyToSend = _server->_clientHeadList[_server->_masterID].size() + _server->_clientHandList[_server->_masterID].size();
	    int index = 0;
	    cubu = new struct BodyUpdate[numBodyToSend];
	    for(int i = 0; i < _server->_clientHeadList[_server->_masterID].size(); i++)
	    {
		cubu[index] = _server->_clientHeadList[_server->_masterID][i];
		index++;
	    }
	    for(int i = 0; i < _server->_clientHandList[_server->_masterID].size(); i++)
	    {
		cubu[index] = _server->_clientHandList[_server->_masterID][i];
		index++;
	    }
	}
    }
    else if(_server->_threadList.size() > 1)
    {
	numToSend = _server->_threadList.size() - 1;
	culist = new struct ClientUpdate[numToSend];
	int index = 0;
	for(int i = 0; i < _server->_threadList.size(); i++)
	{
	    if(_server->_threadList[i]->getID() != _id)
	    {
		numBodyToSend += _server->_clientHeadList[_server->_threadList[i]->getID()].size() + _server->_clientHandList[_server->_threadList[i]->getID()].size();
		culist[index] = _server->_clientMap[_server->_threadList[i]->getID()];
		index++;
	    }
	}

	cubu = new struct BodyUpdate[numBodyToSend];
	index = 0;

	for(int i = 0; i < _server->_threadList.size(); i++)
	{
	    if(_server->_threadList[i]->getID() != _id)
	    {
		continue;
	    }
	    for(int j = 0; j < _server->_clientHeadList[_server->_threadList[i]->getID()].size(); j++)
	    {
		cubu[index] = _server->_clientHeadList[_server->_threadList[i]->getID()][j];
		index++;
	    }
	}

	for(int i = 0; i < _server->_threadList.size(); i++)
	{
	    if(_server->_threadList[i]->getID() != _id)
	    {
		continue;
	    }
	    for(int j = 0; j < _server->_clientHandList[_server->_threadList[i]->getID()].size(); j++)
	    {
		cubu[index] = _server->_clientHandList[_server->_threadList[i]->getID()][j];
		index++;
	    }
	}
	std::cerr << "Sending " << numBodyToSend << std::endl;
    }

    _server->_clientMap[_id] = cu;

    int bindex = 0;
    for(int i = 0; i < _server->_clientHeadList[_id].size(); i++)
    {
	_server->_clientHeadList[_id][i] = bu[bindex];
	bindex++;
    }
    for(int i = 0; i < _server->_clientHandList[_id].size(); i++)
    {
	_server->_clientHandList[_id][i] = bu[bindex];
	bindex++;
    }

    if(bu)
    {
	delete[] bu;
    }

    _server->_serverLock.unlock();

    std::queue<CollaborativeMessage *> localQueue;

    _messageLock.lock();

    su.numMes = _messageQueue.size();

    CollaborativeMessageHeader * cucmh = NULL;

    if(su.numMes)
    {
	cucmh = new CollaborativeMessageHeader[su.numMes];
    }

    for(int i = 0; i < su.numMes; i++)
    {
	
	localQueue.push(_messageQueue.front());
	cucmh[i] = _messageQueue.front()->getHeader();
	_messageQueue.pop();
	std::cerr << "Creating message: type: " << cucmh[i].type << " target: " << cucmh[i].target << " size: " << cucmh[i].size << std::endl;
    }

    _messageLock.unlock();

    if(!_socket->send(&su,sizeof(struct ServerUpdate)))
    {
	if(culist)
	{
	    delete[] culist;
	}
	if(cubu)
	{
	    delete[] cubu;
	}
	if(cucmh)
	{
	    delete[] cucmh;
	}
	queueCleanup(localQueue);
	return false;
    }

    if(cucmh)
    {
	if(!_socket->send(cucmh,sizeof(struct CollaborativeMessageHeader)*su.numMes))
	{
	    delete[] cucmh;
	    if(culist)
	    {
		delete[] culist;
	    }
	    if(cubu)
	    {
		delete[] cubu;
	    }
	    queueCleanup(localQueue);
	    return false;
	}
	delete[] cucmh;
    }

    for(int i = 0; i < su.numMes; i++)
    {
	if(localQueue.front()->getHeader().size)
	{
	    std::cerr << "Sending message data of size: " << localQueue.front()->getHeader().size << std::endl;
	    if(!_socket->send(localQueue.front()->getData(),localQueue.front()->getHeader().size))
	    {
		if(culist)
		{
		    delete[] culist;
		}
		if(cubu)
		{
		    delete[] cubu;
		}
		queueCleanup(localQueue);
		return false;
	    }
	}
	localQueue.front()->unref();
	localQueue.pop();
    }

    if(culist)
    {
	if(!_socket->send(culist,sizeof(struct ClientUpdate)*numToSend))
	{
	    delete[] culist;
	    if(cubu)
	    {
		delete[] cubu;
	    }
	    return false;
	}
	delete[] culist;
    }
    if(cubu)
    {
	if(!_socket->send(cubu,sizeof(struct BodyUpdate)*numBodyToSend))
	{
	    delete[] cubu;
	    return false;
	}
	delete[] cubu;
    }

    return true;
}

bool SocketThread::processMessage(CollaborativeMessageHeader & cmh)
{
    char * data = NULL;
    if(cmh.size)
    {
	data = new char[cmh.size];
	if(!_socket->recv(data, cmh.size))
	{
	    delete[] data;
	    return false;
	}
    }

    switch(cmh.type)
    {
	default:
	{
	    _server->_serverLock.lock();

	    if(_server->_threadList.size() > 1)
	    {
		CollaborativeMessage * cm = new CollaborativeMessage(_server->_threadList.size() - 1, cmh, data);
		for(int i = 0; i < _server->_threadList.size(); i++)
		{
		    if(_server->_threadList[i] != this)
		    {
			_server->_threadList[i]->addMessage(cm);
		    }
		}
	    }

	    _server->_serverLock.unlock();
	    break;
	}
    }

    if(data)
    {
	delete[] data;
    }

    return true;
}

void SocketThread::queueCleanup(std::queue<CollaborativeMessage *> & queue)
{
    while(queue.size())
    {
	queue.front()->unref();
	queue.pop();
    }
}

CollaborativeMessage::CollaborativeMessage(int refs, int type, std::string target, int size, char * data)
{
    _header.type = type;
    _header.target[255] = '\0';
    strncpy(_header.target, target.c_str(),255);
    _header.size = size;
    _data = data;
    _refs = refs;
}

CollaborativeMessage::CollaborativeMessage(int refs, cvr::CollaborativeMessageHeader & cmh, char * data)
{
    _header = cmh;
    _data = data;
    _refs = refs;
}

CollaborativeMessage::~CollaborativeMessage()
{
    if(_data)
    {
	delete[] _data;
    }
}

CollaborativeMessageHeader & CollaborativeMessage::getHeader()
{
    return _header;
}

char * CollaborativeMessage::getData()
{
    return _data;
}

void CollaborativeMessage::unref()
{
    _refMutex.lock();
    _refs--;

    if(_refs <= 0)
    {
	delete this;
	return;
    }

    _refMutex.unlock();
}

int main(int argc, char ** argv)
{
    CollaborativeServer * cs = new CollaborativeServer(11050);
    if(!cs->init())
    {
	return 0;
    }

    cs->run();

    return 1;
}
