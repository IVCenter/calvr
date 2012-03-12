#include "OASServerInfo.h"

using namespace oas;

ServerInfo::ServerInfo()
{

}

ServerInfo::ServerInfo( std::string const& cacheDirectory, 
                        unsigned short port):
                        _cacheDirectory(cacheDirectory),
                        _port(port)
{
    
}

std::string const& ServerInfo::getCacheDirectory() const
{
    return this->_cacheDirectory;
}

unsigned short ServerInfo::getPort() const
{
    return this->_port;
}

std::string const& ServerInfo::getAudioDeviceString() const
{
    return this->_audioDeviceString;
}

void ServerInfo::setAudioDeviceString(std::string const& audioDevice)
{
    this->_audioDeviceString = audioDevice;
}

bool ServerInfo::useGUI() const
{
    return this->_useGUI;
}

void ServerInfo::setGUI(bool useGUI)
{
    this->_useGUI = useGUI;
}

