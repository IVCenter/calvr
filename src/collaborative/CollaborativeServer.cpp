#include <util/MultiListenSocket.h>
#include <util/CVRSocket.h>
#include <collaborative/CollaborativeManager.h>

#include <sys/types.h>
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <csignal>
#include <cstring>
#include <time.h>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

class CollaborativeMessage;

class SocketThread : public OpenThreads::Thread
{
    public:
	SocketThread(cvr::CVRSocket * socket, std::string name, int id);
	~SocketThread();

	virtual void run();

	int getID();
	std::string getName();
	void quit();
    
	void addMessage(CollaborativeMessage * message);

    protected:
	bool checkSocket();
	bool processEvents();

	std::queue<CollaborativeMessage *> _messageQueue;
	OpenThreads::Mutex _messageLock;

	cvr::CVRSocket * _socket;
	std::string _name;
	int _id;

	bool _quit;
};

class CollaborativeMessage
{
    public:
	CollaborativeMessage(int refs, int type, std::string target, int size, char * data);
	CollaborativeMessage(int refs, cvr::CollaborativeMessageHeader & cmh, char * data);
	~CollaborativeMessage();

	cvr::CollaborativeMessageHeader & getHeader();
	char * getData();

	void unref();

    protected:
	OpenThreads::Mutex _refMutex;

	int _refs;
	cvr::CollaborativeMessageHeader _header;
	char * _data;
};

using namespace cvr;

std::map<int,std::string> namemap;
std::map<int,struct ClientUpdate> clientmap;
std::map<int, std::vector<struct BodyUpdate> > clientHandList;
std::map<int, std::vector<struct BodyUpdate> > clientHeadList;
std::vector<SocketThread *> threadlist;
CollabMode currentMode = UNLOCKED;
int masterID = 0;
OpenThreads::Mutex globalLock;
// to ensure message order
OpenThreads::Mutex messageAddLock;

SocketThread::SocketThread(CVRSocket * socket, std::string name, int id)
{
    _socket = socket;
    _name = name;
    _id = id;
    _quit = false;
}

SocketThread::~SocketThread()
{
    if(_socket)
    {
	delete _socket;
    }
}

void SocketThread::run()
{
    while(1)
    {
	if(_quit)
	{
	    return;
	}

	if(!checkSocket())
	{
	    return;
	}
    }
}

