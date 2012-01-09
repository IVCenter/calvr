/**
 * @file OASServerWindow.cpp
 * @author Shreenidhi Chowkwale
 */

#include "OASServerWindow.h"

using namespace oas;

// Statics
Fl_Browser*         ServerWindow::_browser = NULL;
Fl_Double_Window*   ServerWindow::_window = NULL;
pthread_t           ServerWindow::_windowThread;
bool                ServerWindow::_isInitialized = false;
unsigned int        ServerWindow::_browserSize = 1;
const unsigned int  ServerWindow::_kWindowWidth;
const unsigned int  ServerWindow::_kWindowHeight;
const char* const   ServerWindow::_boldBrowserFormatter     = "@b";
const char* const   ServerWindow::_italicsBrowserFormatter  = "@i";
const char* const   ServerWindow::_nullBrowserFormatter     = "@.";

// public, static
bool ServerWindow::initialize(int argc, char **argv)
{
    // Create the window
	ServerWindow::_window  = new Fl_Double_Window( ServerWindow::_kWindowWidth,
                                                   ServerWindow::_kWindowHeight, 
                                                   WINDOW_TITLE);
                                                 
    // Create the browser
	ServerWindow::_browser = new Fl_Browser( 0, 
											 0, 
											 ServerWindow::_kWindowWidth, 
											 ServerWindow::_kWindowHeight);
	
    // Set up the browser and window
	// ServerWindow::_browser->position(0);
	ServerWindow::_window->resizable(ServerWindow::_browser);
    ServerWindow::_window->end();
	ServerWindow::_window->show(argc, argv);
	
    ServerWindow::_browserSize = 1;
	ServerWindow::_browser->add("Starting up the OpenAL Audio Server...");
    ServerWindow::_browserSize++;

    // This lock should be the first Fl::lock() called during initialization.
    // It lets fltk know to enable multi-threading support for the rest of the program,
    // and the function call should not block on anything, returning immediately
    Fl::lock();

    // Create the window thread
	pthread_attr_t threadAttr;
	
	pthread_attr_init(&threadAttr);
	pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
	
	int threadError = pthread_create(&oas::ServerWindow::_windowThread, 
                                     &threadAttr, 
                                     &ServerWindow::_windowLoop, 
                                     NULL);

	pthread_attr_destroy(&threadAttr);
	
	if (threadError)
	{
		std::cerr << "Could not create window thread!\n";
        return false;
	}

    ServerWindow::_isInitialized = true;

    return true;
}

// public, static
void ServerWindow::addToBrowser(const char *line)
{
    // Wait for a lock on the GUI environment in case other threads are using it
    Fl::lock();

    // Add the line
    ServerWindow::_browser->add(line);
    ServerWindow::_browserSize++;
    
    // Scroll the browser to the bottom, as necessary
    ServerWindow::_browser->bottomline(ServerWindow::_browserSize);

    // Release the lock
    Fl::unlock();
    // Let the main thread know that the window should be redrawn
    Fl::awake();
}

// public, static
void ServerWindow::replaceBottomLine(const char *line)
{
    // Wait for a lock on the GUI
    Fl::lock();

    // Replaces the text on the bottommost line
    ServerWindow::_browser->text(ServerWindow::_browserSize, line);
    
    // Release lock, let main thread know
    Fl::unlock();
    Fl::awake();
}

// public, static
void* ServerWindow::_windowLoop(void *parameter)
{

    return NULL;
}

// public, static
bool ServerWindow::isInitialized()
{
    return _isInitialized;
}

// public, static
const char* const ServerWindow::getBoldBrowserFormatter()
{
    return ServerWindow::_boldBrowserFormatter;
}

// public, static
const char* const ServerWindow::getItalicsBrowserFormatter()
{
    return ServerWindow::_italicsBrowserFormatter;
}

// public, static
const char* const ServerWindow::getNullBrowserFormatter()
{
    return ServerWindow::_nullBrowserFormatter;
}

