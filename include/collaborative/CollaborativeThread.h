#ifndef CVR_COLLABORATIVE_THREAD_H
#define CVR_COLLABORATIVE_THREAD_H

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <collaborative/CollaborativeManager.h>
#include <util/CVRSocket.h>

#ifdef __APPLE__
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

namespace cvr
{

class CollaborativeThread : public OpenThreads::Thread
{
    public:
        CollaborativeThread(std::map<int,struct ClientInitInfo> * clientInitMap);
        ~CollaborativeThread();

        void init(cvr::CVRSocket * socket, int id);

        virtual void run();

        bool isConnected();
        cvr::CVRSocket * getSocket();
        void setSocket(cvr::CVRSocket * socket);

        void startUpdate(struct cvr::ClientUpdate & cu, int numBodies, struct cvr::BodyUpdate * bodies, int numMessages, CollaborativeMessageHeader * messageHeaders, char** messageData);
        bool updateDone();

        void quit();

        //TODO add message stuff
        void getUpdate(cvr::ServerUpdate * & su,
                       cvr::ClientUpdate * & clientlist, BodyUpdate * & bodyList, CollaborativeMessageHeader * & messageHeaders, char ** messageData);

    protected:
        bool _connected;
        bool _updating;
        bool _updateDone;
        bool _quit;

        int _id;

        struct ClientUpdate _myInfo;

        int _numBodies;
        struct BodyUpdate * _myTrackedBodies;

        int _numMessages;
        CollaborativeMessageHeader * _messageHeaders;
        char ** _messageData;

        std::map<int,struct ClientInitInfo> * _clientInitMap;

        struct ServerUpdate * _serverUpdate;
        struct ClientUpdate * _clientUpdate;
        struct BodyUpdate * _bodiesUpdate;
        struct CollaborativeMessageHeader * _messageHeaderUpdate;
        char ** _messageDataUpdate;

        cvr::CVRSocket * _socket;

        OpenThreads::Mutex _quitLock;
        OpenThreads::Mutex _statusLock;
};

}

#endif
