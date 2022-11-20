// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CMDLINE_H
#define CMDLINE_H

#include <asl/String.h>
#include <asl/Map.h>

namespace asl {

/**
CmdArgs is a utility to read command line arguments with options, similar to `getopt()` but simpler.
It allows to read named options (starting with a `-`), optionally followed by a value, and the rest of
arguments after the last option. Options without value are flags and should end with a '!'.

If an option is given more than once its value is the last one, but all its values are accessible with the () operator.
The arguments given by this object with the `length()` property and the `[int]` operator are the free arguments, not
counting options or the program name. The whole array of arguments received at `main()` can be obtained with the `all()`
function.

For example:

~~~
convert -format jpeg -fast! -q 85 image1.png image2.bmp
~~~

Has the option `format` with value `jpeg`, the flag `fast`, the option `q` with value 85 and 2 free arguments.

That can be read with:

~~~
CmdArgs args(argc, argv);

String format = args["format"];
int quality = args["q"] | 90;             // use value 90 if the option is not given
bool fast = args.is("fast");

for( int i=0; i < args.length(); i++)
{
	convertFile( args[i], format, quality, fast );
}
~~~

In case of multiple option values, use the () operator to get them:

~~~
compile -I /include -I /mylib/include a.cpp b.cpp
~~~

Their values can be still retrieved:

~~~
CmdArgs args(argc, argv);

Array<String> includes = args("I");

for(int i=0; i < includes.length(); i++)
{
	add_include_file( includes[i] );
}
~~~

The object can be given a specification of valid options. This is currently very simple, a comma-separated
list of options. Options requiring a value must be followed by a ':' character.

~~~
CmdArgs(argc, argv, "q:,fast,format:");
~~~

This object can be created without arguments and anywhere in a program (but this is less portable,
currently working on Windows and Linux).

~~~
CmdArgs args;
String name = args["name"];
~~~

*/


class ASL_API CmdArgs
{
public:
#if defined _WIN32 || defined __linux__
	/**
	Constructs a CmdArgs object with the arguments of the current process (only on Windows and Linux).
	*/
	CmdArgs(const String& spec = "");
#endif
	/** Constructs a CmdArgs object from the arguments to main() and an optional specification*/
	CmdArgs(int argc, char** argv, const String& spec = "") { parse(argc, argv, spec); }
	/** Tests if the given option exists */
	bool has(const String& opt) const;
	/** Returns the value of a flag option (if it exists with no value or has a value 1, true or yes) */
	bool is(const String& opt) const;
	/** Returns the value of option named `opt` or an empty string of it was not defined */
	String operator[](const String& opt) const;
	/** Returns the value of option named `opt` or the default value `def` if it was not defined */
	String operator()(const String& opt, const String& def) const;
	/** Returns the rest argument at index `i` (after the options) */
	String operator[](int i) const;
	/** Returns an array with all values of option `opt` if more than one was given */
	Array<String> operator()(const String& opt) const;
	/** Returns the free arguments, arguments excluding options */
	Array<String> rest() const;
	/** Returns all options and their values as a Map (only the last value if an option appears more than once) */
	Dic<String> options() const { return _opts; }
	/** Returns all the arguments, as received at main() */
	Array<String> all() const;
	/** Returns the number of free arguments, those excluding options */
	int length() const;
	/** Returns the options that have not been tested (probably wrong options) */
	const Array<String> untested() const { return _unused; }
private:
	void parse(int argc, char** argv, const String& spec = "");
	void use(const String& opt) const;
	Array<String> _args;
	Array<String> _rest;
	Dic<String> _opts;
	Dic< Array<String> > _multi;
	mutable Array<String> _unused;
};

}

#endif
