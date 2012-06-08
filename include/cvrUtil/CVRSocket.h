/**
 * @file CVRSocket.h
 */

#ifndef CVR_SOCKET_H
#define CVR_SOCKET_H

#include <cvrUtil/Export.h>

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
 * @addtogroup util
 * @{
 */

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
 *
 * Note: currenly only tcp is supported
 */
class CVRUTIL_EXPORT CVRSocket
{
    public:
        /**
         * @brief Constructor
         * @param socket socket descriptor to use instead of creating a connection
         */
        CVRSocket(int socket);

        /**
         * @brief Constructor
         * @param type socket type
         * @param host host to connect to or interface to bind to
         * @param port port to connect to or port to bind to
         * @param family socket family
         * @param sockType socket type
         */
        CVRSocket(SocketType type, std::string host, int port, int family =
                AF_INET, int sockType = SOCK_STREAM);
        ~CVRSocket();

        /**
         * @brief Bind to a port
         *
         * Only needed for a LISTEN type socket
         */
        bool bind();

        /**
         * @brief Listen for an incomming connection
         * @param backlog listen backlog queue size
         * 
         * Only needed for LISTEN type socket
         */
        bool listen(int backlog = 5);

        /**
         * @brief Accept an incomming connection
         *
         * Only needed for LISTEN type socket
         */
        bool accept();

        /**
         * @brief Connect to a remote host
         * @param timeout connect timeout in sec
         *
         * Only needed for CONNECT type socket
         */
        bool connect(int timeout = 0);

        /**
         * @brief Wrapper for setsockopt call
         */
        int setsockopt(int level, int optname, void * val, socklen_t len);

        /**
         * @brief Set the socket option to enable/disable nagle algorithm
         *
         * Algorithm can delay the sending of small packets
         */
        void setNoDelay(bool b);

        /**
         * @brief Set the socket option to enable/disable rebinding an address/port combo
         *
         * Will let you rebind even if an old socket has not fully closed
         */
        void setReuseAddress(bool b);

        /**
         * @brief Set the blocking state of the socket
         */
        void setBlocking(bool b);

        /**
         * @brief Send data through the socket
         * @param buf data to send
         * @param len length of the data
         * @param flags flags to use in the send call
         */
        bool send(void * buf, size_t len, int flags = 0);

        /**
         * @brief Receive data from the socket
         * @param buf buffer to store data in
         * @param len length of data to read from the socket
         * @param flags flags to use in the recv call
         */
        bool recv(void * buf, size_t len, int flags = 0);

        /**
         * @brief Returns if the socket descriptor is valid
         */
        bool valid();

        /**
         * @brief Set the descriptor for the socket
         */
        void setSocketFD(int socket);

        /**
         * @brief Get the descriptor for the socket
         */
        int getSocketFD();

    protected:
        int _socket; ///< socket descriptor
        SocketType _type; ///< socket connection type
        int _family; ///< socket family
        int _sockType; ///< socket type
        std::string _host; ///< remote/local interface
        int _port; ///< socket port
        struct addrinfo * _res;

        fd_set _connectTest;
        bool _blockingState; ///< socket current blocking state
        bool _printErrors; ///< should socket errors be printed
};

/**
 * @}
 */

}

#endif

