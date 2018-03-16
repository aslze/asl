#ifndef ASL_BUFFER_H
#define ASL_BUFFER_H

#include "Array.h"

#ifndef ASL_BIGENDIAN
#define ASL_OTHERENDIAN BIGENDIAN
#else
#define ASL_OTHERENDIAN LITTLEENDIAN
#endif

namespace asl {

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

* **THIS CLASS IS STILL ALPHA**
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
