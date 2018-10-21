/**
\mainpage ASL - All-purpose Simple Library

ASL is a collection of general purpose classes and utilities intended to be easy to use. It is mostly compatible
with **Windows**, **Linux**, **macOS** and **Android**, 32 and 64 bits.

__Operating system functionalities__:

- Threads, mutexes and semaphores
- Processes (run programs and read their output)
- Binary and text files
- Directory enumeration and file system operations (copy, move, delete)
- Sockets TCP, UDP, Unix and SSL/TLS encrypted sockets, IPv4/IPv6
- Runtime dynamically loadable libraries (DLLs/shared libraries)
- Console attributes: text color and cursor position
- Serial ports
- Shared memory

__Utilities__:

- JSON, XML and XDL parsing/encoding
- HTTP/HTTPS server and client
- WebSocket server and client
- Configuration INI files reading/writing
- CSV file reading/writing, ARFF write
- Log messages to console and/or files (with limited growth)
- Command line arguments and options parsing
- Singletons and Factories
- Base64, hex encoding/decoding

__Basic data types__:

- Strings with easy conversions and UTF8 support
- Dynamic and static arrays
- Maps and hashmaps
- Date/time
- 2D, 3D, 4D vectors, 3x3, 4x4 matrices and quaternions
- Variants (similar to JavaScript vars)

# Compilation and use #

ASL can be used as a static or as a dynamic/shared library. The easiest way to 
build and use it is via *CMake* (2.8.11 or later). By default both the dynamic 
and static libraries will be built. You can enable the `ASL_USE_LOCAL8BIT` 
option to disable interpretation of strings as UTF-8 and treat them as local 8-
bit strings.

Just compile the library with whatever compilers and architecturs (32/64bit) you need. There is no need to *install*.

In order to use the library in another project just find the ASL package and link against one of
the imported targets, `asl` for the dynamic version or `asls` for the static version. The static library is recommended
as you don't need to copy or distribute a DLL at runtime.

~~~
find_package( ASL REQUIRED )

target_link_libraries( my_application asls ) # for the static version (recommended), or

target_link_libraries( my_application asl ) # for the dynamic library
~~~

There is no need to provide the library directory or set *include_directories*. `find_package` will
find the library compatible with the current project among the versions compiled.

Remember that all header files have the `asl/` prefix directory and are named like the class they define
(case-sensitively), and that all symbols are in the `asl` namespace. So, for example, to use the `Directory` class:

~~~
#include <asl/Directory.h>

asl::Directory dir;
~~~

## SSL/TLS sockets and HTTPS support

This requires the *mbedTLS* library ( https://tls.mbed.org ). Download and compile the library, enable `ASL_TLS` in CMake
and provide the *mbedTLS* install directory and library location.

*/

