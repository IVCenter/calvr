/**
 * @file OASServerWindow.h
 * @author Shreenidhi Chowkwale
 */

#ifndef _OAS_SERVER_WINDOW_H_
#define _OAS_SERVER_WINDOW_H_

#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Double_Window.H>
#include <pthread.h>
#include <iostream>

namespace oas
{

class ServerWindow
{
#define WINDOW_TITLE "OpenAL Audio Server"

public:
    // Create new thread, make double window, set up browser
    static bool initialize(int argc, char **argv);

    static void addToBrowser(const char *line);
    static void replaceBottomLine(const char *line);
    static const char* const getBoldBrowserFormatter();
    static const char* const getItalicsBrowserFormatter();
    static const char* const getNullBrowserFormatter();
    static bool isInitialized();

protected:
    static Fl_Browser *_browser;
    static Fl_Double_Window *_window;
    static pthread_t _windowThread;
    static bool _isInitialized;
    static unsigned int _browserSize;
    
    static const unsigned int _kWindowWidth = 800;
    static const unsigned int _kWindowHeight = 600;    
    static const char* const _boldBrowserFormatter;
    static const char* const _italicsBrowserFormatter;
    static const char* const _nullBrowserFormatter;
    
private:
	static void* _windowLoop(void *parameter);
};

}

#endif
