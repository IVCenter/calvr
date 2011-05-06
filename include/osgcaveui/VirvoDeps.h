#ifndef VIRVO_DEPS_H
#define VIRVO_DEPS_H

/**
 * Contain the few things from virvo used by osgcaveui
 */

#include "cvrvvtoolshed.h"
#include <sys/time.h>


using namespace std;


class vvStopwatch
{
  private:

#ifdef WIN32
    clock_t baseTime;                             ///< system time when stop watch was triggered last
    bool useQueryPerformance;                     ///< true=use QueryPerformance API
    LARGE_INTEGER baseTimeQP;                     ///< base time when using QueryPerformance API
    LARGE_INTEGER freq;                           ///< frequency if QueryPerformance API is used
#else
    timeval baseTime;                             ///< system time when stop watch was triggered last
#endif
    float lastTime;

  public:
    vvStopwatch();
    void  start();
    float getTime();
    float getDiff();
};


#endif
