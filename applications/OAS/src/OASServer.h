/**
 * @file OASServer.h
 * @author Shreenidhi Chowkwale
 */

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
#include "OASServerInfo.h"
#include "OASLogger.h"


namespace oas
{
class Server
{
public:
    static Server& getInstance();
    void initialize(int argc, char **argv);
    ServerInfo const* getServerInfo();

private:
    pthread_t _serverThread;

    ServerInfo* _serverInfo;

    // private worker methods
    void _readConfigFile(int argc, char **argv);
    static void* _serverLoop(void *parameter);
    void _processMessage(const Message &message);
    void _fatalError(const char *errorMessage);
    static void _atExit();

    double _computeElapsedTime(struct timeval start, struct timeval end);

};
}

#endif

