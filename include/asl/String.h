// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_STRING_H
#define ASL_STRING_H

#include <asl/defs.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <asl/Array.h>

#if defined(MINGW) || !defined __GNUC__
#define ASL_LONG_FMT "I64i"
#else
#define ASL_LONG_FMT "lli"
#endif

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#define ASL_STR_SPACE 16

#ifdef ASL_ANSI
 #define ASL_QTFROM fromLocal8Bit
 #define ASL_QTTO toLocal8Bit
#else
 #define ASL_QTFROM fromUtf8
 #define ASL_QTTO toUtf8
#endif
#ifndef QSTRING_H
class QString;
#endif

namespace asl {
	
template<class T> class Dic;

/**
A substitute of `printf` that works on MingW with UTF8 text
*/
void ASL_API printf_(const char* fmt, ...);

/**
String represents a character string that behaves similarly to JavaScript strings in that it can be converted
to/from other types easily. Future versions might allow only explicit conversions but now they are implicit.

Strings are byte-based using UTF-8 (or Local 8-bit if ASL_ANSI is defined). In order to use string literals with
non-ASCII characters, source code must be encoded in UTF-8. Indices and substrings operate on
code-units.

~~~
String firstName = "John", lastName = "Doe";
String name = firstName + " " + lastName;       // use += when possible, it's faster
~~~

Unicode (UTF-8) case conversions and case-insensitive comparisons are supported.

~~~
String babel = "Ñandú εξέλιξη жизни"; // UTF-8 string with Latin, Greek and Cyrilic text
babel.toUpperCase() -> "ÑANDÚ ΕΞΈΛΙΞΗ ЖИЗНИ"
babel.equalsNocase("ñanDÚ εΞΈλΙξΗ ЖиЗНИ") -> true
~~~

There are convenience methods for simple transformations:

~~~
if( filename.toLowerCase().endsWith(".xml") )

String id = text.trim(); // remove whitespace before and after

if( !email.contains('@') ) {}

Array<String> parts = path.split("/"); // cut a path into its parts
~~~

Remember in UTF-8 characters can have different byte-length. Use the length() method to get the number of bytes
(i.e. code units) and count() to get the number of code points (i.e. actual characters).
To get the individual wide characters you can use the chars() method which returns an array of code points or iterate
the string with a `foreach` loop.

~~~
String euros = "3€";
euros.length() // returns 4 (bytes)
euros.count()  // returns 2 (characters)
euros.chars()  // returns the array [51, 8364] (the codes of '3' and '€')
for(int c : euros)
	printf("%i ", c); // prints "51 8364"
~~~

A String automatically converts to `char*` or `wchar_t*` when needed. When converting to `wchar_t*` the UTF-8 string
will be expanded to wide characters in a temporary buffer. This way you can call functions that require wide
characters transparently.

~~~
String dirName = "Mis imágenes"  // this is UTF-8
CreateDirectoryW( dirName );     // this gets converted to UTF-16 LPWSTRING
~~~

There are automatic conversions to/from many basic types, but please use with care!

~~~
String number = 14;            //                  int to string
Array<double> a(20);
a[number] = number + ".5";     // -> a[14] = 14.5  string to double
fopen(number + ".txt", "r");   // -> "14.txt"      string to const char*
String items = array(1, 2, 3); // -> "[1,2,3]"     array of ints to string
~~~
*/

class ASL_API String
{
protected:
	int _size, _len;
	union {
		char _space[ASL_STR_SPACE];
		char* _str;
	};
	void alloc(int n)
	{
		if (n < ASL_STR_SPACE)
			_size = 0;
		else
		{
			_size = max(++n, 20);
			_str = (char*)malloc(_size);
			if (!_str)
				ASL_BAD_ALLOC();
		}
	}
	void free();
	void init(int n) {alloc(n); _len=n;}
	char* str() const {return (_size==0)? (char*)_space : (char*)_str;}
	String(void*) {} // avoid accidental construction from arbitrary pointers
public:
	/**
	Constructs an empty string
	*/
	String(): _size(0),_len(0)
	{
		*_space='\0';
	}
	/**
	Constructs an uninitialized string with internal capacity `cap` and length `n`,
	useful for writing to it externally.
	*/
	ASL_EXPLICIT String(int cap, int n)
	{
		init(max(cap, n));
		_len=n;
		str()[n] = '\0';
	}
	/**
	Constructs a string from a C string (pointer to null-terminated string)
	*/
	String(char* txt)
	{
		init((int)strlen(txt));
		memcpy(str(), txt, _len+1);
	}
	/**
	Constructs a string from a C string (pointer to null-terminated string)
	*/
	String(const char* txt)
	{
		init((int)strlen(txt));
		memcpy(str(), txt, _len+1);
	}
	/**
	Constructs a string from the first `n` characters of a character buffer pointed by `txt`
	*/
	ASL_EXPLICIT String(const char* txt, int n)
	{
		init(n);
		memcpy(str(), txt, n);
		str()[n] = '\0';
	}
	/**
	Constructs a string from a byte array`
	*/
	String(const Array<char>& txt)
	{
		int n = txt.length();
		init(n);
		memcpy(str(), txt.ptr(), n);
		str()[n] = '\0';
	}
	String(const Array<byte>& txt)
	{
		int n = txt.length();
		init(n);
		memcpy(str(), txt.ptr(), n);
		str()[n] = '\0';
	}
	/**
	Constructs a string from a character
	*/
	String(char c)
	{
		init(1);
		str()[0] = c;
		str()[1] = '\0';
	}
	/**
	Constructs a string consisting of character `c` repeated `n` times.
	@deprecated Use `repeat()` instead.
	*/
	ASL_DEPRECATED(ASL_EXPLICIT String(char c, int n), "Use String::repeat()")
	{
		if (n < 0) n = 0;
		init(n);
		char* p = str();
		memset(p, c, n);
		p[n] = '\0';
	}
	/**
	Constructs a string consisting of character `c` repeated `n` times
	*/
	static String repeat(char c, int n)
	{
		if (n < 0) n = 0;
		String s(n, n);
		char* p = s.str();
		memset(p, c, n);
		p[n] = '\0';
		return s;
	}
	/**
	Constructs a string from another string object
	*/
	String(const String& s)
	{
		init(s._len);
		memcpy(str(), s.str(), _len + 1);
	}
#ifdef ASL_HAVE_MOVE
	String(String&& s) {
		memcpy(this, &s, sizeof(String));
		s._size = 0;
	}
	void operator=(String&& s) {
		swap(*this, s);
	}
#endif
	/**
	Constructs a string from an Unicode UTF16 string
	*/
	String(const wchar_t* s);
	~String()
	{
		if (_size != 0)
			::free(_str);
	}

