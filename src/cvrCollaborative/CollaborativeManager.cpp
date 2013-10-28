#include <cvrCollaborative/CollaborativeManager.h>
#include <cvrCollaborative/CollaborativeThread.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrKernel/SceneManager.h>
#include <cvrKernel/PluginManager.h>
#include <cvrKernel/ComController.h>
#include <cvrKernel/CalVR.h>
#include <cvrUtil/CVRSocket.h>
#include <cvrUtil/ComputeBoundingBoxVisitor.h>

#include <iostream>
#include <cstring>

#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osgDB/ReadFile>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

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
    _masterID = -1;
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
    _clientInitMap.clear();

    ClientInitInfo cii;

    if(ComController::instance()->isMaster())
    {
        if(_thread->isRunning())
        {
            disconnect();
        }

        //gethostname(cii.name, 254);
        cii.name[255] = '\0';
        strncpy(cii.name,CalVR::instance()->getHostName().c_str(),254);
        cii.numHeads = TrackingManager::instance()->getNumHeads();
        cii.numHands = TrackingManager::instance()->getNumHands();

        if(_socket)
        {
            delete _socket;
            _socket = NULL;
        }

        _socket = new CVRSocket(CONNECT,host,port);

        _socket->setBlocking(false);
        _socket->setNoDelay(true);

        if(_socket->connect(5))
        {
            std::cerr << "Connected to Collaborative Server: " << host
                    << " Port: " << port << std::endl;
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

            _socket->setBlocking(true);

            std::cerr << "Sending hostname: " << cii.name << " NumHeads: "
                    << cii.numHeads << " NumHands: " << cii.numHands
                    << std::endl;

            if(!_socket->send(&cii,sizeof(struct ClientInitInfo),MSG_NOSIGNAL))
            {
                res = false;
            }

            ServerInitInfo sii;
            if(!_socket->recv(&sii,sizeof(struct ServerInitInfo)))
            {
                res = false;
            }

            std::cerr << "There are " << sii.numUsers
                    << " other users connected to collab server." << std::endl;
            numUsers = sii.numUsers;

            if(res && sii.numUsers)
            {
                //std::cerr << "Getting cii from " << sii.numUsers << " users." << std::endl;
                ciiList = new ClientInitInfo[sii.numUsers];
                if(!_socket->recv(ciiList,
                        sizeof(struct ClientInitInfo) * sii.numUsers))
                {
                    res = false;
                }
            }

            _thread->init(_socket,id);
            _thread->start();

            //startUpdate();
        }
        else
        {
            std::cerr << "Unable to connect to Collaborative Server: " << host
                    << " Port: " << port << std::endl;
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
            ComController::instance()->sendSlaves(&cii,
                    sizeof(struct ClientInitInfo));
            ComController::instance()->sendSlaves(&id,sizeof(int));
            ComController::instance()->sendSlaves(&numUsers,sizeof(int));
            if(numUsers)
            {
                ComController::instance()->sendSlaves(ciiList,
                        sizeof(struct ClientInitInfo) * numUsers);
            }
        }
        else
        {
            ComController::instance()->readMaster(&cii,
                    sizeof(struct ClientInitInfo));
            ComController::instance()->readMaster(&id,sizeof(int));
            ComController::instance()->readMaster(&numUsers,sizeof(int));
            if(numUsers)
            {
                ciiList = new ClientInitInfo[numUsers];
                ComController::instance()->readMaster(ciiList,
                        sizeof(struct ClientInitInfo) * numUsers);
            }
        }
        _id = id;
        _myName = cii.name;

        //_clientInitMap[_id] = cii;

        //std::cerr << "Init with " << numUsers << " users." << std::endl;

        for(int i = 0; i < numUsers; i++)
        {
            _clientInitMap[ciiList[i].id] = ciiList[i];
            std::cerr << "Client id: " << ciiList[i].id << " name: "
                    << ciiList[i].name << " numHeads: " << ciiList[i].numHeads
                    << " numHands: " << ciiList[i].numHands << std::endl;
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
                _collabHeads[ciiList[i].id] = std::vector<
                        osg::ref_ptr<osg::MatrixTransform> >();
                for(int j = 0; j < ciiList[i].numHeads; j++)
                {
                    _headBodyMap[ciiList[i].id].push_back(bu);
                    _collabHeads[ciiList[i].id].push_back(
                            new osg::MatrixTransform());
                    _collabHeads[ciiList[i].id][j]->addChild(
                            makeHead(ciiList[i].id));
                }
            }

            if(ciiList[i].numHands)
            {
                _handBodyMap[ciiList[i].id] = std::vector<BodyUpdate>();
                _collabHands[ciiList[i].id] = std::vector<
                        osg::ref_ptr<osg::MatrixTransform> >();
                for(int j = 0; j < ciiList[i].numHands; j++)
                {
                    _handBodyMap[ciiList[i].id].push_back(bu);
                    _collabHands[ciiList[i].id].push_back(
                            new osg::MatrixTransform());
                    _collabHands[ciiList[i].id][j]->addChild(
                            makeHand(ciiList[i].id));
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
    _clientInitMap.clear();
    _collabRoot->removeChildren(0,_collabRoot->getNumChildren());
    _collabHands.clear();
    _collabHeads.clear();
    _handBodyMap.clear();
    _headBodyMap.clear();
    _connected = false;
}

void CollaborativeManager::startUpdate()
{
    if(!ComController::instance()->isMaster() || !_thread
            || !_thread->isRunning())
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
            cu.objTrans[(4 * i) + j] = m(i,j);
        }
    }

    int numBodies = TrackingManager::instance()->getNumHeads()
            + TrackingManager::instance()->getNumHands();
    BodyUpdate * bodies = NULL;

    if(numBodies)
    {
        bodies = new BodyUpdate[numBodies];
        int bindex = 0;
        for(int i = 0; i < TrackingManager::instance()->getNumHeads(); i++)
        {
            osg::Vec3 pos =
                    TrackingManager::instance()->getHeadMat(i).getTrans();
            osg::Quat rot =
                    TrackingManager::instance()->getHeadMat(i).getRotate();
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
            osg::Vec3 pos =
                    TrackingManager::instance()->getHandMat(i).getTrans();
            osg::Quat rot =
                    TrackingManager::instance()->getHandMat(i).getRotate();
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

    CollaborativeMessageHeader * mheaders = NULL;
    char ** mData = NULL;

    cu.numMes = _messageQueue.size();
    if(cu.numMes)
    {
        mheaders = new CollaborativeMessageHeader[cu.numMes];
        mData = new char*[cu.numMes];
        for(int i = 0; i < cu.numMes; i++)
        {
            mheaders[i] = _messageQueue.front().first;
            mData[i] = _messageQueue.front().second;
            _messageQueue.pop();
        }
    }

    _thread->startUpdate(cu,numBodies,bodies,cu.numMes,mheaders,mData);
}

void CollaborativeManager::update()
{
    double startTime, endTime;

    osg::Stats * stats;
    stats = CVRViewer::instance()->getViewerStats();
    if(stats && !stats->collectStats("CalVRStatsAdvanced"))
    {
        stats = NULL;
    }

    if(stats)
    {
        startTime = osg::Timer::instance()->delta_s(
                CVRViewer::instance()->getStartTick(),
                osg::Timer::instance()->tick());
    }

    if(!_connected || ComController::instance()->getIsSyncError())
    {
        if(stats)
        {
            endTime = osg::Timer::instance()->delta_s(
                    CVRViewer::instance()->getStartTick(),
                    osg::Timer::instance()->tick());
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative begin time",startTime);
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative end time",endTime);
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative time taken",endTime - startTime);
        }
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

        if(stats)
        {
            endTime = osg::Timer::instance()->delta_s(
                    CVRViewer::instance()->getStartTick(),
                    osg::Timer::instance()->tick());
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative begin time",startTime);
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative end time",endTime);
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative time taken",endTime - startTime);
        }
        return;
    }

    if(!toUpdate[0])
    {
        if(stats)
        {
            endTime = osg::Timer::instance()->delta_s(
                    CVRViewer::instance()->getStartTick(),
                    osg::Timer::instance()->tick());
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative begin time",startTime);
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative end time",endTime);
            stats->setAttribute(
                    CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                    "Collaborative time taken",endTime - startTime);
        }
        return;
    }

    struct ServerUpdate su;
    struct ClientUpdate * culist = NULL;
    struct BodyUpdate * bodies = NULL;
    struct CollaborativeMessageHeader * cmh = NULL;
    char ** messageData = NULL;

    if(ComController::instance()->isMaster())
    {
        ServerUpdate * sup;
        _thread->getUpdate(sup,culist,bodies,cmh,messageData);
        su = *sup;

        ComController::instance()->sendSlaves(&su,sizeof(struct ServerUpdate));
    }
    else
    {
        ComController::instance()->readMaster(&su,sizeof(struct ServerUpdate));
    }

    if(su.numMes)
    {
        if(ComController::instance()->isMaster())
        {
            ComController::instance()->sendSlaves(cmh,
                    sizeof(struct CollaborativeMessageHeader) * su.numMes);
            for(int i = 0; i < su.numMes; i++)
            {
                if(cmh[i].size)
                {
                    ComController::instance()->sendSlaves(messageData[i],
                            cmh[i].size);
                }
            }
        }
        else
        {
            cmh = new CollaborativeMessageHeader[su.numMes];
            ComController::instance()->readMaster(cmh,
                    sizeof(struct CollaborativeMessageHeader) * su.numMes);
            messageData = new char*[su.numMes];
            for(int i = 0; i < su.numMes; i++)
            {
                if(cmh[i].size)
                {
                    messageData[i] = new char[cmh[i].size];
                    ComController::instance()->readMaster(messageData[i],
                            cmh[i].size);
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

    _mode = su.mode;
    _masterID = su.masterID;

    if(su.mode == LOCKED)
    {
        SceneManager::instance()->getObjectsRoot()->removeChild(
                _collabRoot.get());
        if(_masterID >= 0 && _masterID != _id)
        {
            int numBodies;
            if(ComController::instance()->isMaster())
            {
                ComController::instance()->sendSlaves(culist,
                        sizeof(struct ClientUpdate));
                _clientMap[culist[0].numMes] = culist[0];
                numBodies = _clientInitMap[culist[0].numMes].numHeads
                        + _clientInitMap[culist[0].numMes].numHands;
                ComController::instance()->sendSlaves(bodies,
                        sizeof(struct BodyUpdate) * numBodies);
            }
            else
            {
                culist = new ClientUpdate[1];
                ComController::instance()->readMaster(culist,
                        sizeof(struct ClientUpdate));
                _clientMap[culist[0].numMes] = culist[0];
                numBodies = _clientInitMap[culist[0].numMes].numHeads
                        + _clientInitMap[culist[0].numMes].numHands;
                bodies = new BodyUpdate[numBodies];
                ComController::instance()->readMaster(bodies,
                        sizeof(struct BodyUpdate) * numBodies);
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
    else
    {
        if(_collabRoot->getNumParents() == 0)
        {
            SceneManager::instance()->getObjectsRoot()->addChild(
                    _collabRoot.get());
        }

        _clientMap.clear();

        if(su.numUsers > 1)
        {
            int numBodies;
            if(ComController::instance()->isMaster())
            {
                ComController::instance()->sendSlaves(culist,
                        sizeof(struct ClientUpdate) * (su.numUsers - 1));
                numBodies = 0;

                for(int i = 0; i < su.numUsers - 1; i++)
                {
                    numBodies += _clientInitMap[culist[i].numMes].numHeads
                            + _clientInitMap[culist[i].numMes].numHands;
                }

                if(numBodies)
                {
                    ComController::instance()->sendSlaves(bodies,
                            sizeof(struct BodyUpdate) * numBodies);
                }
            }
            else
            {
                culist = new struct ClientUpdate[su.numUsers - 1];
                ComController::instance()->readMaster(culist,
                        sizeof(struct ClientUpdate) * (su.numUsers - 1));

                numBodies = 0;

                for(int i = 0; i < su.numUsers - 1; i++)
                {
                    numBodies += _clientInitMap[culist[i].numMes].numHeads
                            + _clientInitMap[culist[i].numMes].numHands;
                }

                if(numBodies)
                {
                    bodies = new BodyUpdate[numBodies];
                    ComController::instance()->readMaster(bodies,
                            sizeof(struct BodyUpdate) * numBodies);
                }
            }

            int bindex = 0;

            for(int i = 0; i < su.numUsers - 1; i++)
            {
                _clientMap[culist[i].numMes] = culist[i];

                for(int j = 0; j < _clientInitMap[culist[i].numMes].numHeads;
                        j++)
                {
                    _headBodyMap[culist[i].numMes][j] = bodies[bindex];
                    bindex++;
                }

                for(int j = 0; j < _clientInitMap[culist[i].numMes].numHands;
                        j++)
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

    updateCollabNodes();

    startUpdate();

    if(stats)
    {
        endTime = osg::Timer::instance()->delta_s(
                CVRViewer::instance()->getStartTick(),
                osg::Timer::instance()->tick());
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "Collaborative begin time",startTime);
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "Collaborative end time",endTime);
        stats->setAttribute(
                CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(),
                "Collaborative time taken",endTime - startTime);
    }
}

void CollaborativeManager::setMode(CollabMode mode)
{
    if(ComController::instance()->isMaster())
    {
        CollaborativeMessageHeader cmh;
        cmh.type = SET_COLLAB_MODE;
        strcpy(cmh.target,"CollabServer");
        cmh.size = sizeof(CollabMode);
        cmh.deleteData = true;
        CollabMode * cmode = new CollabMode[1];
        cmode[0] = mode;

        _messageQueue.push(
                std::pair<CollaborativeMessageHeader,char*>(cmh,(char*)cmode));
    }
}

void CollaborativeManager::setMasterID(int id)
{
    if(ComController::instance()->isMaster())
    {
        CollaborativeMessageHeader cmh;
        cmh.type = SET_MASTER_ID;
        strcpy(cmh.target,"CollabServer");
        cmh.size = sizeof(int);
        cmh.deleteData = true;
        int * mid = new int[1];
        mid[0] = id;

        _messageQueue.push(
                std::pair<CollaborativeMessageHeader,char*>(cmh,(char*)mid));
    }
}

int CollaborativeManager::getClientNumHeads(int id)
{
    std::map<int,ClientInitInfo>::iterator it;
    if((it = _clientInitMap.find(id)) != _clientInitMap.end())
    {
        return it->second.numHeads;
    }
    return 0;
}

int CollaborativeManager::getClientNumHands(int id)
{
    std::map<int,ClientInitInfo>::iterator it;
    if((it = _clientInitMap.find(id)) != _clientInitMap.end())
    {
        return it->second.numHands;
    }
    return 0;
}

const osg::Matrix & CollaborativeManager::getClientHeadMat(int id, int head)
{
    static osg::Matrix defaultReturn;

    std::map<int,std::vector<osg::ref_ptr<osg::MatrixTransform> > >::iterator it;
    if((it = _collabHeads.find(id)) != _collabHeads.end())
    {
        if(head >= 0 && head < it->second.size())
        {
            return it->second[head]->getMatrix();
        }
    }

    return defaultReturn;
}

const osg::Matrix & CollaborativeManager::getClientHandMat(int id, int hand)
{
    static osg::Matrix defaultReturn;

    std::map<int,std::vector<osg::ref_ptr<osg::MatrixTransform> > >::iterator it;
    if((it = _collabHands.find(id)) != _collabHands.end())
    {
        if(hand >= 0 && hand < it->second.size())
        {
            return it->second[hand]->getMatrix();
        }
    }

    return defaultReturn;
}

void CollaborativeManager::sendCollaborativeMessageAsync(std::string target,
        int type, char * data, int size, bool sendLocal)
{
    if(ComController::instance()->isMaster())
    {
        CollaborativeMessageHeader cmh;
        cmh.type = PLUGIN_MESSAGE;
        cmh.pluginMessageType = type;
        cmh.target[255] = '\0';
        strncpy(cmh.target,target.c_str(),255);
        cmh.size = size;
        cmh.deleteData = true;

        _messageQueue.push(
                std::pair<CollaborativeMessageHeader,char *>(cmh,data));
    }

    if(sendLocal)
    {
        PluginManager::instance()->sendMessageByName(target,type,data);
    }
}

void CollaborativeManager::sendCollaborativeMessageSync(std::string target,
        int type, char * data, int size, bool sendLocal)
{
    if(_connected && !ComController::instance()->getIsSyncError())
    {
        bool done = true;
        if(ComController::instance()->isMaster())
        {
            CollaborativeMessageHeader cmh;
            cmh.type = PLUGIN_MESSAGE;
            cmh.pluginMessageType = type;
            cmh.target[255] = '\0';
            strncpy(cmh.target,target.c_str(),255);
            cmh.size = size;
            cmh.deleteData = false;

            _messageQueue.push(
                    std::pair<CollaborativeMessageHeader,char *>(cmh,data));

            while(_thread->isRunning() && !_thread->updateDone())
            {
                //TODO:nanosleep
            }

            update();

            while(_thread->isRunning() && !_thread->updateDone())
            {
                //TODO:nanosleep
            }

            ComController::instance()->sendSlaves(&done,sizeof(bool));
        }
        else
        {
            update();
            ComController::instance()->readMaster(&done,sizeof(bool));
        }
    }

    if(sendLocal)
    {
        PluginManager::instance()->sendMessageByName(target,type,data);
    }
}

void CollaborativeManager::updateCollabNodes()
{
    _collabRoot->removeChildren(0,_collabRoot->getNumChildren());

    if(_mode == UNLOCKED)
    {
        for(std::map<int,ClientInitInfo>::iterator it = _clientInitMap.begin();
                it != _clientInitMap.end(); it++)
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
                    objMatInv(i,j) =
                            _clientMap[it->first].objTrans[(4 * i) + j];
                }
            }

            objMatInv = osg::Matrix::inverse(objMatInv);

            osg::Matrix objScaleInv;
            objScaleInv.makeScale(
                    osg::Vec3(1.0 / _clientMap[it->first].objScale,
                            1.0 / _clientMap[it->first].objScale,
                            1.0 / _clientMap[it->first].objScale));

            osg::Matrix m;
            for(int i = 0; i < it->second.numHeads; i++)
            {
                BodyUpdate * bu = &_headBodyMap[it->first][i];
                osg::Vec3 pos(bu->pos[0],bu->pos[1],bu->pos[2]);
                osg::Quat quat(bu->rot[0],bu->rot[1],bu->rot[2],bu->rot[3]);
                m.makeRotate(quat);
                m.setTrans(pos);

                _collabHeads[it->first][i]->setMatrix(
                        m * objMatInv * objScaleInv);

                _collabRoot->addChild(_collabHeads[it->first][i].get());
            }

            for(int i = 0; i < it->second.numHands; i++)
            {
                BodyUpdate * bu = &_handBodyMap[it->first][i];
                osg::Vec3 pos(bu->pos[0],bu->pos[1],bu->pos[2]);
                osg::Quat quat(bu->rot[0],bu->rot[1],bu->rot[2],bu->rot[3]);
                m.makeRotate(quat);
                m.setTrans(pos);

                _collabHands[it->first][i]->setMatrix(
                        m * objMatInv * objScaleInv);

                _collabRoot->addChild(_collabHands[it->first][i].get());
            }
        }
    }
    else
    {
        if(_masterID >= 0 && _masterID != _id)
        {
            osg::Matrix objMat;
            for(int i = 0; i < 4; i++)
            {
                for(int j = 0; j < 4; j++)
                {
                    objMat(i,j) = _clientMap[_masterID].objTrans[(4 * i) + j];
                }
            }

            SceneManager::instance()->setObjectMatrix(objMat);
            SceneManager::instance()->setObjectScale(
                    _clientMap[_masterID].objScale);
        }
    }
}

osg::Node * CollaborativeManager::makeHand(int num)
{
    osg::Cone * cone = new osg::Cone(osg::Vec3(0,500,0),10,2000);
    osg::Quat q = osg::Quat(-M_PI / 2.0,osg::Vec3(1.0,0,0));
    cone->setRotation(q);
    osg::ShapeDrawable * sd = new osg::ShapeDrawable(cone);
    osg::Geode * geode = new osg::Geode();
    geode->addDrawable(sd);
    return geode;
}

osg::Node * CollaborativeManager::makeHead(int num)
{
    if(!_headModelNode)
    {
        std::string headGraphic = ConfigManager::getEntry("type",
                "Collaborative.HeadGraphic","CONE");

        if(headGraphic == "MODEL")
        {
            std::string modelFile = ConfigManager::getEntry("file",
                    "Collaborative.HeadGraphic","");

            osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(modelFile);
            if(model)
            {
                float scale = 1.0;
                osg::Vec3 center;
                bool autoAdjust = ConfigManager::getBool("autoAdjust",
                        "Collaborative.HeadGraphic",true);
                if(autoAdjust)
                {
                    ComputeBoundingBoxVisitor cbb;
                    model->accept(cbb);
                    osg::BoundingBox bb = cbb.getBound();
                    center.x() = bb.xMin() + (bb.xMax() - bb.xMin()) * 0.5;
                    center.y() = bb.yMin() + (bb.yMax() - bb.yMin()) * 0.5;
                    center.z() = bb.zMin() + (bb.zMax() - bb.zMin()) * 0.5;
#ifndef WIN32
                    scale = std::min(250.0 / (bb.xMax() - bb.xMin()),
                            250.0 / (bb.yMax() - bb.yMin()));
                    scale = std::min((double)scale,
                            250.0 / (bb.zMax() - bb.zMin()));
#else
                    scale = min(250.0 / (bb.xMax() - bb.xMin()), 250.0 / (bb.yMax() - bb.yMin()));
                    scale = min((double)scale,250.0 / (bb.zMax() - bb.zMin()));
#endif
                }
                else
                {
                    scale = ConfigManager::getFloat("scale",
                            "Collaborative.HeadGraphic",1.0);
                    center = ConfigManager::getVec3(
                            "Collaborative.HeadGraphic");
                }

                osg::Matrix offsetScale;
                offsetScale.makeTranslate(-center);
                offsetScale = offsetScale
                        * osg::Matrix::scale(osg::Vec3(scale,scale,scale));

                static const float DEG_2_RAD = M_PI / 180.0;
                float h, p, r;
                h = ConfigManager::getFloat("h","Collaborative.HeadGraphic",
                        0.0);
                p = ConfigManager::getFloat("p","Collaborative.HeadGraphic",
                        0.0);
                r = ConfigManager::getFloat("r","Collaborative.HeadGraphic",
                        0.0);

                osg::MatrixTransform * mt = new osg::MatrixTransform();
                osg::Matrix m;
                m.makeRotate(r * DEG_2_RAD,osg::Vec3(0,1,0),p * DEG_2_RAD,
                        osg::Vec3(1,0,0),h * DEG_2_RAD,osg::Vec3(0,0,1));
                mt->setMatrix(offsetScale * m);
                mt->addChild(model);
                _headModelNode = mt;
            }
        }

        if(!_headModelNode)
        {
            osg::Cone * cone = new osg::Cone(osg::Vec3(0,0,0),50,200);
            osg::Quat q = osg::Quat(-M_PI / 2.0,osg::Vec3(1.0,0,0));
            cone->setRotation(q);
            osg::ShapeDrawable * sd = new osg::ShapeDrawable(cone);
            osg::Geode * geode = new osg::Geode();
            geode->addDrawable(sd);
            _headModelNode = geode;
        }
    }
    return _headModelNode.get();
}

void CollaborativeManager::processMessage(CollaborativeMessageHeader & cmh,
        char * data)
{
    switch(cmh.type)
    {
        case ADD_CLIENT:
        {
            ClientInitInfo * cii = (ClientInitInfo*)data;
            std::cerr << "Adding client id: " << cii->id << " name: "
                    << cii->name << " NumHeads: " << cii->numHeads
                    << " NumHands: " << cii->numHands << std::endl;
            if(!ComController::instance()->isMaster())
            {
                _clientInitMap[cii->id] = *cii;
            }

            BodyUpdate bu;
            bu.pos[0] = 0;
            bu.pos[1] = 0;
            bu.pos[2] = 0;
            bu.rot[0] = 0;
            bu.rot[1] = 0;
            bu.rot[2] = 0;
            bu.rot[3] = 1.0;

            _headBodyMap[cii->id] = std::vector<BodyUpdate>();
            _collabHeads[cii->id] = std::vector<
                    osg::ref_ptr<osg::MatrixTransform> >();
            if(cii->numHeads)
            {
                for(int j = 0; j < cii->numHeads; j++)
                {
                    _headBodyMap[cii->id].push_back(bu);
                    _collabHeads[cii->id].push_back(new osg::MatrixTransform());
                    _collabHeads[cii->id][j]->addChild(makeHead(cii->id));
                }
            }

            _handBodyMap[cii->id] = std::vector<BodyUpdate>();
            _collabHands[cii->id] = std::vector<
                    osg::ref_ptr<osg::MatrixTransform> >();
            if(cii->numHands)
            {
                for(int j = 0; j < cii->numHands; j++)
                {
                    _handBodyMap[cii->id].push_back(bu);
                    _collabHands[cii->id].push_back(new osg::MatrixTransform());
                    _collabHands[cii->id][j]->addChild(makeHand(cii->id));
                }
            }
            break;
        }
        case REMOVE_CLIENT:
        {
            //std::cerr << "Remove client bodies." << std::endl;
            int id = *((int*)data);
            std::cerr << "Removing client with id: " << id << std::endl;

            if(!ComController::instance()->isMaster())
            {
                _clientInitMap.erase(id);
            }

            _headBodyMap.erase(id);
            for(int i = 0; i < _collabHeads[id].size(); i++)
            {
                if(_collabHeads[id][i]->getNumParents())
                {
                    _collabHeads[id][i]->getParent(0)->removeChild(
                            _collabHeads[id][i]);
                }
            }
            _collabHeads.erase(id);

            for(int i = 0; i < _collabHands[id].size(); i++)
            {
                if(_collabHands[id][i]->getNumParents())
                {
                    _collabHands[id][i]->getParent(0)->removeChild(
                            _collabHands[id][i]);
                }
            }
            _collabHands.erase(id);
            break;
        }
        case PLUGIN_MESSAGE:
        {
            CVRPlugin * plugin = PluginManager::instance()->getPlugin(
                    cmh.target);
            if(plugin)
            {
                plugin->message(cmh.pluginMessageType,data,true);
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
