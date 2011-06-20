#ifndef CVR_COLLABORATIVE_THREAD_H
#define CVR_COLLABORATIVE_THREAD_H

#include <OpenThreads/Thread>
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
        CollaborativeThread();
        ~CollaborativeThread();

        void init(cvr::CVRSocket * socket, int id);

        virtual void run();

        bool isConnected();
        cvr::CVRSocket * getSocket();
        void setSocket(cvr::CVRSocket * socket);

        void startUpdate(struct cvr::ClientUpdate & cu);
        bool updateDone();

        void quit();

        //TODO add message stuff
        void getUpdate(cvr::ServerUpdate & su,
                       cvr::ClientUpdate * & clientlist);

    protected:
        bool _connected;
        bool _updating;
        bool _updateDone;
        bool _quit;

        int _id;

        struct ClientUpdate _myInfo;
        struct ServerUpdate _serverUpdate;
        struct ClientUpdate * _clientUpdate;

        cvr::CVRSocket * _socket;
};

}

#endif
