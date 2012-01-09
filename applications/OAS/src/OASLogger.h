#ifndef _OAS_LOGGER_H_
#define _OAS_LOGGER_H_

#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include "OASServerWindow.h"
#include "OASFileHandler.h"


namespace oas
{
class Logger
{
#define MAX_LOG_MESSAGE_SIZE    512
#define MAX_STRERROR_BUF_SIZE   256

public:
    static void logf(const char *message, ...);
    static void warnf(const char *message, ...);
    static void errorf(const char *message, ...);
    static void error(const char *message);
    static void logReplaceBottomLine(const char *message, ...);

private:
    static void _sendFormattedOutput(const char *format, va_list args);
    static void _replaceBottomLineFormattedOutput(const char *format, va_list args);
};
}

#endif

