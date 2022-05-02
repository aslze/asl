// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_INIFILE_H
#define ASL_INIFILE_H

#include <asl/Map.h>
#include <asl/HashMap.h>

namespace asl {

/**
A utility to read and write configuration files in INI format. On destruction, if there was any variable added or modified
the file is automatically saved. When reading a variable name, it is read from the current section. But if the name
contains a `/` then it is considered a "section/name" pair.

Example:

~~~
[main]
color = white

[network]
num_retries = 3
~~~

Can be read and written with:

~~~
IniFile config("settings.ini");
int num_retries = config["network/num_retries"];
config["main/color"] = "black";
~~~

The file in that example will be saved if the original `color` in section `main` was not "black".

The file can be cheked for correct reading and whether a variable exists:

~~~
IniFile config("settings.ini");
if(!config.ok()) {
	printf("Error reading %s\n", *config.fileName());
	return;
}
int num_retries = 5;
if(config.has("network/num_retries"))
	num_retries = config["network/num_retries"];
~~~

That last part can also be written as:

~~~
int num_retries = config["network/num_retries"] | 5;
~~~
*/

class ASL_API IniFile
{
public:
	class Section
	{
		String _title;
		HashDic<String> _vars;
	public:
		Section() {}
		Section(const String& t) : _title(t) {}
		const String& title() const {return _title;}
		String& operator[](const String& k) {return _vars[k];}
		const HashDic<String>& vars() const {return _vars;}
		Section clone() const;
		bool has(const String& k) const { return _vars.has(k); }
		friend class IniFile;
	};

	/**
	Opens an INI file from the given file
	*/
	IniFile(const String& fname, bool shouldwrite = true);
	/**
	Destroys the object and save the file if there were modifications
	*/
	~IniFile();

	/**
	Returns true if the file was read correctly
	*/
	bool ok() const {return _ok;}

	/**
	Returns the file name of this IniFile
	*/
	const String& fileName() const {return _filename;} 

	/**
	Returns the value associated with the key `name` in the current section
	if the name is like `section/name` then the given section is used.
	*/
	String& operator[](const String& name);

	const String operator[](const String& name) const;
	
	/**
	Returns the value associated with the key `name` or `defaultVal` if it was not found. 
	*/
	const String operator()(const String& name, const String& defaultVal) const;
	
	/**
	Writes the file with its modifications; this is done automatically on destruction, just call this if you need
	the file written before the object is detroyed
	*/
	void write(const String& fname="");

	/**
	Sets the current section to 'name' (variables will be read from here by default)
	*/
	Section& section(const String& name);

	const Dic<Section>& sections() const { return _sections; }

	/**
	Returns true if the file contains a key named `name`. If the name is like
	`section/name` then function tests a variable `name` in section `section`.
	*/
	bool has(const String& name) const;

	/**
	Returns the length of an "array" named `name` as written by the Qt library and enables reading its values.
	*/
	int arraysize(const String& name) const;

	/**
	Returns the value associated with field `name` at the array position `index` of the array last specified
	with `arraysize()`.
	*/
	String array(const String& name, int index) const;

protected:
	Dic<Section> _sections;
	mutable String _currentTitle;
	String _filename;
	String _indent;
	Array<String> _lines;
	bool _modified;
	bool _shouldwrite;
	bool _ok;
};

}
#endif
