#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#endif
#include <asl/Directory.h>
#include <asl/Process.h>
#include <asl/TextFile.h>
#include <asl/Set.h>
#include <asl/Queue.h>
#include <stdio.h>
#ifdef __APPLE__
#include <sys/syslimits.h>
#endif
#ifdef _WIN32
#include <shlobj.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace asl {

String Directory::name() const
{
	return Path(_path).name();
}

String Directory::directory() const
{
	return Path(_path).directory().string();
}

bool Directory::create(const String& name)
{
	if (!name.ok())
		return false;
	if (createOne(name))
		return true;
	Path   path = Path(name).absolute();
	String parent = path.directory();
	if (Directory(parent).directory().ok())
		if (parent.ok() && !Directory(parent).exists())
		{
			create(parent);
		}

	return createOne(name);
}

String Directory::createTemp()
{
	String dir;
	unsigned num = (unsigned)(2e9 * fract(0.01 * now()));
	unsigned p = (unsigned int)(size_t)&dir;
#ifdef _WIN32
	wchar_t tmpdir[MAX_PATH];
	DWORD ret = GetTempPathW(MAX_PATH, tmpdir);
	String tmpDir = tmpdir;
	if (ret > MAX_PATH || ret == 0)
		return dir;
	unsigned pid = (unsigned int)GetCurrentProcessId();
#else
	String tmpDir = "/tmp";
	unsigned pid = (unsigned)getpid();
#endif
	do {
		dir = String::f("%s/%04x%08x%08x", *tmpDir, pid, p, num++);
	} while (File(dir).exists());
	create(dir);
	return dir;
}

bool Directory::removeRecursive(const String& path, bool onlyContent)
{
	if (!path.ok())
		return false;
	bool ok = true;
	String abs = Path(path).absolute().string().toLowerCase().replace('\\', '/');
	if (abs.endsWith('/'))
		abs.resize(abs.length() - 1);
	if (abs.length() == 2 && abs[1] == ':')
		return false;
	if (((abs.length() > 3 && abs.substr(3) == "windows") || abs.substr(3) == "program files") || !abs.ok())
		return false;
	Directory dir(path);
	foreach (File& file, dir.files())
		ok = ok && file.remove();
	foreach (File& d, dir.subdirs())
		ok = ok && removeRecursive(d.path());
	if (!onlyContent)
		ok = ok && remove(path);
	return ok;
}

bool File::copy(const String& to)
{
	return Directory::copy(_path, to);
}

bool File::move(const String& to)
{
	return Directory::move(_path, to);
}

}

#ifdef _WIN32
#undef WIN32_FIND_DATA
#undef FindFirstFile
#undef FindNextFile
#undef CreateDirectory
#undef SetCurrentDirectory
#ifndef ASL_ANSI
#define WIN32_FIND_DATA     WIN32_FIND_DATAW
#define FindFirstFile       FindFirstFileW
#define FindNextFile        FindNextFileW
#define CreateDirectory     CreateDirectoryW
#define SetCurrentDirectory SetCurrentDirectoryW
#define STR_PREFIX(x)       L##x
#define strcmpX             wcscmp
#else
#define WIN32_FIND_DATA     WIN32_FIND_DATAA
#define FindFirstFile       FindFirstFileA
#define FindNextFile        FindNextFileA
#define CreateDirectory     CreateDirectoryA
#define SetCurrentDirectory SetCurrentDirectoryA
#define STR_PREFIX(x)       x
#define strcmpX             strcmp
#endif

