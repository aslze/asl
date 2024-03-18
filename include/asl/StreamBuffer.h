// Copyright(c) 1999-2024 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_BUFFER_H
#define ASL_BUFFER_H

#include "Array.h"
#include "String.h"

namespace asl
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812 26495)
#endif

class File;
class Socket;

template <class T>
struct AsBytes
{
	union { T x; byte b[sizeof(T)]; };
	AsBytes() {}
	AsBytes(const T& a) { memcpy(b, &a, sizeof(T)); }
	byte& operator[](int i) { return b[i]; }
};

template <class T, class T2>
struct AsOther
{
	byte b[sizeof(T)];

	AsOther(const T& x) { memcpy(b, &x, sizeof(T)); }

	T2 other() const
	{
		T2 y;
		memcpy(&y, b, sizeof(T));
		return y;
	}
};

/**
This class allows reading a memory buffer as a binary stream. It can read bytes, integers of different sizes and floating
point numbers in big-endian or little-endian byte order. You have to make sure you don't read past the bounds of the buffer.

~~~
ByteArray data = File("data.bin").content();
StreamBufferReader buffer (data, ENDIAN_BIG);
int n = buffer.read<int>();
double x, y, z;
buffer >> x >> y >> z;
~~~

\ingroup Binary
*/

class ASL_API StreamBufferReader
{
public:
	/**
	Constructs a buffer reader from a byte array
	*/
	StreamBufferReader(const ByteArray& data, Endian e = ENDIAN_LITTLE) : _ptr(data.data()), _end(data.data() + data.length()), _endian(e) {}
	/**
	Constructs a buffer reader from a raw byte array
	*/
	StreamBufferReader(const byte* data, int n, Endian e = ENDIAN_LITTLE) : _ptr(data), _end(data + n), _endian(e) {}
	/**
	Sets the endianness for reading (can be changed on the fly)
	*/
	void setEndian(Endian e) { _endian = e; }

	operator bool() const { return _ptr < _end; }

	const byte* ptr() const { return _ptr; }

	const byte* end() const { return _end; }

	int length() const { return (int)(_end - _ptr); }

	/**
	Skips a number of bytes
	*/
	StreamBufferReader& skip(int n)
	{
		_ptr += n;
		return *this;
	}

	template <class T>
	StreamBufferReader& read2(T& x)
	{
		AsOther<unsigned short, T> a((_endian == ENDIAN_BIG)
		                                 ? ((unsigned short)_ptr[0] << 8) | ((unsigned short)_ptr[1])
		                                 : ((unsigned short)_ptr[1] << 8) | ((unsigned short)_ptr[0]));
		x = a.other();
		_ptr += 2;
		return *this;
	}
	
	template <class T>
	StreamBufferReader& read4(T& x)
	{
		AsOther<unsigned, T> a(
		    (_endian == ENDIAN_BIG)
		        ? ((unsigned)_ptr[0] << 24) | ((unsigned)_ptr[1] << 16) | ((unsigned)_ptr[2] << 8) | ((unsigned)_ptr[3])
		        : ((unsigned)_ptr[3] << 24) | ((unsigned)_ptr[2] << 16) | ((unsigned)_ptr[1] << 8) | ((unsigned)_ptr[0]));
		x = a.other();
		_ptr += 4;
		return *this;
	}
	
	template <class T>
	StreamBufferReader& read8(T& x)
	{
		AsOther<ULong, T> a(
		    (_endian == ENDIAN_BIG)
		        ? ((ULong)_ptr[0] << 56) | ((ULong)_ptr[1] << 48) | ((ULong)_ptr[2] << 40) | ((ULong)_ptr[3] << 32) |
		              ((ULong)_ptr[4] << 24) | ((ULong)_ptr[5] << 16) | ((ULong)_ptr[6] << 8) | ((ULong)_ptr[7])
		        : ((ULong)_ptr[7] << 56) | ((ULong)_ptr[6] << 48) | ((ULong)_ptr[5] << 40) | ((ULong)_ptr[4] << 32) |
		              ((ULong)_ptr[3] << 24) | ((ULong)_ptr[2] << 16) | ((ULong)_ptr[1] << 8) | ((ULong)_ptr[0]));
		x = a.other();
		_ptr += 8;
		return *this;
	}
	
