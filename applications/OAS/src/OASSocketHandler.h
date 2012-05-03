/**
 * @file    OASSocketHandler.h
 * @author  Shreenidhi Chowkwale
 */

#ifndef _OAS_SOCKET_HANDLER_H_
#define _OAS_SOCKET_HANDLER_H_

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <queue>
#include <pthread.h>
#include <AL/alut.h>
#include <cerrno>

#include "OASFileHandler.h"
#include "OASMessage.h"
#include "OASLogger.h"

#define MAX_MESSAGE_SIZE 256

#define MAX_CIRCULAR_BUFFER_SIZE 2048
#define MAX_TRANSMIT_BUFFER_SIZE 256
#define MAX_BINARY_READ_SIZE 512

namespace oas
{

/**
 * Manages socket traffic
 */
class SocketHandler
{
    public:
        static bool initialize(unsigned short listeningPort);
        static void terminate();
        static bool isSocketOpen();
        static unsigned int numberOfIncomingMessages();
        static void populateQueueWithIncomingMessages(std::queue<Message*> &destination, struct timespec timeout);
        static void addOutgoingResponse(const char *response);
        static void addOutgoingResponse(const long response);
        static pthread_t getSocketThread();

    protected:
        static struct sockaddr_in _stSockAddr;
        static int _socketHandle;
        static unsigned short _listeningPort;
        static pthread_t _socketThread;

        static std::queue<Message*> _incomingMessages;
        static pthread_mutex_t _inMutex;
        static pthread_cond_t _inCondition;
        static std::queue<char *> _outgoingResponses;
        static pthread_mutex_t _outMutex;
        static pthread_cond_t _outCondition;

        static bool _isSocketOpen;

    private:
        static bool _openSocket();
        static void _closeSocket();
        static int _acceptNewConnection();
        static void _closeConnection(const int connection);
        static void* _socketLoop(void* parameter);
        static void  _receiveBinaryFile(int connection, const Message& ptfi);
        static void  _addToIncomingMessages(Message *message);
        static char* _getNextOutgoingResponse();
        SocketHandler();
        ~SocketHandler();
}; // End class SocketHandler

} // End namespace oas

#endif

