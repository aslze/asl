#ifndef _ASL_TIME_H_
#define _ASL_TIME_H_

#include "defs.h"
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
Returns the current time as a real number in seconds since a fixed epoch (precision is platform-dependent, often around a microsecond).
*/
double ASL_API now();

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
Makes the current thread sleep for the given time in seconds (can be fractional)
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
Makes the current thread sleep for the given time in seconds (can be fractional)
*/
inline void sleep(double s)
{
	usleep((int)(1e6*s));
}
#endif

/**@}*/
}
#endif