namespace asl
{

inline double ft2t(const FILETIME& ft)
{
	LARGE_INTEGER t;
	t.HighPart = ft.dwHighDateTime;
	t.LowPart = ft.dwLowDateTime;
	return t.QuadPart * 100e-9 - 11644473600.0;
}

#define nat(x) x

static FileInfo infoFor(const WIN32_FIND_DATA& data)
{
	FileInfo info;
	info.lastModified = ft2t(data.ftLastWriteTime);
	info.creationDate = ft2t(data.ftCreationTime);
	info.size = data.nFileSizeLow + ((Long)data.nFileSizeHigh << 32);
	info.flags = data.dwFileAttributes;
	return info;
}

Array<File> Directory::items(const String& which, Directory::ItemType t)
{
	_files = Array<File>();

	if (File(_path).isFile()) // if path is a file, return that file only
	{
		return _files << File(_path);
	}

	if (which.contains('|')) // handle multiple patterns separated by '|'
	{
		Array<File>   all;
		Array<String> parts = which.split('|');
		foreach (const String& part, parts)
		{
			all.append(items(part, t));
		}
		_files = all;
		return all;
	}
	WIN32_FIND_DATA data;
	String          basedir = (_path.endsWith('/') || _path.endsWith('\\')) ? nat(_path) : nat(_path) + '/';
	String          name;
	HANDLE          hdir = FindFirstFile(basedir + which, &data);
	if (hdir == INVALID_HANDLE_VALUE)
		return _files;
	do
	{
		if ((t == DIRE && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) ||
		    (t == FILE && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0))
			continue;
		if (t == DIRE && (!strcmpX(data.cFileName, STR_PREFIX(".")) || !strcmpX(data.cFileName, STR_PREFIX(".."))))
			continue;
		name = basedir;
		name += String(data.cFileName);
		_files << File(name, infoFor(data));
	} while (FindNextFile(hdir, &data));
	FindClose(hdir);
	return _files;
}

FileInfo Directory::getInfo(const String& path)
{
	WIN32_FIND_DATA data;
	HANDLE          hdir = FindFirstFile(nat(path), &data);
	if (hdir == INVALID_HANDLE_VALUE)
	{
		FileInfo info;
		return info;
	}
	FindClose(hdir);
	return infoFor(data);
}

Array<String> Directory::roots()
{
	DWORD         bits = GetLogicalDrives();
	char          d = 'A';
	Array<String> drives;
	for (int i = 0; i < 32; i++, d++)
	{
		if ((bits & 1) != 0)
			drives << String(d) + ":/";
		bits >>= 1;
	}
	return drives;
}

bool Directory::createOne(const String& name)
{
	bool ok = CreateDirectory(name, 0) != 0;
	return ok || (GetLastError() == ERROR_ALREADY_EXISTS); // if it exists we don't consider that an error
}

String Directory::current()
{
	wchar_t buffer[1024];
	GetCurrentDirectoryW(1024, buffer);
	return buffer;
}

bool Directory::change(const String& dir)
{
	return SetCurrentDirectory(dir) != 0;
}

bool Directory::copy(const String& from, const String& to, bool onlyContent)
{
	String dst = to;
	File   tofile(to);
	if (!onlyContent && tofile.isDirectory())
		dst << '/' << File(from).name();
	if (File(from).isDirectory())
	{
		bool ok = true;
		if (!Directory(dst).exists())
			Directory::create(dst);
		Array<File> items = Directory(from).items();
		foreach (File& item, items)
		{
			if (item.name() == "." || item.name() == "..")
				continue;
			String itemDst = dst + '/' + item.name();
			ok &= copy(item.path(), itemDst);
		}
		return ok;
	}

	return CopyFileW(from, dst, FALSE) != 0;
}

bool Directory::move(const String& from, const String& to)
{
	String dst = to;
	File   tofile(to);
	if (tofile.isDirectory())
		dst = to + '/' + File(from).name();

	return MoveFileW(from, dst) != 0;
}

bool Directory::remove(const String& path)
{
	if (File(path).isDirectory())
		return RemoveDirectoryW(path) != 0;
	else
		return DeleteFileW(path) != 0;
}

struct WaitData
{
	HANDLE          hdir;
	ByteArray       buffer;
	Set<File>       items;
	Queue<DirEvent> events;
};

String Directory::special(Place p)
{
	switch (p)
	{
	case TEMP:
	case MYTEMP:
	{
		wchar_t tmpdir[MAX_PATH];
		DWORD   ret = GetTempPathW(MAX_PATH, tmpdir);
		return tmpdir;
	}
	case DESKTOP:
	{
		wchar_t path[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, path, CSIDL_DESKTOP, FALSE))
			return path;
		break;
	}
	case DOCUMENTS:
	{
		wchar_t path[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, path, CSIDL_PERSONAL, FALSE) && path[0] != 0)
			return path;
		else
			return special(HOME) + "/Documents";
		break;
	}
	case DOWNLOAD:
	{
		// wchar_t path[MAX_PATH];
		// if (SHGetSpecialFolderPathW(NULL, path, CSIDL_DOWNLOADS, FALSE))
		//	return path;
		return special(HOME) + "/Downloads"; // TODO use newer API
		break;
	}
	case APPS:
	{
		wchar_t path[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, path, CSIDL_PROGRAM_FILES, FALSE))
			return path;
		break;
	}
	case APPDATA:
	{
		wchar_t path[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, path, CSIDL_LOCAL_APPDATA, FALSE))
			return path;
		break;
	}
	case APPCONFIG:
	{
		wchar_t path[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, path, CSIDL_APPDATA, FALSE))
			return path;
		break;
	}
	case APPDATA_ALL:
	{
		wchar_t path[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, path, CSIDL_COMMON_APPDATA, FALSE) && path[0] != 0)
			return path;
		return Process::env("ProgramData") | "C:/ProgramData";
		break;
	}
	case HOME:
	{
		wchar_t path[MAX_PATH];
		if (SHGetSpecialFolderPathW(NULL, path, CSIDL_PROFILE, FALSE))
			return path;
		break;
	}
	default:
		break;
	}
	return String();
}

