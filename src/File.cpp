#undef __STRICT_ANSI__
#include <asl/File.h>
#include <asl/Directory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define fdopen _fdopen
#define dup _dup

namespace asl {

const char File::SEP='\\';
#else
#include <sys/stat.h>
#include <utime.h>
namespace asl {
const char File::SEP='/';
#endif

#if defined(_WIN32) && !defined(ASL_ANSI)
#define STR_PREFIX(x) L##x
#define fopenX _wfopen
#define CHART wchar_t
#else
#define STR_PREFIX(x) x
#define fopenX fopen
#define CHART char
#endif

bool File::open(const String& name, File::OpenMode mode)
{
	if(name == "")
		return false;
	const CHART* fopen_mode;
	if(!(mode & TEXT))
	{
		fopen_mode = (mode==READ)? STR_PREFIX("rb"):
			(mode==WRITE)? STR_PREFIX("wb"):
			(mode==APPEND)? STR_PREFIX("ab"):
			STR_PREFIX("rb+");
	}
	else
	{
		mode = (OpenMode) (mode & ~TEXT);
		fopen_mode = (mode==READ)? STR_PREFIX("rt"):
			(mode==WRITE)? STR_PREFIX("wt"):
			(mode==APPEND)? STR_PREFIX("at"):
			STR_PREFIX("rt+");
	}
	_file = fopenX(name, fopen_mode);

	_path = name;

	return _file != 0;
}

bool File::openfd(int fd)
{
	int fd2 = dup(fd);
	_file = fdopen(fd2, "wb");
	return _file != 0;
}

bool TextFile::openfd(int fd)
{
	int fd2 = dup(fd);
	_file = fdopen(fd2, "wt");
	return _file != 0;
}


void File::close()
{
	if(_file)
		fclose(_file);
	_file = 0;
	_info = FileInfo();
}

void File::setBuffering(char mode, int size)
{
	int m=(mode=='n')?_IONBF:(mode=='l')?_IOLBF:_IOFBF;
	setvbuf(_file, 0, m, size);
}

Long File::position()
{
#if (defined( _MSC_VER ) && _MSC_VER > 1310)
	return _ftelli64(_file);
#else
	return ftell(_file);
#endif
}


void File::seek(Long offset, SeekMode from)
{
#if (defined( _MSC_VER ) && _MSC_VER > 1310)
	_fseeki64(_file, offset, (from==START)? SEEK_SET :(from==HERE)? SEEK_CUR: SEEK_END);
#else
	fseek(_file, offset, (from==START)? SEEK_SET :(from==HERE)? SEEK_CUR: SEEK_END);
#endif
}

bool File::isDirectory() const
{
	if (_path.endsWith('/')
#ifdef _WIN32
		|| _path.endsWith('\\')
#endif		
		)
		return File(_path.substring(0, _path.length() - 1)).isDirectory();

	if(!_info)
		_info = Directory::getInfo(_path);
#ifdef _WIN32
	return (_info.flags & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
	return (S_ISDIR(_info.flags)) != 0;
#endif
}


String File::name() const
{
	int n = _path.lastIndexOf(SEP);
#ifdef _WIN32
	int m = _path.lastIndexOf('/');
	n = (m > n)? m : n;
#endif
	return _path.substring(n+1);
}

String File::extension() const
{
	int n = _path.lastIndexOf(SEP);
#ifdef _WIN32
	int m = _path.lastIndexOf('/');
	n = (m > n) ? m : n;
#endif
	int dot = _path.lastIndexOf('.');
	return (dot >= 0 && dot > n) ? _path.substring(dot + 1) : "";
}

bool File::hasExtension(const String& extensions) const
{
	Array<String> exts = extensions.toLowerCase().split('|');
	String ext = extension().toLowerCase();
	for (int i = 0; i < exts.length(); i++)
		if (ext == exts[i])
			return true;
	return false;
}

String File::directory() const
{
	int n = _path.lastIndexOf(SEP);
#ifdef _WIN32
	int m = _path.lastIndexOf('/');
	n = (m > n)? m : n;
#endif
	return (n>=0)? _path.substring(0, n) : ".";
}

Long File::size() const
{
	if(!_info)
		_info = Directory::getInfo(_path);
	return _info.size;
}

Date File::creationDate() const
{
	if(!_info)
		_info = Directory::getInfo(_path);
	return _info.creationDate;
}

Date File::lastModified() const
{
	if(!_info)
		_info = Directory::getInfo(_path);
	return _info.lastModified;
}

bool File::setLastModified(const Date& t)
{
#ifdef _WIN32
	HANDLE hfile = CreateFileW(_path, FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (!hfile)
		return false;
	LARGE_INTEGER ti;
	ti.QuadPart = LONGLONG((t.time() + 11644473600.0) * 1e7);
	FILETIME ft;
	ft.dwHighDateTime = ti.HighPart;
	ft.dwLowDateTime = ti.LowPart;
	int ok = SetFileTime(hfile, 0, 0, &ft);
	CloseHandle(hfile);
	return ok != 0;
#else
	utimbuf times;
	struct stat data;
	if (stat(_path, &data) != 0)
		return false;
	times.actime = data.st_mtime;
	times.modtime = (time_t)t.time();
	return utime(_path, &times) == 0;
#endif
}

Array<byte> File::content()
{
	return firstBytes((int)size());
}

bool File::put(const Array<byte>& data)
{
	if(!_file && !open(_path, WRITE))
		return false;
	return write(data.ptr(), data.length()) == data.length();
}

Array<byte> File::firstBytes(int n)
{
	Array<byte> data(n);
	if (!_file && !open(_path)) {
		data.clear();
		return data;
	}
	data.resize(read(&data[0], n));
	return data;
}

int File::read(void* p, int n)
{
	return (int)fread(p, 1, n, _file);
}

int File::write(const void* p, int n)
{
	return (int)fwrite(p, 1, n, _file);
}

File& File::operator<<(const String& x)
{
	write(*x, x.length());
	return *this;
}

File File::temp(const String& ext)
{
	File file;
	unsigned num = (unsigned)(2e9 * fract(0.01 * now()));
	unsigned p = (unsigned int)(size_t)&file;
#ifdef _WIN32
	TCHAR tmpdir[MAX_PATH];
	DWORD ret = GetTempPath(MAX_PATH, tmpdir);
	String tmpDir = tmpdir;
	if (ret > MAX_PATH || ret == 0)
		return file;
	unsigned pid = (unsigned int)GetCurrentProcessId();
#else
	String tmpDir = "/tmp";
	unsigned pid = (unsigned)getpid();
#endif
	do {
		file._path = String(0, "%s/%04x%08x%08x%s", *tmpDir, pid, p, num++, *ext);
	} while (file.exists());
	file.open(File::WRITE);
	return file;
#if 0
	char nam[500];
	strcpy(nam, P_tmpdir "/XXXXXX");
	#ifdef __ANDROID_API__
	int fd = mkstemp(nam);
	#else
	strcat(nam, ext);
	int fd = mkstemps(nam, ext.length());
	#endif
	File file;
	if(fd < 0)
		return file;
	::close(fd);
	file._path = nam;
	return file;
#endif
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

// class TextFile

bool TextFile::printf(const char* fmt, ...)
{
	if(!_file && !open(_path, APPEND))
		return false;
	va_list arg;
	va_start(arg,fmt);
	int ret = vfprintf(_file, (char*)fmt, arg);
	va_end(arg);
	return ret >= 0;
}

int TextFile::scanf(const String& fmt, void* p1, void* p2, void* p3, void* p4)
{
	if(!_file && !open(_path, READ))
		return 0;
	return fscanf(_file, fmt, p1, p2, p3, p4);
}

String TextFile::readLine()
{
	String s;
	readLine(s);
	return s;
	/*
	String s(1000, 0);
	s[0] = '\0';
	if(!_file && !open(_path, READ))
		return s;
	if(!fgets(s, 1000, _file))
		return s;
	int n = (int)strlen(*s)-1;
	s[n]='\0';
	if (s[n - 1] == '\r') {
		n--;
		s[n] = '\0';
	}
	s.fix(n);
	return s;*/
}

String TextFile::readLine(char newline)
{
	String s(1000, 0);
	s[0] = '\0';
	if(!_file && !open(_path, READ))
		return s;
	while (1)
	{
		char c = read<char>();
		if (end() || c == newline)
			break;
		s << c;
	}
	return s;
}

bool TextFile::readLine(String& s)
{
	int chunk = 255;
	int m = 0, n = 0;
	s[0] = '\0';
	if(!_file && !open(_path, READ))
		return false;
	do {
		s.resize(m + chunk);
		char* r = fgets(&s[m], chunk, _file);
		if (!r)
		{
			s[m] = '\0';
			s.fix(m);
			return false;
		}
		n = (int)strlen(*s + m) + m;
		if (s[n-1] == '\n') {
			n--;
			s[n] = '\0';
			if (s[n-1] == '\r') {
				n--;
				s[n] = '\0';
			}
			break;
		}
		m = n;
	} while (1);
	s.fix(n);
	return true;
}

Array<String> TextFile::lines()
{
	Array<String> lines;
	if(!_file && !open(_path, READ))
		return lines;
	while (!end()) {
		lines << String();
		readLine(lines.last());
	}
	return lines;
}

String TextFile::text()
{
	int n = (int)(size() & 0x7fffffff); // truncate
	String text;
	if(!open(_path)) {
		return text;
	}
	byte head[8];
	if (n >= 2) // Read BOM
	{
		read(head, 2);
		if (head[0] == 0xff && head[1] == 0xfe) // UTF16LE
		{
			Array<wchar_t> a;
			wchar_t c = 0, c0 = 0;
			byte b[2];
			while (1)
			{
				if (read(b, 2) < 2)
					break;
				c = b[0] | (((wchar_t)b[1]) << 8);
				if (c == '\n' && c0 == '\r')
					a.resize(a.length() - 1);
				a << c;
				c0 = c;
			}
			a << 0;
			text = a.ptr();
			return text;
		}
		else if (head[0] == 0xfe && head[1] == 0xff) // UTF16BE
		{
			Array<wchar_t> a;
			wchar_t c = 0, c0 = 0;
			byte b[2];
			while (1)
			{
				if (read(b, 2) < 2)
					break;
				c = b[1] | (((wchar_t)b[0]) << 8);
				if (c == '\n' && c0 == '\r')
					a.resize(a.length() - 1);
				a << c;
				c0 = c;
			}
			a << 0;
			text = a.ptr();
			return text;
		}
		else if (head[0] == 0xef && head[1] == 0xbb && n>=3 && read<byte>() == 0xbf) // UTF8
		{
		}
		else
		{
			seek(0);
		}
	}
	text.resize(n, false, false);
	n = read(&text[0], n);
	text[n]='\0';
	text.fix(n);
	return text;
}

bool TextFile::append(const String& text)
{
	if(!open(_path, APPEND))
		return false;
	return fprintf(_file, "%s", *text) >= 0;
}

bool TextFile::put(const String& text)
{
	if(!_file && !open(_path, WRITE))
		return false;
	return fprintf(_file, "%s", *text) >= 0;
}

TextFile& TextFile::operator>>(char &x)
{
	if(!_file && !open(_path, READ))
		return *this;
	x=getc(_file);
	return *this;
}

TextFile& TextFile::operator>>(byte &x)
{
	if(!_file && !open(_path, READ))
		return *this;
	x=getc(_file);
	return *this;
}

TextFile& TextFile::operator>>(int &x)
{
	if (!_file && !open(_path, READ))
		return *this;
	int n = fscanf(_file, "%i", &x); if (n < 1) {}
	return *this;
}

TextFile& TextFile::operator>>(unsigned &x)
{
	if(!_file && !open(_path, READ))
		return *this;
	int n = fscanf(_file, "%ui", &x); if (n < 1) {}
	return *this;
}

TextFile& TextFile::operator>>(float &x)
{
	if(!_file && !open(_path, READ))
		return *this;
	int n = fscanf(_file, "%f", &x); if (n < 1) {}
	return *this;
}

TextFile& TextFile::operator>>(double &x)
{
	if(!_file && !open(_path, READ))
		return *this;
	int n = fscanf(_file, "%lf", &x); if (n < 1) {}
	return *this;
}

TextFile& TextFile::operator>>(String &x)
{
	if(!_file && !open(_path, READ))
		return *this;
	char s[256];
	int n = fscanf(_file, "%255s", s); if (n < 1) {}
	x = &s[0];
	return *this;
}

TextFile& TextFile::operator<<(char x)
{
	if(!_file && !open(_path, APPEND))
		return *this;
	putc(x, _file);
	return *this;
}

TextFile& TextFile::operator<<(byte x)
{
	if(!_file && !open(_path, APPEND))
		return *this;
	putc(x, _file);
	return *this;
}

TextFile& TextFile::operator<<(int x)
{
	if(!_file && !open(_path, APPEND))
		return *this;
	fprintf(_file, "%i", x);
	return *this;
}

TextFile& TextFile::operator<<(unsigned x)
{
	if(!_file && !open(_path, APPEND))
		return *this;
	fprintf(_file, "%ui", x);
	return *this;
}

TextFile& TextFile::operator<<(float x)
{
	if(!_file && !open(_path, APPEND))
		return *this;
	fprintf(_file, "%.8g", x);
	return *this;
}

TextFile& TextFile::operator<<(double x)
{
	if(!_file && !open(_path, APPEND))
		return *this;
	fprintf(_file, "%.16lg", x);
	return *this;
}

TextFile& TextFile::operator<<(const String& x)
{
	if(!_file && !open(_path, APPEND))
		return *this;
	fprintf(_file, "%s", (const char*)x);
	return *this;
}

}
