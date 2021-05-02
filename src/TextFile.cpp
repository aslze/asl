#undef __STRICT_ANSI__
#include <asl/TextFile.h>
#include <asl/Directory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define fdopen _fdopen
#define dup _dup
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#endif

namespace asl {

#if defined(_WIN32) && !defined(ASL_ANSI)
#define STR_PREFIX(x) L##x
#define fopenX _wfopen
#define CHART wchar_t
#else
#define STR_PREFIX(x) x
#define fopenX fopen
#define CHART char
#endif

bool TextFile::openfd(int fd)
{
	int fd2 = dup(fd);
	_file = fdopen(fd2, "wt");
	return _file != 0;
}

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
			if (n > 0 && s[n-1] == '\r') {
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
	if (!_file && !open(_path, READ)) {
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
	if (_file) close();
	if (!open(_path, APPEND))
		return false;
	return fprintf(_file, "%s", *text) >= 0;
}

bool TextFile::put(const String& text)
{
	if (_file) close();
	if(!open(_path, WRITE))
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