	/*
	*/
	int cap() const { return (_size == 0) ? ASL_STR_SPACE : _size; }
	/**
	Constructs a string by formatting values using `printf`-style specification `fmt`.
	The first argument can give an initial buffer size, but will be automatically calculated if it is 0. In
	any case, the function will automatically allocate _space as needed.
	*/
	ASL_EXPLICIT String(int n, const char* fmt, ...);
	/**
	Constructs a string from a 64bit long integer number
	*/
	String(Long x);
	/**
	Constructs a string from a 64bit unsigned long integer number
	*/
	String(ULong x);
	/**
	Constructs a string from an integer number
	*/
	String(int x);
	/**
	Constructs a string from an unsigned integer number
	*/
	String(unsigned x);
	/**
	Constructs a string from a float floating-point number
	*/
	String(float x);
	/**
	Constructs a string from a double floating-point number
	*/
	String(double x);
	/**
	Constructs a string from a boolean value
	*/
	String(bool x);
#if defined(QSTRING_H) && defined(ASL_STATIC)
	String(const QString& s)
	{
		QByteArray a = s.ASL_QTTO();
		init(a.size());
		memcpy(str(), a.data(), _len+1);
	}
	void operator=(const QString& s)
	{
		QByteArray a = s.ASL_QTTO();
		assign(a.data(), a.size());
	}
	operator QString() const
	{
		return QString::ASL_QTFROM(str());
	}
#endif
	/**
	Creates a string by formatting values using `printf`-style specification `fmt`.
	*/
	static String f(const char* fmt, ...);
	/**
	Converts this string to an integer number
	*/
	operator int() const {return myatoi(str());}
	//operator int() {return atoi(str());}
	/**
	Converts this string to an unsigned integer number
	*/
	operator unsigned() const {return atoi(str());}
	/**
	Converts this string to a 32-bit floating-point number
	*/
	operator float() const {return (float)myatof(str());}
	/**
	Converts this string to a 64-bit floating-point number
	*/
	operator double() const {return atof(str());}
	/**
	Converts this string to a 64-bit integer number
	*/
	operator Long() const {return toLong();}
	/**
	Returns true if this string is not empty (warning: this might change in the future to mean isTrue(), use `ok()`)
	*/
	operator bool() const {return _len > 0;}
	/**
	Returns true if this string is empty (warning: this might change in the future to mean !isTrue(), use `!ok()`)
	*/
	bool operator!() const {return _len == 0;}
	/**
	Returns true if this string is not empty
	*/
	bool ok() const { return _len > 0; }
	/**
	Returns a const pointer to the beginning of the character data (suitable for functions
	requiring C-style strings)
	*/
	operator const char*() const {return str();}
	/**
	Returns a pointer to the beginning of the character data (suitable for functions
	requiring C-style strings)
	*/
	operator char*() const {return str();}
	/**
	Returns a const pointer to a Unicode UCS2 representation of this string by expanding from the internal
	byte representation (suitable for functions requiring C-style wide strings (LPWSTR))
	*/
	operator const wchar_t*() const;

