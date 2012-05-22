#include "OASServerWindowLogBrowser.h"

using namespace oas;

const unsigned int ServerWindowLogBrowser::_kMaxLogLineLength = 1000;


ServerWindowLogBrowser::ServerWindowLogBrowser(int X, int Y, int W, int H, const char *L)
: Fl_Browser(X, Y, W, H, L)
{
	// The Fl_Browser super class's constructor is called first, via initializor list
	this->_browserSize = 1;
}

void ServerWindowLogBrowser::add(const char *line)
{
	if (!line)
		return;

	static char buffer[_kMaxLogLineLength];

	// Set buffer contents to 0
	bzero(buffer, _kMaxLogLineLength);

	// Copy line contents into buffer
	strncpy(buffer, line, _kMaxLogLineLength - 1);

    // Wait for a lock on the GUI environment in case other threads are using it
    Fl::lock();

    // FlBrowser does not like newline characters in the string.
    // So, we split the string into multiple lines for each linefeed character
    // that is found.
    char *pNewline, *pStr = buffer;
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

    // Add whatever is left, after skipping newline characters
    if ('\0' != *pStr)
    {
        Fl_Browser::add(pStr);
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

	// Replaces the text on the bottom line
	Fl_Browser::text(this->getBrowserSize(), line);

	// Release lock, let main thread know
	Fl::unlock();
	Fl::awake();
}

void ServerWindowLogBrowser::copyToClipboard()
{
    unsigned long bufferLength = 1;
    char *pLine, *pBuffer, *pMover;

    // Wait for a lock on the GUI
    Fl::lock();

    // This loop calculates the size of the buffer that will need to be allocated
    for (void *i = item_first(); i; i = item_next(i))
    {
        pLine = (char *) strstr(item_text(i), ServerWindowLogBrowser::getNullBrowserFormatter());
        if (pLine)
        {
            pLine += ServerWindowLogBrowser::getBrowserFormatterLength();
            bufferLength += strlen(pLine) + 1;  // Plus one, for the new line character at the end
        }
    }

    // Allocate buffer
    pBuffer = new char[bufferLength];
    pMover = pBuffer;

    // Copy contents of browser into the buffer
    for (void *i = item_first(); i; i = item_next(i))
    {
        pLine = (char *) strstr(item_text(i), ServerWindowLogBrowser::getNullBrowserFormatter());
        if (pLine)
        {
            pLine += ServerWindowLogBrowser::getBrowserFormatterLength();
            sprintf(pMover, "%s\n", pLine);
            pMover += strlen(pLine) + 1;
        }
    }

    // Make sure it is null terminated
    pBuffer[bufferLength - 1] = '\0';

    Fl::copy(pBuffer, bufferLength, 1);

    delete[] pBuffer;

    // Release GUI lock
    Fl::unlock();
}
