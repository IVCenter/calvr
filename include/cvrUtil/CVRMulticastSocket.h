/**
 * @file CVRMulticastSocket.h
 */
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

/**
 * @addtogroup util
 * @{
 */

/**
 * @brief Creates and manages a multicast socket
 */
class CVRMulticastSocket
{
    public:
        /**
         * @brief Operation the multicast socket will do
         */
        enum MCSocketType
        {
            SEND = 0,
            RECV
        };

        /**
         * @brief Constructor
         * @param st type for this socket
         * @param groupAddress ip address of the desired multicast group
         * @param port port to use for communication
         */
        CVRMulticastSocket(MCSocketType st, std::string groupAddress, int port);
        virtual ~CVRMulticastSocket();

        /**
         * @brief Set the interface to use for sending multicast
         * @param iface ip address or resolvable hostname of interface
         *
         * This function only has an effect if the socket is a SEND type
         */
        void setMulticastInterface(std::string iface);

        /**
         * @brief Wrapper to the setsockopt function
         */
        int setsockopt(int level, int optname, void * val, socklen_t len);

        /**
         * @brief Send data out the socket
         * @param buf data to send
         * @param len length of the data
         * @param flags flags for the sendto() call
         *
         * This function only has an effect if the socket is a SEND type
         */
        bool send(void * buf, size_t len, int flags = 0);

        /**
         * @brief Receive data from the socket
         * @param buf buffer to write data to
         * @param len length of data to receive
         * @param flags flags for the recvfrom() call
         */
        bool recv(void * buf, size_t len, int flags = 0);

        /**
         * @brief Set the socket descriptor
         */
        void setSocketFD(int socket);

        /**
         * @brief Get the socket descriptor
         */
        int getSocketFD();

        /**
         * @brief Returns if the socket descriptor is valid
         */
        bool valid();

    protected:
        int _socket; ///< socket descriptor
        MCSocketType _type; ///< type for this socket

        std::string _groupAddress; ///< multicast group address
        int _port; ///< multicast port
        struct sockaddr_in _address;
        socklen_t _addrlen;
};

/**
 * @}
 */

}

#endif
