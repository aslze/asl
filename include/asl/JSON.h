// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_JSON_H
#define ASL_JSON_H
#include <asl/Var.h>

namespace asl {

/**
 * \defgroup XDL XML, XDL, and JSON
 * @{
 */

/**
Functions to encode/decode data as JSON. These functions use class Var to represent JSON values.

~~~
Var data = Json::decode("{\"a\":\"abc\", \"b\":[1.5, 3]}"); // decode JSON from a string
data["c"] = true;                                           // add a property
String json = Json::encode(data);                           // encode to string

Json::write(data, "data.json");                             // write to file
~~~

The same `data` object can be built in one statement, in C++11 compilers:

~~~
Var data {
    {"a", "abc"},
    {"b", {1.5, 3.0}},
    {"c", true}
};
~~~

Or in older compilers:

~~~
Var data = Var("a", "abc")
              ("b", array<Var>(1.5, 3.0))
              ("c", true);
~~~
*/
struct ASL_API Json
{
	/**
	Options for Json::encode and Json::write
	*/
	enum Mode {
		NONE = 0,    //!< Compact format in a single line
		PRETTY = 1,  //!< Format with newlines and indentations
		SIMPLE = 2,  //!< Format real numbers with reduced precision
		COMPACT = 4,
		JSON = 8,
		EXACT = 16,
		SHORTF = 32, //!< Format doubles as short as floats
		NICE = 3     //!< Same as PRETTY and SIMPLE
	};

	/**
	Reads and decodes data from a file in JSON format
	*/
	static Var read(const String& file);
	/**
	Writes a var to a file in JSON format
	*/
	static bool write(const String& file, const Var& v, Mode mode = PRETTY);

	static bool write(const Var& v, const String& file, Mode mode = PRETTY)
	{
		return write(file, v, mode);
	}
	/**
	Decodes the JSON-encoded string into a Var that will contain all the structure. It is similar to JavaScript's
	`JSON.parse()`. If there are format parsing errors, the result will be a `Var::NONE` typed variable.
	*/
	static Var decode(const String& json);

	/**
	Encodes the given Var into a JSON-format representation. It is similar to JavaScript's
	`JSON.stringify()`.
	*/
	static String encode(const Var& v, Mode mode = NONE);
};

inline Json::Mode operator|(Json::Mode a, Json::Mode b)
{
	return Json::Mode(int(a) | int(b));
}

/**@}*/

}
#endif
