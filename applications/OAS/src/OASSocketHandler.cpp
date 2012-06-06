/**
 * @file OASSocketHandler.cpp
 * @author Shreenidhi Chowkwale
 *
 */

#include "OASSocketHandler.h"

using namespace oas;

// Statics
struct sockaddr_in      SocketHandler::_stSockAddr;
int                     SocketHandler::_socketHandle;
unsigned short          SocketHandler::_listeningPort;
pthread_t               SocketHandler::_socketThread;
std::queue<Message*>    SocketHandler::_incomingMessages;
pthread_mutex_t         SocketHandler::_inMutex;
pthread_cond_t          SocketHandler::_inCondition;
std::queue<char *>      SocketHandler::_outgoingResponses;
pthread_mutex_t         SocketHandler::_outMutex;
pthread_cond_t          SocketHandler::_outCondition;
bool                    SocketHandler::_isSocketOpen;
bool                    SocketHandler::_isConnectedToClient = false;


// static, public
bool SocketHandler::initialize(unsigned short listeningPort)
{
    // If already initialized, close the socket so that it can be re-opened
    if (SocketHandler::isSocketOpen())
    {
        SocketHandler::_closeSocket();
    }

    SocketHandler::_listeningPort = listeningPort;

    // Initialize mutexes
    pthread_mutex_init(&SocketHandler::_inMutex, NULL);
    pthread_mutex_init(&SocketHandler::_outMutex, NULL);

    // Initialize condition variables
    pthread_cond_init(&SocketHandler::_inCondition, NULL);
    pthread_cond_init(&SocketHandler::_outCondition, NULL);

    // Thread attribute variable
    pthread_attr_t threadAttr;

    // Init thread attribute
    pthread_attr_init(&threadAttr);

    // Set joinable thread attribute
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

    // Spawn thread to run the loop that handles connections
    int threadError = pthread_create( &SocketHandler::_socketThread,
                                      &threadAttr,
                                      &SocketHandler::_socketLoop,
                                      NULL);

    // Destroy thread attribute
    pthread_attr_destroy(&threadAttr);

    // If error occured in making new thread...
    if (threadError)
    {
        oas::Logger::errorf("SocketHandler - Failed to create the socket thread.");
        close(SocketHandler::_socketHandle);
        return false;
    }

    Logger::logf("SocketHandler initialized...");
    return true;
}

// static, public
void SocketHandler::terminate()
{
    // End the socket processing thread
    pthread_cancel(SocketHandler::_socketThread);
    // Close the socket and clean up the associated data
    SocketHandler::_closeSocket();
}

// static, public
bool SocketHandler::isConnectedToClient()
{
    return _isConnectedToClient;
}

// static, private
bool SocketHandler::_openSocket()
{
    if (SocketHandler::isSocketOpen())
    {
        SocketHandler::_closeSocket();
    }
    
    SocketHandler::_isSocketOpen = false;

    // Create a new socket
    SocketHandler::_socketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (-1 == SocketHandler::_socketHandle)
    {
        oas::Logger::error("SocketHandler - Failed to create a new socket");
        return false;
    }

    int enableReuse = 1;

    // Set socket option SO_REUSEADDR - lets the server reuse the port if there is a disconnect
    if (-1 == setsockopt( SocketHandler::_socketHandle, 
                          SOL_SOCKET, 
                          SO_REUSEADDR, 
                          &enableReuse, 
                          sizeof(enableReuse)))
    {
        oas::Logger::error("SocketHandler - Failed to set the socket as reusable");
        close(SocketHandler::_socketHandle);
        return false;
    }

    // Set socket address data to 0
    memset(&SocketHandler::_stSockAddr, 0, sizeof(SocketHandler::_stSockAddr));

    // Set initial socket address data
    SocketHandler::_stSockAddr.sin_family = AF_INET;
    SocketHandler::_stSockAddr.sin_port = htons(SocketHandler::_listeningPort);
    SocketHandler::_stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket address data to handle
    if (-1 == bind( SocketHandler::_socketHandle, 
                    (struct sockaddr *) &SocketHandler::_stSockAddr, 
                    sizeof(SocketHandler::_stSockAddr)))
    {
        oas::Logger::error("SocketHandler - Failed to bind socket to address");
        close(SocketHandler::_socketHandle);
        return false;
    }
    
    // Prepare socket for listening
    if (-1 == listen(SocketHandler::_socketHandle, 1))
    {
        oas::Logger::error("SocketHandler - Failed to listen on socket");
        close(SocketHandler::_socketHandle);
        return false;
    }

    SocketHandler::_isSocketOpen = true;


    char buf[MAXHOSTNAMELEN], ipBuf[INET_ADDRSTRLEN];
    struct hostent *host;

    // Note: gethostbyname is deprecated due to lack of support for IPv6. Change if needed.
    if (-1 != gethostname(buf, MAXHOSTNAMELEN)
            && (NULL != (host = gethostbyname(buf)))
            && (NULL != inet_ntop(AF_INET, *host->h_addr_list, ipBuf, INET_ADDRSTRLEN)))
    {
        oas::Logger::logf("Audio Server \"%s\" (%s) is waiting for client to connect on port %d...",
                          buf, ipBuf, SocketHandler::_listeningPort);
    }
    else
    {
        oas::Logger::logf("Audio Server is listening on port %d...",
                               SocketHandler::_listeningPort);
    }

    return true;
}

