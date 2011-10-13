#include <kernel/ComController.h>
#include <kernel/CalVR.h>
#include <config/ConfigManager.h>
#include <util/CVRSocket.h>
#include <util/CVRMulticastSocket.h>
#include <util/MultiListenSocket.h>

#include <osg/Timer>

#include <iostream>
#include <sstream>
#include <cstring>
#include <csignal>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#ifdef WIN32
#pragma comment(lib, "wsock32.lib")
#endif

using namespace cvr;

ComController * ComController::_myPtr = NULL;

ComController::ComController()
{
    _listenSocket = NULL;
    _masterSocket = NULL;
    _CCError = false;

    _maxSocketFD = -1;
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
}

ComController::~ComController()
{
}

ComController * ComController::instance()
{
    if(!_myPtr)
    {
        _myPtr = new ComController();
    }
    return _myPtr;
}

bool ComController::init(osg::ArgumentParser * ap)
{
    if(ap->read("-s",_slaveNum))
    {
	if(!ap->read("-h",_masterInterface))
	{
	    std::cerr << "Error: no \"-h\" (master interface) option given on command line." << std::endl;
	    return false;
	}
	if(!ap->read("-p",_port))
	{
	    std::cerr << "Error: no \"-p\" (master port) option given on command line." << std::endl;
	    return false;
	}
	_isMaster = false;
    }
    else
    {
	_isMaster = true;
    }

    /*std::cerr << "Argc: " << ap->argc() << std::endl;
    for(int i = 0; i < ap->argc(); i++)
    {
	std::cerr << "argv: " << ap->argv()[i] << std::endl;
    }*/

    /*if(ap->argc() > 6)
    {
        if(!strcmp(ap->argv()[1], "-s") && !strcmp(ap->argv()[3], "-h")
                && !strcmp(ap->argv()[5], "-p"))
        {
            _masterInterface = ap->argv()[4];
            _slaveNum = atoi(ap->argv()[2]);
            _port = atoi(ap->argv()[6]);
            _isMaster = false;
        }
        else
        {
            fileArg = ap->argv()[1];
            _isMaster = true;
        }
    }
    else
    {
        if(ap->argc() > 1)
        {
            fileArg = ap->argv()[1];
        }
        _isMaster = true;
    }*/

    _numSlaves = ConfigManager::getInt("MultiPC.NumSlaves", 0);

    bool ret;
    if(_isMaster)
    {
        //char hostname[51];
        //gethostname(hostname, 50);
        //std::string myHost = hostname;
        //std::cerr << "myHost: " << myHost << std::endl;
        _masterInterface = ConfigManager::getEntry("value",
                                                   "MultiPC.MasterInterface",
                                                   CalVR::instance()->getHostName());
        std::cerr << "Starting up as Master." << std::endl;
        ret = setupConnections();
    }
    else
    {
        std::cerr << "Starting up as Node: " << _slaveNum << " with Master: "
                << _masterInterface << std::endl;
        ret = connectMaster();
    }

    if(ret)
    {
        setupMulticast();
    }

    return ret;
}

bool ComController::sendSlaves(void * data, int size)
{
    if(!_isMaster || !data || _CCError)
    {
        return false;
    }

    if(!size)
    {
	return true;
    }

    bool ret = true;
    for(std::map<int,CVRSocket *>::iterator it = _slaveSockets.begin(); it != _slaveSockets.end(); it++)
    {
        if(!it->second->send(data, size))
	{
	    std::cerr << "ComController Error: send failure, sendSlaves, to node " << it->first << std::endl;
	    _CCError = true;
	    ret = false;
	}
    }

    return ret;
}

bool ComController::readMaster(void * data, int size)
{
    if(_isMaster || !data || _CCError)
    {
        return false;
    }

    if(!size)
    {
	return true;
    }

    if(!_masterSocket->recv(data, size))
    {
	std::cerr << "ComController Error: recv failure, readMaster." << std::endl;
	_CCError = true;
	return false;
    }
    return true;
}

