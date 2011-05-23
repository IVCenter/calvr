#include <util/CVRSocket.h>

#include <iostream>
#include <sstream>

#include <errno.h>

#ifndef WIN32
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#else
#include <winsock2.h>
#include <stdlib.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

using namespace cvr;

CVRSocket::CVRSocket(int socket)
{
    _socket = socket;
    _type = CONNECT;
    _printErrors = false;
    _blockingState = true;
}

CVRSocket::CVRSocket(SocketType type, std::string host, int port, int family,
                     int sockType)
{
    _type = type;
    _family = family;
    _sockType = sockType;
    _host = host;
    _port = port;
    _blockingState = true;

    _socket = -1;

    struct ::addrinfo hints;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = _family;
    hints.ai_socktype = _sockType;

    std::stringstream ss;
    ss << port;

    getaddrinfo(_host.c_str(), ss.str().c_str(), &hints, &_res);

    if((_socket = (int) socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol))
            == -1)
    {
	perror("socket");
    }
}

CVRSocket::~CVRSocket()
{
    if(valid())
    {
#ifdef WIN32
        closesocket(_socket);
#else
        close(_socket);
#endif
    }
}

bool CVRSocket::bind()
{
    if(_type != LISTEN)
    {
        std::cerr << "Error: Calling bind on Connect Socket." << std::endl;
        return false;
    }

    if(!valid())
    {
        std::cerr << "Error: Calling bind on invalid socket." << std::endl;
        return false;
    }

    if(::bind(_socket, _res->ai_addr, _res->ai_addrlen) == -1)
    {
	if(_printErrors)
	{
	    perror("bind");
	}
        return false;
    }

    return true;
}

bool CVRSocket::listen(int backlog)
{
    if(_type != LISTEN)
    {
        std::cerr << "Error: Calling listen on Connect Socket." << std::endl;
        return false;
    }

    if(!valid())
    {
        std::cerr << "Error: Calling listen on invalid socket." << std::endl;
        return false;
    }

    if(::listen(_socket, backlog) == -1)
    {
	if(_printErrors)
	{
	    perror("listen");
	}
        return false;
    }

    return true;
}

bool CVRSocket::accept()
{
    if(_type != LISTEN)
    {
        std::cerr << "Error: Calling accept on Connect Socket." << std::endl;
        return false;
    }

    if(!valid())
    {
        std::cerr << "Error: Calling accept on invalid socket." << std::endl;
        return false;
    }

    struct sockaddr_storage node_addr;
    socklen_t addr_size;
    addr_size = sizeof(node_addr);
    int tmpSock;
    if((tmpSock = (int) ::accept(_socket, (struct sockaddr *)&node_addr, &addr_size))
            == -1)
    {
	if(_printErrors)
	{
	    perror("accept");
	}
#ifdef WIN32
        closesocket(_socket);
#else
        close(_socket);
#endif
        _socket = -1;
        return false;
    }
#ifdef WIN32
    closesocket(_socket);
#else
    close(_socket);
#endif
    _socket = tmpSock;
    return true;
}

bool CVRSocket::connect(int timeout)
{
    if(_type != CONNECT)
    {
        std::cerr << "Error: Calling connect on Listen Socket." << std::endl;
        return false;
    }

    if(!valid())
    {
        std::cerr << "Error: Calling connect on invalid socket." << std::endl;
        return false;
    }

    bool resetBlocking = false;
    if(timeout > 0)
    {
	if(_blockingState)
	{
	    setBlocking(false);
	    resetBlocking = true;
	}
    }

    int currentTimeout = timeout;

    while(::connect(_socket, _res->ai_addr, _res->ai_addrlen) == -1)
    {
	//perror("connect");
        if(timeout == 0)
        {
            std::cerr << "Error: Unable to connect to host: " << _host
                    << " on port: " << _port << std::endl;
            return false;
        }
	/*else if(errno == EINPROGRESS)
	{
	    continue;
	}
	else if(errno == EAGAIN || errno == EALREADY)
	{
	    FD_ZERO(&_connectTest);

	    FD_SET(_socket, &_connectTest);

	    struct timeval tv;
	    tv.tv_sec = currentTimeout;
	    tv.tv_usec = 0;

	    select(_socket+1,NULL,&_connectTest,NULL,&tv);
	    if(!FD_ISSET(_socket,&_connectTest))
	    {
		std::cerr << "Error: Unable to connect to host: " << _host
                    << " on port: " << _port << std::endl;
		return false;
	    }
	    break;
	}*/
	else
	{
	    if(currentTimeout <= 0)
	    {
		std::cerr << "Error: Unable to connect to host: " << _host
                    << " on port: " << _port << std::endl;
		return false;
	    }
#ifndef WIN32
	    sleep(1);
#else
		Sleep(1000);
#endif
	    currentTimeout--;
	}
    }

    if(resetBlocking)
    {
	setBlocking(true);
    }

    return true;
}

