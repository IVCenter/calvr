#include <collaborative/CollaborativeThread.h>

#include <iostream>

using namespace cvr;
using namespace OpenThreads;

CollaborativeThread::CollaborativeThread(std::map<int,struct ClientInitInfo> * clientInitMap)
{
    _clientInitMap = clientInitMap;
}

void CollaborativeThread::init(CVRSocket * socket, int id)
{
    _socket = socket;
    _connected = _socket ? _socket->valid() : false;
    _updating = false;
    _updateDone = false;
    _clientUpdate = NULL;
    _bodiesUpdate = NULL;
    _messageHeaderUpdate = NULL;
    _messageDataUpdate = NULL;
    _serverUpdate = new ServerUpdate;
    _quit = false;
    _id = id;
}

CollaborativeThread::~CollaborativeThread()
{
    quit();
    join();
    delete _serverUpdate;
}

void CollaborativeThread::run()
{

    while(1)
    {
	_quitLock.lock();
	if(_quit)
	{
	    _quitLock.unlock();
	    //std::cerr << "Thread Quit exit." << std::endl;
	    return;
	}
	_quitLock.unlock();

	_statusLock.lock();
	if(_updating)
	{
	    //std::cerr << "Updating" << std::endl;
	    _statusLock.unlock();
	    if(!_socket->send(&_myInfo, sizeof(ClientUpdate),MSG_NOSIGNAL))
	    {
		return;
	    }

	    if(!_socket->send(_myTrackedBodies, sizeof(BodyUpdate) * _numBodies,MSG_NOSIGNAL))
	    {
		return;
	    }

	    if(_numMessages)
	    {
		if(!_socket->send(_messageHeaders, sizeof(CollaborativeMessageHeader) * _numMessages,MSG_NOSIGNAL))
		{
		    return;
		}
		for(int i = 0; i < _numMessages; i++)
		{
		    if(!_socket->send(_messageData[i], _messageHeaders[i].size,MSG_NOSIGNAL))
		    {
			return;
		    }
		}
		// clean up message meta data after send
		delete[] _messageHeaders;
		_messageHeaders = NULL;
		delete[] _messageData;
		_messageData = NULL;
		_myInfo.numMes = 0;
	    }

	    //std::cerr << "Data sent" << std::endl;

	    if(!_socket->recv(_serverUpdate, sizeof(ServerUpdate)))
	    {
		return;
	    }

	    //std::cerr << "Got server update." << std::endl;
	    //std::cerr << "Num Messages: " << _serverUpdate->numMes << std::endl;
	    if(_serverUpdate->numMes)
	    {
		_messageHeaderUpdate = new CollaborativeMessageHeader[_serverUpdate->numMes];
		_messageDataUpdate = new char*[_serverUpdate->numMes];

		if(!_socket->recv(_messageHeaderUpdate, sizeof(struct CollaborativeMessageHeader) * _serverUpdate->numMes))
		{
		    return;
		}

		for(int i = 0; i < _serverUpdate->numMes; i++)
		{
		    std::cerr << "type: " << _messageHeaderUpdate[i].type << " target: " << _messageHeaderUpdate[i].target << " size: " << _messageHeaderUpdate[i].size << std::endl;
		    if(_messageHeaderUpdate[i].size)
		    {
			_messageDataUpdate[i] = new char[_messageHeaderUpdate[i].size];
			if(!_socket->recv(_messageDataUpdate[i],_messageHeaderUpdate[i].size))
			{
			    return;
			}
		    }
		    else
		    {
			_messageDataUpdate[i] = NULL;
		    }
		}
	    }

	    //std::cerr << "Got messages." << std::endl;

	    // process add/remove events
	    for(int i = 0; i < _serverUpdate->numMes; i++)
	    {
		processMessage(_messageHeaderUpdate[i],_messageDataUpdate[i]);
	    }

	    if(_serverUpdate->mode == LOCKED)
	    {
		if(_serverUpdate->masterID != _id)
		{
		    _clientUpdate = new struct ClientUpdate[1];
		    if(!_socket->recv(_clientUpdate, sizeof(struct ClientUpdate)))
		    {
			return;
		    }
		    int numBodies = (*_clientInitMap)[_clientUpdate[0].numMes].numHeads + (*_clientInitMap)[_clientUpdate[0].numMes].numHands;
		    _bodiesUpdate = new BodyUpdate[numBodies];
		    if(!_socket->recv(_bodiesUpdate, sizeof(struct BodyUpdate) * numBodies))
		    {
			return;
		    }
		}
	    }
	    else if(_serverUpdate->numUsers > 1)
	    {
		_clientUpdate = new struct ClientUpdate[_serverUpdate->numUsers - 1];
		if(!_socket->recv(_clientUpdate,sizeof(struct ClientUpdate) * (_serverUpdate->numUsers - 1)))
		{
		    return;
		}
		int numBodies = 0;
		for(int i = 0; i < (_serverUpdate->numUsers - 1); i++)
		{
		    //std::cerr << "counting id: " << _clientUpdate[i].numMes << std::endl;
		    numBodies += (*_clientInitMap)[_clientUpdate[i].numMes].numHeads + (*_clientInitMap)[_clientUpdate[i].numMes].numHands;
		    //std::cerr << "Num bodies: " << numBodies << std::endl;
		}
		if(numBodies)
		{
		    _bodiesUpdate = new BodyUpdate[numBodies];
		    if(!_socket->recv(_bodiesUpdate, sizeof(struct BodyUpdate) * numBodies))
		    {
			return;
		    }
		}
		//std::cerr << "Num Users: " << _serverUpdate->numUsers << " NumBodies: " << numBodies << std::endl;
	    } 

	    //std::cerr << "Got bodies." << std::endl;

	    _statusLock.lock();
	    _updating = false;
	    _updateDone = true;
	    _statusLock.unlock();
	}
	else
	{
	    _statusLock.unlock();
	}

	microSleep(500);

    }
}