// static, private
void SocketHandler::_closeSocket()
{
    if (!SocketHandler::isSocketOpen())
    {
        return;
    }

    close(SocketHandler::_socketHandle);
    SocketHandler::_isSocketOpen = false;

    // Empty the incoming and outgoing queues

    pthread_mutex_lock(&SocketHandler::_inMutex);
    while (!SocketHandler::_incomingMessages.empty())
    {
        delete SocketHandler::_incomingMessages.front();
        SocketHandler::_incomingMessages.pop();
    }
    pthread_mutex_unlock(&SocketHandler::_inMutex);

    pthread_mutex_lock(&SocketHandler::_outMutex);
    while (!SocketHandler::_outgoingResponses.empty())
    {
        delete SocketHandler::_outgoingResponses.front();
        SocketHandler::_outgoingResponses.pop();
    }
    pthread_mutex_unlock(&SocketHandler::_outMutex);
}

// static, private
int SocketHandler::_acceptNewConnection()
{
    if (!SocketHandler::isSocketOpen())
    {
        return 0;
    }

    struct sockaddr_in clientAddr;
    socklen_t addrSize;

    addrSize = sizeof (clientAddr);
    int connection = accept(SocketHandler::_socketHandle,
                            (struct sockaddr *) &clientAddr,
                            &addrSize);

    if (0 > connection)
    {
        oas::Logger::error("SocketHandler - Could not accept new connection");
    }
    else
    {
        char buf[INET_ADDRSTRLEN];
        const char *addrPtr = inet_ntop(AF_INET,
                                        &clientAddr.sin_addr,
                                        buf,
                                        INET_ADDRSTRLEN);

        oas::Logger::logf("A new client has connected from %s.",
                          addrPtr ? addrPtr : "(null)");
    }
    return connection;
}

// static, private
void SocketHandler::_closeConnection(const int connection)
{
    oas::Logger::logf("SocketHandler - Closing connection with client.");
    // Close the connection - causes client to disconnect
    close(connection);
    // Close the socket
    SocketHandler::_closeSocket();
    // Then finally add the QUIT message
    SocketHandler::_addToIncomingMessages(new Message(Message::MT_QUIT));
}

// static, public
bool SocketHandler::isSocketOpen()
{
    return _isSocketOpen;
}

