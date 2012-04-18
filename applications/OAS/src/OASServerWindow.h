/**
 * @file OASServerWindow.h
 * @author Shreenidhi Chowkwale
 */

#ifndef _OAS_SERVER_WINDOW_H_
#define _OAS_SERVER_WINDOW_H_

#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <FL/fl_ask.H>
#include <pthread.h>
#include <iostream>
#include "OASLogger.h"

namespace oas
{

class ServerWindow
{
#define WINDOW_TITLE "OpenAL Audio Server"

public:
    // Create new thread, make double window, set up browser
    static bool initialize(int argc, char **argv, void (*atExitCallback) (void));

    static void addToBrowser(char *line);
    static void replaceBottomLine(const char *line);
    static const char* const getBoldBrowserFormatter();
    static const char* const getItalicsBrowserFormatter();
    static const char* const getNullBrowserFormatter();
    static const int getBrowserFormatterLength();
    static int getBrowserSize();
    static bool isInitialized();

protected:
    // The double window class gives us a double buffered window. This will contain
    // the rest of the UI
    static Fl_Double_Window *_window;
    // The tabs give us an entity that will contain all of the other tab groups
    static Fl_Tabs *_tabs;

    // Tab group 1 contains the browser and any other buttons associated with the
    // browser window (i.e. clear, copy, etc.)
    static Fl_Group *_tabGroup1;
    static Fl_Browser *_browser;

    // Tab group 2 contains a tabular representation of the sound source data
    static Fl_Group *_tabGroup2;

    // Tab group 3 contains a visual representation of the sound source data
    static Fl_Group *_tabGroup3;

    // Tab group 4 can have statistical information about the server
    static Fl_Group *_tabGroup4;

    // The thread that will do all of the window processing
    static pthread_t _windowThread;

    static bool _isInitialized;

    // Keeps track of the browser size
    static unsigned int _browserSize;

    // Function pointer that will be called when window is closed, before program exits
    static void (*_atExitCallback) (void);

    static const unsigned int _kWindowWidth;
    static const unsigned int _kWindowHeight;
    static const unsigned int _kTabHeight;
    static const char* const _boldBrowserFormatter;
    static const char* const _italicsBrowserFormatter;
    static const char* const _nullBrowserFormatter;
    static const int _browserFormatterLength;
    
private:
	static void* _windowLoop(void *parameter);
    static void _incrementBrowserSize();
    static void _confirmExitCallback(Fl_Widget*, void*);
};

}

#endif
