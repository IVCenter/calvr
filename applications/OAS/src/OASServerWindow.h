/**
 * @file OASServerWindow.h
 * @author Shreenidhi Chowkwale
 */

#ifndef _OAS_SERVER_WINDOW_H_
#define _OAS_SERVER_WINDOW_H_

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <pthread.h>
#include <iostream>
#include "OASLogger.h"
#include "OASServerWindowLogBrowser.h"
#include "OASServerWindowTable.h"

namespace oas
{

class ServerWindow
{
#define WINDOW_TITLE "OpenAL Audio Server"

public:
    // Create new thread, make double window, set up browser
    static bool initialize(int argc, char **argv, void (*atExitCallback) (void));

    static inline bool isInitialized()
    {
        return _isInitialized;
    }

    static inline void reset()
    {
        if (isInitialized())
            ServerWindow::_table->reset();
    }

    static inline void audioUnitWasModified(const AudioUnit* audioUnit)
    {
        if (isInitialized())
            ServerWindow::_table->audioUnitWasModified(audioUnit);
    }

    static inline void audioUnitsWereModified(std::queue<const AudioUnit*> &audioUnits)
    {
        if (isInitialized())
            ServerWindow::_table->audioUnitsWereModified(audioUnits);
    }

    static inline void addToLogWindow(const char *line)
    {
        if (isInitialized())
            ServerWindow::_browser->add(line);
    }

    static inline void replaceBottomLine(const char *line)
    {
        if (isInitialized())
            ServerWindow::_browser->replaceBottomLine(line);
    }

    static inline const char* const getBoldBrowserFormatter()
    {
        return ServerWindowLogBrowser::getBoldBrowserFormatter();
    }

    static inline const char* const getItalicsBrowserFormatter()
    {
        return ServerWindowLogBrowser::getItalicsBrowserFormatter();
    }

    static const char* const getNullBrowserFormatter()
    {
        return ServerWindowLogBrowser::getNullBrowserFormatter();
    }

    static const int getBrowserFormatterLength()
    {
        return ServerWindowLogBrowser::getBrowserFormatterLength();
    }


protected:
    // The double window class gives us a double buffered window. This will contain
    // the rest of the UI
    static Fl_Double_Window *_window;
    // The tabs give us an entity that will contain all of the other tab groups
    static Fl_Tabs *_tabs;

    // Tab group 1 contains the browser and any other buttons associated with the
    // browser window (i.e. clear, copy, etc.)
    static Fl_Group *_tabGroup1;
    static ServerWindowLogBrowser *_browser;
    static Fl_Button *_copyToClipboardButton;
    static Fl_Button *_clearButton;

    // Tab group 2 contains a tabular representation of the sound source data
    static Fl_Group *_tabGroup2;
    static ServerWindowTable *_table;

    // Tab group 3 contains a visual representation of the sound source data
    static Fl_Group *_tabGroup3;

    // Tab group 4 can have statistical information about the server
    static Fl_Group *_tabGroup4;

    // The thread that will do all of the window processing
    static pthread_t _windowThread;

    static bool _isInitialized;

    // Function pointer that will be called when window is closed, before program exits
    static void (*_atExitCallback) (void);

    static const unsigned int _kWindowWidth;
    static const unsigned int _kWindowHeight;
    static const unsigned int _kTabHeight;
    static const unsigned int _kTabGroupHeight;
    static const unsigned int _kTabGroupWidth;
    static const unsigned int _kBrowserHeight;
    static const unsigned int _kBrowserWidth;
    static const unsigned int _kTableHeight;
    static const unsigned int _kTableWidth;
    static const unsigned int _kButtonHeight;
    static const unsigned int _kButtonWidth;
    
private:

	static void* _windowLoop(void *parameter);
    static void _confirmExitCallback(Fl_Widget*, void*);
    static void _copyToClipboardButtonCallback(Fl_Widget*, void*);
    static void _clearButtonCallback(Fl_Widget*, void*);
};

}

#endif