/**
\defgroup XDL XML, XDL, and JSON

These functions allow parsing and writing data in XMl, XDL and JSON formats.

Data can be decoded and encoded from/to a text **string** with:

~~~
Var data = Json::decode(json);
String json = Json::encode(data);

Var data = Xdl::decode(xdl);
String xdl = Xdl::encode(data, true); // optional flag to indent code

Xml root = Xml::decode(xmlstring);
String xmlstring = Xml::encode(root);
~~~

The recommended way to load and parse a **file** is:

~~~
Var data = Json::read("file.json");
Var data = Xdl::read("file.xdl");
if(!data.ok()) {...} // incorrect format

Xml xml = Xml::read("file.xml");
if(!xml) {...} // incorrect format
~~~

And the recommended way to encode and write data in these formats to a file is:

~~~
Json::write("file.json", data );
Xdl::write("file.xdl", data );
Xml::write("file.xml", xml );
~~~

XDL, *eXtensible Data Language*, is a format for describing structured data 
as text. It has some similarities with the *VRML* syntax and with the *JSON* 
semantics.

The main differences with VRML syntax are that field name and value are 
separated with a '=', that boolean values are `Y` nd `N` and that array or 
property elements in the same line are separated with a ','. The main 
differences with JSON are that property names are not quoted, that objects can 
have a *class* name prepended, as in VRML, that elements can be separated with a 
*newline* instead of a comma, and that there can be C/C++ style comments. 
Whitespace and newlines can be removed to produce more compact representations.

Consider this XDL code:

~~~
Garage {
	capacity = 10
	open = Y
	dimensions = [10, 12, 2.5]
	vehicles = [
		Car {
			brand = "VW"
		}
		Truck {
			brand = "Volvo"
			length = 6.5
		}
	]
}
~~~

That code represents an object of class `Garage` with 3 properties: an integer 
`capacity`, a boolean `open` flag, a numeric array `dimensions` and the `vehicles` array. And that array 
contains two objects of different classes. If that fragment is stored in String 
`garagex` it could be read like this:


~~~
Var garage = Xdl::decode(garagex);

if(!garage.is("Garage"))
	return;
	
int capacity = garage["capacity"];
bool open = garage["open"];

float height = (garge.has("dimensions", Var::ARRAY) && garage["dimensions"].length() == 3) ?
	garage["dimensions"][2] : 0;

foreach(Var& vehicle, garage["vehicles"])
{
	double length = vehicle.has("length", Var::NUMBER) ? vehicle["length"] : 0.0;
	String brand = vehicle["brand"];
	
	my_garage << vehicle.is("Car") ? new Car(brand, length) : new Truck(brand, length);
}
~~~

We may construct a new Var or modify the one just parsed, and rewrite it as an XDL string.

~~~~
garage["vehicles"] << Var("_class", "Car")("brand", "Limo")("length", 8.34);
garage["open"] = false;

garagex = Xdl::encode(garage, true);
~~~~

The `true` parameter makes the function write the code with new lines and indentations. By default
the result is compact.

The **JSON** functions `Json::encode()`, `Json::decode()`do the same using JSON syntax. Class names are represented 
by a propery named `_` in this case (actually, the macro ASL_XDLCLASS). All of JSON syntax is supported.

*/


/**
\defgroup Factory Factory class and macros

A Factory allows creating objects given a class name as a String. They are used through pointers
to their base class and through virtual functions. A Factory is created for a given base class, and
subclasses must be registered with `ASL_FACTORY_REGISTER()` before use.

For example, assuming there is a generic possibly abstract base class `Animal`, 
we can create a subclass `Dog` and register it for instantiation by name.

~~~
class Dog : public Animal
{
	...
};

ASL_FACTORY_REGISTER( Animal, Dog );
~~~

Now an object of class `Animal` can be created anywhere only knowing the 
definition of the abstract class `Animal`.

~~~
#include <Animal.h>

Animal* animal = Factory<Animal>::create("Dog");
~~~

A Factory can be exported from a dynamic library for use with runtime dynamic loading. In this case
the library must export its catalog of subclasses with a macro:

~~~
ASL_FACTORY_EXPORT( Animal );
~~~

Now a client application can load the library at runtime with the Library class 
import its Animal factory and create objects.

~~~
Library lib("animals.dll");
ASL_FACTORY_IMPORT( Animal, lib );
auto cat = Factory<Animal>::create("Cat");
~~~

*/


/**
\defgroup Sockets Socket communication

These classes enable network communication with TCP (plain or TLS encrypted) and UDP sockets.

This example would get a web page from an HTTP server (just an example, you should use the
`Http` class for that).

~~~
String path = "/index.html", host = "somehost.com";
Socket socket;
socket.connect(host, 80);
socket << String(0, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", *path, *host);
String response;
char buffer[1001];
while(socket.waitInput())
{
	if(socket.disconnected())
		break;
	int n = socket.read(buffer, min(socket.available(), 1000));
	if(n <= 0)
		break;
	response += String(buffer, n);
}
~~~
*/

