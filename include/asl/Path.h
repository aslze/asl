// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_PATH
#define ASL_PATH

#include <asl/String.h>
#ifdef ASL_HAVE_MOVE
#include <utility>
#endif

namespace asl {

/**
This class is a utility to process file system path names.

~~~
Path path = "models/box.step";
String basename = path.nameNoExt();   // -> "box"
String newname = path.noExt() + ".stl";  // -> "models/box.stl"
if(path.hasExtension("STP|STEP")) {...}  // -> true (case insensitive test for extensions)

newpath = path.directory() / "subdir" / basename + ".json"; -> "/models/subdir/box.json"
~~~
*/

class ASL_API Path
{
	String _path;
	operator int() const;
public:
	Path() {}
	/**
	Constructs a path with a string (turns backslashes into slashes).
	*/
	Path(const String& p): _path(p) {_path.replaceme('\\', '/');}
	Path(const char* p) : _path(p) { _path.replaceme('\\', '/'); }
	operator const String&() const { return _path; }
#ifdef ASL_HAVE_MOVE
	operator String && () {
		return std::move(_path);
	}
#endif
	const char* operator*() const { return *_path; }
	const String& string() const { return _path; }
	/**
	Tests if this path is equal to the given string (ignoring backslash vs slash).
	*/
	bool operator==(const String& s) const { return _path == s.replace('\\', '/'); }

	bool operator==(const char* s) const { return _path == String(s).replace('\\', '/'); }

	bool operator!=(const String& s) const { return !(*this == s); }

	bool operator!=(const char* s) const { return !(*this == s); }

	bool operator!() const { return !_path.ok(); }

	operator bool() const { return _path.ok(); }
	/**
	Tests if this path refers to the same as p as absolute paths.
	*/
	bool equals(const Path& p) const { return absolute().string() == p.absolute().string(); }
	/**
	Returns the path file name (without its directory)
	*/
	String name() const;
	/**
	Returns the path extension (what follows the last dot)
	*/
	String extension() const;
	/**
	Returns true if this path's extension is any of those given (separated by '|'), case-insensitively.
	*/
	bool hasExtension(const String& exts) const;
	/**
	Returns the directory containing this path (without a trailing '/')
	*/
	Path directory() const;
	/**
	Returns this path without its extension
	*/
	Path noExt() const;
	/**
	Returns this path's file name without its extension
	*/
	String nameNoExt() const { return noExt().name(); }
	/**
	Returns the absolute path corresponding to this, possibly relative, path
	*/
	Path absolute() const;
	/**
	Returns true if this is an absolute path
	*/
	bool isAbsolute() const;
	/**
	Removes double dots in a path by stepping up one directory each time.
	*/
	Path& removeDDots();
	/**
	Concatenates this path with a suffix.
	*/
	Path operator+(const String& s) const { return _path + s; }
	Path operator+(const char* s) const { return _path + s; }
	Path operator+(char s) const { return _path + s; }
	/**
	Concatenates this path with another, and removes possible double slashes.
	*/
	Path operator/(const String& p) const { String s = _path; s << '/' << p; return s.replace("//", '/'); }
	Path operator/(const char* p) const { String s = _path; s << '/' << p; return s.replace("//", '/'); }
	//Path operator/(const Path& p) const {return *this/p._path;}
};

}

#endif
