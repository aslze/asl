#ifndef _ASL_TIME_H_
#define _ASL_TIME_H_

#include "defs.h"

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

/*
Makes the current thread sleep for the given time in seconds
*/
void ASL_API sleep(int s);

/**
Makes the current thread sleep for the given time in seconds (can be fractional)
*/
void ASL_API sleep(double s);

#ifdef _WIN32
/**
Makes the current thread sleep for the given time in microseconds
*/
void ASL_API usleep(int us);
#endif

/**@}*/
}
#endif
