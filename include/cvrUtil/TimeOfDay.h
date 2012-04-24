/**
 * @file TimeOfDay.h
 */
#ifndef TIME_OF_DAY_H
#define TIME_OF_DAY_H

#ifdef WIN32
#include <cvrUtil/Export.h>

#include <time.h>
#include <windows.h>

/**
 * @brief Substitute for linux function under windows
 */
CVRUTIL_EXPORT int gettimeofday(struct timeval *tv, void *tz);
#endif
#endif