DirEvent Directory::wait()
{
	if (!_waitdata)
	{
		_waitdata = new WaitData;
		_waitdata->hdir = CreateFileW(_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, NULL,
		                              OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		_waitdata->buffer.resize(1000);
	}

	if (!_waitdata->events.empty())
		return _waitdata->events.get();

	DWORD n = 0;
	ReadDirectoryChangesW(_waitdata->hdir, _waitdata->buffer.data(), _waitdata->buffer.length(), FALSE,
	                      FILE_NOTIFY_CHANGE_FILE_NAME, &n, NULL, NULL);

	DWORD c = 0;
	while (n == 0 && c++ < 10)
	{
		ReadDirectoryChangesW(_waitdata->hdir, _waitdata->buffer.data(), _waitdata->buffer.length(), FALSE,
		                      FILE_NOTIFY_CHANGE_FILE_NAME, &n, NULL, NULL);
		if (n == 0)
			_waitdata->buffer.resize(_waitdata->buffer.length() * 2);
	};

	FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)_waitdata->buffer.data();

	// RENAME => FILE_ACTION_RENAMED_OLD_NAME followed by FILE_ACTION_RENAMED_NEW_NAME

	while (1)
	{
		int            action = info->Action;
		Array<wchar_t> wname(info->FileName, info->FileNameLength / sizeof(wchar_t));
		wname << 0;
		String name = wname.data();

		DirEvent item;
		item.name = name;
		item.type = action == FILE_ACTION_ADDED     ? DirEvent::NEW_FILE
		            : action == FILE_ACTION_REMOVED ? DirEvent::DELETED
		                                            : DirEvent::MODIFIED;

		if (action == FILE_ACTION_RENAMED_OLD_NAME)
			item.type = DirEvent::DELETED;
		else if (action == FILE_ACTION_RENAMED_NEW_NAME)
			item.type = DirEvent::NEW_FILE;

		_waitdata->events.put(item);

		if (info->NextEntryOffset == 0)
			break;
		info = (FILE_NOTIFY_INFORMATION*)((byte*)info + info->NextEntryOffset);
	}

	return !_waitdata->events.empty() ? _waitdata->events.get() : DirEvent();
}

Directory::Space Directory::freeSpace(const String& dir)
{
	Space          space = { 0, 0 };
	ULARGE_INTEGER freeBytesToCaller, totalBytes, totalFreeBytes;
	if (!GetDiskFreeSpaceExW(dir, &freeBytesToCaller, &totalBytes, &totalFreeBytes))
		return space;

	space.free = (Long)totalFreeBytes.QuadPart;
	space.total = (Long)totalBytes.QuadPart;

	return space;
}

}
#else // Linux

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#ifdef __linux__
#include <features.h>
#if (__GLIBC__ * 1000 + __GLIBC_MINOR__) > 2004
#define ASL_HAVE_INOTIFY
#include <sys/inotify.h>
#endif
#endif
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

