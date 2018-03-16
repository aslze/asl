#include <asl/Path.h>
#include <asl/Directory.h>

using namespace asl;

#ifdef _WIN32
#define SEP '\\'
#else
#define SEP '/'
#endif

String Path::name() const
{
	int n = _path.lastIndexOf(SEP);
#ifdef _WIN32
	int m = _path.lastIndexOf('/');
	n = (m > n)? m : n;
#endif
	return _path.substring(n+1);
}

String Path::extension() const
{
	int n = _path.lastIndexOf(SEP);
#ifdef _WIN32
	int m = _path.lastIndexOf('/');
	n = (m > n) ? m : n;
#endif
	int dot = _path.lastIndexOf('.');
	return (dot >= 0 && dot > n) ? _path.substring(dot + 1) : "";
}

bool Path::hasExtension(const String& extensions) const
{
	Array<String> exts = extensions.toLowerCase().split('|');
	String ext = extension().toLowerCase();
	for (int i = 0; i < exts.length(); i++)
		if (ext == exts[i])
			return true;
	return false;
}

Path Path::directory() const
{
	int n = _path.lastIndexOf(SEP);
#ifdef _WIN32
	int m = _path.lastIndexOf('/');
	n = (m > n)? m : n;
#endif
	return (n>=0)? _path.substring(0, n) : ".";
}

Path& Path::removeDDots()
{
	Array<String> parts = _path.split('/');
	for(int i=1; i<parts.length(); i++)
	{
		if(parts[i]=="..") {
			parts.remove(i-1, 2);
			i-=2;
		}
	}
	_path = parts.join('/').replace("//", "/");
	return *this;
}

Path Path::absolute() const
{
	if (isAbsolute()) {
		Path p(_path);
		p.removeDDots();
		return p;
	}
	String current = Directory::current();
	current.replaceme('\\', '/');
	String all;
	all << current << '/' << (_path.startsWith("./")? _path.substring(2) : _path);
	Path p(all);
	p.removeDDots();
	return p;
}

bool Path::isAbsolute() const
{
	return _path[0] == '/' || _path[0] == '\\' || (_path.length() > 1 && _path[1] == ':');
}

Path Path::noExt() const
{
	int n = _path.lastIndexOf(SEP);
#ifdef _WIN32
	int m = _path.lastIndexOf('/');
	n = (m > n) ? m : n;
#endif
	int dot = _path.lastIndexOf('.');
	return (dot >= 0 && dot > n) ? _path.substring(0, dot) : _path;
}


