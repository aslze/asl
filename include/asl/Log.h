// Copyright(c) 1999-2024 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_LOGGER
#define ASL_LOGGER

#include <asl/defs.h>
#include <asl/Singleton.h>
#include <asl/String.h>

namespace asl {

class Mutex;

/**
Log is a utility to log messages to either the console, a file or both. Messages have a *category*
(usuarlly a module or area identifier) and a *level* (to distinguish errors, warnings,
information and debug messages), and are written together with the current date and time. By default
messages are written to a file named "log.log" and to the console (with errors and warnings colored).
These, as well as the maximum log level of messages logged, can be configured.

Default settings can be changed like this:

~~~
Log::setFile("app.log");     // write to this file (default is "log.log")
Log::useConsole(false);      // do not write to the console
Log::setMaxLevel(Log::INFO); // skip debug and verbose messages (default is DEBUG)
~~~

The easiest way to write messages is with the provided macro ASL_LOG_ similar to a `printf` call.
It uses the current source file as *category*.

~~~
ASL_LOG_(ERR, "Port %i already in use", port);
~~~

Writes something like:

~~~
[2021-03-18T12:35:00][Server] ERROR: Port 80 already in use
~~~

There are recommended shorter macros for each log level: ASL_LOG_X (with X one of E, W, I, D, V):

~~~
ASL_LOG_E("Port %i already in use", port);
~~~

On Android this class uses the system logger.

Log files do not grow indefinitely. When reaching about 1 MB, they will be moved to a file with "-1" appended
to its name (like "log-1.log"), and a new empty file will be started. Any logs older than that will be lost.
\ingroup Logging
*/

class ASL_API Log : public Singleton<Log>
{
	bool _useconsole;
	bool _usefile;
	String _logfile;
	int _maxLevel;
	Mutex* _mutex;
	void storeState();
	void updateState();
	Log(const Log&) : _mutex(0) { _maxLevel = 0; _useconsole = _usefile = false; }
	void operator=(const Log&) { _mutex = 0; _maxLevel = 0; _useconsole = _usefile = false;}
public:
	Log();
	~Log();
	
	/**
	Message levels (ERR, WARNING, INFO, DEBUG, VERBOSE).
	*/
	enum Level {
		ERR, WARNING, INFO, DEBUG, VERBOSE
	};

	friend ASL_API void log(const String& cat, Log::Level level, const String& message);
	friend ASL_API void log(const String& cat, Log::Level level, ASL_PRINTF_W1 const char* fmt, ...) ASL_PRINTF_W2(3);

	/**
	Sets the name of the file to write messages to.
	*/
	static void setFile(const String& file);
	/**
	Enables or disables logging.
	*/
	static void enable(bool on);
	/**
	Enables or disables writing messages to the console.
	*/
	static void useConsole(bool on);
	/**
	Enables or disables writing messages to a file.
	*/
	static void useFile(bool on);
	/**
	Sets the maximum level of messages to be logged; By default this is 3, so messages up to *level DEBUG* are logged.
	*/
	static void setMaxLevel(int level);
	/**
	Gets the current maximum log level
	*/
	static int maxLevel();

	void log(const String& cat, Log::Level level, const String& message);
};

	/**
	Writes a log message with the given category and log level.
\ingroup Logging
	*/
ASL_API void log(const String& cat, Log::Level level, const String& message);

/**
Writes a printf-like formatted log message with the given category and log level
\ingroup Logging
*/
ASL_API void log(const String& cat, Log::Level level, ASL_PRINTF_W1 const char* fmt, ...) ASL_PRINTF_W2(3);


#if (defined( _MSC_VER ) && _MSC_VER > 1310) || !defined(_MSC_VER)

/**
Log a formatted message with the given level (`ERR`, `WARNING`, `INFO`, `DEBUG` or `VERBOSE`)
\ingroup Logging
\hideinitializer
*/
#define ASL_LOG_(LEVEL, ...) asl::log(__FILE__, asl::Log::LEVEL, __VA_ARGS__)

/**
Log a formatted message with the ERROR level
\ingroup Logging
\hideinitializer
*/
#define ASL_LOG_E(...) asl::log(__FILE__, asl::Log::ERR, __VA_ARGS__)
/**
Log a formatted message with the WARNING level
\ingroup Logging
\hideinitializer
*/
#define ASL_LOG_W(...) asl::log(__FILE__, asl::Log::WARNING, __VA_ARGS__)
/**
Log a formatted message with the INFO level
\ingroup Logging
\hideinitializer
*/
#define ASL_LOG_I(...) asl::log(__FILE__, asl::Log::INFO, __VA_ARGS__)
/**
Log a formatted message with the DEBUG level
\ingroup Logging
\hideinitializer
*/
#define ASL_LOG_D(...) asl::log(__FILE__, asl::Log::DEBUG, __VA_ARGS__)
/**
Log a formatted message with the VERBOSE level
\ingroup Logging
\hideinitializer
*/
#define ASL_LOG_V(...) asl::log(__FILE__, asl::Log::VERBOSE, __VA_ARGS__)

#endif

#define ASL_LOG_WHERE_AM_I() ASL_LOG_(DEBUG, "At %s: %i [%s]", \
	*asl::String(__FILE__).split(ASL_PATH_SEP).last(), __LINE__, __FUNCTION__)
}

#endif
