namespace asl {

/**
\mainpage ASL - All-purpose Simple %Library

ASL is a collection of general purpose classes and utilities intended to be easy to use. It is mostly compatible
with **Windows**, **Linux**, **macOS** and **Android**, 32 and 64 bits.

__Operating system functionalities__:

- Threads, mutexes and semaphores
- Processes (run programs and read their output)
- Binary and text files
- %Directory enumeration and file system operations (copy, move, delete)
- Sockets TCP, UDP, Unix and SSL/TLS encrypted sockets, IPv4/IPv6
- Runtime dynamically loadable libraries (DLLs/shared libraries)
- %Console attributes: text color and cursor position
- Serial ports
- Shared memory

__Utilities__:

- JSON, XML and XDL parsing/encoding
- HTTP/HTTPS 1.1 server and client
- %WebSocket server and client
- Configuration INI files reading/writing
- CSV file reading/writing, ARFF write
- %Log messages to console and/or files (with limited growth)
- Command line arguments and options parsing
- Singletons and Factories
- Base64, hex encoding/decoding
- Binary buffer endian-aware reading/writing
- %Random number generator
- Simple testing functionality
- Linear and non-linear systems solving (with the Matrix class)

__Basic data types__:

- Strings with UTF8 support
- Dynamic and static arrays, including 2D arrays
- Maps and hashmaps
- Date/time
- 2D, 3D, 4D vectors, 3x3, 4x4, NxM matrices and quaternions
- Variants (similar to JavaScript vars)

# Compilation and use #

ASL can be used as a static or as a dynamic/shared library. The easiest way to 
build and use it is via *CMake* (2.8.12 or later). By default both the dynamic 
and static libraries will be built. You can enable the `ASL_USE_LOCAL8BIT` 
option to disable interpretation of strings as UTF-8 and treat them as local 8-
bit strings.

Just compile the library with whatever compilers and architecturs (32/64bit) you need. There is no need to *install*.

In order to use the library in another project just find the ASL package and link against one of
the imported targets, `asl` for the dynamic version or `asls` for the static version. The static library is recommended
as you don't need to copy or distribute a DLL at runtime.

~~~
find_package(ASL REQUIRED)

target_link_libraries(my_application asls) # for the static version (recommended), or

target_link_libraries(my_application asl) # for the dynamic library
~~~

There is no need to provide the library directory or set *include_directories*. `find_package` will
find the library compatible with the current project among the versions compiled.

With CMake 3.14+, instead of using `find_package()`, you can download and build the library automatically as a subproject
(and then link with it as before):

~~~
include(FetchContent)
FetchContent_Declare(asl URL https://github.com/aslze/asl/archive/1.11.13.zip)
FetchContent_MakeAvailable(asl)
~~~

Remember that all header files have the `asl/` prefix directory and are named like the class they define
(case-sensitively), and that all symbols are in the `asl` namespace. So, for example, to use the Directory class:

~~~
#include <asl/Directory.h>

asl::Directory dir;
~~~

## SSL/TLS sockets and HTTPS support

This requires the [mbedTLS](https://github.com/Mbed-TLS/mbedtls) library (up to v3.2.1). Download and compile the library, enable `ASL_TLS` in CMake
and provide the *mbedTLS* install directory (and library locations, which should normally be automatically found).

In Ubuntu Linux you can just install package **libmbedtls-dev** with:

```
sudo apt-get install libmbedtls-dev
```

With a recent CMake you can also build mbedTLS together with ASL as subprojects (e.g. using `FetchContent`):

```
set(ASL_TLS ON)
set(ENABLE_PROGRAMS OFF CACHE BOOL "") # skip samples
FetchContent_Declare(mbedtls URL https://github.com/Mbed-TLS/mbedtls/archive/v3.2.1.zip)
FetchContent_Declare(asl URL https://github.com/aslze/asl/archive/1.11.13.zip)
FetchContent_MakeAvailable(mbedtls asl)
```

Then just link your project to `asls` after that block.
*/

/**
\defgroup XDL XML, XDL, and JSON

These functions allow parsing and writing data in XML, XDL and JSON formats.

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
Json::write(data, "file.json");
Xdl::write(data, "file.xdl");
Xml::write(xml, "file.xml");
~~~

__XDL__, *eXtensible Data Language*, is a format for describing structured data 
as text. It has some similarities with the *VRML* syntax and with the *JSON* 
semantics.

The main differences with VRML syntax are that field name and value are 
separated with a '=', that boolean values are `Y` nd `N` and that array or 
property elements in the same line are separated with a ','. The main 
differences with JSON are that property names are not quoted, that objects can 
have a _class_ name prepended, as in VRML, that elements can be separated with a 
_newline_ instead of a comma, and that there can be C/C++ style comments. 
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

float height = (garage.has("dimensions", Var::ARRAY) && garage["dimensions"].length() == 3) ?
	garage["dimensions"][2] : 0;

for(auto& vehicle : garage["vehicles"])
{
	double length = vehicle.has("length", Var::NUMBER) ? vehicle["length"] : 0.0;
	String brand = vehicle["brand"];
	
	my_garage << vehicle.is("Car") ? new Car(brand, length) : new Truck(brand, length);
}
~~~

We may construct a new Var or modify the one just parsed, and rewrite it as an XDL string.

~~~~
garage["vehicles"] << Var{{ASL_XDLCLASS, "Car"}, {"brand", "Limo"}, {"length", 8.34}};
garage["open"] = false;

garagex = Xdl::encode(garage, true);
~~~~

The `true` parameter makes the function write the code with new lines and indentations. By default
the result is compact.

The **JSON** functions `Json::encode()`, `Json::decode()`do the same using JSON syntax. Class names are represented 
by a propery named `$type` in this case (actually, the macro ASL_XDLCLASS). All of JSON syntax is supported.

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
\defgroup HTTP HTTP clients and servers

These classes enable creating servers and clients for the HTTP 1.1 protocol. Additionally, there
is functionality to create WebSocket clients and servers. Encrypted communication (HTTPS, TLS WebSocket) require the *mbedTLS* library
and to enable the `ASL_TLS` Cmake option.

Use Http to send HTTP requests and receive the response.

~~~
HttpResponse resp = Http::get("http://someserver/something");
if (resp.ok())
{
	String text = resp.text();
	String type = resp.header("Content-Type");
}
~~~

Subclass HttpServer or WebSocketServer and implement the `serve()` function to create application-specific servers.
*/

/**
\defgroup Sockets Socket communication

These classes enable network communication with TCP (plain or TLS encrypted), UDP and Unix/Local sockets.

The `SocketServer` class implements a multithreaded socket server and is easier to use than using
`Socket` objects directly for serving multiple clients in parallel.

This simplified example would connect to a server (at some host and TCP port), send a question and receive an answer.

~~~
Socket socket;
socket.connect("thehost", 8000);
socket << "getname\n";
String answer = socket.readLine();
~~~
*/

This example reads messages as text lines, not very efficient. Sockets can read and write binary data.

/**
\defgroup Containers Containers

There are several container classes that can hold elements of different types (Array, Array_, Array2, 
Map, Dic, Stack, HashMap). Currently these containers can only contain elements of POD types or classes that do not hold pointers into
themselves (all ASL classes are fine). These classes are reference-counted, so they are copied by reference. If a separate copy
is required, use the `clone()` method:

~~~
Array<int> a = { 1, 2, 3 };
Array<int> b = a;               // b is the same as a
Array<int> c = a.clone();       // c is a separate copy of a
~~~

In C++11 compilers you can use *range-based for* loops to iterate over their items:

~~~
for(float x: numbers)
{
    sum += x;
}
~~~

Maps (Map, Dic, HashMap), can be iterated like this in C++11:

~~~
for(auto& e : constants)
{
    file << e.key << " = " e.value << '\n';
}
~~~

Or this way in C++17:

~~~
for(auto& [name, value] : constants)
{
    file << name << " = " value << '\n';
}
~~~

For older compilers there are special macros for iterating, `foreach` and `foreach2` loops. The
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

In order to support iterating the elements contained, these classes implement an _Enumerator_. These are similar to
_iterators_ but don't have to come in pairs (*begin* and *end*). Just one Enumerator knows how to go to the
next element and when to stop iterating.

All containers have an `all()` enumerator that represents all its elements. All enumerators must implement
operator `*` to dereference the currently pointed element, and may implement operator `~` to get their associated
_key_, if any. For example, in an Array, they key is the integer index associated to each element. And in a
Dic, the key is the element's name, a string.

This way, enumerating is done like this: (the keyword `auto` is useful if your compiler supports it)

~~~
for(Array<T>::Enumerator e = container.all(); e; ++e)
{
    cout << ~e << ':' << *e << endl;
}
~~~

That way, if container was a Dic, it would be printing the names and values of all elements.
*/

/**
\defgroup Shared Reference-counted objects

Several classes implement *reference counting* so that objects are copied by reference (shared). The object will be
destroyed and freed when it no longer has references. For a separate copy of the object, there is a `clone()`
method.

All containers such as Array, Map, Dic, Stack, HashMap, are shared this way. String is currently not shared but
might be in the future.

Other shared classes also support inheritance and polymorphism: Xml, XmlText, Socket, TlsSocket.

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
\defgroup Math3D Math

## Vectors and matrices for 2D/3D geometry

Classes and functions for math, including vector and matrix types useful in 2D/3D geometric computations and graphics.

These classes are templates that can be used with elements of any numeric type, but some functions oly make sense with floating point types.

~~~
Vec3_<int> ivec(1, 2, 3);
Matrix4_<float> m;
~~~

They all have two predefined specializations, for `float` and `double` elements. The
type for double has a `d` suffix, and the type for float has no suffix:

~~~
Vec3 v;   // = Vec3_<float>
Vec3d w;  // = Vec3_<double>
~~~

Classes Vec2_, Vec3_ and Vec4_ represent 2D, 3D and 4D vectors. Classes Matrix3_ and Matrix4_ represent 3x3 and 4x4 matrices, useful to work
with spatial transformations. A Matrix4_ can multiply a 3D vector to apply an affine transform (the vector will be assumed to have a 4th
component with value 1). Additionally, class Quaternion_ represent a quaternion and is useful to work with rotations in 3D.

~~~
Matrix4 transform = Matrix4::translate(position) * Matrix4::rotateX(angleX);
Vec3 worldPos = transform * position;
~~~

## Representation of 3D rotations

Rotations in 3D space can be represented in different ways and they can be converted to one another (via a Matrix4).

A rotation matrix can be constructed these ways:

~~~
m = Matrix4::rotateX(alpha); // a rotation angle around the X axis (or Y or Z)

m = Matrix4::rotate(axis, angle); // rotation angle around an arbitrary axis

m = Matrix4::rotate(axisAngle); // axis-angle: the axis vector with magnitude equal to the rotation angle

m = Matrix4::rotateE({ alpha, beta, gamma }, "XYZ*"); // Euler angles with an arbitrary axis sequence

m = quaternion.matrix(); // a quaternion
~~~

The matrix can then be converted to other rotation representations:

~~~
auto angles = m.eulerAngles("XYZ*"); // to Euler angles

auto aa = m.axisAngle(); // to axis-angle

Quaternion q = m.rotation(); // to a quaternion
~~~

## Vectors and matrices of any size

Class Matrix_ represents a matrix of any size (predefined `Matrix` and `Matrixd` for `float`/`double`). It uses dynamic
memory and is less efficient than the fixed size types above. Matrix_ can be used to represent systems of linear equations
or arbitrary vectors.

Functions solve() and solveZero() can be used to solve linear or non-linear systems with equal number of equations and
unknowns, or more equations (least-squares).

*/

/**
\defgroup Binary Binary data

Reading and writing binary data as a stream (in big or little endian order).

Files can be read or written as binary streams with the File class in specific endianness (byte order).

~~~
File file("data.bin", File::WRITE);
file.setEndian(ENDIAN_LITTLE);

file << 3 << 0.5; // writes a 32-bit int and an 64-bit double (12 bytes total) in little endian byte order.
~~~

Using class TextFile instead woud write those variables as text (as "30.5" because we did not add whitespace separation).

TCP sockets (class Socket) can also be read or written as binary streams:

~~~
Socket socket;
socket.connect("someserver", 9000);
socket.setEndian(ENDIAN_BIG);
socket << 3 << 0.5;
int32_t x, y;
socket >> x >> y;
~~~

That will call an actual socket read or write operation for each variable. Alternatively we can use binary buffers and
read or write to them as streams with classes StreamBuffer and StreamBufferReader.

~~~
StreamBuffer buffer(ENDIAN_BIG);
buffer << 3 << 0.5;                                // fill binary buffer
socket << *buffer;                                 // send it all at once
~~~

~~~
ByteArray data(12);
socket.read(data.data(), 12);                       // read 12 bytes into a buffer
StreamBufferReader reader(ENDIAN_BIG);
int32_t x, y;
reader >> x >> y;                                  // read 2 int32 variables from the buffer
~~~
*/


/**
\defgroup Testing Testing

Tiny testing functionality. The `ASL_TEST_ENABLE_MAIN()` macro adds testing functionality and creates a main function
that runs all defined tests. Use the `ASL_TEST_ENABLE()` macro if you will provide your own main function.

~~~
#include <asl/testing.h>

ASL_TEST_ENABLE_MAIN()

ASL_TEST(Car)
{
	Car car;
	ASL_ASSERT(car.isStopped());
	ASL_EXPECT(car.numWheels(), >, 2);
}

ASL_TEST(Volume)
{
    Sphere sphere(1.0);
    ASL_EXPECT_NEAR(sphere.volume(), 4.1888, 1e-3);
}
~~~
*/

/**
\defgroup Library Dynamically loadable libraries

Class Library allows loading a dynamic/shared library at runtime and import symbols from it (usually functions).

It also allows importing classes exported from a library and instantiating objects with them.

A dynamic library can contain this:

~~~
class Cat : public Animal {...}; // implements virtual functions from class Animal

ASL_EXPORT_CLASS_AS(Cat, Animal)
~~~

And a program can load it at runtime and instantiate the exported class `Cat`, with the name `"Animal"`.

~~~
#include "Animal.h"

Library lib("plugins/cat"); // Loads "cat.dll" on Widows or "libcat.so" on Linux
Animal* cat = (Animal*) lib.create("Animal");
cat->speak();
~~~

The program does not need the declaration of class `Cat`, only that of the base class `Animal`.

\sa Factory
*/

/**
\defgroup Logging Message logger

Class Log and related macros allow logging messages to the console and/or to a file.

~~~
ASL_LOG_E("Cannot load file %s", *filename);
~~~
*/

}
