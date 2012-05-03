#ifndef CALVR_COLLABORATIVE_SERVER_H
#define CALVR_COLLABORATIVE_SERVER_H

#include <cvrCollaborative/CollaborativeManager.h>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#include <map>
#include <queue>
#include <string>
#include <vector>
#include <csignal>
#include <cstring>
#include <time.h>
#include <sys/types.h>

namespace cvr
{

class CollaborativeMessage;
class SocketThread;
class MultiListenSocket;
class CVRSocket;

class CollaborativeServer
{
        friend class SocketThread;
    public:
        CollaborativeServer(int port);
        virtual ~CollaborativeServer();

        bool init();

        void run();

    protected:
        void initConnection(int id, CVRSocket * sock);
        void checkSockets();

        std::map<int,struct ClientUpdate> _clientMap;
        std::map<int,struct ClientInitInfo> _clientInitMap;
        std::map<int,std::vector<struct BodyUpdate> > _clientHandList;
        std::map<int,std::vector<struct BodyUpdate> > _clientHeadList;
        std::vector<SocketThread *> _threadList;
        CollabMode _currentMode;
        int _masterID;
        int _port;
        OpenThreads::Mutex _serverLock;
        OpenThreads::Mutex _messageAddLock;
        MultiListenSocket * _listenSocket;
};

class SocketThread : public OpenThreads::Thread
{
    public:
        SocketThread(cvr::CVRSocket * socket, std::string name, int id,
                CollaborativeServer * server);
        virtual ~SocketThread();

        virtual void run();

        int getID();
        std::string getName();
        void quit();

        void addMessage(CollaborativeMessage * message);

    protected:
        bool checkSocket();
        bool processEvents();
        bool processMessage(CollaborativeMessageHeader & cmh);

        void queueCleanup(std::queue<CollaborativeMessage *> & queue);

        CollaborativeServer * _server;

        std::queue<CollaborativeMessage *> _messageQueue;
        OpenThreads::Mutex _messageLock;
        OpenThreads::Mutex _quitLock;

        cvr::CVRSocket * _socket;
        std::string _name;
        int _id;

        bool _quit;
};

class CollaborativeMessage
{
    public:
        CollaborativeMessage(int refs, int type, std::string target, int size,
                char * data);
        CollaborativeMessage(int refs, cvr::CollaborativeMessageHeader & cmh,
                char * data);
        virtual ~CollaborativeMessage();

        cvr::CollaborativeMessageHeader & getHeader();
        char * getData();

        void unref();

    protected:
        OpenThreads::Mutex _refMutex;

        int _refs;
        cvr::CollaborativeMessageHeader _header;
        char * _data;
};

}

#endif
