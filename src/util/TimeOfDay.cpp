#include <util/TimeOfDay.h>

int gettimeofday(struct timeval *tv, void *tz)
{
	FILETIME ft;

	if(tv != NULL)
	{
		GetSystemTimeAsFileTime(&ft);

		unsigned __int64 tmp = 0;
		tmp |= ft.dwHighDateTime;
		tmp <<= 32;
		tmp |= ft.dwLowDateTime;

		tmp /= 10;

		tv->tv_sec = (long)(tmp / 1000000UL);
		tv->tv_usec = (long)(tmp % 1000000UL);
	}

	return 0;
}