	operator wchar_t*() const {return (wchar_t*)(const wchar_t*)*this;}
	
	/**
	Returns a const pointer to the beginning of the character data (suitable for functions
	for functions requiring C-style strings)
	*/
	const char* operator*() const {return str();}
	int toInt() const {return atoi(str());}
	double toDouble() const { return atof(str()); }
	float toFloat() const {return (float)atof(str());}
	/**
	Returns true if this string represents a non-false value (e.g. none of: "", "0", "N", "false", "no")
	*/
	bool isTrue() const;
	Long toLong() const;
	/**
	Converts this string to an unsigned integer number by interpreting it as an hexadecimal number
	*/
	unsigned hexToInt() const {return (unsigned int)strtoul(str(), NULL, 16);}
	/**
	Resizes this string to a length of `n`, keeping or not its original contents
	*/
	String& resize(int n, bool keep=true, bool newlen=true);
	/**
	Restores the internal state of a String that has been modified externally and changed its length
	*/
	String& fix();
	String& fixW();
	/**
	Restores the internal state of a String that has been modified externally and changed its length to a known value
	*/
	String& fix(int n) { _len = n; return *this; }
	/**
	Returns a list of Unicode characters (code points) in this string
	*/
	Array<int> chars() const;

	String concat(const char* b, int n) const;
	void append(const char* b, int n);
	void assign(const char* b, int n);

	void clear() { _len = 0; str()[0] = '\0'; }
	void operator=(const String& s) {assign(s.str(), s._len);}
	void operator=(const char* s) {assign(s, (int)strlen(s));}
	void operator=(char* const s) {assign(s, (int)strlen(s));}
	void operator=(int n) {(*this)=(String)n;}
	void operator=(double n) {(*this)=(String)n;}
	void operator=(bool n) {(*this)=(String)n;}
	template <class T>
	void operator=(const T& x) { String s=x; *this = s; }
	String operator+(const String& b) const {return concat(b.str(), b._len);}
	String operator+(const char* b) const {return concat(b, (int)strlen(b));}
	String operator+(char b) const {return concat(&b, 1);}
	void operator+=(const String& b) {append(b.str(), b._len);}
	void operator+=(const char* b) {append(b, (int)strlen(b));}
	void operator+=(char b) {int n=_len+1; if(n>=_size) resize(n); char*s = str(); s[n-1] = b; s[n] = '\0'; _len=n;}

	template<class T>
	String& operator<<(const T& x) {*this += String(x); return *this;}

	//template<>
	String& operator<<(const String& x) {*this += x; return *this;}

	String& operator<<(char x) {*this += x; return *this;}

	String& operator<<(const char* x) {*this += x; return *this;}

	bool operator==(const String& s) const
	{return (_len!=s._len)?false:!memcmp(str(), s.str(), _len);}
	bool operator==(const char* s) const {return !strcmp(str(),s);}
	bool operator==(char c) const {return _len==1 && str()[0]==c;}
	bool operator!=(const String& s) const
	{return (_len!=s._len)?true:memcmp(str(),s.str(),_len)!=0;}
	bool operator!=(const char* s) const {return strcmp(str(),s)!=0;}
	bool operator!=(char c) const {return _len!=1 || str()[0]!=c;}
	bool operator<(const String& s) const {return strcmp(str(), s.str())<0;}
	/**
	Returns the left string if it is not empty, or the right otherwise
	*/
	const String& operator|(const String& s) const { return (_len == 0) ? s : *this; }
	/**
	Returns the left string if it is not empty, or the right otherwise
	*/
	template<class T>
	String operator|(const T& s) const { return (*this) | String(s); }