bool ComController::sendSlavesMulticast(void * data, int size)
{
    if(!_isMaster || !data || _CCError)
    {
	return false;
    }

    if(!size)
    {
	return true;
    }

    if(_multicastUsable)
    {
	if(!_slaveMCSocket->send(data, size))
	{
	    std::cerr << "ComController Error: send failure, sendSlaves, multicast" << std::endl;
	    _CCError = true;
	    return false;
	}
    }
    else
    {
	return sendSlaves(data,size);
    }
    return true;
}

bool ComController::readMasterMulticast(void * data, int size)
{
    if(_isMaster || !data || _CCError)
    {
        return false;
    }

    if(!size)
    {
	return true;
    }

    if(_multicastUsable)
    {
	if(!_masterMCSocket->recv(data, size))
	{
	    std::cerr << "ComController Error: recv failure, readMasterMulticast." << std::endl;
	    _CCError = true;
	    return false;
	}
    }
    else
    {
	return readMaster(data,size);
    }
    return true;
}

bool ComController::readSlaves(void * data, int size)
{
    if(!_isMaster || _CCError)
    {
        return false;
    }

    if(!size)
    {
	return true;
    }

    bool ret = true;

    char * recBuf;
    if(data)
    {
        recBuf = (char *)data;
    }
    else
    {
        recBuf = new char[size * _numSlaves];
    }

    /*int nodesRead = 0;
	fd_set _sockets;
    std::map<int,bool> readMap;
    while(nodesRead < _numSlaves)
    {
	FD_ZERO(&_sockets);
	for(std::map<int,CVRSocket *>::iterator it = _slaveSockets.begin(); it != _slaveSockets.end(); it++)
	{
	    FD_SET(it->second->getSocketFD(),&_sockets);
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	select(_maxSocketFD+1,&_sockets,NULL,NULL,&tv);

	char * tmpPtr = recBuf;
	for(std::map<int,CVRSocket *>::iterator it = _slaveSockets.begin(); it != _slaveSockets.end(); it++)
	{
	    if(!readMap[it->first] && FD_ISSET(it->second->getSocketFD(),&_sockets))
	    {
		if(!it->second->recv(tmpPtr, size))
		{
		    std::cerr << "ComController Error: recv failure, readSlaves, node " << it->first << std::endl;
		    _CCError = true;
		    ret = false;
		    break;
		}
		readMap[it->first] = true;
		nodesRead++;
	    }
	    tmpPtr += size;
	}
    }*/

    char * tmpPtr = recBuf;
    for(std::map<int,CVRSocket *>::iterator it = _slaveSockets.begin(); it != _slaveSockets.end(); it++)
    {
        if(!it->second->recv(tmpPtr, size))
	{
	    std::cerr << "ComController Error: recv failure, readSlaves, node " << it->first << std::endl;
	    _CCError = true;
	    ret = false;
	}
        tmpPtr += size;
    }
    if(!data)
    {
        delete[] recBuf;
    }

    return ret;
}

bool ComController::sendMaster(void * data, int size)
{
    if(_isMaster || !data || _CCError)
    {
        return false;
    }

    if(!size)
    {
	return true;
    }

    if(!_masterSocket->send(data, size))
    {
	std::cerr << "ComController Error: send failure, sendMaster." << std::endl;
	_CCError = true;
	return false;
    }
    return true;
}

bool ComController::sync()
{
    if(_CCError)
    {
	return false;
    }

    //osg::Timer_t start, end;
    //start = osg::Timer::instance()->tick();

    //std::cerr << "In Sync." << std::endl;
    char msg = 'n';
    if(_isMaster)
    {
        if(_numSlaves > 0)
        {
            char * resp = new char[_numSlaves];
            if(!readSlaves(resp, sizeof(char)) || !sendSlaves(&msg, sizeof(char)))
	    {
		_CCError = true;
	    }
            delete[] resp;
        }

    }
    else
    {
        msg = (char)_slaveNum;
        if(!sendMaster(&msg, sizeof(char)) || !readMaster(&msg, sizeof(char)))
	{
	    _CCError = true;
	}
    }

    //end = osg::Timer::instance()->tick();

    //std::cerr << "Sync time: " << osg::Timer::instance()->delta_m(start,end) << std::endl;

    return _CCError;
    //std::cerr << "Done Sync." << std::endl;
}