// static, private
void* SocketHandler::_socketLoop(void* parameter)
{
    int connection;

    // Strategy:
    // One outer infinite loop keeps the server listening for connections over time, allowing 
    // multiple clients to connect and disconnect. The inner infinite loop reads data from the
    // current active connection, parses the data into messages, and queues up the messages to 
    // be read by another thread.

    while (1)
    {
        // Open the socket
        if (!SocketHandler::_openSocket())
        {
            oas::Logger::logf("SocketHandler - Resetting the server...");
            alutSleep(10.0);
            continue; 
        }

        // This thread will block until a client connects.
        connection = SocketHandler::_acceptNewConnection();
        
        // If connection is an invalid socket descriptor, some error occured
        if (0 > connection)
        {
            // Close the socket and retry
            SocketHandler::_closeSocket();
            continue;
        }

        int amountRead, amountWritten;
        int amountParsed = 0;
        bool validConnection = true;

        // Use a circular buffer to read data
        static char circularBuf[MAX_CIRCULAR_BUFFER_SIZE];
        char *bufPtr = circularBuf;

        _isConnectedToClient = true;

        while (validConnection)
        {
            // If the circular buffer is out of space, reset it to the beginning
            if ( ((bufPtr + MAX_TRANSMIT_BUFFER_SIZE) - circularBuf) >= MAX_CIRCULAR_BUFFER_SIZE)
            {
                bufPtr = circularBuf;
            }

            // Zero out the section of the buffer we're reading into
            bzero(bufPtr, MAX_TRANSMIT_BUFFER_SIZE);

            // Read from the socket
            amountRead = read(connection, bufPtr, MAX_TRANSMIT_BUFFER_SIZE);

            // Error occured
            if (-1 == amountRead)
            {
                oas::Logger::error("SocketHandler - Error occured while reading information");
                SocketHandler::_closeConnection(connection);
                break;
            }

            // Client disconnected
            if (0 == amountRead)
    	    {
                oas::Logger::logf("SocketHandler - Client disconnected.");
                SocketHandler::_closeConnection(connection);
                break;
		    }
        
            /* IMPORTANT:   We don't know if bufPtr points to the beginning of an actual message.
             *              We have to parse starting from the endpoint of the previous message
             *              for a more robust implementation.
             */

            // Keep parsing the current input data until the amount parsed is equal to the amount read 
            while (amountParsed < amountRead)
            {
                // Message needs to be parsed, starting from bufPtr
                Message *newMessage = new Message();
                Message::MessageError parseError;

                parseError = newMessage->parseString(bufPtr, amountRead, amountParsed);

                // check parseError to keep track as necessary
                if (Message::MERROR_NONE != parseError)
                {
                    // If the message is empty, we are done parsing this input
                    if (Message::MERROR_EMPTY_MESSAGE == parseError)
                    {
                        delete newMessage;
                        break;
                    }
                    // Else there was some parsing error 
                    else
                    {
                        oas::Logger::warnf("SocketHandler - Parsing failed for incoming message: \"%s\" "
                                            "This message will be ignored.",  bufPtr);
                        delete newMessage;
                        break;
                    }
                }
                // Else, there was no error in parsing
                else
                {
                    // If the message is to quit
                    if (Message::MT_QUIT == newMessage->getMessageType())
                    {
                        oas::Logger::logf("SocketHandler - Client disconnected.");
                        delete newMessage;
                        SocketHandler::_closeConnection(connection);
                        validConnection = false;
                        break;
                    }

                    // If a binary file is incoming, call _receiveBinaryFile(),
                    // which will use FileHandler to append binary data to file
                    else if (Message::MT_PTFI_FN_1I == newMessage->getMessageType())
                    {
                        SocketHandler::_receiveBinaryFile(connection, *newMessage);
                    }

                    // Queue up the parsed message
                    SocketHandler::_addToIncomingMessages(newMessage);

                    // If the server needs to send a response back to the client for this message
                    if (newMessage->needsResponse())
                    {
                        // The socket thread will block until the server has generated the response
                        char *response = SocketHandler::_getNextOutgoingResponse();

                        // write the response to the socket connection
                        amountWritten = write(connection, response, strlen(response));
                        if (-1 == amountWritten)
                        {
                            oas::Logger::errorf("SocketHandler - Error occurred writing a response to the client.");
                        }
                        // delete the response that was allocated
                        delete[] response;
                    }
                }
            } // End message parsing loop

        } // End connection loop. Client has disconnected, or an error occured.
        _isConnectedToClient = false;
    }

    close(SocketHandler::_socketHandle);
    pthread_exit(NULL);
    return NULL;
}

// static, private
void SocketHandler::_receiveBinaryFile(int connection, const Message& ptfi)
{
    char *data, *dataPtr;
	int fileSize, bytesLeft, bytesRead;
    oas::FileHandler fileHandler;
	bool errorOccured = false;

	fileSize = ptfi.getIntegerParam();
	bytesLeft = fileSize;
	data = new char[fileSize];
	dataPtr = data;

    oas::Logger::logf("> Receiving file \"%s\" (%d bytes) over socket.",
                      ptfi.getFilename().c_str(),
                      bytesLeft);

    long count = 0;

	while (bytesLeft > 0)
	{
		bytesRead = read(connection, dataPtr, bytesLeft);

		if (bytesRead == 0 || bytesRead == -1)
		{
			oas::Logger::errorf("SocketHandler - Error occured while receiving %s!",
			                    ptfi.getFilename().c_str());
			errorOccured = true;
			break;
		}
		
        bytesLeft -= bytesRead;
        dataPtr += bytesRead;

        count++;

        if (!errorOccured && ((0 == (count % 50)) || 0 == bytesLeft))
        {
            float percentage = ((float) (fileSize - bytesLeft) / fileSize) * 100.0;
            oas::Logger::logf("> File %s...%.2f%% complete.",
                             ptfi.getFilename().c_str(), percentage);
        }
	}

    // Write data to file
    if (!errorOccured && !fileHandler.writeFile(ptfi.getFilename(), data, fileSize))
    {
        oas::Logger::errorf("SocketHandler - Error occured while writing %s to disk.",
                            ptfi.getFilename().c_str());
        errorOccured = true;
    }

    if (!errorOccured)
	{
	    oas::Logger::logf("> File transmission complete.");
	    oas::Logger::logf("> %s is %d bytes", ptfi.getFilename().c_str(), fileSize);
	}

    delete[] data;
}

