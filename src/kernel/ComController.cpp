#include <kernel/ComController.h>
#include <config/ConfigManager.h>

#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <string>

using namespace cvr;

ComController * ComController::_myPtr = NULL;

ComController::ComController()
{
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
    std::string fileArg;
    // TODO: do this better
    if(ap->argc() > 6)
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
    }

    _numSlaves = ConfigManager::getInt("MultiPC.NumSlaves", 0);

    if(_isMaster)
    {
        char hostname[51];
        gethostname(hostname, 50);
        std::string myHost = hostname;
        //std::cerr << "myHost: " << myHost << std::endl;
        _masterInterface = ConfigManager::getEntry("value",
                                                   "MultiPC.MasterInterface",
                                                   myHost);
        std::cerr << "Starting up as Master." << std::endl;
        return setupConnections(fileArg);
    }
    else
    {
        std::cerr << "Starting up as Node: " << _slaveNum << " with Master: "
                << _masterInterface << std::endl;
        return connectMaster();
    }
}

void ComController::sendSlaves(void * data, int size)
{
    if(!_isMaster || !data || !size)
    {
        return;
    }
    for(int i = 0; i < _slaveSockets.size(); i++)
    {
        _slaveSockets[i]->send(data, size);
    }
}

void ComController::readMaster(void * data, int size)
{
    if(_isMaster || !data || !size)
    {
        return;
    }

    _masterSocket->recv(data, size);
}

void ComController::readSlaves(void * data, int size)
{
    if(!_isMaster && !size)
    {
        return;
    }

    char * recBuf;
    if(data)
    {
        recBuf = (char *)data;
    }
    else
    {
        recBuf = new char[size * _numSlaves];
    }

    for(int i = 0; i < _slaveSockets.size(); i++)
    {
        _slaveSockets[i]->recv(recBuf, size);
        recBuf += size;
    }
    if(!data)
    {
        delete[] recBuf;
    }
}

void ComController::sendMaster(void * data, int size)
{
    if(_isMaster || !data || !size)
    {
        return;
    }

    _masterSocket->send(data, size);
}

void ComController::sync()
{
    //std::cerr << "In Sync." << std::endl;
    char msg = 'n';
    if(_isMaster)
    {
        if(_numSlaves > 0)
        {
            char * resp = new char[_numSlaves];
            readSlaves(resp, sizeof(char));
            sendSlaves(&msg, sizeof(char));
            delete[] resp;
        }

    }
    else
    {
        msg = (char)_slaveNum;
        sendMaster(&msg, sizeof(char));
        readMaster(&msg, sizeof(char));
    }
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

bool ComController::setupConnections(std::string & fileArg)
{
    int baseport = 11000;

    std::vector<std::string> startupList;
    bool found;
    for(int i = 0; i < _numSlaves; i++)
    {
        std::stringstream ss;
        ss << "MultiPC.Startup:" << i;
        startupList.push_back(ConfigManager::getEntry(ss.str(), "", &found));
        if(!found)
        {
            std::cerr << "No Startup entry for node " << i << std::endl;
            return false;
        }
    }

    for(int i = 0; i < _numSlaves; i++)
    {
        std::stringstream ss;
        ss << "CalVR -s " << i << " -h " << _masterInterface << " -p "
                << baseport + 2 * i << " " << fileArg;
        size_t location = startupList[i].find("CalVR");
        if(location != std::string::npos)
        {
            startupList[i].replace(location, 5, ss.str());
            continue;
        }
        location = startupList[i].find("opencover");
        if(location != std::string::npos)
        {
            startupList[i].replace(location, 9, ss.str());
            continue;
        }
        std::cerr << "No CalVR found in startup value." << std::endl;
        return false;
    }

    for(int i = 0; i < _numSlaves; i++)
    {
        std::cerr << startupList[i] << std::endl;
        system((startupList[i] + " &").c_str());
    }

    bool ok = true;
    for(int i = 0; i < _numSlaves; i++)
    {
        CVRSocket * sock;

        sock = new CVRSocket(LISTEN, _masterInterface, baseport + 2 * i);

        if(!sock->valid())
        {
            ok = false;
            continue;
        }

        sock->setReuseAddress(true);
        sock->setNoDelay(true);

        int bufferSize = 1024;
        if(sock->setsockopt(SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(int))
                == -1)
        {
            perror("SO_SNDBUF");
        }

        if(sock->setsockopt(SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(int))
                == -1)
        {
            perror("SO_RCVBUF");
        }

        if(!sock->bind())
        {
            ok = false;
            continue;
        }

        if(!sock->listen())
        {
            ok = false;
            continue;
        }

        if(!sock->accept())
        {
            ok = false;
            continue;
        }

        sock->setNoDelay(true);

        _slaveSockets.push_back(sock);

        std::cerr << "Connected to Node: " << i << std::endl;
    }

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

    int bufferSize = 1024;
    if(sock->setsockopt(SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(int)) == -1)
    {
        perror("SO_SNDBUF");
    }

    if(sock->setsockopt(SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(int)) == -1)
    {
        perror("SO_RCVBUF");
    }

    int count = 0;

    if(!sock->connect(60))
    {
        return false;
    }

    _masterSocket = sock;

    struct InitMsg im;
    im.ok = false;
    readMaster(&im, sizeof(struct InitMsg));

    return im.ok;
}
