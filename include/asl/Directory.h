// Copyright(c) 1999-2019 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_DIR_H
#define ASL_DIR_H

#include <asl/String.h>
#include <asl/Date.h>
#include <asl/File.h>
#include <asl/Path.h>

namespace asl {

/**
This class allows enumerating the contents of a directory: its files and subdirectories.

This example lists the files of the c:/windows directory modified in the last 10 days:

~~~
Directory dir("c:/windows");
foreach(File& file, dir.files("*.dll"))
{
	if(file.lastModified() > Date::now() - 10*Date::DAY)
		cout << *file.name() << " size: " file.size() << endl;
}
~~~

In a Directory object, `files()` enumerates files, `subdirs()` enumerates subdirectories, and
`items()` enumerates both. In all three a wildcard can be given as argument to filter the search.

Each File object returned has the following members:

- `path()`: the full path of this item
- `name()`: the name of the item
- `directory()`: the full directory containing the item
- `size()`: the file size in bytes (or 0 if it is a directory)
- `lastModified()`: a Date indicating the last modification time
- `creationDate()`: a Date indicating the item's creation time

In fact they are File objects and any function of that class can be used on them.
*/

class ASL_API Directory
{
protected:
	String _path;
	Array<File> _files;
	enum ItemType { FILE, DIRE, ALL };

public:
	Directory() {}
	/**
	Constructs a directory from a relative or absolute path
	*/
	Directory(const String& name) : _path(name) {}
	Directory(const File& file) : _path(file.path()) {}
	Directory(const Path& file) : _path(file.string()) {}
	Directory(const char* file) : _path(file) {}

	/**
	Returns the name of the directory
	*/
	String name() const;
	/**
	Returns the full path of the directory
	*/
	String path() const {return _path;}
	/**
	Returns the parent directory containing this directory
	*/
	String directory() const;
	/**
	Returns true if this directory exists, or false if it does not exist or refers to a file
	*/
	bool exists() const
	{
		return File(_path).isDirectory();
	}
	/**
	Returns the contents of a directory
	*/
	const Array<File>& items(const String& which="*", ItemType t=ALL);
	/**
	Returns the files in a directory
	*/
	const Array<File>& files(const String& which="*") {return items(which, FILE);}
	/**
	Returns the subdirectories of a directory
	*/
	const Array<File>& subdirs(const String& which="*") {return items(which, DIRE);}
	static FileInfo getInfo(const String& path);
	/**
	Returns the current working directory
	*/
	static String current();
	/**
	Sets the current working directory
	*/
	static bool change(const String& dir);
	/**
	Creates a new directory (and its ancestors if they don't exist), returns false on failure
	*/
	static bool create(const String& name);
	/**
	Creates a new directory, returns false on failure (e.g if its parent does not exist)
	*/
	static bool createOne(const String& name);
	/**
	Creates a new temporary directory with an arbitrary name and returns its path
	*/
	static String createTemp();
	/**
	Copies file `from` to `to` which can be a full name or a destination directory
	*/
	static bool copy(const String& from, const String& to);
	/**
	Moves or renames file `from` to `to` which can be a full name or a destination directory
	*/
	static bool move(const String& from, const String& to);
	/**
	Deletes file or directory `path` (if it is a directory it must be empty)
	*/
	static bool remove(const String& path);
	/**
	Removes the given directory with all its content recursively and returns true on success (USE WITH CARE!)
	*/
	static bool removeRecursive(const String& path);
};

}
#endif
