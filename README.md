# ASL - All-purpose Simple Library #

_**An old, bad and outdated C++ utility library**_

ASL is a collection of multiplatform general purpose classes and utilities focused on simplicity and ease of use. Builds in seconds and facilitates writing code that works on different operating systems and compilers.

- Crossplatform (**Windows**, **Linux**, **macOS**, **Android**), 32/64 bit
- Works on older compilers (e.g. VisualStudio 2005, gcc 3.4) but can use some C++11 features if available (e.g. lambdas, range-based for, initializer lists)
- Almost no dependencies. Optionally the **mbedTLS** library for TLS sockets (e.g. for HTTPS)
- [Online API documentation](https://aslze.github.io/asl-doc/)



## Features

__OS-related functionalities__:

- Threads, mutexes and semaphores
- Processes (run programs and read their output or write input)
- Binary and text files
- Directory enumeration and file system operations (copy, move, delete)
- Sockets TCP, UDP and Unix (where available), IPv4 and IPv6, with optional SSL/TLS
- Runtime dynamically loadable libraries (DLLs or shared libraries)
- Console: text and background color, cursor position, etc.
- Serial ports
- Shared memory

__Utilities__:

- JSON, XML and XDL parsing/encoding
- HTTP/HTTPS 1.1 server and client
- WebSocket server and client
- Configuration INI files reading/writing
- CSV file reading/writing, ARFF writing
- Log messages to console and/or files (with limited growth)
- Command line arguments and options parsing
- Singletons and Factories
- Base64 and hex encoding/decoding
- Binary buffer reading/writing (endian-aware)
- Random number generator
- Basic testing functionality
- Linear and nonlinear systems solving

__Basic data types__:

- Strings with UTF8 support
- Dynamic and static arrays, plus 2-D arrays
- Maps and hashmaps
- Date/time
- 2D, 3D, 4D vectors, 3x3, 4x4 matrices, MxN matrices and quaternions
- Variants (similar to JavaScript vars)
- Python bindings of some of these via pybind11

## Features by example

Here are some snippets that showcase some simple uses of ASL. There are also more complex ways
of using the library with added functionalities. Namespace `asl` omitted for clarity.

Get command line options (suppose we run `program.exe -iterations 10`):

```cpp
CmdArgs args(argc, argv);
int iterations = args["iterations"] | 10;  // default to 10 if no parameter given
```

Read or write a configuration INI file ("config.ini" contains this):

```
[parameters]
threshold=0.25
```

```cpp
IniFile config("config.ini");
float threshold = config["parameters/threshold"] | 0.2;
config["parameters/threshold"] = 0.5;
```

Do HTTP requests (you can post a body or send headers, too):

```cpp
HttpResponse resp = Http::get("https://www.somewhere.com/page.xhtml");
if(resp.code() != 200)
    return -1;
String type = resp.header("Content-Type");
String text = resp.text();
```

Or create HTTP services:

```cpp
struct TimeServer : public HttpServer
{
    void serve(HttpRequest& req, HttpResponse& resp)
    {
        if(req.is("GET", "/time")
        {
            resp.put(Var{{"time", Date::now().toString()}});
        }
        else
        {
            resp.put(Var{{"status", "error"}});
            resp.setCode(500);
        }
    }
};
TimeServer server;
server.bind(80);
server.start();
```

Decode XML:

```cpp
Xml html = Xml::decode( "<html><head><meta charset='utf8'/></head></html>" );
String charset = html("head")("meta")["charset"];   // -> "utf8"
String rootTag = html.tag();                        // -> "html"
```

Read or write a file in one line:

```cpp
String content = TextFile("somefile-uнicoδe.json").text();
```

The path given above can be Unicode UTF8, and the file content can be UTF8 with or wothout BOM,
UTF16-BE or UTF16-LE. And it will be converted to UTF8.

Emulate a simple `wget`:

```cpp
File("image.png").put( Http::get("http://hello.com/image.png").body() );
```

Decode JSON (but you can also directly load/save JSON from a file):

```cpp
Var data = Json::decode(content);
String name = data["name"];
int number = data["age"];
```

Write JSON:

```cpp
Var particle = Var{
    {"name", "proton"},
    {"mass", 1.67e-27},
    {"position", {x, y, z}}
};
String json = Json::encode(particle);
```

Write to the console with colors:

```cpp
Console console;
console.color(Console::BRED);    // bright red text
console.bgcolor(Console::CYAN);  // cyan background
printf("some highlighted text");
```

Create threads (but you can also use lambdas):

```cpp
class MyThread : public Thread
{
    void run() { do_stuff(); }
};

MyThread thread;
thread.start();
```

Send data through a TCP socket (that will try different hosts
if DNS "host" maps to several IPv4 or IPv6 addresses):

```cpp
Socket socket;                  // or TlsSocket for SSL/TLS
socket.connect("host", 9000);
socket << "hello\n";
```

Or a message through a WebSocket:

```cpp
WebSocket socket;
socket.connect("host", 9000); // or "ws://host:9000"
socket.send("hello");
```

Strings are 8 bit, by default assumed to be UTF8, and can be case converted even beyond ASCII:

```cpp
String country = "Ελλάδα";
String upper = country.toUpperCase(); // -> "ΕΛΛΆΔΑ"
```

Strings have some additional handy methods:

```cpp
if(filename.startsWith(prefix) || filename.contains("-"))
```

A string can be split and joined back:

```cpp
String names = "one,two,three";
Array<String> numbers = names.split(",");
String s123 = numbers.join(" "); // -> "one two three"
```

and can be automatically converted to UTF16:

```cpp
String dirname = "newdir";
CreateDirectoryW( dirname ); // autoconverted to UTF16
```

But don't use the above Windows-only function when you can (in UTF8):

```cpp
Directory::create("newdir");
```

or enumerate the contents of a directory:

```cpp
Directory dir("some/dir");
for(File& file : dir.files("*.txt"))
{
    String path = file.path();
    Long size = file.size();
    Date date = file.lastModified();
}
```

Copy, move or delete files:

```cpp
File("letter.doc").copy("/backup/");
File("file.conf").move("file.ini");
File("trash.doc").remove();
Directory::remove("/trash/"); // must be empty
```

Start a subprocess and read its output:

```cpp
Process p = Process::execute("someprogram.exe");
if(p.success())
    output = p.output();
```

Get the parent directory, file name and extension of a path:

```cpp
Path path = "/some/dir/file.txt";
path.directory() // -> "/some/dir/"
path.name()      // -> "file.txt"
path.nameNoExt() // -> "file"
path.extension() // -> "txt"
```

Time an operation with precision (around microseconds) and sleep for some time:

```cpp
double t1 = now();
sleep(0.5);               // 0.5 seconds
double t2 = now();
double elapsed = t2 - t1; // should be around 0.5
```

Log a message to the console and to a file:

```cpp
ASL_LOG_W("Only %i bytes available", bytesAvailable);
```

And much more.


## Compilation and use

ASL can be used as a static or as a dynamic/shared library. The easiest way to 
build and use it is with **CMake** (2.8.12 or later). By default both the dynamic 
and static libraries are built.

Just compile the library with whatever compilers and architecturs (32/64bit) you need. There is no need to *install*.

To use the library in another project just find the ASL package and link against one of
the imported targets, `asl` for the dynamic version or `asls` for the static version. The static library is recommended
as you don't need to copy or distribute a DLL at runtime.

```cmake
find_package(ASL REQUIRED)

target_link_libraries(my_application asls) # for the static version, or

target_link_libraries(my_application asl)  # for the dynamic library
```

There is no need to provide the library directory or set *include_directories*. `find_package` will
find the library compatible with the current project among the versions compiled.

With CMmake 3.14+, instead of using `find_package()`, you can download and build the library automatically as a subproject (and then link with it as before):

```cmake
include(FetchContent)
FetchContent_Declare(asl URL https://github.com/aslze/asl/archive/1.11.8.zip)
FetchContent_MakeAvailable(asl)
```


Remember that all header files have the `asl/` prefix directory and are named like the class they define
(case-sensitively), and that all symbols are in the `asl` namespace. So, for example, to use the `Directory` class:

```cpp
#include <asl/Directory.h>

asl::Directory dir;
```

## SSL/TLS sockets and HTTPS support

HTTPS, TLS WebSockets and TlsSocket require the [mbedTLS](https://github.com/Mbed-TLS/mbedtls) library (up to v3.2.1). Download and compile
the library, enable `ASL_TLS` in CMake and provide the *mbedTLS* install directory (and library locations, which should
normally be automatically found).

On Ubuntu Linux you can just install package **libmbedtls-dev** with:

```
sudo apt-get install libmbedtls-dev
```

On FreeBSD use:

```
pkg install mbedtls
```

With a recent CMake (3.14+) you can also build mbedTLS together with ASL as subprojects (e.g. using `FetchContent`):

```
set(ASL_TLS ON)
set(ENABLE_PROGRAMS OFF CACHE BOOL "") # skip samples
FetchContent_Declare(mbedtls URL https://github.com/Mbed-TLS/mbedtls/archive/v3.2.1.zip)
FetchContent_Declare(asl URL https://github.com/aslze/asl/archive/1.11.8.zip)
FetchContent_MakeAvailable(mbedtls asl)
```

Then just link your project to `asls` after that block.

