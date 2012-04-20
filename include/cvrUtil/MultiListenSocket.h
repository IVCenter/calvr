/**
 * @file MultiListenSocket.h
 */
#ifndef CALVR_MULTI_LISTEN_SOCKET_H
#define CALVR_MULTI_LISTEN_SOCKET_H

#include <cvrUtil/Export.h>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#else
#include <WS2tcpip.h>
#endif

namespace cvr
{

class CVRSocket;

/**
 * @brief Sets up a TCP socket to listen for multiple incomming connections
 */
class CVRUTIL_EXPORT MultiListenSocket
{
    public:
        /**
         * @brief Constructor
         * @param port port to listen on for connections
         * @param queue listen queue size
         */
        MultiListenSocket(int port, int queue = 5);
        ~MultiListenSocket();

        /**
         * @brief Creates the socket, binds, and starts listening for connections
         * @return returns false if there is an error
         */
        bool setup();

        /**
         * @brief If there is a socket connection waiting, it is returned as a CVRSocket
         * @return pointer to CVRSocket connection or NULL is no connection was waiting
         */
        CVRSocket * accept();

    protected:
        int _port; ///< port to listen on
        int _queue; ///< size of listen queue

        int _socket; ///< socket descriptor
        bool _valid; ///< is the descriptor valid
};

}

#endif
