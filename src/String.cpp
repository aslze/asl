#include <asl/defs.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <asl/String.h>
#include <asl/Array.h>
#include <asl/Map.h>

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

namespace asl {

void printf_(const char* fmt, ...)
{
	char buffer[1000];
	char* str = buffer;
	va_list arg;
	va_start(arg, fmt);
	int i=0, n=0;
	int space = sizeof(buffer);
	while(((n=vsnprintf(str, space, fmt, arg)) == -1 || n > space) && ++i < 10)
	{
		if(space > sizeof(buffer))
			delete [] str;
		space *= 2;
		str = new char[space];
	}
	va_end(arg);
	fwrite(str, n, 1, stdout);
	if(space > sizeof(buffer))
		delete [] str;
}

int utf16toLocal8(const wchar_t* p, char* u, int nmax)
{
#ifdef _WIN32
	char def = '_';
	BOOL used = false;
	return WideCharToMultiByte(CP_ACP, 0, p, -1, u, nmax + 1, &def, &used) - 1;
#else
	return (int)wcstombs(u, p, nmax);
#endif
}

int local8toUtf16(const char* u, wchar_t* p, int nmax)
{
#ifdef _WIN32
	char def = '_';
	BOOL used = false;
	return MultiByteToWideChar(CP_ACP, 0, u, -1, p, nmax + 1) - 1;
#else
	return (int)mbstowcs(p, u, nmax);
#endif
}

int local8toUtf32(const char* u, int* p, int nmax)
{
	int* p0 = p;
	while (int c = *u++)
		*p++ = c;
	*p = 0;
	return int(p - p0);
}

int utf32toUtf8(const int* p, char* u, int)
{
	char* u0 = u;
	while (int c = *p++) {
		if (c < 0x80) {
			*u++ = c;
		}
		else if (c < 0x0800) {
			*u++ = (c >> 6 | 0xC0);
			*u++ = ((c & 0x3F) | 0x80);
		}
		else if (c < 0x10000) {
			*u++ = (c >> 12 | 0xE0);
			*u++ = ((c >> 6 & 0x3F) | 0x80);
			*u++ = ((c & 0x3F) | 0x80);
		}
		else {
			*u++ = (c >> 18 | 0xF0);
			*u++ = ((c >> 12 & 0x3F) | 0x80);
			*u++ = ((c >> 6 & 0x3F) | 0x80);
			*u++ = ((c & 0x3F) | 0x80);
		}
	}
	*u = '\0';
	return int(u - u0);
}

int utf8toUtf32(const char* u, int* p, int)
{
	int* p0 = p;
	while (int c = *u++)
	{
		if ((c & 0x80) == 0) {
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
			*p++ = ((c & 0x07) << 18) | ((c2 & 0x3f) << 12) | ((c3 & 0x3f) << 6) | (c4 & 0x3f);
		}
	}
	*p = 0;
	return int(p - p0);
}

int utf16toUtf8(const wchar_t* p, char* u, int)
{
	char* u0 = u;
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
			if (c2 == 0 || c2 < 0xdc00 || c2 > 0xdfff)
				break;
			unsigned int d = ((((unsigned int)c - 0xd800) << 10) | (c2 - 0xdc00)) + 0x10000;
			*u++ = (d >> 18 | 0xF0);
			*u++ = ((d >> 12 & 0x3F) | 0x80);
			*u++ = ((d >> 6 & 0x3F) | 0x80);
			*u++ = ((d & 0x3F) | 0x80);
		}
		else break;
	}
	*u='\0';
	return int(u - u0);
}

int utf8toUtf16(const char* u, wchar_t* p, int)
{
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
		else break;
	}
	*p=L'\0';
	return 0;
}

String localToString(const String& a)
{
	Array<wchar_t> ws(a.length() + 1);
	local8toUtf16(a, ws.ptr(), a.length() + 1);
	return String(ws.ptr());
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
	return 0;
#endif
}


String::operator const wchar_t*() const
{
	((String*)this)->resize(_len + 1 + (_len + 2)*sizeof(wchar_t), true, false);
	int offset = (_len + 1) + ((4 - ((_len + 1) & 0x03)) & 0x03);
	wchar_t* wstr = (wchar_t*)(str() + offset);
	from8bit(str(), wstr, _len);
	return wstr;
}

