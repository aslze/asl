#ifndef ASL_BUFFER_H
#define ASL_BUFFER_H

#include "Array.h"

#ifndef ASL_BIGENDIAN
#define ASL_OTHERENDIAN BIGENDIAN
#else
#define ASL_OTHERENDIAN LITTLEENDIAN
#endif

namespace asl {


template <class T>
struct AsBytes
{
	union { T x; byte b[sizeof(T)]; };
	AsBytes() {}
	AsBytes(const T& a) : x(a) {}
	byte& operator[](int i) { return b[i]; }
};

template <class T, class T2>
struct AsOther
{
	union { T x; T2 y; };
	AsOther() {}
	AsOther(const T& a) : x(a) {}
};

/**
This class allows reading a memory buffer as a binary stream. It can read bytes, integers of different sizes and floating
point numbers in big-endian or little-endian byte order. You have to make sure you don't read past the bounds of the buffer.

~~~
Array<byte> data = File("data.bin").content();
StreamBufferReader buffer (data, StreamBufferReader::BIGENDIAN);
int n = buffer.read<int>();
double x, y, z;
buffer >> x >> y >> z;
~~~

\ingroup Binary
*/

class ASL_API StreamBufferReader
{
public:
	enum Endian { BIGENDIAN, LITTLEENDIAN };
	/**
	Constructs a buffer reader from a byte array
	*/
	StreamBufferReader(const Array<byte>& data, Endian e = LITTLEENDIAN) : _ptr(data.ptr()), _end(data.ptr() + data.length()), _endian(e) {}
	/**
	Constructs a buffer reader from a raw byte array
	*/
	StreamBufferReader(const byte* data, int n, Endian e = LITTLEENDIAN) : _ptr(data), _end(data + n), _endian(e) {}
	/**
	Sets the endianness for reading (can be changed on the fly)
	*/
	void setEndian(Endian e) { _endian = e; }
	operator bool() const { return _ptr < _end; }

	const byte* ptr() const { return _ptr; }

	/**
	Skips a number of bytes
	*/
	void skip(int n) { _ptr += n; }

	template <class T>
	StreamBufferReader& read2(T& x)
	{
		AsOther<T, unsigned short> a;
		if (_endian == BIGENDIAN)
			a.y = ((unsigned short)_ptr[0] << 8) | ((unsigned short)_ptr[1]);
		else
			a.y = ((unsigned short)_ptr[1] << 8) | ((unsigned short)_ptr[0]);
		x = a.x;
		_ptr += 2;
		return *this;
	}
	
	template <class T>
	StreamBufferReader& read4(T& x)
	{
		AsOther<T, unsigned> a;
		if (_endian == BIGENDIAN)
			a.y = ((unsigned)_ptr[0] << 24) | ((unsigned)_ptr[1] << 16) | ((unsigned)_ptr[2] << 8) | ((unsigned)_ptr[3]);
		else
			a.y = ((unsigned)_ptr[3] << 24) | ((unsigned)_ptr[2] << 16) | ((unsigned)_ptr[1] << 8) | ((unsigned)_ptr[0]);
		x = a.x;
		_ptr += 4;
		return *this;
	}
	
	template <class T>
	StreamBufferReader& read8(T& x)
	{
		AsOther<T, ULong> a;
		if (_endian == BIGENDIAN)
			a.y = ((ULong)_ptr[0] << 56) | ((ULong)_ptr[1] << 48) | ((ULong)_ptr[2] << 40) | ((ULong)_ptr[3] << 32)
			    | ((ULong)_ptr[4] << 24) | ((ULong)_ptr[5] << 16) | ((ULong)_ptr[6] << 8) | ((ULong)_ptr[7]);
		else
			a.y = ((ULong)_ptr[7] << 56) | ((ULong)_ptr[6] << 48) | ((ULong)_ptr[5] << 40) | ((ULong)_ptr[4] << 32)
			    | ((ULong)_ptr[3] << 24) | ((ULong)_ptr[2] << 16) | ((ULong)_ptr[1] << 8) | ((ULong)_ptr[3]);
		x = a.x;
		_ptr += 8;
		return *this;
	}
	
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

protected:
	const byte* _ptr;
	const byte* _end;
	Endian _endian;
};


/**
This class is a buffer that can be written to as a binary stream. The buffer is initially
empty and grows as you append variables. You can change endianness at any moment.

You can then get the content as an Array<byte> and for example write it to a file or send it
through a socket.
~~~
StreamBuffer buffer(StreamBuffer::BIGENDIAN);
buffer << 1 << "abc" << 1.5 << short(33);

File("data").put(buffer);

socket << *buffer;
~~~

\ingroup Binary
*/

class StreamBuffer : public Array<byte>
{
public:
	enum Endian { NATIVEENDIAN, BIGENDIAN, LITTLEENDIAN };

	StreamBuffer(Endian e = NATIVEENDIAN) : _endian(e) {}

	Array<byte>& operator*() { return (Array<byte>&)*this; }
	const Array<byte>& operator*() const { return (const Array<byte>&)*this; }
	
	/**
	Set endianness for binary writing
	*/
	void setEndian(Endian e) { _endian = e; }
	void write(const void* data, int n)
	{
		append((const byte*)data, n);
	}
	/**
	Writes variable x to the buffer respecting endianness in binary form
	*/
	template<class T>
	StreamBuffer& operator<<(const T& x)
	{
		T y = (_endian == ASL_OTHERENDIAN) ? bytesSwapped(x) : x;
		write(&y, sizeof(x));
		return *this;
	}

	template<class T>
	StreamBuffer& operator<<(const byte& x)
	{
		(*this) << x;
		return *this;
	}

	template<class T>
	StreamBuffer& operator<<(const char& x)
	{
		(*this) << (byte&)x;
		return *this;
	}

	template<class T>
	StreamBuffer& operator<<(const signed char& x)
	{
		(*this) << (byte&)x;
		return *this;
	}

	template<class T>
	StreamBuffer& operator<<(const Array<T>& x)
	{
		if (_endian == ASL_OTHERENDIAN)
		{
			foreach(const T& y, x)
				*this << y;
		}
		else
			write(&x[0], x.length());
		return *this;
	}

	StreamBuffer& operator<<(const Array<byte>& x)
	{
		write(x.ptr(), x.length());
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

}

#undef ASL_OTHERENDIAN
#endif