bool ComController::isMaster()
{
    return _isMaster;
}

int ComController::getNumSlaves()
{
    return _numSlaves;
}

bool ComController::setupConnections()
{
    int baseport = ConfigManager::getInt("port","MultiPC.MasterInterface",11000);

    bool found;
    int nodecount = 0;

    if(_numSlaves)
    {
	for(int i = 0; i < 99; i++)
	{
	    std::stringstream ss;
	    ss << "MultiPC.Startup:" << i;
	    std::string startup = ConfigManager::getEntry(ss.str(), "", &found);
	    if(found)
	    {
		_startupMap[i] = startup;
		nodecount++;
		if(nodecount == _numSlaves)
		{
		    break;
		}
	    }
	}
    }

    if(_startupMap.size() != _numSlaves)
    {
	std::cerr << "ComController Error: NumSlaves set to " << _numSlaves << ", but only " << _startupMap.size() << " Startup entries found." << std::endl;
	return false;
    }

    if(_numSlaves > 0)
    {
	_listenSocket = new MultiListenSocket(baseport, _numSlaves);
	if(!_listenSocket->setup())
	{
	    delete _listenSocket;
	    _listenSocket = NULL;
	    std::cerr << "ComController Error setting up MultiListen Socket." << std::endl;
	    std::cerr << "Warning: may be CalVR processes running on slave nodes." << std::endl;
	    std::string cleanup;
	    cleanup = ConfigManager::getEntry("value","MultiPC.CleanupScript","NONE");
	    if(cleanup == "NONE")
	    {
		std::cerr << "No MultiPC.CleanupScript entry found." << std::endl;
	    }
	    else
	    {
		std::cerr << "Running Cleanup Script: " << cleanup << std::endl;
		system(cleanup.c_str());
		_listenSocket = new MultiListenSocket(baseport, _numSlaves);
		if(!_listenSocket->setup())
		{
		    delete _listenSocket;
		    _listenSocket = NULL;
		}
	    }

	    if(!_listenSocket)
	    {
		int rcount = 0;
		while(rcount < 10)
		{
		    baseport += 2;
		    std::cerr << "Trying port: " << baseport << std::endl;

		    _listenSocket = new MultiListenSocket(baseport, _numSlaves);
		    if(!_listenSocket->setup())
		    {
			delete _listenSocket;
			_listenSocket = NULL;
		    }
		    else
		    {
			break;
		    }

		    rcount++;
		}

		if(!_listenSocket)
		{
		    std::cerr << "ComController Failure setting up MultiListen Socket." << std::endl;
		    delete _listenSocket;
		    _listenSocket = NULL;
		    return false;
		}
	    }
	}
    }

    for(std::map<int,std::string>::iterator it = _startupMap.begin(); it != _startupMap.end(); it++)
    {
        std::stringstream ss;
        ss << "CalVR -s " << it->first << " -h " << _masterInterface << " -p "
                << baseport;
        size_t location = it->second.find("CalVR");
        if(location != std::string::npos)
        {
            it->second.replace(location, 5, ss.str());
            continue;
        }
        location = it->second.find("opencover");
        if(location != std::string::npos)
        {
            it->second.replace(location, 9, ss.str());
            continue;
        }
        std::cerr << "No CalVR found in startup value for node " << it->first << std::endl;
	std::cerr << "Startup line: " << it->second << std::endl;
        return false;
    }  

    for(std::map<int,std::string>::iterator it = _startupMap.begin(); it != _startupMap.end(); it++)
    {
        std::cerr << it->second << std::endl;
        system((it->second + " &").c_str());
    }
    
    bool ok = true;
    int retryCount = 20;
    std::map<int,bool> connectedMap;

    while(_slaveSockets.size() < _numSlaves)
    {
        CVRSocket * sock;

	while((sock = _listenSocket->accept()))
	{
	    sock->setNoDelay(true);

	    int nodeNum;
	    if(!sock->recv(&nodeNum,sizeof(int)))
	    {
		std::cerr << "ComController Error durring socket setup, recv failure." << std::endl;
		ok = false;
		delete sock;
		break;
	    }

	    _slaveSockets[nodeNum] = sock;
	    connectedMap[nodeNum] = true;

	    if(sock->getSocketFD() > _maxSocketFD)
	    {
		_maxSocketFD = sock->getSocketFD();
	    }

	    std::cerr << "Connected to Node: " << nodeNum << std::endl;
	}

#ifndef WIN32
	sleep(1);
#else
	Sleep(1000);
#endif
	retryCount--;
	if(!retryCount)
	{
	    std::cerr << "ComController Error: Only got connections from " << _slaveSockets.size() << " nodes, " << _numSlaves << " expected." << std::endl;

	    for(std::map<int,std::string>::iterator it = _startupMap.begin(); it != _startupMap.end(); it++)
	    {
		if(!connectedMap[it->first])
		{
		    std::cerr << "Node: " << it->first << " startup: " << it->second << std::endl;
		}
	    }
	    ok = false;
	    break;
	}
    }

    delete _listenSocket;
    _listenSocket = NULL;

    struct InitMsg im;
    im.ok = ok;
    sendSlaves(&im, sizeof(struct InitMsg));

    return ok;
}

