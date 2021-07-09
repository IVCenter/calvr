#include <cvrUtil/CVRMulticastSocket.h>

#ifndef WIN32
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdio>

using namespace cvr;

CVRMulticastSocket::CVRMulticastSocket(MCSocketType st,
        std::string groupAddress, int port)
{
    _type = st;
    _groupAddress = groupAddress;
    _port = port;

    _socket = (int)socket(AF_INET,SOCK_DGRAM,0);
    if(_socket < 0)
    {
        std::cerr << "CVRMulticastSocket: error creating socket." << std::endl;
        return;
    }

    if(_type == SEND)
    {
        unsigned char ttl = 1;
        if(::setsockopt(_socket,IPPROTO_IP,IP_MULTICAST_TTL,(const char *)&ttl,
                sizeof(ttl)) < 0)
        {
            std::cerr
                    << "CVRMulticastSocket: warning, unable to set IP_MULTICAST_TTL."
                    << std::endl;
        }

        unsigned char loop = 0;
        if(::setsockopt(_socket,IPPROTO_IP,IP_MULTICAST_LOOP,
                (const char *)&loop,sizeof(loop)) < 0)
        {
            std::cerr
                    << "CVRMulticastSocket: warning, unable to set IP_MULTICAST_LOOP."
                    << std::endl;
        }

        memset(&_address,0,sizeof(_address));
        _address.sin_family = AF_INET;
        _address.sin_addr.s_addr = inet_addr(_groupAddress.c_str());
        _address.sin_port = htons(_port);
    }
    else if(_type == RECV)
    {
        int yes = 1;
        if(::setsockopt(_socket,SOL_SOCKET,SO_REUSEADDR,(const char *)&yes,
                sizeof(int)) == -1)
        {
            std::cerr
                    << "CVRMulticastSocket: warning, unable to set SO_REUSEADDR."
                    << std::endl;
        }

        memset(&_address,0,sizeof(_address));
        _address.sin_family = AF_INET;
        _address.sin_addr.s_addr = htonl(INADDR_ANY);
        _address.sin_port = htons(_port);
        _addrlen = sizeof(_address);

        if(bind(_socket,(struct sockaddr *)&_address,sizeof(_address)) < 0)
        {
            std::cerr << "CVRMulticastSocket: error on bind." << std::endl;
#ifdef WIN32
            closesocket(_socket);
#else
            close(_socket);
#endif
            _socket = -1;
            return;
        }

        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(_groupAddress.c_str());
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if(::setsockopt(_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,
                (const char *)&mreq,sizeof(mreq)) < 0)
        {
            std::cerr << "CVRMulticastSocket: error joing multicast group: "
                    << _groupAddress << std::endl;
#ifdef WIN32
            closesocket(_socket);
#else
            close(_socket);
#endif
            _socket = -1;
            return;
        }

    }
}

CVRMulticastSocket::~CVRMulticastSocket()
{
    if(_socket >= 0)
    {
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(_groupAddress.c_str());
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        ::setsockopt(_socket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(const char *)&mreq,
                sizeof(mreq));
#ifdef WIN32
        closesocket(_socket);
#else
        close(_socket);
#endif
    }
}

void CVRMulticastSocket::setMulticastInterface(std::string iface)
{
    if(_type == SEND)
    {
        struct addrinfo hints;
        struct addrinfo * res;
        memset(&hints,0,sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        std::stringstream ss;
        ss << _port;

        if(getaddrinfo(iface.c_str(),ss.str().c_str(),&hints,&res))
        {
            std::cerr
                    << "CVRMulticastSocket: Error socket getaddrinfo interface: "
                    << iface << " port: " << _port << std::endl;
            return;
        }

        struct sockaddr_in * addr = (struct sockaddr_in*)res->ai_addr;

        struct in_addr interface_addr;
        interface_addr.s_addr = addr->sin_addr.s_addr;
        if(::setsockopt(_socket,IPPROTO_IP,IP_MULTICAST_IF,
                (const char *)&interface_addr,sizeof(interface_addr)) < 0)
        {
            std::cerr
                    << "CVRMulticastSocket: warning, unable to set multicast interface."
                    << std::endl;
        }
    }
}

int CVRMulticastSocket::setsockopt(int level, int optname, void * val,
        socklen_t len)
{
    return ::setsockopt(_socket,level,optname,(const char *)val,len);
}

bool CVRMulticastSocket::send(void * buf, size_t len, int flags)
{
    if(_type != SEND)
    {
        std::cerr
                << "CVRMulticastSocket: error, calling send on a RECV type socket."
                << std::endl;
        return false;
    }

    if(!buf)
    {
        std::cerr << "CVRMulticastSocket: Error sending NULL buffer."
                << std::endl;
        return false;
    }

    if(_socket < 0)
    {
        std::cerr
                << "CVRMulticastSocket: Error: Calling send on invalid socket."
                << std::endl;
        return false;
    }

    int bytesToSend = (int)len;
    int sent;
    char * data = (char *)buf;
    while(bytesToSend > 0)
    {
        if((sent = sendto(_socket,(const char *)data,bytesToSend,flags,
                (struct sockaddr *)&_address,sizeof(_address))) <= 0)
        {
            std::cerr << "CVRMulticastSocket: Error sending data." << std::endl;
            perror("sendto");
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

bool CVRMulticastSocket::recv(void * buf, size_t len, int flags)
{
    if(_type != RECV)
    {
        std::cerr
                << "CVRMulticastSocket: error calling recv from a SEND type socket."
                << std::endl;
    }

    if(!buf)
    {
        std::cerr << "CVRMulticastSocket: Error recv with NULL buffer."
                << std::endl;
        return false;
    }

    if(_socket < 0)
    {
        std::cerr
                << "CVRMulticastSocket: Error: Calling recv on invalid socket."
                << std::endl;
        return false;
    }

    int bytesToRead = (int)len;
    int read;
    char * data = (char *)buf;
    while(bytesToRead > 0)
    {
        if((read = recvfrom(_socket,data,bytesToRead,flags,
                (struct sockaddr *)&_address,&_addrlen)) <= 0)
        {
            std::cerr << "CVRMulticastSocket: Error on recv." << std::endl;
            perror("recv");
            break;
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

void CVRMulticastSocket::setSocketFD(int socket)
{
    _socket = socket;
}

int CVRMulticastSocket::getSocketFD()
{
    return _socket;
}

bool CVRMulticastSocket::valid()
{
    return _socket >= 0;
}
