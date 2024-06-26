#include <asl/String.h>
#include <asl/Array.h>
#include <asl/Map.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <wchar.h>

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

#if !defined(ASL_ANSI)
#define to8bit utf16toUtf8
#define from8bit utf8toUtf16
#define to32bit utf8toUtf32
#else
#define to8bit utf16toLocal8
#define from8bit local8toUtf16
#define to32bit local8toUtf32
#endif

#ifdef _MSC_VER
#pragma warning(disable : 26451 26812 6386)
#endif

namespace asl {

void printf_(const char* fmt, ...)
{
	const int N = 1000;
	char    buffer[N];
	char* str = buffer;
	va_list arg;
	va_start(arg, fmt);
	int i=0, n=0;
	int space = N;
	while(((n=vsnprintf(str, space, fmt, arg)) == -1 || n > space) && ++i < 10)
	{
		if (space > N)
			delete [] str;
		space *= 2;
		str = new char[space];
	}
	va_end(arg);
	fwrite(str, n, 1, stdout);
	if (space > N)
		delete [] str;
}

int utf16toLocal8(const wchar_t* p, char* u, int n)
{
#ifdef _WIN32
	char def = '_';
	BOOL used = false;
	return WideCharToMultiByte(CP_ACP, 0, p, -1, u, n + 1, &def, &used) - 1;
#else
	return (int)wcstombs(u, p, n + 1);
#endif
}

int local8toUtf16(const char* u, wchar_t* p, int n)
{
#ifdef _WIN32
	return MultiByteToWideChar(CP_ACP, 0, u, -1, p, n + 1) - 1;
#else
	return (int)mbstowcs(p, u, n + 1);
#endif
}

int local8toUtf32(const char* u, int* p, int n)
{
	const int* p0 = p;
	while (int c = *u++)
	{
		*p++ = c;
		if (--n == 0)
			break;
	}
	*p = 0;
	return int(p - p0);
}

int utf32toUtf8(const int* p, char* u, int n)
{
	const char* u0 = u;
	while (int c = *p++) {
		if (c < 0x80) {
			*u++ = c & 0xff;
		}
		else if (c < 0x0800) {
			*u++ = (c >> 6 | 0xc0);
			*u++ = ((c & 0x3f) | 0x80);
		}
		else if (c < 0x10000) {
			*u++ = (c >> 12 | 0xe0);
			*u++ = ((c >> 6 & 0x3f) | 0x80);
			*u++ = ((c & 0x3f) | 0x80);
		}
		else {
			*u++ = (c >> 18 | 0xf0);
			*u++ = ((c >> 12 & 0x3f) | 0x80);
			*u++ = ((c >> 6 & 0x3f) | 0x80);
			*u++ = ((c & 0x3f) | 0x80);
		}
		if (--n == 0)
			break;
	}
	*u = '\0';
	return int(u - u0);
}

int utf8toUtf32(const char* u, int* p, int n)
{
	const int* p0 = p;
	while (int c = *u++)
	{
		if ((c & 0x80) == 0) {
			*p++ = c & 0xff;
		}
		else if ((c & 0xe0) == 0xc0) {
			char c2 = *u++;
			if (c2 == 0) break;
			*p++ = ((c & 0x1f) << 6) | (c2 & 0x3f);
		}
		else if ((c & 0xf0) == 0xe0) {
			char c2 = *u++;
			if (c2 == 0) break;
			char c3 = *u++;
			if (c3 == 0) break;
			*p++ = ((c & 0x0f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);
		}
		else if ((c & 0xf8) == 0xf0) {
			char c2 = *u++;
			if (c2 == 0) break;
			char c3 = *u++;
			if (c3 == 0) break;
			char c4 = *u++;
			if (c4 == 0) break;
			*p++ = ((c & 0x07) << 18) | ((c2 & 0x3f) << 12) | ((c3 & 0x3f) << 6) | (c4 & 0x3f);
		}
		if (--n == 0)
			break;
	}
	*p = 0;
	return int(p - p0);
}

int utf16toUtf8(const wchar_t* p, char* u, int n)
{
	const char* u0 = u;
	while(wchar_t c = *p++) {
		if(c < 0x80) {
			*u++ = c & 0xff;
		}
		else if (c < 0x0800) {
			*u++ = (c >> 6 | 0xC0);
			*u++ = ((c & 0x3F) | 0x80);
		}
		else if (c < 0xd800 || c > 0xdfff) {
			*u++ = (c >> 12 | 0xE0);
			*u++ = ((c >> 6 & 0x3F) | 0x80);
			*u++ = ((c & 0x3F) | 0x80);
		}
		else if (c < 0xdc00) { // 1st surrogate
			int c2 = *p++;
			if (c2 < 0xdc00 || c2 > 0xdfff)
				break;
			unsigned int d = ((((unsigned int)c - 0xd800) << 10) | (c2 - 0xdc00)) + 0x10000;
			*u++ = (d >> 18 | 0xF0);
			*u++ = ((d >> 12 & 0x3F) | 0x80);
			*u++ = ((d >> 6 & 0x3F) | 0x80);
			*u++ = ((d & 0x3F) | 0x80);
		}
		else
			break;
		if (--n == 0)
			break;
	}
	*u='\0';
	return int(u - u0);
}

int utf8toUtf16(const char* u, wchar_t* p, int n)
{
	const wchar_t* p0 = p;
	while(int c = *u++)
	{
		if((c & 0x80) == 0) {
			*p++ = c;
		}
		else if ((c & 0xe0) == 0xc0) {
			char c2 = *u++;
			if (c2 == 0) break;
			*p++ = ((c & 0x1f) << 6) | (c2 & 0x3f);
		}
		else if ((c & 0xf0) == 0xe0) {
			char c2 = *u++;
			if (c2 == 0) break;
			char c3 = *u++;
			if (c3 == 0) break;
			*p++ = ((c & 0x0f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);
		}
		else if ((c & 0xf8) == 0xf0) {
			char c2 = *u++;
			if (c2 == 0) break;
			char c3 = *u++;
			if (c3 == 0) break;
			char c4 = *u++;
			if (c4 == 0) break;
			unsigned int d = (((c & 0x07) << 18) | ((c2 & 0x3f) << 12) | ((c3 & 0x3f) << 6) | (c4 & 0x3f)) - 0x10000;
			*p++ = (d >> 10) + 0xd800;
			*p++ = (d & 0x3ff) + 0xdc00;
		}
		else
			break;
		if (--n == 0)
			break;
	}
	*p=L'\0';
	return int(p - p0);
}

String localToUtf8(const String& a)
{
	Array<wchar_t> ws(a.length() + 1);
	int n = local8toUtf16(*a, ws.data(), a.length());
	String u(a.length() * 4, 0);
	n = utf16toUtf8(ws.data(), u.data(), n);
	return u.fix(n);
}

String utf8ToLocal(const String& a)
{
	String s(a.length() * 2, 0);
	Array<wchar_t> ws(a.length() + 1);
	int n = utf8toUtf16(*a, ws.data(), a.length());
	utf16toLocal8(ws.data(), s.data(), n);
	return s.fix();
}

int String::Enumerator::operator*()
{
#ifdef ASL_ANSI
	n = 1;
	return (int)(byte)u[0];
#else
	char c = u[0];
	if((c & 0x80) == 0) {
		n = 1;
		return c;
	}
	else if ((c & 0xe0) == 0xc0) {
		char c2 = u[1];
		if (c2 == 0) { n = 1; return 0; }
		n = 2;
		return ((c & 0x1f) << 6) | (c2 & 0x3f);
	}
	else if ((c & 0xf0) == 0xe0) {
		char c2 = u[1];
		if (c2 == 0) { n = 1; return 0; }
		char c3 = u[2];
		if (c3 == 0) { n = 2; return 0; }
		n = 3;
		return ((c & 0x0f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);
	}
	else {
		n = 4;
		char c2 = u[1];
		if (c2 == 0) { n = 1; return 0; }
		char c3 = u[2];
		if (c3 == 0) { n = 2; return 0; }
		char c4 = u[3];
		if (c4 == 0) { n = 3; return 0; }
		return ((c & 0x07) << 18) | ((c2 & 0x3f) << 12) | ((c3 & 0x3f) << 6) | (c4 & 0x3f);
	}
#endif
}


const wchar_t* String::dataw() const
{
	((String*)this)->resize(_len + 1 + (_len + 2) * sizeof(wchar_t), true, false);
	int      offset = (_len + 1) + ((4 - ((_len + 1) & 0x03)) & 0x03);
	wchar_t* wstr = (wchar_t*)(str() + offset);
	from8bit(str(), wstr, _len);
	return wstr;
}

String::String(const wchar_t* s)
{
	init(4*(int)wcslen(s));
	_len = to8bit(s, str(), cap());
}

String::String(const Array<wchar_t>& txt)
{
	init(4 * txt.length());
	Array<wchar_t> a = txt.clone(); // hack to append a nul
	a << 0;
	_len = to8bit(a.data(), str(), cap());
}

String String::fromCodes(const Array<int>& codes)
{
#ifndef ASL_ANSI
	Array<int> a = codes.clone(); // hack to append a nul
	a << 0;
	String s(codes.length() * 4, 0);
	s.fix(utf32toUtf8(a.data(), s.str(), a.length()));
#else
	String s(codes.length(), codes.length());
	char* p = s.str();
	for (int i = 0; i < codes.length(); i++)
	{
		p[i] = codes[i] < 128 ? (char)codes[i] : '?';
	}
#endif
	return s;
}

String String::fromCode(int code)
{
#ifndef ASL_ANSI
	const int codes[] = { code, 0 };
	String s(4, 0);
	s.fix(utf32toUtf8(codes, s.str(), 1));
#else
	String s(code < 128 ? (char)code : '?');
#endif
	return s;
}

String String::fromLocal(const String& a)
{
#ifdef ASL_ANSI
	return a;
#else
	Array<wchar_t> ws(a.length() + 1);
	local8toUtf16(a, ws.data(), a.length());
	return String(ws.data());
#endif
}

String& String::fix()
{
	_len = (int)strlen(str());
	return *this;
}

String& String::fixW()
{
	int offset = (_len + 1) + ((4 - ((_len + 1) & 0x03)) & 0x03);
	to8bit((wchar_t*)(str() + offset), str(), cap());
	_len = (int)strlen(str());
	return *this;
}

String::String(int n, ASL_PRINTF_W1 const char* fmt, ...)
{
	alloc(n? n : 100);
	va_list arg;
	va_start(arg, fmt);
	int i=0;
	int space = _size? _size : ASL_STR_SPACE;
	while (((n = vsnprintf(str(), space, fmt, arg)) == -1 || n >= space) && ++i < 10) {
		resize((n >= space)? n : 2*space, false);
		space = _size? _size : ASL_STR_SPACE;
		va_end(arg);
		va_start(arg, fmt);
	}
	va_end(arg);
	_len=n;
}

String String::f(ASL_PRINTF_W1 const char* fmt, ...)
{
	String s;
	char    ss[256];
	char* p = ss;
	va_list arg;
	va_start(arg, fmt);
	int i = 0, n = 0, space = 255;
	while (((n = vsnprintf(p, space, fmt, arg)) == -1 || n >= space) && ++i < 16) {
		s.resize((n >= space) ? n : 2 * space, false);
		space = s._size ? s._size : ASL_STR_SPACE;
		p = s.str();
		va_end(arg);
		va_start(arg, fmt);
	}
	va_end(arg);
	if (p == ss)
		s.assign(ss, n);
	s._len = n;
	return s;
}

/*String::~String()
{
	if(_size>0)
		::free(str());
}*/

void String::free()
{
	if(_size>0)
		::free(_str);
}

String& String::resize(int n, bool keep, bool newlen)
{
	if(_size==0)
	{
		if(n < ASL_STR_SPACE)
		{
			if (newlen) {
				_space[n] = '\0';
				_len = n;
			}
			return *this;
		}
		else
		{
			_size = max(n+1, 24);
			char* str2 = (char*) malloc(_size);
			if (!str2) ASL_BAD_ALLOC();
			if(keep)
				memcpy(str2, _space, _len+1);
			_str = str2;
			if (newlen) {
				_str[n] = '\0';
				_len = n;
			}
			return *this;
		}
	}
	int size2 = n+1;
	int size3 = (_size < (1 << 30)) ? 2 * _size : 2147483647;
	size2 = (size2 > _size)? max(size3, size2) : _size;
	if(size2 == _size)
	{}
	else // grow
	if(_size < 1024)
	{
		char* str2 = (char*) malloc(size2);
		if (!str2) ASL_BAD_ALLOC();
		if(keep)
			memcpy(str2, _str, min(n, _len + 1));
		::free(_str);
		_str = str2;
		_size = size2;
	}
	else
	{
		char* str2 = (char*) realloc(_str, size2);
		if (!str2) ASL_BAD_ALLOC();
		_str = str2;
		_size = size2;
	}
	if (newlen) {
		_len = n;
		_str[n] = '\0';
	}
	return *this;
}

String::String(Long x)
{
	alloc((x < 1000000000000000ll && x > -100000000000000ll) ? ASL_STR_SPACE-1 : 21);
	_len = myltoa(x, str());
}

String::String(ULong x)
{
	alloc(x < 1000000000000000ll ? ASL_STR_SPACE - 1 : 21);
	_len = snprintf(str(), cap(), "%llu", x);
}

String::String(int x)
{
	alloc(11);
	_len = myitoa(x, str());
}

String::String(unsigned x)
{
	alloc(10);
	_len = snprintf(str(), cap(), "%u", x);
}

String::String(float x)
{
	alloc(15);
	_len = snprintf(str(), cap(), "%.7g", x);
}

String::String(double x)
{
	char s[32];
	_len = snprintf(s, 32, "%.15g", x);
	alloc(_len);
	strcpy(str(), s);
}

String::String(bool x)
{
	alloc(5);
	strcpy(str(), x ? "true" : "false");
	_len = (int)strlen(str());
}

Long String::toLong() const
{
	return myatol(str());
}

String String::toLocal() const
{
#ifdef ASL_ANSI
	return *this;
#else
	Array<char> s(length() * 2 + 1);
	utf16toLocal8(dataw(), s.data(), s.length());
	return String(s.data());
#endif
}

bool String::isTrue() const
{
	char c = str()[0];
	return length() > 0 && *this != "0" && c != 'N' && c != 'n' && c != 'f' && c != 'F';
}

Array<int> String::chars() const
{
	Array<int> c(length() + 1);
	int        n = utf8toUtf32(str(), c.data(), length());
	return c.resize(n);
}

void String::assign(const char* b, int n)
{
	resize(n, false);
	char* s = str();
	memcpy(s, b, _len);
	s[_len] = '\0';
}

String String::concat(const char* b, int n) const
{
	String s(_len+n, _len+n);
	char* p = s.str();
	memcpy(p, str(), _len);
	memcpy(p+_len, b, n);
	p[_len+n] = '\0';
	return s;
}

void String::append(const char* b, int n)
{
	if(_len+n >= _size)
		resize(_len+n);
	else
		_len += n;
	char* s = str();
	memcpy(s+_len-n, b, n);
	s[_len] = '\0';
}

int String::wlength() const
{
	const wchar_t* w(*(String*)this);
	return (int)wcslen(w);
}

int String::count() const 
{
	const char* u = str();
	int count_ = 0;
	while (int c = *u++)
	{
		if ((c & 0x80) == 0) {
			++count_;
		}
		else if ((c & 0xe0) == 0xc0) {
			++u; ++count_;
		}
		else if ((c & 0xf0) == 0xe0) {
			++count_;
			char c2 = *u++;
			if (c2 == 0) break;
			char c3 = *u++;
			if (c3 == 0) break;
		}
		else if ((c & 0xf8) == 0xf0) {
			++count_;
			char c2 = *u++;
			if (c2 == 0) break;
			char c3 = *u++;
			if (c3 == 0) break;
			char c4 = *u++;
			if (c4 == 0) break;
		}
	}
	return count_;
}

String String::substring(int i, int j) const
{
	String s(j-i, j-i);
	memcpy(s.str(), str()+i, j-i);
	s.str()[j-i]='\0';
	return s;
}

String String::substr(int i, int n) const
{
	if (i < 0) i += _len;
	if (i >= _len)
		i = _len;
	int j = i + n;
	if (j > _len)
		j = _len;
	String s(j - i, j - i);
	memcpy(s.str(), str() + i, j - i);
	s.str()[j - i] = '\0';
	return s;
}

int String::indexOf(char c, int i0) const
{
	const char* p = str();
	const char* ch = strchr(p+i0,c);
	return (!ch)? -1 : int(ch - p);
}

int String::indexOf(const char* s, int i0) const
{
	const char* p = str();
	const char* c = strstr(p+i0,s);
	return (!c)? -1 : int(c - p);
}

int String::lastIndexOf(const char* s) const
{
	int i = 0, j = -1;
	while((i = indexOf(s, i)) >= 0)
	{
		j = i;
		i++;
	}
	return j;
}

extern char toUppercaseU8[];
extern char toLowercaseU8[];

String String::toUpperCase() const
{
	String s(_len, _len);
	char* p = s.str();
#ifdef ASL_ANSI
	const char* p0 = str();
	for(int i=0; i<_len; i++)
		p[i] = toupper(p0[i]);
	p[_len] = '\0';
#else
	int   u[2] = { 0, 0 };
	for (Enumerator e = all(); e; ++e)
	{
		int code = *e;
		if (code < 1415)
	{
		char c1 = toUppercaseU8[code*2];
		char c2 = toUppercaseU8[code*2 + 1];
			*p++ = c1;
		if(c2 != 0)
				*p++ = c2;
		}
		else
		{
			u[0] = code;
			p += utf32toUtf8(u, p, 1);
	}
	}
	*p++ = '\0';
	s.fix(int(p - s.str() - 1));
#endif
	return s;
}

String String::toLowerCase() const
{
	String s(_len, _len);
	char* p = s.str();
#ifdef ASL_ANSI
	const char* p0 = str();
	for(int i=0; i<_len; i++)
		p[i] = tolower(p0[i]);
	p[_len] = '\0';
#else
	int   u[2] = { 0, 0 };
	for (Enumerator e = all(); e; ++e)
	{
		int  code = *e;
		if (code < 1415)
	{
		char c1 = toLowercaseU8[code*2];
		char c2 = toLowercaseU8[code*2 + 1];
			*p++ = c1;
		if(c2 != 0)
				*p++ = c2;
		}
		else
		{
			u[0] = code;
			p += utf32toUtf8(u, p, 1);
		}
	}
	*p++ = '\0';
	s.fix(int(p - s.str() - 1));
#endif
	return s;
}

bool String::equalsNocase(const String& s) const
{
#ifdef ASL_ANSI
	if (length() != s.length())
		return false;
	const char* p1 = str();
	const char* p2 = s.str();
	for (; *p1 && *p2; p1++, p2++)
		if (tolower(*p1) != tolower(*p2))
			return false;
	return true;
#else
	Enumerator e1 = all();
	Enumerator e2 = s.all();

	for (; e1 && e2; ++e1, ++e2)
	{
		int code1 = *e1, code2 = *e2;
		
		if (code1 > 1415 || code2 > 1415)
		{
			if (code1 != code2)
				return false;
		}
		else if (toLowercaseU8[code1 * 2] != toLowercaseU8[code2 * 2] ||
		    toLowercaseU8[code1 * 2 + 1] != toLowercaseU8[code2 * 2 + 1])
			return false;
	}

	if ((e1 && !e2) || (!e1 && e2))
		return false;
	
	return true;
#endif
}

String String::trimmed() const
{
	int i,j, n=_len;
	const char* s = str();
	for(i=0; i<n; i++)
		if(!myisspace(s[i])) break;
	for(j=n-1; j>=i; j--)
		if(!myisspace(s[j])) break;
	return substring(i, j+1);
}

String& String::trim()
{
	int i, j, n = _len;
	char* s = str();
	for (i = 0; i<n; i++)
		if (!myisspace(s[i])) break;
	for (j = n - 1; j >= i; j--)
		if (!myisspace(s[j])) break;
	memmove(s, s + i, j - i + 1);
	s[j - i + 1] = '\0';
	_len = j - i + 1;
	return *this;
}

String String::replace(const String& a, const String& b) const
{
	int j = indexOf(a), m = a.length();
	if(j==-1)
		return *this;
	String out(length(), 0);
	out << substring(0, j);
	for(int i=j+m; i<=length(); i=j+m)
	{
		j = indexOf(a, i);
		if(j==-1) j=length();
		out << b;
		out.append(str() + i, j - i);
	}
	return out;
}

String& String::replaceme(char a, char b)
{
	char* p = str();
	do {
		if(*p == a)
			*p = b;
	}
	while(*p++);
	return *this;
}

void String::split(const String& sep, Array<String>& out) const
{
	out.clear();
	int j=0, m=sep.length(), n=length();
	for(int i=0; i<=n; i=j+m)
	{
		j=indexOf(sep, i);
		if(j==-1) j=n;
		out << substring(i, j);
	}
}

void String::split(Array<String>& a) const
{
	a.clear();
	const char* s = str();
	for(int i=0; i<=length(); i++)
	{
		if(myisspace(s[i]))
			continue;
		for(int j=i+1; j<length()+1; j++)
		{
			if(myisspace(s[j]) || s[j]=='\0')
			{
				a << substring(i, j);
				i=j;
				break;
			}
		}
	}
}

Dic<String> String::split(const String& sep1, const String& sep2) const
{
	Dic<String> dic;
	Array<String> pairs = split(sep1);
	for (int i = 0; i < pairs.length(); i++)
	{
		int j = pairs[i].indexOf(sep2);
		if (j > 0)
			dic[pairs[i].substring(0, j)] = pairs[i].substring(j + sep2.length());
	}
	return dic;
}

int myatoi(const char* s)
{
	int y = 0, sgn = 1;
	if (s[0] == '-') { sgn = -1; s++; }
	else if (s[0] == '+') s++;
	int c;
	while (c = *s++, c >= '0' && c <= '9')
		y = 10 * y + (c - '0');
	return y*sgn;
}

int myatoiz(const char* s)
{
	int y = 0, sgn = 1;
	if (s[0] == '-') { sgn = -1; s++; }
	else if (s[0] == '+') s++;
	int c;
	while ((c = *s++))
		y = 10 * y + (c - '0');
	return y*sgn;
}

Long myatol(const char* s)
{
	Long y = 0, sgn = 1;
	if (s[0] == '-') { sgn = -1; s++; }
	else if (s[0] == '+') s++;
	int c;
	while (c = *s++, c >= '0' && c <= '9')
		y = 10 * y + (c - '0');
	return y*sgn;
}

double myatof(const char* s)
{
	double y = 0;
	double m = 1;
	int exp = 0;
	if (s[0] == '-') { m = -1; s++; }
	const char* p = strchr(s, '.');
	if (p) {
		p++;
		while (*p != '\0' && *p != 'e' && *p != 'E') {
			exp--;
			p++;
		}
		p--;
	}
	if (!p) p = s;
	while (*p++)
		if (*p == 'e' || *p == 'E'){
			if (*(p + 1) == '+')
				p++;
			exp += myatoiz(p + 1);
			break;
		}
	long long y1 = 0;
	while (int c = *s++)
	{
		if (c == '.') continue;
		if (c == 'E' || c == 'e') break;
		y1 = 10 * y1 + (c - '0');
	}
	y = double(y1) * pow(10.0, exp);
	return y * m;
}

int myitoa(int x, char* s)
{
	char ss[16];
	int i = 0, j = 0;
	if (x == 0)
	{
		s[0] = '0';
		s[1] = '\0';
		return 1;
	}
	if (x < 0)
	{
		if (x == (-2147483647 - 1)) {
			strcpy(s, "-2147483648");
			return 11;
		}
		s[j++] = '-';
		x = -x;
	}
	while (x != 0) {
		ss[i++] = (x % 10) + '0';
		x = x / 10;
	}
	while (i > 0)
		s[j++] = ss[--i];
	s[j] = '\0';
	return j;
}

int myltoa(Long x, char* s)
{
	char ss[32];
	int i = 0, j = 0;
	if (x == 0)
	{
		s[0] = '0';
		s[1] = '\0';
		return 1;
	}
	if (x<0)
	{
		s[j++] = '-';
		x = -x;
	}
	while (x != 0) {
		ss[i++] = char(x % 10) + '0';
		x = x / 10;
	}
	while (i > 0)
		s[j++] = ss[--i];
	s[j] = '\0';
	return j;
}

}
