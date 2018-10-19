#ifndef _ASL_TIME_H_
#define _ASL_TIME_H_

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace asl {

/**
 * \defgroup Global Global functions
 * @{
 */

/**
Returns the current time as a real number in seconds since a fixed epoch.
It has a platform-dependent precision, usually around the microsecond.
*/
inline double now()
{
#ifdef _WIN32
	static double qpcPeriod=0;
	if(qpcPeriod==0)
	{
		LARGE_INTEGER n;
		QueryPerformanceFrequency(&n);
		qpcPeriod = 1.0/n.QuadPart;
	}
	LARGE_INTEGER n;
	QueryPerformanceCounter(&n);
	return ((double)(n.QuadPart))*qpcPeriod;
	// return 0.001*GetTickCount();
#else
	timeval t;
	gettimeofday(&t, 0);
	return t.tv_sec + 1e-6*t.tv_usec;
#endif
}

// current time in us

Long ASL_API inow();

#ifdef _WIN32

/*
Makes the current thread sleep for the given number of seconds
*/
inline void sleep(int s)
{
	Sleep(s*1000);
}

/**
Makes the current thread sleep for the given number of microseconds
*/
inline void usleep(int us)
{
	Sleep(us/1000);
}

/**
Makes the current thread sleep for the given number of seconds
*/
inline void sleep(double s)
{
	Sleep((int)(1e3*s));
}
#else

/**
Makes the current thread sleep for the given number of seconds
*/
inline void sleep(int s)
{
	::sleep((unsigned)s);
}

/**
Makes the current thread sleep for the given number of seconds 
*/
inline void sleep(double s)
{
	usleep((int)(1e6*s));
}
#endif

/**@}*/
}
#endif
