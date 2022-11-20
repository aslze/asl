// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_TEXTFILE_H
#define ASL_TEXTFILE_H

#include <asl/File.h>

namespace asl {

/**
Class TextFile represents a text file in the filesystem. It can be used to get information about
a file (its size, its modification date, whether it exists), to read or write in it in one step, or to
open it and perform typical read/write operations as text, just like `File`.

Functions text()`, `lines()`, `put()`, `append()`, `printf()`, etc. read or write in the file by first opening it. Intended
as a short-hand for one-line operations.

~~~
TextFile("info.log").printf("Error connecting to port %i\n", port);

String text = TextFile("data.txt").text();

TextFile("data.json").put( Json::encode(data) );

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
	ASL_EXPLICIT TextFile(const String& name) : File(name) {}
	ASL_DEPRECATED(ASL_EXPLICIT TextFile(FILE* f), "Use File::use(FILE*)") { _file = f; }
	TextFile(const File& f) : File(f) {}
	ASL_EXPLICIT TextFile(const String& name, OpenMode mode) { open(name, mode); }
	~TextFile() {}

	bool open(const String& name, OpenMode mode = READ)
	{
		return File::open(name, OpenMode(mode | TEXT));
	}
	bool open(OpenMode mode = READ)
	{
		return open(_path, mode);
	}

	bool end() { return (_file || open(_path, READ)) ? feof(_file) != 0 : true; }
	
	/** Prints formatted text as with the regular printf. Returns false on failure */
	bool printf(const char* fmt, ...);
	/** Reads formatted text as with the regular scanf up to 4 items only! */
	int scanf(const String& fmt, void* p1, void* p2 = 0, void* p3 = 0, void* p4 = 0);
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
	TextFile& operator<<(char* x) { *this << String(x); return *this; }
	TextFile& operator<<(const char* x) { *this << String(x); return *this; }
};

}
#endif