bool CollaborativeThread::isConnected()
{
    return _connected;
}

CVRSocket * CollaborativeThread::getSocket()
{
    return _socket;
}

void CollaborativeThread::startUpdate(struct ClientUpdate & cu, int numBodies, struct cvr::BodyUpdate * bodies, int numMessages, CollaborativeMessageHeader * messageHeaders, char** messageData)
{
    //std::cerr << "Start Update." << std::endl;
    _updateDone = false;
    _myInfo = cu;
    if(_numBodies)
    {
	delete[] _myTrackedBodies;
    }
    _numBodies = numBodies;
    _myTrackedBodies  = bodies;
    _numMessages = numMessages;
    _messageHeaders = messageHeaders;
    _messageData = messageData;
    //TODO update
    if(_clientUpdate)
    {
	delete[] _clientUpdate;
	_clientUpdate = NULL;
    }
    if(_bodiesUpdate)
    {
	delete[] _bodiesUpdate;
	_bodiesUpdate = NULL;
    }
    if(_messageHeaderUpdate)
    {
	delete[] _messageHeaderUpdate;
	_messageHeaderUpdate = NULL;
    }
    if(_messageDataUpdate);
    {
	delete[] _messageDataUpdate;
	_messageDataUpdate = NULL;
    }
    _updating = true;
}

bool CollaborativeThread::updateDone()
{
    _statusLock.lock();
    bool ret = _updateDone;
    _statusLock.unlock();
    return ret;
}

void CollaborativeThread::quit()
{
    _quitLock.lock();
    _quit = true;
    _quitLock.unlock();
}

void CollaborativeThread::getUpdate(ServerUpdate * & su, ClientUpdate * & clientList, BodyUpdate * & bodyList, CollaborativeMessageHeader * & messageHeaders, char ** & messageData)
{
    su = _serverUpdate;
    clientList = _clientUpdate;
    bodyList = _bodiesUpdate;
    messageHeaders = _messageHeaderUpdate;
    messageData = _messageDataUpdate;
}

void CollaborativeThread::processMessage(CollaborativeMessageHeader & cmh, char * data)
{
    switch(cmh.type)
    {
	case ADD_CLIENT:
	{
	    ClientInitInfo * cii = (ClientInitInfo*)data;
	    (*_clientInitMap)[cii->id] = *cii;
	    break;
	}
	case REMOVE_CLIENT:
	{
	    int id = *((int*)data);
	    _clientInitMap->erase(id);
	    break;
	}
	default:
	    break;
    }
}