String::String(const wchar_t* s)
{
	init(4*(int)wcslen(s));
	_len = to8bit(s, str(), cap());
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

String::String(int n, const char* fmt, ...)
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

String String::f(const char* fmt, ...)
{
	String s;
	char ss[128];
	char* p = ss;
	va_list arg;
	va_start(arg, fmt);
	int i = 0, n = 100;
	//int space = s._size ? s._size : ASL_STR_SPACE;
	int space = 127;
	while (((n = vsnprintf(p, space, fmt, arg)) == -1 || n >= space) && ++i < 10) {
		s.resize((n >= space) ? n : 2 * space, false);
		space = s._size ? s._size : ASL_STR_SPACE;
		p = s.str();
		va_end(arg);
		va_start(arg, fmt);
	}
	va_end(arg);
	if (p == ss)
		s = p;
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


void String::alloc(int n)
{
	if(n < ASL_STR_SPACE)
	{
		_size = 0;
	}
	else
	{
		_size = max(++n, 20);
		_str = (char*) malloc(_size);
		if (!_str) ASL_BAD_ALLOC();
	}
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
	_len = snprintf(s, cap(), "%.15g", x);
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

bool String::isTrue() const
{
	char c = str()[0];
	return length() > 0 && *this != "0" && c != 'N' && c != 'n' && c != 'f' && c != 'F';
}

Array<int> String::chars() const
{
	Array<int> c(length());
	int n = utf8toUtf32(str(), c.ptr(), 1);
	c.resize(n);
	return c;
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
	char* p = str();
	char* ch = strchr(p+i0,c);
	return (!ch)? -1 : int(ch - p);
}

int String::indexOf(const char* s, int i0) const
{
	char* p = str();
	char* c = strstr(p+i0,s);
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
#ifdef ASL_ANSI
	for(int i=0; i<_len; i++)
		s[i] = toupper(str()[i]);
	s[_len]='\0';
#else
	unsigned char* p = (unsigned char*)str();
	char* p2 = s.str();
	for(int i=0; i<_len; i++)
	{
		int code = (unsigned char)p[i];
		if((code & 0x80) == 0) {}
		else if((code & 0xe0) == 0xc0){
			code = ((p[i] & 0x1f) << 6) | (p[i+1] & 0x3f);
			i++;
		}
		else {
			*p2++ = p[i++];
			*p2++ = p[i++];
			*p2++ = p[i];
			continue;
		}
		char c1 = toUppercaseU8[code*2];
		char c2 = toUppercaseU8[code*2 + 1];
		*p2++ = c1;
		if(c2 != 0)
			*p2++ = c2;
	}
	*p2++='\0';
	s.fix(int(p2-s.str()-1));
#endif
	return s;
}

String String::toLowerCase() const
{
	String s(_len, _len);
#ifdef ASL_ANSI
	for(int i=0; i<_len; i++)
		s[i] = tolower(str()[i]);
	s[_len]='\0';
#else
	unsigned char* p = (unsigned char*)str();
	char* p2 = s.str();
	for(int i=0; i<_len; i++)
	{
		int code = (unsigned char)p[i];
		if((code & 0x80) == 0) {}
		else if((code & 0xe0) == 0xc0){
			code = ((p[i] & 0x1f) << 6) | (p[i+1] & 0x3f);
			i++;
		}
		else {
			*p2++ = p[i++];
			*p2++ = p[i++];
			*p2++ = p[i];
			continue;
		}
		char c1 = toLowercaseU8[code*2];
		char c2 = toLowercaseU8[code*2 + 1];
		*p2++ = c1;
		if(c2 != 0)
			*p2++ = c2;
	}
	*p2++='\0';
	s.fix(int(p2-s.str()-1));
#endif
	return s;
}

String String::trimmed() const
{
	int i,j, n=_len;
	char* s = str();
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
	String out;
	int j=0, m=a.length();
	j = indexOf(a);
	if(j==-1) {
		out = *this;
		return out;
	}
	out << substring(0, j);
	for(int i=j+m; i<=length(); i=j+m)
	{
		j = indexOf(a, i);
		if(j==-1) j=length();
		out << b << substring(i, j);
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
	char* s = str();
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

/*
double myatof(const char* s)
{
double y = 0;
double m = 1;
int exp = 0;
if (s[0] == '-') { m = -1; s++; }
const char* p = strchr(s, '.');
if(p) {
p++;
while (*p != '\0' && *p!='e' && *p!='E') {
exp--;
p++;
}
p--;
}
if (!p) p = s;
while(*p++)
if(*p == 'e' || *p == 'E'){
if(*(p+1) == '+')
p++;
exp += myatoiz(p+1);
break;
}
while(int c=*s++)
{
if(c=='.') continue;
if(c=='E' || c=='e') break;
y = 10.0*y + (c-'0');
}
if(exp != 0)
y *= pow(10.0, exp);
return y * m;
}
*/

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
