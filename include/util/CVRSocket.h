/**
 * @file CVRSocket.h
 */

#ifndef CVR_SOCKET_H
#define CVR_SOCKET_H

#include <string>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#else
#include <WS2tcpip.h>
#endif

namespace cvr
{

/**
 * @brief Will this socket connect to another, or listen for a connection
 */
enum SocketType
{
    LISTEN,
    CONNECT
};

/**
 * @brief Cross platform (windows not yet tested) socket class
 */
class CVRSocket
{
    public:
        CVRSocket(int socket);
        CVRSocket(SocketType type, std::string host, int port, int family =
                AF_INET, int sockType = SOCK_STREAM);
        ~CVRSocket();

        bool bind();
        bool listen(int backlog = 5);
        bool accept();
        bool connect(int timeout = 0);

        int setsockopt(int level, int optname, void * val, socklen_t len);

        void setNoDelay(bool b);
        void setReuseAddress(bool b);
        void setBlocking(bool b);

        bool send(void * buf, size_t len, int flags = 0);
        bool recv(void * buf, size_t len, int flags = 0);

        bool valid();

        void setSocketFD(int socket);
        int getSocketFD();

    protected:
        int _socket;
        SocketType _type;
        int _family;
        int _sockType;
        std::string _host;
        int _port;
        struct addrinfo * _res;

        fd_set _connectTest;
        bool _blockingState;
        bool _printErrors;
};

}

#endif