namespace asl
{

struct WaitData
{
	int             inotfd;
	int             watch_desc;
	ByteArray       buffer;
	Set<File>       items;
	Queue<DirEvent> events;
};

inline double ft2t(const time_t& ft)
{
	return ft; // TODO! Date(time_t)
}

static FileInfo infoFor(const struct stat& data)
{
	FileInfo info;
	info.lastModified = ft2t(data.st_mtime);
	info.creationDate = ft2t(data.st_ctime);
	info.size = data.st_size;
	info.flags = data.st_mode;
	return info;
}

static bool match(const String& a, const String& patt)
{
	int i = patt.indexOf('*');
	if (i == -1)
		return a == patt;
	return a.startsWith(patt.substring(0, i)) && a.endsWith(patt.substring(i + 1));
}

Array<File> Directory::items(const String& which, Directory::ItemType t)
{
	_files = Array<File>();
	if (File(_path).isFile())
	{
		return _files << File(_path);
	}

	if (which.contains('|'))
	{
		Array<File>   all;
		Array<String> parts = which.split('|');
		foreach (const String& part, parts)
		{
			all.append(items(part, t));
		}
		_files = all;
		return all;
	}

	DIR* d = opendir(_path.ok() ? *_path : "/");
	if (!d)
		return _files;
	String dir = _path.endsWith('/') ? _path : (_path + '/');
	String name;
	bool   wildcard = (which.contains('*') && which != "*") || !which.contains('*');

	while (dirent* entry = readdir(d))
	{
		struct stat data;
		if (wildcard && !match(entry->d_name, which))
			continue;
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;
		name = dir;
		name += (const char*)entry->d_name;
		if (!stat(name, &data))
		{
			if ((t == DIRE && !S_ISDIR(data.st_mode)) || (t == FILE && S_ISDIR(data.st_mode)))
				continue;
			_files << File(name, infoFor(data));
		}
	}
	closedir(d);
	return _files;
}

FileInfo Directory::getInfo(const String& path)
{
	struct stat data;
	if (stat(path, &data))
	{
		FileInfo info;
		return info;
	}
	return infoFor(data);
}

Array<String> Directory::roots()
{
	return array<String>("/");
}

bool Directory::createOne(const String& name)
{
	int a = mkdir(name, 0777);
	return a == 0 || (a < 0 && errno == EEXIST);
}

String Directory::current()
{
	String dir;
	char*  d = getcwd(SafeString(dir, PATH_MAX), PATH_MAX);
	return d ? dir : "";
}

bool Directory::change(const String& dir)
{
	return chdir(dir) == 0;
}

bool Directory::copy(const String& from, const String& to, bool onlyContent)
{
	File src(from, File::READ);
	if (!src)
		return false;
	String dst = to;
	File   tofile(to);
	if (!onlyContent && tofile.isDirectory())
		dst << '/' << File(from).name();
	if (File(from).isDirectory())
	{
		bool ok = true;
		if (!Directory(dst).exists())
			Directory::create(dst);
		Array<File> items = Directory(from).items();
		foreach (File& item, items)
		{
			if (item.name() == "." || item.name() == "..")
				continue;
			String itemDst = dst + '/' + item.name();
			ok &= copy(item.path(), itemDst);
		}
		return ok;
	}

	tofile = File(dst);
	if (!tofile.open(File::WRITE))
		return false;

	byte buffer[65536];
	int  n = 0;
	do
	{
		n = src.read(buffer, sizeof(buffer));
		if (n < 0)
			return false;
		int m = tofile.write(buffer, n);
		if (m != n)
			return false;
	} while (n == sizeof(buffer));
	return true;
}

bool Directory::move(const String& from, const String& to)
{
	String dst = to;
	File   tofile(to);
	if (tofile.isDirectory())
		dst = to + '/' + File(from).name();

	if (rename(from, dst) == 0)
		return true;
	if (errno == EXDEV) // different file systems: copy and del
	{
		copy(from, dst);
		remove(from);
	}
	return false;
}

bool Directory::remove(const String& path)
{
	if (File(path).isDirectory())
		return rmdir(path) == 0;
	else
		return unlink(path) == 0;
}

Directory::Space Directory::freeSpace(const String& dir)
{
	Space          space = { 0, 0 };
	struct statvfs buf;
	if (statvfs(dir, &buf) != 0)
		return space;

	space.free = (Long)buf.f_frsize * buf.f_bavail;
	space.total = (Long)buf.f_frsize * buf.f_blocks;

	return space;
}

String Directory::special(Place p)
{
	if (p == TEMP)
	{
		String tmp = Process::env("TMPDIR");
		if (!tmp.ok())
			tmp = "/tmp";
		return tmp;
	}
	String home = Process::env("HOME");
	if (p == HOME)
		return home;

	String xdg = home + "/.config/user-dirs.dirs";
	Dic<String> dirs;
	if (File(xdg).isFile())
	{
		TextFile f(xdg, File::READ);
		while (!f.end())
		{
			String        line = f.readLine();
			if (line.startsWith("#"))
				continue;
			Array<String> parts = line.split('=');
			if (parts.length() == 2)
				dirs[parts[0]] = parts[1].replace('"', "").replace("$HOME", home);
		}
	}

	switch (p)
	{
	case DESKTOP:
		return dirs["XDG_DESKTOP_DIR"] | (home + "/Desktop");
	case DOCUMENTS:
		return dirs["XDG_DOCUMENTS_DIR"] | (home + "/Documents");
	case DOWNLOAD:
		return dirs["XDG_DOWNLOAD_DIR"] | (home + "/Downloads");
	case APPS:
		return dirs["XDG_APPLICATIONS_DIR"] | (home + "/Applications");
	case APPDATA:
		return home + "/.local/share";
		break;
	case APPCONFIG:
		return home + "/.config";
	case APPDATA_ALL:
		return "/var/lib";
	case MYTEMP:
		return dirs["XDG_RUNTIME_DIR"] | ("/tmp");
		break;
	default:
		break;
	}
	return String();
}

DirEvent Directory::wait()
{
#ifdef ASL_HAVE_INOTIFY
	if (!File(_path).isDirectory())
		return DirEvent();
	if (_path.startsWith("/proc") || _path.startsWith("/sys") || _path.startsWith("/mnt"))
		return waitPoll();

	if (!_waitdata)
	{
		_waitdata = new WaitData;
		int bufsiz = 4 * (sizeof(inotify_event) + PATH_MAX + 1);
		_waitdata->buffer.resize(bufsiz);
		_waitdata->inotfd = inotify_init();
		_waitdata->watch_desc = inotify_add_watch(_waitdata->inotfd, *_path, /*IN_MODIFY |*/ IN_CREATE | IN_DELETE | IN_MOVE);
	}
	
	if (!_waitdata->events.empty())
		return _waitdata->events.get();

	inotify_event* event = (inotify_event*)_waitdata->buffer.data();
	unsigned       lastmask = 0;
	while (1)
	{
		int n = read(_waitdata->inotfd, event, _waitdata->buffer.length());
		if (n <= 0)
		{
			break;
		}
		
		for (byte* ptr = _waitdata->buffer.data(); ptr < _waitdata->buffer.data() + n; ptr += sizeof(struct inotify_event) + event->len)
		{
			inotify_event* event = (inotify_event*)ptr;
			// printf("inotify: cookie %u mask %u name %s\n", event->cookie, event->mask, event->name);
			DirEvent item;
			lastmask = event->mask;
			item.name = event->name;
			item.type = event->mask == IN_CREATE   ? DirEvent::NEW_FILE
			            : event->mask == IN_DELETE ? DirEvent::DELETED
			                                       : DirEvent::MODIFIED;
			if (event->mask == IN_MOVED_FROM)
				item.type = DirEvent::DELETED;
			else if (event->mask == IN_MOVED_TO)
				item.type = DirEvent::NEW_FILE;
			_waitdata->events.put(item);
		}
		break;
	}
	return !_waitdata->events.empty() ? _waitdata->events.get() : DirEvent();
#else
	return waitPoll();
#endif
}

}
#endif