	/**
	Returns a reference to the `i`-th character in this string (byte-based)
	*/
	char& operator[](int i) {return str()[i];}
	/**
	Returns a reference to the `i`-th character in this string (byte-based)
	*/
	const char& operator[](int i) const {return str()[i];}
	int compare(const String& s) const {return strcmp(str(), s.str());}
	int compare(const char* s) const {return strcmp(str(), s);}
	bool equalsNocase(const String& s) const {return toUpperCase() == s.toUpperCase();}
	/**
	Returns the first index where character `c` appears in this string, optionally starting search at position
	`i0`, or -1 if it is not found.
	*/
	int indexOf(char c, int i0=0) const;
	/**
	Returns the first index where substring `s` appears in this string, optionally starting search at position
	`i0`, or -1 if it is not found.
	*/
	int indexOf(const char* s, int i0=0) const;
	/**
	Returns the first index where substring `s` appears in this string, optionally starting search at position
	`i0`, or -1 if it is not found.
	*/
	int indexOf(const String& s, int i0=0) const {return indexOf((const char*)s, i0);}
	/**
	Returns the last index where character `c` appears in this string, or -1 if it is not found.
	*/
	int lastIndexOf(char c) const {char* p=strrchr(str(), c); return p?int(p-str()):-1;}
	/**
	Returns the last index where string `s` appears in this string, or -1 if it is not found.
	*/
	int lastIndexOf(const char* s) const;
	/**
	Returns the length of this string in bytes
	*/
	int length() const {return _len;}
	/**
	Returns the number of full characters in the string (may be different from `length()` )
	*/
	int count() const {const wchar_t* w(*(String*)this); return (int)wcslen(w);} // to improve
	/**
	Return the substring starting at position `i` and up to but not including position `j`
	*/
	String substring(int i, int j) const;
	/**
	Return the substring starting at position `i` and up to the end
	*/
	String substring(int i) const {return substring(i, _len);}
	/**
	Return the substring starting at position `i` with at most `n` chars, if `i` is negative it counts from the end
	*/
	String substr(int i, int n) const;
	String substr(int i) const { return substr(i, _len); }
	/**
	Return a string that is like this but without space at the beginning or end
	*/
	String trimmed() const;
	/**
	Removes space at the beginning and end of the string
	*/
	String& trim();
	/**
	Tests if this string starts with the given substring
	*/
	bool startsWith(const String& s) const { return _len >= s.length() && strncmp(str(), s, s.length()) == 0; }
	bool startsWith(const char* s) const { int n = (int)strlen(s); return _len >= n && strncmp(str(), s, n) == 0; }
	/**
	Tests if this string ends with the given substring
	*/
	bool endsWith(const String& s) const { return _len >= s.length() && strncmp(str() + _len - s.length(), s, s.length()) == 0; }
	bool endsWith(const char* s) const { int n = (int)strlen(s); return _len >= n && strncmp(str() + _len - n, s, n) == 0; }
	/**
	Tests if this string starts with the given character
	*/
	bool startsWith(char c) const { return str()[0] == c; }
	/**
	Tests if this string ends with the given character
	*/
	bool endsWith(char c) const { return _len > 0 && str()[_len-1] == c; }
	/**
	Tests if this string contains the given substring
	*/
	bool contains(const char* s) const {return indexOf(s)>=0;}
	bool contains(const String& s) const {return indexOf(s)>=0;}
	/**
	Tests if this string contains the given character
	*/
	bool contains(char s) const {return indexOf(s)>=0;}
	/**
	Returns a lowercase version of this string
	*/
	String toLowerCase() const;
	/**
	Returns an uppercase version of this string
	*/
	String toUpperCase() const;

	void split(Array<String>& out) const;

	/**
	Returns a list of strings obtained by cutting this string by whitespace.

	~~~
	String("  a  \t\n b  c   ").split() -> ["a", "b", "c"]
	~~~
	*/
	Array<String> split() const
	{
		Array<String> a;
		split(a);
		return a;
	}

