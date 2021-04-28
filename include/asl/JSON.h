// Copyright(c) 1999-2021 aslze
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
Var data = Json::decode("{\"x\":\"abc\", \"y\":[1,true]}"); // decode JSON from a string
data["z"] = 3.14;                                           // add a property
String json = Json::encode(data);                           // encode to string

Json::write("data.json", data);                             // write to file
~~~

The same `data` object can be built in one statement:

~~~ 
Var data = Var("x", "abc")
              ("y", array<Var>(1, true))
              ("z", 3.14);
~~~

Or in C++11 compilers, also like this:

~~~
Var data {
    {"x", "abc"},
    {"y", Var::array({1, true})},
    {"z", 3.14}
};
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
		JSON = 8
	};
	/**
	Reads and decodes data from a file in JSON format
	*/
	static Var read(const String& file);
	/**
	Writes a var to a file in JSON format
	*/
	static bool write(const String& file, const Var& v, Mode mode = PRETTY);
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

	static String encode(const Var& v, bool pretty) { return encode(v, pretty ? PRETTY : NONE); }
};

/**@}*/

}
#endif
