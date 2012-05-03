/**
 * @file OASServerWindowLogBrowser.h
 * @author Shreenidhi Chowkwale
 */

#ifndef _OAS_SERVER_WINDOW_LOG_BROWSER_H_
#define _OAS_SERVER_WINDOW_LOG_BROWSER_H_

#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <iostream>

namespace oas
{

class ServerWindowLogBrowser : public Fl_Browser
{

public:
	ServerWindowLogBrowser(int X, int Y, int W, int H, const char *L = 0);

	void add(char *line);
    void replaceBottomLine(const char *line);

    inline int getBrowserSize() const
    {
    	return _browserSize;
    }

    static inline const char* const getBoldBrowserFormatter()
    {
        return "@b";
    }

    static inline const char* const getItalicsBrowserFormatter()
    {
        return "@i";
    }

    static inline const char* const getNullBrowserFormatter()
    {
        return "@.";
    }

    static inline const int getBrowserFormatterLength()
    {
        return 2;
    }


protected:
    // Keeps track of the browser size
    unsigned int _browserSize;

private:
    inline void _incrementBrowserSize()
    {
    	_browserSize++;
    }
};

}

#endif