/**
\defgroup Containers Containers

There are several container classes that can hold elements of any type (`Array`, 
`Map`, `Dic`, `Stack`, `HashMap`). These classes are reference-counted, so they 
are copied by reference. If a separate copy is required, use the `clone()` method:

~~~
Array<int> a = array(1, 2, 3);
Array<int> b = a;               // b is the same as a
Array<int> c = a.clone();       // c is a separate copy of a
~~~

In order to support iterating the elements contained, they implement an 
*Enumerator*. These are similar to *iterators* but don't have to come in pairs 
(*begin* and *end*). Just one Enumerator knows how to go to the next element and 
when to stop iterating.

All containers have an `all()` enumerator that represents all its elements. All enumerators must implement
operator `*` to dereference the currently pointed element, and may implement operator `~` to get their associated
*key*, if any. For example, in an `Array`, they key is the integer index associated to each element. And in a
`Dic`, the key is the element's name, a string.

This way, enumerating is done like this: (the keyword `auto` is useful if your compiler supports it)

~~~
for(Array<T>::Enumerator e = container.all(); e; ++e)
{
	cout << ~e << ':' << *e << endl;
}
~~~

That way, if container was a `Dic`, it would be printing the names and values of all elements.

But there is a shorthand notation for iterating, inspired by other languages, `foreach` and `foreach2` loops. The
first iterates on the values of elements. And the second on both the keys and values of elements.

~~~
float sum = 0;

foreach(float x, numbers)
{
	sum += x;
}

Dic<int> ages;
ages["John"] = 23;
ages["Bob"] = 55;

foreach2(String& name, int age, ages)
{
	cout << name << " is " << age << " years old" << endl;
}
~~~

In C++11 compilers you can also use *range-based for* loops:

~~~
for(float x: numbers)
{
	sum += x;
}
~~~
*/

/**
\defgroup Shared Reference-counted objects

Several classes implement *reference counting* so that objects are copied by reference (shared). The object will be
destroyed and freed when it no longer has references. For a separate copy of the object, there is a `clone()`
method.

All containers such as `Array`, `Map`, `Dic`, `Stack`, `HashMap`, are shared this way. `String` is currently not shared but
might be in the future.

Other shared classes also support inheritance and polymorphism: `Xml`, `XmlText`, `Socket`, `TlsSocket`.

It works like this. Suppose we have a shared class `Shape` and derived classes `Circle` and `Square`.

~~~
Shape shape1 = Circle(r);      // a Circle is created in the heap and stored in shape1
a = shape1.area();             // gives PI * r^^2

Shape shape2 = shape1;         // shape2 and shape1 are the same object in memory

Shape shape3 = shape1.clone(); // shape3 is a separate copy of shape1

Square shape4(l);              // a Square is created with length l
a = shape4.area();             // gives l^^2
~~~

We can keep them in an array:

~~~
Array<Shape> shapes = {shape1, shape2, shape3, shape4};
~~~

We can check if an object is actually of a derived class, and cast it dynamically:

~~~
if(shape1.is<Circle>())    // returns true because now shape1 contains a Circle
{
	shape1.as<Circle>().setRadius(10.0);  // I can call a method that is only in Circle
}
~~~

For a non-owning pointer to the underlying object (e.g. to avoid circular references in a graph),
these classes have a `ptr()` method that returns a `C::Ptr` class object.

~~~
Shape::Ptr link = shape1.ptr();
~~~

We can check if two objects are the same (they point to the same memory):

~~~
if(shape1.is(shape2))   // true
~~~

Objects of these types are automatically constructed (initialized), as is usual in C++, and
directly usable. If for some reason you need to create a non-initialized object and defer construction
you can create the object with a null pointer argument.

~~~
Shape shape((Shape::Ptr)0);  // or Shape shape(nullptr); in C++11

shape = isCircleRequired ? Circle() : Square();
~~~
*/

/**
\defgroup Global Global functions

These utilities allow simple code like:

Measuring time and sleeping for some time:

~~~
double t1 = now();
sleep(0.5);         // sleep for 500 ms
double t2 = now();
double elapsed = t2 - t1;  // -> around 0.5
~~~

Convert radians to degrees and limit to 0-360 deg interval:

~~~
double degrees = clamp(rad2deg(angle), 0, 360);
~~~

Get random numbers in an interval:

~~~
double x = random(-1.0, 1.0);

String item = names[ random( names.length() ) ];  // pick random array item
~~~

Turn a file into a base64 representation:

~~~
TextFile("image.jpg.b64").put( encodeBase64(File("image.jpg").content()) );
~~~
*/

/**
\defgroup Math3D 3D Math

Types useful in 2D/3D geometric computations and graphics.

These classes are templates that can be used with elements of any numeric type.

~~~
Vec3_<int> ivec(1, 2, 3);
Matrix4_<float> m;
~~~

They all have two predefined specializations, for `float` and `double` elements. The
type for doubles has a `d` suffix.

~~~
Vec3 v;   // Vec3_<float>
Matrix4;  // Matrix4_<float>
Vec3d v;  // Vec3_<double>
Matrix3d; // Matrix3_<double>
~~~

*/
