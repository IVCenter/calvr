#include <osgcaveui/VirvoDeps.h>

vvStopwatch::vvStopwatch()
{
#ifdef _WIN32
  baseTime = 0;
  baseTimeQP.QuadPart = 0;
  useQueryPerformance = (QueryPerformanceFrequency(&freq)) ? true : false;
#else
  baseTime.tv_sec  = 0;
  baseTime.tv_usec = 0;
#endif
  lastTime = 0.0f;
}

void vvStopwatch::start()
{
#ifdef _WIN32

  if (useQueryPerformance) QueryPerformanceCounter(&baseTimeQP);
  else baseTime = clock();

#elif defined(__linux__) || defined(LINUX) || defined(__APPLE__)

  struct timezone tz;
  gettimeofday(&baseTime, &tz);

#else

  void* v = NULL;
  gettimeofday(&baseTime, v);
#endif

  lastTime = 0.0f;
}

float vvStopwatch::getTime()
{
  float dt;                                       // measured time difference [seconds]

#ifdef _WIN32

  if (useQueryPerformance)
  {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    dt = float((float(now.QuadPart) - float(baseTimeQP.QuadPart)) / float(freq.QuadPart));
  }
  else
  {
    clock_t now = clock();
    dt = float(now - baseTime) / float(CLOCKS_PER_SEC);
  }

#else

#if defined(__linux__) || defined(LINUX) || defined(__APPLE__)
  struct timezone dummy;
#else
  void* dummy = NULL;
#endif
  timeval now;                                    // current system time

  gettimeofday(&now, &dummy);
  time_t sec  = now.tv_sec  - baseTime.tv_sec;
  long   usec = now.tv_usec - baseTime.tv_usec;
  dt   = (float)sec + (float)usec / 1000000.0f;
#endif

  lastTime = dt;
  return dt;
}

float vvStopwatch::getDiff()
{
  float last = lastTime;
  return getTime() - last;
}