int CVRSocket::setsockopt(int level, int optname, void * val, socklen_t len)
{
    return ::setsockopt(_socket, level, optname, (const char *)val, len);
}

void CVRSocket::setNoDelay(bool b)
{
    if(!valid())
    {
        std::cerr << "Error: setNoDelay: invalid socket." << std::endl;
        return;
    }

    int yes;
    if(b)
    {
        yes = 1;
    }
    else
    {
        yes = 0;
    }

    if(::setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&yes,
                    sizeof(int)) == -1)
    {
        perror("NO_DELAY");
    }
}

void CVRSocket::setReuseAddress(bool b)
{
    if(!valid())
    {
        std::cerr << "Error: setReuseAddress: invalid socket." << std::endl;
        return;
    }

    int yes;
    if(b)
    {
        yes = 1;
    }
    else
    {
        yes = 0;
    }

    if(::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes,
                    sizeof(int)) == -1)
    {
        perror("setsockopt");
    }
}

void CVRSocket::setBlocking(bool b)
{
    if(!valid())
    {
        std::cerr << "Error: setBlocking: invalid socket." << std::endl;
        return;
    }

    int res;

#ifndef WIN32
    int flags = fcntl(_socket, F_GETFL, 0);
    if(b)
    {
        res = fcntl(_socket, F_SETFL, flags & ~(O_NONBLOCK));
    }
    else
    {
        res = fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
    }
#else
	u_long val;
	if(b)
	{
		val = 0;
	}
	else
	{
		val = 1;
	}
	res = ioctlsocket(_socket, FIONBIO, &val);
#endif

    if(res < 0)
    {
        std::cerr << "Error setting socket blocking." << std::endl;
    }
    else
    {
	_blockingState = b;
    }
}

bool CVRSocket::send(void * buf, size_t len, int flags)
{
    if(!buf)
    {
        std::cerr << "Error sending NULL buffer." << std::endl;
        return false;
    }

    if(!valid())
    {
        std::cerr << "Error: Calling send on invalid socket." << std::endl;
        return false;
    }

    int bytesToSend = (int) len;
    int sent;
    char * data = (char *)buf;
    while(bytesToSend > 0)
    {
        if((sent = ::send(_socket, (const char *)data, bytesToSend, flags))
                <= 0)
        {
	    if(_printErrors)
	    {
		std::cerr << "Error sending data." << std::endl;
		perror("send");
	    }
            break;
        }
        bytesToSend -= sent;
        data += sent;
    }

    if(bytesToSend)
    {
        return false;
    }
    return true;
}

bool CVRSocket::recv(void * buf, size_t len, int flags)
{
    if(!buf)
    {
        std::cerr << "Error recv with NULL buffer." << std::endl;
        return false;
    }

    if(!valid())
    {
        std::cerr << "Error: Calling recv on invalid socket." << std::endl;
        return false;
    }

    int bytesToRead = (int) len;
    int read;
    char * data = (char *)buf;
    while(bytesToRead > 0)
    {
        if((read = ::recv(_socket, data, bytesToRead, flags)) <= 0)
        {
            //if(errno != EAGAIN)
            //{
	    if(_printErrors)
	    {
		std::cerr << "Error on recv." << std::endl;
		perror("recv");
	    }
            break;
            //}
            //std::cerr << "EAGAIN error" << std::endl;
            //read = 0;
        }
        bytesToRead -= read;
        data += read;
    }

    if(bytesToRead)
    {
        return false;
    }
    return true;
}

bool CVRSocket::valid()
{
    return _socket >= 0;
}

void CVRSocket::setSocketFD(int socket)
{
    _socket = socket;
}

int CVRSocket::getSocketFD()
{
    return _socket;
}
