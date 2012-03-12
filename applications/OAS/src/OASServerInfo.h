/** 
 * @file OASServerInfo.h
 * @author Shreenidhi Chowkwale
 */

#ifndef _OAS_SERVER_INFO_H_
#define _OAS_SERVER_INFO_H_

#include <string>
#include <cstdlib>

namespace oas
{

class ServerInfo
{
public:

    std::string const& getCacheDirectory() const;
    unsigned short getPort() const;


    std::string const& getAudioDeviceString() const;
    void setAudioDeviceString(std::string const& audioDevice);

    bool useGUI() const;
    void setGUI(bool useGUI);

    ServerInfo(std::string const& cacheDirectory, unsigned short port);

private:
    ServerInfo();

    std::string _cacheDirectory;
    unsigned short _port;
    std::string _audioDeviceString;
    bool _useGUI;
};

}

#endif