	StreamBufferReader& operator>>(bool& x) { x = *_ptr != 0; _ptr++; return *this; }

	StreamBufferReader& operator>>(signed char& x) { x = *(const char*)_ptr; _ptr++; return *this; }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(char& x) { x = *(const char*)_ptr; _ptr++; return *this; }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(byte& x) { x = *_ptr++; return *this; }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(short& x) { return read2(x); }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(unsigned short& x) { return read2(x); }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(int& x) { return read4(x); }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(unsigned& x) { return read4(x); }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(float& x) { return read4(x); }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(Long& x) { return read8(x); }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(ULong& x) { return read8(x); }
	/**
	Read one value from the buffer
	*/
	StreamBufferReader& operator>>(double& x) { return read8(x); }

	/**
	Read one value from the buffer: `float x = buffer.read<float>()`
	*/
	template <class T>
	T read() { T x; *this >> x; return x; }

	/**
	Reads n bytes into an Array (or all remaining bytes by default)
	*/
		ByteArray read(int n = -1) { if (n < 0) n = length();  ByteArray a(n); memcpy(a.data(), _ptr, n); _ptr += n; return a; }

protected:
	const byte* _ptr;
	const byte* _end;
	Endian _endian;
};

typedef StreamBufferReader BufferReader;

/**
This class is a buffer that can be written to as a binary stream. The buffer is initially
empty and grows as you append variables. You can change endianness at any moment.

You can then get the content as a ByteArray and for example write it to a file or send it
through a socket.
~~~
StreamBuffer buffer(ENDIAN_BIG);
buffer << 1 << "abc" << 1.5 << short(33);

File("data").put(buffer);

socket << *buffer;
~~~

\ingroup Binary
*/

class StreamBuffer : public Array<byte>
{
public:
	ASL_EXPLICIT StreamBuffer(Endian e = ENDIAN_LITTLE) : _endian(e) {}

	ByteArray&       operator*() { return (ByteArray&)*this; }
	const ByteArray& operator*() const { return (const ByteArray&)*this; }
	
	/**
	Set endianness for binary writing
	*/
	void setEndian(Endian e) { _endian = e; }

	void write(const void* data, int n) { append((const byte*)data, n); }
	/**
	Writes variable x to the buffer respecting endianness in binary form
	*/
	template<class T>
	StreamBuffer& operator<<(const T& x)
	{
		AsBytes<T> y(x);
		if (_endian == ASL_OTHER_ENDIAN)
			swapBytes(y);
		write(y.b, sizeof(T));
		return *this;
	}

	StreamBuffer& operator<<(const bool& x)
	{
		(ByteArray&)(*this) << byte(x ? 1 : 0);
		return *this;
	}

	StreamBuffer& operator<<(const byte& x)
	{
		(ByteArray&)(*this) << x;
		return *this;
	}

	StreamBuffer& operator<<(const char& x)
	{
		(ByteArray&)(*this) << *(byte*)&x;
		return *this;
	}

	StreamBuffer& operator<<(const signed char& x)
	{
		(ByteArray&)(*this) << *(byte*)&x;
		return *this;
	}

	template<class T>
	StreamBuffer& operator<<(const Array<T>& x)
	{
		if (_endian == ASL_OTHER_ENDIAN)
		{
			foreach(const T& y, x)
				*this << y;
		}
		else
			write(&x[0], x.length());
		return *this;
	}

	StreamBuffer& operator<<(const ByteArray& x)
	{
		write(x.data(), x.length());
		return *this;
	}

	StreamBuffer& operator<<(const char* x)
	{
		write(x, (int)strlen(x));
		return *this;
	}

	StreamBuffer& operator<<(const String& x)
	{
		write(*x, x.length());
		return *this;
	}

protected:
	Endian _endian;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

#endif
