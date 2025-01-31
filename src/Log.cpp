#include <asl/Log.h>
#include <asl/Console.h>
#include <asl/Date.h>
#include <asl/TextFile.h>
#include <asl/Path.h>
#include <asl/Directory.h>
#include <asl/Mutex.h>
#include <asl/Process.h>
#include <stdarg.h>

#define ASL_LOG_MAX_SIZE 1000000

#ifdef _MSC_VER
#pragma warning(disable : 26812)
#endif

namespace asl {

Log::Log()
{
	_logfile = "log.log";
	_useconsole = true;
	_usefile = true;
	_maxLevel = 2;
	_mutex = new Mutex;
}

Log::~Log()
{
	delete _mutex;
}

void Log::setFile(const String& file)
{
	Log::instance()->_logfile = file;
	Log::instance()->storeState();
}

void Log::enable(bool on)
{
	if ((!on && Log::instance()->_maxLevel < 0) || (on && Log::instance()->_maxLevel >= 0))
		return;
	Log::instance()->_maxLevel = -Log::instance()->_maxLevel - 1;
	Log::instance()->storeState();
}

void Log::useConsole(bool on)
{
	Log::instance()->_useconsole = on;
	Log::instance()->storeState();
}

void Log::useFile(bool on)
{
	Log::instance()->_usefile = on;
	Log::instance()->storeState();
}

void Log::setMaxLevel(int level)
{
	Log::instance()->_maxLevel = level;
	Log::instance()->storeState();
}

int Log::maxLevel()
{
	Log::instance()->updateState();
	return Log::instance()->_maxLevel;
}

void Log::storeState()
{
#ifndef __ANDROID_API__
	String value;
	int flags = (_usefile ? 1 : 0) | (_useconsole ? 2 : 0);
	value << char(_maxLevel + '0') << char(flags + '0') << _logfile;
	Process::setEnv("ASL_LOG", value);
#endif
}

void Log::updateState()
{
#ifndef __ANDROID_API__
	String s = Process::env("ASL_LOG");
	if (s.length() > 2)
	{
		_maxLevel = s[0] - '0';
		int flags = s[1] - '0';
		_usefile = (flags & 1) != 0;
		_useconsole = (flags & 2) != 0;
		_logfile = &s[2];
	}
#endif
}

void log(const String& cat, Log::Level level, const String& message)
{
	Log::instance()->log(cat, level, message);
}

void log(const String& cat, Log::Level level, ASL_PRINTF_W1 const char* fmt, ...)
{
	String message(100, 0);
	va_list arg;
	va_start(arg, fmt);
	int i = 0, n = 0;
	int space = message.cap();
	while (((n = vsnprintf(message.data(), space, fmt, arg)) == -1 || n >= space) && ++i < 10)
	{
		message.resize((n >= space) ? n : 2 * space, false);
		space = message.cap();
		va_end(arg);
		va_start(arg, fmt);
	}
	va_end(arg);
	message.fix(n);

	Log::instance()->log(cat, level, message);
}


#ifndef __ANDROID_API__

void Log::log(const String& cat, Log::Level level, const String& message)
{
	updateState();
	if (level > _maxLevel)
		return;

	Date now = Date::now();

	// remove directory and extension, so __FILE__ can be used as category
	int slash = max(cat.lastIndexOf('\\'), cat.lastIndexOf('/'));
	int i0 = (slash<0) ? 0 : slash + 1;
	int dot = cat.lastIndexOf('.');
	int i1 = (dot<0) ? cat.length() : dot;
	String catg = cat.substring(i0, i1);
	bool useconsole = _useconsole;

	Lock lock(*_mutex);

#ifndef __ANDROID_API__
	String logfile = _logfile;
	if (_usefile && TextFile(logfile).size() > ASL_LOG_MAX_SIZE)
	{
		Path   path = logfile;
		String oldfile = path.noExt() + "-1." + path.extension();
		if (File(oldfile).exists())
			Directory::remove(oldfile);
		Directory::move(logfile, oldfile);
	}
#endif

	const char* slevel = "";
	Console::Color color = Console::COLOR_DEFAULT;

	switch (level)
	{
	case Log::WARNING:
		slevel = "WARNING: ";
		color = Console::BYELLOW;
		break;
	case Log::ERR:
		slevel = "ERROR: ";
		color = Console::BRED;
		break;
	case Log::DEBUG:
		color = Console::GREEN;
		break;
	case Log::VERBOSE:
		color = Console::CYAN;
		break;
	default:
		break;
	}

	if (useconsole && color != Console::COLOR_DEFAULT)
		console.color(color);

	String line(0, "[%s][%s] %s%s\n", *now.toString(), *catg, slevel, *message);

	if (message.endsWith('\n'))
		line.resize(line.length() - 1);

	if (_usefile)
		TextFile(_logfile).append(line);

	if (useconsole)
	{
		printf("%s", *line);
		if (color != Console::COLOR_DEFAULT)
			console.color();
	}
}

#else // ANDROID

#include <android/log.h>

void Log::log(const String& cat, Log::Level level, const String& message)
{
	if (level > _maxLevel)
		return;

	// remove directory and extension, so __FILE__ can be used as category
	int slash = max(cat.lastIndexOf('\\'), cat.lastIndexOf('/'));
	int i0 = (slash < 0) ? 0 : slash + 1;
	int dot = cat.lastIndexOf('.');
	int i1 = (dot < 0) ? cat.length() : dot;
	String catg = cat.substring(i0, i1);

	int androidlevel;
	switch (level) {
	case Log::INFO:
		androidlevel = ANDROID_LOG_INFO;
		break;
	case Log::WARNING:
		androidlevel = ANDROID_LOG_WARN;
		break;
	case Log::ERR:
		androidlevel = ANDROID_LOG_ERROR;
		break;
	case Log::VERBOSE:
		androidlevel = ANDROID_LOG_VERBOSE;
		break;
	default:
		androidlevel = ANDROID_LOG_DEBUG;
		break;
	}

	__android_log_print(androidlevel, *catg, "%s", *message);
}

#endif

}
