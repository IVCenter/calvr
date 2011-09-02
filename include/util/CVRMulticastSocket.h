#ifndef CVR_MULTICAST_SOCKET_H
#define CVR_MULTICAST_SOCKET_H

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#else
#include <WS2tcpip.h>
#endif

#include <string>

namespace cvr
{

class CVRMulticastSocket
{
    public:
        enum MCSocketType
        {
            SEND = 0,
            RECV
        };

        CVRMulticastSocket(MCSocketType st, std::string groupAddress, int port);
        virtual ~CVRMulticastSocket();

        void setMulticastInterface(std::string interface);

        int setsockopt(int level, int optname, void * val, socklen_t len);

        bool send(void * buf, size_t len, int flags = 0);
        bool recv(void * buf, size_t len, int flags = 0);

        void setSocketFD(int socket);
        int getSocketFD();

        bool valid();

    protected:
        int _socket;
        MCSocketType _type;

        std::string _groupAddress;
        int _port;
        struct sockaddr_in _address;
        socklen_t _addrlen;
};

}

#endif
