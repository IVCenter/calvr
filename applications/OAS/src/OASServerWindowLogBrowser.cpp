#include "OASServerWindowLogBrowser.h"

using namespace oas;

ServerWindowLogBrowser::ServerWindowLogBrowser(int X, int Y, int W, int H, const char *L)
: Fl_Browser(X, Y, W, H, L)
{
	// The Fl_Browser super class's constructor is called first, via initializor list
	this->_browserSize = 1;
}

void ServerWindowLogBrowser::add(char *line)
{
	if (!line)
		return;

    // Wait for a lock on the GUI environment in case other threads are using it
    Fl::lock();

    // FlBrowser does not like newline characters in the string.
    // So, we split the string into multiple lines for each linefeed character
    // that is found.
    char *pNewline, *pStr = line;
    int numNewLines = 0;

    // Keep looping until we don't find any newline characters in the string
    while (NULL != (pNewline = strchr(pStr, '\n')))
    {
        // Set the newline character to the null character
        *pNewline = '\0';
        // Add pStr to the browser window
        Fl_Browser::add(pStr);
        this->_incrementBrowserSize();
        // Move pStr
        pStr = pNewline + 1;
        numNewLines++;
    }

    // If there were no newline characters found, just add the entire line
    if (0 == numNewLines)
    {
        Fl_Browser::add(line);
        this->_incrementBrowserSize();
    }

    // Scroll the browser to the bottom, as necessary
    Fl_Browser::bottomline(this->getBrowserSize());

    // Release the lock
    Fl::unlock();
    // Let the main thread know that the window should be redrawn
    Fl::awake();
}

void ServerWindowLogBrowser::replaceBottomLine(const char *line)
{
	// Wait for a lock on the GUI
	Fl::lock();

	// Replaces the text on the bottommost line
	Fl_Browser::text(this->getBrowserSize(), line);

	// Release lock, let main thread know
	Fl::unlock();
	Fl::awake();

}
