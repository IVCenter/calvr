#include <util/MultiListenSocket.h>

#include <iostream>
#include <errno.h>
#include <cstdio>
#include <cstring>

#ifndef WIN32
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#else
#include <winsock2.h>
#endif

using namespace cvr;

MultiListenSocket::MultiListenSocket(int port, int queue)
{
    _port = port;
    _queue = queue;
    _valid = false;
}

MultiListenSocket::~MultiListenSocket()
{
    if(_valid)
    {
#ifndef WIN32
	close(_socket);
#else
		closesocket(_socket);
#endif
    }
}

bool MultiListenSocket::setup()
{
    if(_valid)
    {
#ifndef WIN32
	close(_socket);
#else
		closesocket(_socket);
#endif
	_valid = false;
    }

    _socket = (int) socket(AF_INET, SOCK_STREAM, 0);

    if(_socket == -1)
    {
        std::cerr << "Error creating socket." << std::endl;
        return false;
    }

    int yes = 1;
    if(setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes,
                  sizeof(int)) == -1)
    {
        perror("setsockopt");
        std::cerr << "Error setting reuseaddress option." << std::endl;
        return false;
    }

#ifndef WIN32
    int flags = fcntl(_socket, F_GETFL, 0);
    fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
#else
	u_long val = 1;
	ioctlsocket(_socket, FIONBIO, &val);
#endif

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if(bind(_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        std::cerr << "Error on socket bind." << std::endl;
	perror("bind");
        return false;
    }

    if(listen(_socket, _queue) == -1)
    {
        std::cerr << "Error on socket listen." << std::endl;
        return false;
    }

    _valid = true;

    return true;
}

CVRSocket * MultiListenSocket::accept()
{
    if(!_valid)
    {
	return NULL;
    }

    sockaddr_in addr;
    int length;
    int val = (int) ::accept(_socket, (sockaddr *)&addr, (socklen_t *)&length);
    if(val == -1)
    {
        if(errno != EWOULDBLOCK)
        {
            perror("accept");
        }
        return NULL;
    }
    return new CVRSocket(val);
}
