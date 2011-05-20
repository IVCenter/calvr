#include <util/MultiListenSocket.h>

#include <iostream>
#include <errno.h>

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
}

MultiListenSocket::~MultiListenSocket()
{
}

bool MultiListenSocket::setup()
{
    _socket = socket(AF_INET, SOCK_STREAM, 0);

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

    int flags = fcntl(_socket, F_GETFL, 0);
    fcntl(_socket, F_SETFL, flags | O_NONBLOCK);

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

    return true;
}

CVRSocket * MultiListenSocket::accept()
{
    sockaddr_in addr;
    int length;
    int val = ::accept(_socket, (sockaddr *)&addr, (socklen_t *)&length);
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
