// Copyright(c) 1999-2018 ASL author
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
	FileInfo() {size=-1;}
	bool operator!() const {return size==-1;}
};

/**
Class File represents a file in the filesystem. It can be used to get information about
a file (its size, its modification date, whether it exists), to read or write in it in one step,
without explicitly opening it, or to read and write in a more conventional way. The file is
automatically closed on object destruction. For text files you should use class TextFile.

~~~
Date lasttime = File("access.log").lastModified();
Array<byte> data = File("data.bin").content();
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
public:
	enum OpenMode{READ, WRITE, APPEND, RW, TEXT=8};
	enum SeekMode{START, HERE, END};
	static const char SEP;
	friend class Directory;
	/**
	Constructs a File object with no associated file.
	*/
	File() {_file=0;}
	File(const String& name, const FileInfo& inf) : _path(name) {_file=0; _info=inf;}
	/**
	Constructs a File object with the given path name but does not open it
	*/
	File(const String& name) : _path(name) {_file=0;}
	/**
	Constructs a File object with the given path name and opens it with the given access mode.
	\param name File name optionally including path
	\param mode Access mode: READ, WRITE, APPEND, RW (read+write)
	*/
	File(const String& name, OpenMode mode) : _path(name)
	{
		open(name, mode);
	}
	File(const File& f) : _path(f._path)
	{
		_file = 0;
		_info = f._info;
	}
	~File()
	{
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
	void use(FILE* f) {_file=f;}
	FILE* stdio() { return _file; }
	/**
	Returns true if this object refers to an *open* file
	*/
	operator bool() const {return _file!=0;}
	/**
	Returns true if this object does not refer to an *open* file
	*/
	bool operator==(const File& f) const { return _path == f._path; }
	bool operator<(const File& f) const { return _path < f._path; }
	bool operator!() const {return _file==0;}
	/** Returns the file's last modification date */
	Date lastModified() const;
	/** Returns the file's creation date */
	Date creationDate() const;
	/**
	Sets the file's last modification date
	*/
	bool setLastModified(const Date& t);
	/** Returns the file size */
	Long size() const;
	/** Returns the file's name (without its directory) */
	String name() const;
	/** Returns the file's extension (what follows the last dot) */
	String extension() const;

	/**
	Creates a temporary file with optional extension and returns its path
	*/
	static File temp(const String& ext = ".tmp");

	/**
	Returns true if this file's extension is any of those given (separated by '|'), case-insensitively.
	*/
	bool hasExtension(const String& exts) const;
	/** Returns the full path of the file */
	const String& path() const {return _path;}
	/** Returns the directory containing the file */
	String directory() const;
	/** Returns true if the file is a directory */
	bool isDirectory() const;
	/**	Returns true if the file exists */
	bool exists() const {return creationDate().time() != 0;}

	/** Copies this file to a new directory or name */
	bool copy(const String& to);
	/** Moves or renames this file to `to` which can be a full name or a destination directory */
	bool move(const String& to);
	/** Deletes the file */
	bool remove();

	/** Returns the first n bytes in the file */
	Array<byte> firstBytes(int n);
	/** Returns the binary content of the file as an array of bytes */
	Array<byte> content();
	/** Writes the binary content of the file from an array of bytes. Returns false on failure */
	bool put(const Array<byte>& data);

	/** Opens the file with the specified access mode */
	bool open(const String& name, OpenMode mode=READ);
	/** Opens the file with the specified access mode */
	bool open(OpenMode mode=READ)
	{
		return open(_path, mode);
	}
	bool openfd(int fd);
	/** Closes the file */
	void close();
	void setBuffering(char mode, int s=0); // nlf

	/** Returns the current position in the file */
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

	/** Reads n bytes from the file into the buffer pointed to by p */
	int read(void* p, int n);
	/** Writes n bytes from the buffer pointed to by p into the file */
	int write(const void* p, int n);

	/**
	Writes variable x in its binary form to this file
	*/
	template<class T>
	File& operator<<(const T& x)
	{
		write(&x, sizeof(x));
		return *this;
	}

	File& operator<<(const String& x);

	File& operator<<(const Array<char>& x)
	{
		write(x.ptr(), x.length());
		return *this;
	}

	File& operator<<(const Array<byte>& x)
	{
		write(x.ptr(), x.length());
		return *this;
	}

	File& operator<<(const char* x)
	{
		write(x, (int)strlen(x));
		return *this;
	}

	template<class T>
	File& operator>>(T& x)
	{
		read(&x, sizeof(x));
		return *this;
	}

	template <class T>
	T read() { T x; *this >> x; return x; }
};

