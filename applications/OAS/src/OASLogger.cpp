/**
 * @file OASLogger.cpp
 * @author Shreenidhi Chowkwale
 */

#include "OASLogger.h"

using namespace oas;

// public, static
void Logger::logf(const char *message, ...)
{
    char buf[MAX_LOG_MESSAGE_SIZE];
    va_list args;

    va_start(args, message);
    sprintf(buf, "%s%s", ServerWindow::getNullBrowserFormatter(), 
                         message ? message : "(null)");
    Logger::_sendFormattedOutput(buf, args);
    va_end(args);
    
}

// public, static
void Logger::warnf(const char *message, ...)
{
    char buf[MAX_LOG_MESSAGE_SIZE];
    va_list args;

    va_start(args, message);
    sprintf(buf, "%s%sWARNING: %s", ServerWindow::getItalicsBrowserFormatter(),
                                    ServerWindow::getNullBrowserFormatter(),
                                    message ? message : "(null)");
    Logger::_sendFormattedOutput(buf, args);
    va_end(args);   
}

// public, static
void Logger::errorf(const char *message, ...)
{
    char buf[MAX_LOG_MESSAGE_SIZE];
    va_list args;

    va_start(args, message);
    sprintf(buf, "%s%sERROR: %s", ServerWindow::getBoldBrowserFormatter(),
                                  ServerWindow::getNullBrowserFormatter(),
                                  message ? message : "(null)");
    Logger::_sendFormattedOutput(buf, args);
    va_end(args);
    
}

// public, static
void Logger::error(const char *message)
{
    int errorNumber;
    char errBuf[MAX_STRERROR_BUF_SIZE];

    errorNumber = errno;
    // Use strerror_r to get the error string for the errno value
    Logger::errorf("%s: %s", message, strerror_r(errorNumber, errBuf, MAX_STRERROR_BUF_SIZE));
}

// public, static
void Logger::logReplaceBottomLine(const char *message, ...)
{
    char buf[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    
    va_start(args, message);
    sprintf(buf, "%s%s", ServerWindow::getNullBrowserFormatter(),
                         message ? message : "(null)");

    Logger::_replaceBottomLineFormattedOutput(buf, args);
    va_end(args);
}

// private, static
void Logger::_sendFormattedOutput(const char *format, va_list args)
{
    char buf[MAX_LOG_MESSAGE_SIZE * 2];

    vsprintf(buf, format, args);

    // If the server window is initialized, output to it
    if (ServerWindow::isInitialized())
    {
        ServerWindow::addToBrowser(buf);
    }
    else
    {
        Logger::_sendToConsole(buf);
    }
}

// private, static
void Logger::_replaceBottomLineFormattedOutput(const char *format, va_list args)
{
    char buf[MAX_LOG_MESSAGE_SIZE * 2];
    
    vsprintf(buf, format, args);
    
    if (ServerWindow::isInitialized())
    {
        ServerWindow::replaceBottomLine(buf);
    }   
    else
    {
        Logger::_sendToConsole(buf);
    }
}

// private, static
void Logger::_sendToConsole(const char *buf)
{
    if (!buf)
        return;
   
    // We have to skip over the formatting characters, if any
    // We assume that the formatters are terminated by a null formatter
    const char *ptr = strstr(buf, ServerWindow::getNullBrowserFormatter());

    // If we didn't find the null formatter, then just print the string as is
    if (!ptr)
        std::cerr << buf << std::endl;
    else
        std::cerr << (ptr + ServerWindow::getBrowserFormatterLength()) << std::endl;
}