bool ComController::connectMaster()
{
    CVRSocket * sock;

    sock = new CVRSocket(CONNECT, _masterInterface, _port);

    if(!sock->valid())
    {
        return false;
    }

    sock->setNoDelay(true);

    /*int bufferSize = 1024;
    if(sock->setsockopt(SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(int)) == -1)
    {
        perror("SO_SNDBUF");
    }

    if(sock->setsockopt(SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(int)) == -1)
    {
        perror("SO_RCVBUF");
    }*/

    if(!sock->connect(30))
    {
	std::cerr << "ComController Error: Unable to connect to master." << std::endl;
        return false;
    }

    if(!sock->send(&_slaveNum,sizeof(int)))
    {
	std::cerr << "ComController Error: Unable to send node number to master." << std::endl;
	return false;
    }

    _masterSocket = sock;

    struct InitMsg im;
    im.ok = false;
    readMaster(&im, sizeof(struct InitMsg));

    return im.ok;
}

void ComController::setupMulticast()
{
    if(_numSlaves && ConfigManager::getBool("value","MultiPC.Multicast",false,NULL))
    {
	std::string groupAddress = ConfigManager::getEntry("groupAddress","MultiPC.Multicast","225.0.0.51");
	int port = ConfigManager::getInt("port","MultiPC.Multicast",12000);
	bool found;
	std::string masterInterface = ConfigManager::getEntry("masterInterface","MultiPC.Multicast","",&found);
	if(isMaster())
	{
	    _slaveMCSocket = new CVRMulticastSocket(CVRMulticastSocket::SEND, groupAddress, port);
	    if(_slaveMCSocket->valid() && found)
	    {
		_slaveMCSocket->setMulticastInterface(masterInterface);
	    }
	    _multicastUsable = _slaveMCSocket->valid();
	    bool * status = new bool[_numSlaves];
	    readSlaves(status,sizeof(bool));
	    for(int i = 0; i < _numSlaves; i++)
	    {
		if(!status[i])
		{
		    _multicastUsable = false;
		}
	    }
	    delete[] status;
	    sendSlaves(&_multicastUsable,sizeof(bool));
	}
	else
	{
	    _masterMCSocket = new CVRMulticastSocket(CVRMulticastSocket::RECV, groupAddress, port);
	    _multicastUsable = _masterMCSocket->valid();
	    sendMaster(&_multicastUsable,sizeof(bool));
	    readMaster(&_multicastUsable,sizeof(bool));
	}
	if(_multicastUsable)
	{
	    std::cerr << "Multicast setup." << std::endl;
	}
	else
	{
	    std::cerr << "Multicast setup failed, converting multicast calls to TCP." << std::endl;
	}
    }
    else
    {
	_multicastUsable = false;
    }
}