	template <class T>
	Array<T> split_() const
	{
		Array<T> a;
		char* s = str();
		for (int i = 0; i <= length(); i++)
		{
			if (myisspace(s[i]))
				continue;
			for (int j = i + 1; j<length() + 1; j++)
			{
				if (myisspace(s[j]) || s[j] == '\0')
				{
					a << (T)substring(i, j);
					i = j;
					break;
				}
			}
		}
		return a;
	}

	void split(const String& sep, Array<String>& out) const;

	/**
	Returns a list of strings obtained by cutting this string by occurences of the separator `sep`.

	~~~
	String("a,b,c").split(",") -> ["a", "b", "c"]
	~~~
	*/
	Array<String> split(const String& sep) const
	{
		Array<String> a;
		split(sep, a);
		return a;
	}

	/**
	Parses a string and creates a `Dic` using `sep1` as pair separator (e.g.\ ','), and `sep2` as key/value separator (e.g.\ '=').
	
	~~~
	String("a:1,b:2,c:3").split(",", ":") -> {"a": "1", "b": "2", "c": "3"}
	~~~
	*/
	Dic<String> split(const String& sep1, const String& sep2) const;

	/**
	Returns a string like this one but in which occurences of substring `a` are replaced by `b`
	*/
	String replace(const String& a, const String& b) const;
	/**
	Replaces all occurences of character `a` with `b` *in place*
	*/
	String& replaceme(char a, char b);

	struct ASL_API Enumerator
	{
		const char* u;
		int i, n;
		Enumerator(const String& s): u(s), i(0), n(1) {}
		bool operator!=(const Enumerator& e) const { return (bool)*this; }
		void operator++() {u += n;}
		int operator*();
		int operator~() {return i;}
		operator bool() const {return *u != 0;}
	};
	Enumerator all() {return Enumerator(*this);}
	Enumerator all() const {return Enumerator(*this);}
};

inline String operator+(const char* a, const String& b)
{
	String s(a);
	s += b;
	return s;
}

inline String operator+(const char a, const String& b)
{
	String s(a);
	s += b;
	return s;
}

#ifdef ASL_STRING_CONCAT_ANY
template <class T>
inline String operator+(const T& a, const String& b)
{
	return String(a)+b;
}

template <class T>
inline String operator+(const String& a, const T& b)
{
	return a+String(b);
}
#endif

template<class T>
String Array<T>::join(const String& sep) const
{
	int n=length();
	if(n==0) return "";
	String s = (_a[0]);
	for(int i=1; i<n; i++) {s+=sep; String v=_a[i]; s+=(v);}
	return s;
}

#ifdef ASL_HAVE_RANGEFOR

inline String::Enumerator begin(const String& s)
{
	return s.all();
}

inline String::Enumerator end(const String& s)
{
	return s.all();
}

#endif


/**
This is a helper class to use a String as an output parameter of a C function using a pointer to a buffer.
At the function return the underlying String will recompute its length. Works with functions using 8-bit
characters or wide characters.

Use it like this. For example, to get the current directory:

~~~
String name;
GetCurrentDirectoryW(200, SafeString(name, 200));
~~~

That will expand the internal space of `name` to 200 characters, then call that Unicode Win32 function, then
convert the resulting data back to UTF-8, and the internal length will be set to the real string length.
*/

class SafeString
{
	String& _s;
	mutable bool _wide;
public:
	SafeString(String& s): _s(s), _wide(false) {}
	SafeString(String& s, int n) : _s(s), _wide(false) { _s.resize(3 * n); }
	~SafeString() { if (_wide) _s.fixW(); else _s.fix(); }
	operator const char*() const {return _s;}
	operator char*() const {return _s;}
	operator const wchar_t*() { _wide = true; return (wchar_t*)_s; }
	operator const wchar_t*() const { _wide = true; return(const wchar_t*)(String*)&_s; }
	operator wchar_t*() { _wide = true; return(wchar_t*)(const wchar_t*)_s; }
};


int ASL_API utf16toLocal8(const wchar_t* p, char* u, int nmax);
int ASL_API local8toUtf16(const char* u, wchar_t* p, int nmax);
int ASL_API utf16toUtf8(const wchar_t* p, char* u, int);
int ASL_API utf8toUtf16(const char* u, wchar_t* p, int);
int ASL_API utf32toUtf8(const int* p, char* u, int);
int ASL_API utf8toUtf32(const char* u, int* p, int);
String ASL_API localToString(const String& a);
}

#ifdef QSTRING_H
#include <asl/qt_interop.h>
#endif

#endif