void SocketThread::quit()
{
    _quit = true;
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

    int numTargets = clientHeadList[_id].size() + clientHandList[_id].size();

    if(numTargets)
    {
	
    }

    if(cu.numMes)
    {
	//Process client messages
    }

    cu.numMes = _id;
    struct ServerUpdate su;

    globalLock.lock();

    su.numUsers = threadlist.size();
    su.mode = currentMode;
    su.masterID = masterID;
    int numMes = 0;
    struct ClientUpdate * culist = NULL;
    int numToSend = 0;

    if(currentMode == LOCKED)
    {
	if(masterID != _id)
	{
	    numToSend = 1;
	    culist = new struct ClientUpdate[1];
	    culist[0] = clientmap[masterID];
	}
    }
    else
    {
	numToSend = threadlist.size() - 1;
	culist = new struct ClientUpdate[numToSend];
	int index = 0;
	for(int i = 0; i < threadlist.size(); i++)
	{
	    if(threadlist[i]->getID() != _id)
	    {
		culist[index] = clientmap[threadlist[i]->getID()];
		index++;
	    }
	}
    }

    clientmap[_id] = cu;

    globalLock.unlock();

    if(!_socket->send(&su,sizeof(struct ServerUpdate)))
    {
	if(culist)
	{
	    delete[] culist;
	}
	return false;
    }
    if(culist)
    {
	if(!_socket->send(culist,sizeof(struct ClientUpdate)*numToSend))
	{
	    delete[] culist;
	    return false;
	}
	delete[] culist;
    }

    return true;
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
    delete[] _data;
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

void checkSockets()
{
    if(!threadlist.size())
    {
	return;
    }

    for(std::vector<SocketThread*>::iterator it = threadlist.begin(); it != threadlist.end();)
    {
	if(!(*it)->isRunning())
	{
	    std::cerr << "Removing client " << (*it)->getName() << std::endl;
	    globalLock.lock();
	    clientmap.erase((*it)->getID());
	    if(currentMode == LOCKED)
	    {
		if(masterID == (*it)->getID())
		{
		    currentMode = UNLOCKED;
		    masterID = 0;
		}
	    }

	    delete (*it);

	    it = threadlist.erase(it);

	    globalLock.unlock();
	    continue;
	}
	it++;
    }
}

void initConnection(int id, CVRSocket * sock)
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

    std::cerr << "Got name: " << cii.name << std::endl;

    globalLock.lock();

    ServerInitInfo sii;
    sii.numUsers = namemap.size();

    // send other users' id and and name size
    int index = 0;
    int totalsize = 0;
    int * userinfo = NULL;
    char * usernames = NULL;
    if(sii.numUsers)
    {
	userinfo = new int[sii.numUsers * 2];
	for(std::map<int, std::string>::iterator it = namemap.begin(); it != namemap.end(); it++)
	{
	    userinfo[index++] = it->first;
	    userinfo[index++] = it->second.size()+1;
	    totalsize += it->second.size()+1;
	}
	usernames = new char[totalsize];
	index = 0;
	for(int i = 0; i < namemap.size(); i++)
	{
	    strcpy(usernames + index, namemap[userinfo[i*2]].c_str());
	    index += userinfo[(i*2)+1];
	}
    }

    globalLock.unlock();

    if(!sock->send(&sii,sizeof(struct ServerInitInfo)))
    {
	delete sock;
	return;
    }

    if(sii.numUsers)
    {
	if(!sock->send(userinfo,sizeof(int)*sii.numUsers*2))
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
	delete[] usernames;
    }

    ClientUpdate * cu = &clientmap[id];
    memset(cu,0,sizeof(struct ClientUpdate));
    cu->objScale = 1.0;
    cu->objTrans[0] = cu->objTrans[5] = cu->objTrans[10] = cu->objTrans[15] = 1.0;
    cu->numMes = id;

    clientHeadList[id] = std::vector<BodyUpdate>();
    clientHandList[id] = std::vector<BodyUpdate>();

    BodyUpdate bu;
    bu.pos[0] = bu.pos[1] = bu.pos[2] = 0.0;
    bu.rot[0] = bu.rot[1] = bu.rot[2] = 0.0;
    bu.rot[3] = 1.0;

    for(int i = 0; i < cii.numHeads; i++)
    {
	clientHeadList[id].push_back(bu);
    }

    for(int i = 0; i < cii.numHands; i++)
    {
	clientHandList[id].push_back(bu);
    }

    namemap[id] = cii.name;

    if(threadlist.size())
    {

	int msize = strlen(cii.name) + 1;
	msize += sizeof(int);
	char * mdata = new char[msize];

	*((int *)mdata) = id;
	strcpy(mdata+sizeof(int), cii.name);

	CollaborativeMessageHeader cmh;
	cmh.type = ADD_CLIENT;
	strcpy(cmh.target,"Collaborative");
	cmh.size = msize;

	CollaborativeMessage * cmessage = new CollaborativeMessage(threadlist.size(), cmh, mdata);

	messageAddLock.lock();

	for(int i = 0; i < threadlist.size(); i++)
	{
	    threadlist[i]->addMessage(cmessage);
	}

	messageAddLock.unlock();

    }

    SocketThread * st;
    st = new SocketThread(sock,cii.name,id);
    threadlist.push_back(st);
    st->start();
    std::cerr << "Added socket from host: " << cii.name << std::endl;
}

int main(int argc, char ** argv)
{
    cvr::MultiListenSocket * mls = new cvr::MultiListenSocket(11050);
    if(mls->setup())
    {
	std::cerr << "Socket setup ok." << std::endl;
    }
    else
    {
	std::cerr << "Socket setup failure." << std::endl;
	exit(0);
    }

    int socketid = 0;

    signal(SIGPIPE, SIG_IGN);

    while(1)
    {
	//std::cerr << "Checking for waiting connection." << std::endl;
	cvr::CVRSocket * con;
	if((con = mls->accept()))
	{
	    std::cerr << "Found connection." << std::endl;
	    con->setNoDelay(true);
	    initConnection(socketid,con);
	    socketid++;
	}
	//sleep(5);
	checkSockets();
    }

    return 1;
}
