#include <asl/Log.h>
#include <asl/Console.h>
#include <asl/Date.h>
#include <asl/TextFile.h>
#include <asl/Path.h>
#include <asl/Directory.h>
#include <asl/Thread.h>
#include <asl/Process.h>

#define ASL_LOG_MAX_SIZE 1000000

namespace asl {

Log::Log()
{
	_logfile = "log.txt";
	_useconsole = true;
	_usefile = true;
	_maxLevel = 4;
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

#ifndef __ANDROID_API__

void Log::log(const String& cat, Log::Level level, const String& message)
{
	updateState();
	if (level > _maxLevel)
		return;
#ifndef __ANDROID_API__
	String logfile = _logfile;
	if (_usefile && TextFile(logfile).size() > ASL_LOG_MAX_SIZE) {
		Path path = logfile;
		String oldfile = path.noExt() + "-1." + path.extension();
		if (File(oldfile).exists())
			Directory::remove(oldfile);
		Directory::move(logfile, oldfile);
	}
#endif
	bool colorchanged = false;
	const char* slevel = "";

	Date now = Date::now();

	// remove directory and extension, so __FILE__ can be used as category
	int slash = max(cat.lastIndexOf('\\'), cat.lastIndexOf('/'));
	int i0 = (slash<0) ? 0 : slash + 1;
	int dot = cat.lastIndexOf('.');
	int i1 = (dot<0) ? cat.length() : dot;
	String catg = cat.substring(i0, i1);
	bool useconsole = _useconsole;

	Lock lock(*_mutex);

	switch (level)
	{
	case Log::WARNING:
		colorchanged = true;
		slevel = "WARNING: ";
		if (useconsole)
			console.color(Console::BYELLOW);
		break;
	case Log::ERR:
		colorchanged = true;
		slevel = "ERROR: ";
		if (useconsole)
			console.color(Console::BRED);
		break;
	default: break;
	}

	String line(0, "[%s][%s] %s%s\n", *now.toString(), *catg, slevel, *message);

	if (message.endsWith('\n'))
		line.resize(line.length() - 1);

	if (_usefile)
		TextFile(_logfile).append(line);

	if (useconsole)
	{
		printf("%s", *line);
		if (colorchanged)
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
	case Log::DEBUG1:
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
