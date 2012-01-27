#ifndef _OAS_SERVER_H_
#define _OAS_SERVER_H_

#include <iostream>
#include <queue>
#include <sys/types.h>
#include <pthread.h>
#include <AL/alut.h>
#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Double_Window.H>
#include "OASFileHandler.h"
#include "OASSocketHandler.h"
#include "OASMessage.h"
#include "OASAudioHandler.h"
#include "OASLogger.h"


namespace oas
{
class Server
{
public:
    static void initialize(int argc, char **argv);

private:
    static pthread_t _serverThread;
    static std::string _cacheDirectory;
    static unsigned short _port;
    static std::string _deviceString;

    // defaults
    static std::string _defaultConfigFile;
    
    // private worker methods
    static void _readConfigFile(int argc, char **argv);
    static void* _serverLoop(void *parameter);
    static void _processMessage(const Message &message);
    static void _fatalError(const char *errorMessage);

};
}

#endif

