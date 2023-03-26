// Copyright(c) 1999-2023 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_FILE_H
#define ASL_FILE_H

#include <asl/String.h>
#include <asl/Date.h>
#include <stdio.h>

namespace asl {

/*
Current solution: copy does not copy the file handle, only the path and _info. Only the object
that opened the file will close it on destruction.
*/

struct FileInfo
{
	Long size;
	Date lastModified;
	Date creationDate;
	unsigned flags;
	FileInfo() : size(-1), lastModified(0), creationDate(0), flags(0) {}
	bool operator!() const {return size==-1;}
};

/**
Class File represents a file in the filesystem. It can be used to get information about
a file (its size, its modification date, whether it exists), to read or write in it in one step,
without explicitly opening it, or to read and write in a more conventional way. The file is
automatically closed on object destruction. For text files you should use class TextFile.

~~~
Date lasttime = File("access.log").lastModified();
ByteArray data = File("data.bin").content();
long long size = File("video.mp4").size();
~~~

There are also conventional functions to explicitly open a file for reading or writing:

~~~
File file("data.bin", File::WRITE);   // opened in constructor
file.seek(16);
file.write(buffer, sizeof(buffer));   // closed in destructor
~~~
*/
class ASL_API File
{
protected:
	FILE* _file;
	String _path;
	mutable FileInfo _info;
	Endian _endian;
public:
	enum OpenMode{READ, WRITE, APPEND, RW, TEXT=8};
	enum SeekMode{START, HERE, END};
	static const char SEP;
	friend class Directory;
	/**
	Constructs a File object with no associated file.
	*/
	File():  _file(0), _endian(ENDIAN_NATIVE) {}
	ASL_EXPLICIT File(const String& name, const FileInfo& inf) : _file(0), _path(name), _info(inf), _endian(ENDIAN_NATIVE) {}
	/**
	Constructs a File object with the given path name but does not open it
	*/
	ASL_EXPLICIT File(const String& name) : _file(0), _path(name), _endian(ENDIAN_NATIVE) {}
	/**
	Constructs a File object with the given path name and opens it with the given access mode.
	\param name File name optionally including path
	\param mode Access mode: READ, WRITE, APPEND, RW (read+write)
	*/
	ASL_EXPLICIT File(const String& name, OpenMode mode) : _path(name), _endian(ENDIAN_NATIVE)
	{
		open(name, mode);
	}
	File(const File& f) : _file(0), _path(f._path), _info(f._info), _endian(f._endian)
	{
	}
	~File()
	{
		if (_file && _path.ok())
			close();
	}
	File& operator=(const File& f)
	{
		_path = f._path;
		if(_file)
			close();
		_info = f._info;
		return *this;
	}

	void setEndian(Endian e) { _endian = e; }

	void use(FILE* f) { _file = f; }

	FILE* stdio() { return _file; }
	/**
	Returns true if this object refers to an *open* file
	*/
	operator bool() const {return _file!=0;}
	/**
	Returns true if this object does not refer to an *open* file
	*/
	bool operator==(const File& f) const { return _path == f._path; }
	bool operator!=(const File& f) const { return _path != f._path; }
	bool operator<(const File& f) const { return _path < f._path; }
	bool operator!() const {return _file==0;}
	/**
	Returns the file's last modification date
	*/
	Date lastModified() const;
	/**
	Returns the file's creation date
	*/
	Date creationDate() const;
	/**
	Sets the file's last modification date
	*/
	bool setLastModified(const Date& t);
	/**
	Returns the file size
	*/
	Long size() const;
	/**
	Returns the file's name (without its directory)
	*/
	String name() const;
	/**
	Returns the file's extension (what follows the last dot)
	*/
	String extension() const;

	/**
	Creates a temporary file with optional extension and returns its path
	*/
	static File temp(const String& ext = ".tmp");

	/**
	Returns true if this file's extension is any of those given (separated by '|'), case-insensitively.
	*/
	bool hasExtension(const String& exts) const;
	/**
	Returns the full path of the file
	*/
	const String& path() const {return _path;}
	/**
	Returns the directory containing the file
	*/
	String directory() const;
	/**
	Returns true if the file is a directory
	*/
	bool isDirectory() const;
	/**
	Returns true if the file exists
	*/
	bool exists() const {return creationDate().time() != 0;}

	/**
	Copies this file to a new directory or name
	*/
	bool copy(const String& to);
	/**
	Moves or renames this file to `to` which can be a full name or a destination directory
	*/
	bool move(const String& to);
	/**
	Deletes the file
	*/
	bool remove();

	/**
	Returns the first n bytes in the file
	*/
	ByteArray firstBytes(int n);
	/**
	Returns the binary content of the file as an array of bytes
	*/
	ByteArray content();
	/**
	Writes the binary content of the file from an array of bytes. Returns false on failure
	*/
	bool put(const ByteArray& data);

	/**
	Opens the file with the specified access mode
	*/
	bool open(const String& name, OpenMode mode=READ);
	/**
	Opens the file with the specified access mode
	*/
	bool open(OpenMode mode=READ)
	{
		return open(_path, mode);
	}
	/**
	Closes the file
	*/
	void close();
	void setBuffering(char mode, int s=0); // nlf

	/**
	Returns the current position in the file
	*/
	Long position();
	/**
	Moves the file pointer to the given position
	\param offset new offset position in file
	\param from from where the offset is expressed: START (beginning of file), HERE (current position), END (file end)
	*/
	void seek(Long offset, SeekMode from=START);
	/**
	Returns true if the file pointer reached the end of the file
	*/
	bool end() {return feof(_file) != 0;}
	/**
	Flushes the write buffers effectively writing data on disk
	*/
	void flush() {fflush(_file);}
	bool error() {return ferror(_file)!=0;}

	/**
	Reads n bytes from the file into the buffer pointed to by p
	*/
	int read(void* p, int n);
	/**
	Writes n bytes from the buffer pointed to by p into the file
	*/
	int write(const void* p, int n);

	/**
	Writes variable x to the file respecting endianness in binary form
	*/
	template<class T>
	File& operator<<(const T& x)
	{
		T y = (_endian == ASL_OTHER_ENDIAN) ? bytesSwapped(x) : x;
		write(&y, sizeof(x));
		return *this;
	}

	/**
	Reads variable x from the file respecting endianness in binary form
	*/
	template<class T>
	File& operator>>(T& x)
	{
		read(&x, sizeof(x));
		if (_endian == ASL_OTHER_ENDIAN)
			swapBytes(x);
		return *this;
	}

	File& operator>>(char& x)
	{
		read(&x, sizeof(x));
		return *this;
	}

	File& operator>>(byte& x)
	{
		read(&x, sizeof(x));
		return *this;
	}

	template<class T>
	File& operator<<(const Array<T>& x)
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

	File& operator<<(const Array<byte>& x)
	{
		write(&x[0], x.length());
		return *this;
	}

	File& operator<<(const char* x)
	{
		write(x, (int)strlen(x));
		return *this;
	}

	File& operator<<(const String& x)
	{
		write(*x, x.length());
		return *this;
	}

	File& operator>>(String& x) // do what? read size then data? read until 0?
	{
		int n;
		*this >> n;
		x.resize(n);
		x[n] = '\0';
		read(&x[0], n);
		return *this;
	}

	template <class T>
	T read() { T x; *this >> x; return x; }
};

template<> template<>
Array<String> Array<File>::with<String>() const;

#ifdef ASL_HASHMAP_H
inline int hash(const File& f)
{
	return hash(f.path());
}
#endif

}
#endif
