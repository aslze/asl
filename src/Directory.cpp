#include <asl/Directory.h>
#include <asl/Path.h>
#include <stdio.h>

namespace asl {

String Directory::name() const
{
	return Path(_path).name();
}


String Directory::directory() const
{
	return Path(_path).directory().string();
}

String Directory::createTemp()
{
	String dir;
	unsigned num = (unsigned)(2e9 * fract(0.01 * now()));
	unsigned p = (unsigned int)(size_t)&dir;
#ifdef _WIN32
	TCHAR tmpdir[MAX_PATH];
	DWORD ret = GetTempPath(MAX_PATH, tmpdir);
	String tmpDir = tmpdir;
	if (ret > MAX_PATH || ret == 0)
		return dir;
	unsigned pid = (unsigned int)GetCurrentProcessId();
#else
	String tmpDir = "/tmp";
	unsigned pid = (unsigned)getpid();
#endif
	do {
		dir = String(0, "%s/%04x%08x%08x", *tmpDir, pid, p, num++);
	} while (File(dir).exists());
	create(dir);
	return dir;
}

bool File::copy(const String& to)
{
	return Directory::copy(_path, to);
}

bool File::move(const String& to)
{
	return Directory::move(_path, to);
}

bool File::remove()
{
	return Directory::remove(_path);
}

}

#ifdef _WIN32

#if !defined(ASL_ANSI)
#define UNICODE
#undef WIN32_FIND_DATA
#undef FindFirstFile
#undef FindNextFile
#undef CreateDirectory
#undef SetCurrentDirectory
#define WIN32_FIND_DATA WIN32_FIND_DATAW
#define FindFirstFile FindFirstFileW
#define FindNextFile FindNextFileW
#define CreateDirectory CreateDirectoryW
#define SetCurrentDirectory SetCurrentDirectoryW
#define STR_PREFIX(x) L##x
#define strcmpX wcscmp
#else
#define STR_PREFIX(x) x
#define strcmpX strcmp
#endif

namespace asl {

inline double ft2t(const FILETIME& ft)
{
	LARGE_INTEGER t;
	t.HighPart = ft.dwHighDateTime;
	t.LowPart = ft.dwLowDateTime;
	return t.QuadPart*100e-9 - 11644473600.0;
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

const Array<File>& Directory::items(const String& which, Directory::ItemType t)
{
	_files.clear();
	WIN32_FIND_DATA data;
	String basedir = nat(_path) + '/';
	HANDLE hdir = FindFirstFile(basedir + which, &data);
	if(hdir == INVALID_HANDLE_VALUE)
		return _files;
	do {
		if( (t==DIRE && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) ||
			(t==FILE && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) )
			continue;
		if(t==DIRE && (!strcmpX(data.cFileName, STR_PREFIX(".")) || !strcmpX(data.cFileName, STR_PREFIX(".."))))
			continue;
		String filename = basedir;
		filename << (String)data.cFileName;
		_files << File(filename, infoFor(data));
	}
	while(FindNextFile(hdir, &data));
	FindClose(hdir);
	return _files;
}

FileInfo Directory::getInfo(const String& path)
{
	WIN32_FIND_DATA data;
	HANDLE hdir = FindFirstFile(nat(path), &data);
	if(hdir == INVALID_HANDLE_VALUE)
	{
		FileInfo info;
		memset(&info, 0, sizeof(FileInfo));
		return info;
	}
	FindClose(hdir);
	return infoFor(data);
}

bool Directory::create(const String& name)
{
	return CreateDirectory(name, 0) != 0;
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

bool Directory::copy(const String& from, const String& to)
{
	String dst = to;
	File tofile(to);
	if(tofile.isDirectory())
		dst << '\\' << File(from).name();
	return CopyFileW(from, dst, FALSE) != 0;
}

bool Directory::move(const String& from, const String& to)
{
	String dst = to;
	File tofile(to);
	if(tofile.isDirectory())
		dst = to + '/' + File(from).name();

	return MoveFileW(from, dst) != 0;
}

bool Directory::remove(const String& path)
{
	if(File(path).isDirectory())
		return RemoveDirectoryW(path) != 0;
	else
		return DeleteFileW(path) != 0;
}


}
#else // Linux

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

namespace asl {

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
	if (i==-1)
		return a==patt;
	return a.startsWith(patt.substring(0,i)) && a.endsWith(patt.substring(i+1));
}

const Array<File>& Directory::items(const String& which, Directory::ItemType t)
{
	_files.clear();
	DIR* d = opendir(_path != ""? _path : "/");
	if(!d)
		return _files;
	String dir = _path+'/';
	bool wildcard = which.contains('*') && which != "*";
	
	while(dirent* entry=readdir(d))
	{
		struct stat data;
		if(wildcard && !match(entry->d_name, which))
			continue;
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;
		String name = dir;
		name += (const char*)entry->d_name;
		if(!stat(name, &data)) {
			if( (t==DIRE && !S_ISDIR(data.st_mode)) ||
				(t==FILE && S_ISDIR(data.st_mode)) )
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
	if(stat(path, &data))
	{
		FileInfo info;
		memset(&info, 0, sizeof(FileInfo));
		return info;
	}
	return infoFor(data);
}

bool Directory::create(const String& name)
{
	return mkdir(name, 0777) == 0;
}

String Directory::current()
{
	String dir;
	char* d = getcwd(SafeString(dir, PATH_MAX), PATH_MAX);
	return d? dir : "";
}

bool Directory::change(const String& dir)
{
	return chdir(dir) == 0;
}

bool Directory::copy(const String& from, const String& to)
{
	File src(from, File::READ);
	if(!src)
		return false;
	String topath = to;
	File tofile(to);
	if(tofile.isDirectory())
		topath = to + '/' + File(from).name();

	File dst(topath, File::WRITE);
	if(!dst)
		return false;

	byte buffer[65536];
	int n=0;
	do {
		n = src.read(buffer, sizeof(buffer));
		if( n < 0)
			return false;
		int m = dst.write(buffer, n);
		if( m != n)
			return false;
	}while (n == sizeof(buffer));
	return true;
}

bool Directory::move(const String& from, const String& to)
{
	String dst = to;
	File tofile(to);
	if(tofile.isDirectory())
		dst = to + '/' + File(from).name();

	if(rename(from, dst) == 0)
		return true;
	if(errno == EXDEV) // different file systems: copy and del
	{
		copy(from, dst);
		remove(from);
	}
	return false;
}

bool Directory::remove(const String& path)
{
	if(File(path).isDirectory())
		return rmdir(path)==0;
	else
		return unlink(path)==0;
}


}
#endif