// static, public
unsigned int SocketHandler::numberOfIncomingMessages()
{
    unsigned retval = 0;
    pthread_mutex_lock(&SocketHandler::_inMutex);
    retval = _incomingMessages.size();
    pthread_mutex_unlock(&SocketHandler::_inMutex);
    return retval;
}

// static, private
void SocketHandler::_addToIncomingMessages(Message *message)
{
    // lock mutex
    pthread_mutex_lock(&SocketHandler::_inMutex);
    // push message into queue
    SocketHandler::_incomingMessages.push(message);
    // use condition variable to signal that queue is not empty
    pthread_cond_signal(&SocketHandler::_inCondition);
    // unlock mutex
    pthread_mutex_unlock(&SocketHandler::_inMutex);
}

// static, public
void SocketHandler::populateQueueWithIncomingMessages(std::queue<Message*> &destination, struct timespec timeout)
{
    // lock mutex
    pthread_mutex_lock(&SocketHandler::_inMutex);

    // while the incoming messages queue is empty
    while (SocketHandler::_incomingMessages.empty())
    {
        // wait (block) on condition variable for queue to have content, or until timeout occurs
        int error = pthread_cond_timedwait(&SocketHandler::_inCondition, &SocketHandler::_inMutex, &timeout);

        // If some error occured (or the wait timed out)
        if (0 != error)
        {
            // Unlock and return
            pthread_mutex_unlock(&SocketHandler::_inMutex);
            return;
        }
    }

    // At this point, we know that the incoming messages queue is not empty.
    // So, we empty out the incoming queue, adding each message to the destination queue
    while (!SocketHandler::_incomingMessages.empty())
    {
        // retrieve data after we're done waiting
        Message* nextMessage = SocketHandler::_incomingMessages.front();
        SocketHandler::_incomingMessages.pop();
        destination.push(nextMessage);
    }

    // unlock mutex
    pthread_mutex_unlock(&SocketHandler::_inMutex);
}

// static, public
void SocketHandler::addOutgoingResponse(const char *response)
{
    // If null or empty string, do nothing
    if (!response || !*response)
    {
        return;
    }

    // create a copy of the response to put into the queue
    char *newString = new char[strlen(response) + 1];
    strcpy(newString, response);

    // lock mutex 
    pthread_mutex_lock(&SocketHandler::_outMutex);

    // add our copy of the response to the queue
    SocketHandler::_outgoingResponses.push(newString);

    // use condition variable to signal that the queue is not empty
    pthread_cond_signal(&SocketHandler::_outCondition);
    // unlock mutex
    pthread_mutex_unlock(&SocketHandler::_outMutex);
}

// static, public
void SocketHandler::addOutgoingResponse(const long response)
{
    // wrap the response into a character buffer
    char buf[MAX_TRANSMIT_BUFFER_SIZE];
    sprintf(buf, "%ld\n", response);
    SocketHandler::addOutgoingResponse(buf);
}

// static, private
char* SocketHandler::_getNextOutgoingResponse()
{
    char *retval = NULL;

    // lock mutex
    pthread_mutex_lock(&SocketHandler::_outMutex);

    // while the outgoing response queue is empty
    while (SocketHandler::_outgoingResponses.empty())
    {
        // wait (block) on the condition variable for queue to have content
        pthread_cond_wait(&SocketHandler::_outCondition, &SocketHandler::_outMutex);
    }

    // retrieve data after we're done waiting
    retval = SocketHandler::_outgoingResponses.front();
    SocketHandler::_outgoingResponses.pop();

    // unlock mutex
    pthread_mutex_unlock(&SocketHandler::_outMutex);

    return retval;
}

