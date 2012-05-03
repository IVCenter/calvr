/**
 * @file OASServerWindow.cpp
 * @author Shreenidhi Chowkwale
 */

#include "OASServerWindow.h"

using namespace oas;

// Statics
Fl_Double_Window*   		ServerWindow::_window = NULL;
Fl_Tabs*            		ServerWindow::_tabs = NULL;
Fl_Group*           		ServerWindow::_tabGroup1 = NULL;
ServerWindowLogBrowser* 	ServerWindow::_browser = NULL;
Fl_Group*           		ServerWindow::_tabGroup2 = NULL;
ServerWindowTable*          ServerWindow::_table = NULL;
Fl_Group*           		ServerWindow::_tabGroup3 = NULL;
Fl_Group*           		ServerWindow::_tabGroup4 = NULL;

pthread_t                   ServerWindow::_windowThread;
bool                        ServerWindow::_isInitialized = false;

void                      (*ServerWindow::_atExitCallback)(void) = NULL;
const unsigned int          ServerWindow::_kWindowWidth = 800;
const unsigned int          ServerWindow::_kWindowHeight = 600;
const unsigned int          ServerWindow::_kTabHeight = 25;

// public, static
bool ServerWindow::initialize(int argc, char **argv, void (*atExitCallback) (void) = NULL)
{
    // Create the window
	ServerWindow::_window  = new Fl_Double_Window( ServerWindow::_kWindowWidth + 20,
                                                   ServerWindow::_kWindowHeight + 20, 
                                                   WINDOW_TITLE);

    // Create the tabs
    ServerWindow::_tabs = new Fl_Tabs(10, 
                                      10, 
                                      ServerWindow::_kWindowWidth,
                                      ServerWindow::_kWindowHeight);
    // Set the tooltip
    ServerWindow::_tabs->tooltip("Select one of the tabs do view different information about the server.");
    // Set the selection color to blue
    ServerWindow::_tabs->selection_color((Fl_Color) 4);
    // Set the label color
    ServerWindow::_tabs->labelcolor(FL_BACKGROUND2_COLOR);

    // Create the first tab group
    ServerWindow::_tabGroup1 = new Fl_Group(10,
                                            ServerWindow::_kTabHeight + 10,
                                            ServerWindow::_kWindowWidth,
                                            ServerWindow::_kWindowHeight - ServerWindow::_kTabHeight,
                                            "Log");
    ServerWindow::_tabGroup1->tooltip("This tab displays the log window.");

    // Create the browser
	ServerWindow::_browser = new ServerWindowLogBrowser(10,
											ServerWindow::_kTabHeight + 10, 
											ServerWindow::_kWindowWidth, 
											ServerWindow::_kWindowHeight - ServerWindow::_kTabHeight);

    ServerWindow::_tabGroup1->end();
    Fl_Group::current()->resizable(ServerWindow::_tabGroup1);

    // Create the second tab group
    ServerWindow::_tabGroup2 = new Fl_Group(10,
                                            ServerWindow::_kTabHeight + 10,
                                            ServerWindow::_kWindowWidth,
                                            ServerWindow::_kWindowHeight - ServerWindow::_kTabHeight,
                                            "Table");
    ServerWindow::_tabGroup2->tooltip("This tab displays information about the server.");

    // Create the table
    ServerWindow::_table = new ServerWindowTable(10,
                                                 ServerWindow::_kTabHeight + 10,
                                                 ServerWindow::_kWindowWidth,
                                                 ServerWindow::_kWindowHeight - ServerWindow::_kTabHeight);

    ServerWindow::_tabGroup2->hide();
    ServerWindow::_tabGroup2->end();

    // Create the third tab group
    ServerWindow::_tabGroup3 = new Fl_Group(10,
                                            ServerWindow::_kTabHeight + 10,
                                            ServerWindow::_kWindowWidth,
                                            ServerWindow::_kWindowHeight - ServerWindow::_kTabHeight,
                                            "Visual");
    ServerWindow::_tabGroup3->tooltip("This tab visualizes the sources and plots them.");
    ServerWindow::_tabGroup3->hide();
    ServerWindow::_tabGroup3->end();

    // Finalize the tabs
    ServerWindow::_tabs->end();

    // Finalize the window
	// ServerWindow::_browser->position(0);
    ServerWindow::_window->callback(ServerWindow::_confirmExitCallback);
    ServerWindow::_window->end();
	ServerWindow::_window->show(argc, argv);
	
	ServerWindow::addToBrowser("Starting up the OpenAL Audio Server...");

    // This lock() should be the first Fl::lock() called during initialization.
    // It lets fltk know to enable multi-threading support for the GUI,
    // and the function call should not block on anything, returning immediately.
    // If multi-threading support is available, the method returns 0.
    if (Fl::lock())
    {
        oas::Logger::warnf("FLTK does not support threading on this platform.\n"
                            "Make sure FLTK was compiled with threading enabled.\n"
                            "OAS will attempt to continue running...");
    }

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
		oas::Logger::errorf("Could not create window thread!\n");
        return false;
	}

    ServerWindow::_isInitialized = true;
    ServerWindow::_atExitCallback = atExitCallback;

    return true;
}

// public, static
void* ServerWindow::_windowLoop(void *parameter)
{
    while (1)
    {
        ServerWindow::_table->update();
    }
    return NULL;
}

// public, static
bool ServerWindow::isInitialized()
{
    return _isInitialized;
}

// private, static
void ServerWindow::_confirmExitCallback(Fl_Widget*, void*)
{
    // Prompt the user to make sure audio server is actually supposed to exit
    if (fl_choice("Are you sure you want to quit the audio server?", "Cancel", "Exit", NULL))
    {
        // If the atExit callback function has been set, call it
        if (NULL != ServerWindow::_atExitCallback)
        {
            (*_atExitCallback)();
        }
        // Exit the entire application
        exit(0);
    }
}

