#ifndef CALVR_MULTI_LISTEN_SOCKET_H
#define CALVR_MULTI_LISTEN_SOCKET_H

#include <util/Export.h>
#include <util/CVRSocket.h>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#else
#include <WS2tcpip.h>
#endif

namespace cvr
{

class CVRUTIL_EXPORT MultiListenSocket
{
    public:
        MultiListenSocket(int port, int queue = 5);
        ~MultiListenSocket();

        bool setup();

        CVRSocket * accept();

    protected:
        int _port;
        int _queue;

        int _socket;
        bool _valid;
};

}

#endif
