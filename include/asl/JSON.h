#ifndef ASL_JSON_H
#define ASL_JSON_H
#include <asl/Var.h>

namespace asl {

/**
 * \defgroup XDL XML, XDL, and JSON
 * @{
 */

/*
Decodes the JSON-encoded string into a Var that will contain all the structure. It is similar to JavaScript's
`JSON.parse()`. If there are format parsing errors, the result will be a `Var::NONE` typed variable.
\deprecated Use Json::decode()
*/
Var ASL_API decodeJSON(const String& json);

/*
Encodes the given Var into a JSON-format representation. It is similar to JavaScript's
`JSON.stringify()`. If parameter `pretty` is true, an indented representation is produced.
\deprecated Use Json::encode()
*/
String ASL_API encodeJSON(const Var& data, bool pretty = false);

/**
Functions to encode/decode data as JSON. These functions use class Var to represent JSON values.

~~~
Var data = Json::decode("{\"x\":1.5, \"y\":[1,true]}");  // decode JSON from a string
data["z"] = 3.14;                                        // add a property
String json = Json::encode(data);                        // encode to string

Json::write("data.json", data);                          // write to file
~~~

The same `data` object can be built in one statement:

~~~ 
Var data = Var("x", 1.5)
              ("y", Var::array({1, true}))
			  ("z", 3.14);
~~~
*/
struct ASL_API Json
{
	/**
	Reads and decodes data from a file in JSON format
	*/
	static Var read(const String& file);
	/**
	Writes a var to a file in JSON format
	*/
	static bool write(const String& file, const Var& v, bool pretty=true);
	/**
	Decodes the JSON-encoded string into a Var that will contain all the structure. It is similar to JavaScript's
	`JSON.parse()`. If there are format parsing errors, the result will be a `Var::NONE` typed variable.
	*/
	static Var decode(const String& json) { return decodeJSON(json); }
	/**
	Encodes the given Var into a JSON-format representation. It is similar to JavaScript's
	`JSON.stringify()`. If parameter `pretty` is true, an indented representation is produced.
	*/
	static String encode(const Var& v, bool pretty = false) { return encodeJSON(v, pretty); }
};

/**@}*/

}
#endif