/**
Class TextFile represents a text file in the filesystem. It can be used to get information about
a file (its size, its modification date, whether it exists), to read or write in it in one step, or to
open it and perform typical read/write operations as text, just like `File`.

Functions text()`, `lines()`, `put()`, `append()`, `printf()`, etc. read or write in the file by first opening it. Intended
as a short-hand for one-line operations.

~~~
TextFile("info.log").printf("Error connecting to port %i\n", port);

String text = TextFile("data.txt").text();

TextFile("data.json").put( encodeJSON(data) );

String last_entry = TextFile("errors.log").lines().last();
~~~

Explicit opening allows reading or writing in an `stdio` conventional style:

~~~
TextFile log("info.log", File::APPEND);
log.printf("There are %i new messages\n", n_messages);
log << "App exited with code " << exitCode << '\n';

TextFile log("http-access.log", File::READ)

while(!log.end())
{
	String line = log.readLine();
	...
}
~~~

*/


class ASL_API TextFile : public File
{
public:
	TextFile() {}
	TextFile(const String& name) : File(name) {}
	TextFile(FILE* f) {_file=f;}
	TextFile(const File& f) : File(f) {}
	TextFile(const String& name, OpenMode mode) {open(name, mode);}
	~TextFile() {}

	bool open(const String& name, OpenMode mode=READ)
	{
		return File::open(name, OpenMode(mode | TEXT));
	}
	bool open(OpenMode mode=READ)
	{
		return open(_path, mode);
	}
	bool openfd(int fd);

	/** Prints formatted text as with the regular printf. Returns false on failure */
	bool printf(const char* fmt, ...);
	/** Reads formatted text as with the regular scanf up to 4 items only! */
	int scanf(const String& fmt, void* p1, void* p2=0, void* p3=0, void* p4=0);
	/** Reads and returns a line from the file */
	String readLine();
	bool readLine(String& s);
	String readLine(char newline);

	/** Returns the textual content of the file as a string. The file does not need to be opened. */
	String text();
	/** Returns all the lines contained in the file as an array of strings. The file does not need to be opened. */
	Array<String> lines();
	/** Writes the given string at the end of the file. Returns false on failure. */
	bool append(const String& line);
	/** Replaces the content of the file with the given text string. Returns false on failure.
	*/
	bool put(const String& t);
	/** Writes the given string into the file replacing its content. Returns false on failure.
	If the file was opened the text is written normally at the end. */
	bool write(const String& t) { return put(t); }

	TextFile& operator>>(char &x);
	TextFile& operator>>(byte &x);
	TextFile& operator>>(int &x);
	TextFile& operator>>(unsigned &x);
	TextFile& operator>>(float &x);
	TextFile& operator>>(double &x);
	TextFile& operator>>(String &x);
	TextFile& operator<<(char x);
	TextFile& operator<<(byte x);
	TextFile& operator<<(int x);
	TextFile& operator<<(unsigned x);
	TextFile& operator<<(float x);
	TextFile& operator<<(double x);
	TextFile& operator<<(const String& x);
	TextFile& operator<<(char* x) {*this << String(x); return *this;}
	TextFile& operator<<(const char* x) {*this << String(x); return *this;}
};

}
#endif