namespace asl
{

DirEvent Directory::waitPoll()
{
	if (!_waitdata)
	{
		_waitdata = new WaitData;
		_waitdata->items = files();
	}

	if (!_waitdata->events.empty())
		return _waitdata->events.get();

	while (1)
	{
		Set<File> items = files();

		if (items != _waitdata->items)
		{
			Set<File> diff = _waitdata->items - items;

			foreach (File& file, diff)
			{
				DirEvent item;
				item.name = file.name();
				item.type = DirEvent::DELETED;
				_waitdata->events.put(item);
			}
			
			diff = items - _waitdata->items;

			foreach (File& file, diff)
			{
				DirEvent item;
				item.name = file.name();
				item.type = DirEvent::NEW_FILE;
				_waitdata->events.put(item);
			}
		}

		_waitdata->items = items;

		sleep(0.2);
	}

	return !_waitdata->events.empty() ? _waitdata->events.get() : DirEvent();
}

int Directory::numEvents() const
{
	return _waitdata ? _waitdata->events.length() : 0;
}

Directory::~Directory()
{
	if (_waitdata)
	{
#ifdef _WIN32
		CloseHandle(_waitdata->hdir);
#elif defined(ASL_HAVE_INOTIFY)
		inotify_rm_watch(_waitdata->inotfd, _waitdata->watch_desc);
		close(_waitdata->inotfd);
#endif
	}
	delete _waitdata;
}
}
