#include <collaborative/CollaborativeThread.h>

#include <iostream>

using namespace cvr;
using namespace OpenThreads;

CollaborativeThread::CollaborativeThread()
{
}

void CollaborativeThread::init(CVRSocket * socket, int id)
{
    _socket = socket;
    _connected = _socket ? _socket->valid() : false;
    _updating = false;
    _updateDone = false;
    _clientUpdate = NULL;
    _quit = false;
    _id = id;
}

CollaborativeThread::~CollaborativeThread()
{
}

void CollaborativeThread::run()
{
    while(1)
    {
	if(_quit)
	{
	    return;
	}

	if(_updating)
	{
	    if(!_socket->send(&_myInfo, sizeof(ClientUpdate),MSG_NOSIGNAL))
	    {
		return;
	    }

	    if(!_socket->recv(&_serverUpdate, sizeof(ServerUpdate)))
	    {
		return;
	    }

	    if(_serverUpdate.mode == LOCKED)
	    {
		if(_serverUpdate.masterID != _id)
		{
		    _clientUpdate = new struct ClientUpdate[1];
		    if(!_socket->recv(_clientUpdate, sizeof(struct ClientUpdate)))
		    {
			return;
		    }
		}
	    }
	    else
	    {
		_clientUpdate = new struct ClientUpdate[_serverUpdate.numUsers];
		if(!_socket->recv(_clientUpdate,sizeof(struct ClientUpdate) * (_serverUpdate.numUsers - 1)))
		{
		    return;
		}
	    }

	    _updating = false;
	    _updateDone = true;
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

void CollaborativeThread::startUpdate(struct ClientUpdate & cu)
{
    _updateDone = false;
    _myInfo = cu;
    if(_clientUpdate)
    {
	delete[] _clientUpdate;
	_clientUpdate = NULL;
    }
    _updating = true;
}

bool CollaborativeThread::updateDone()
{
    return _updateDone;
}

void CollaborativeThread::quit()
{
    _quit = true;
}

void CollaborativeThread::getUpdate(ServerUpdate & su, ClientUpdate * & clientlist)
{
    su = _serverUpdate;
    clientlist = _clientUpdate;
}
