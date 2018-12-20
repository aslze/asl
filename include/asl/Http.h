// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_HTTP
#define ASL_HTTP
#include <asl/Socket.h>
#include <asl/Map.h>
#include <asl/String.h>
#include <asl/Var.h>

namespace asl {

class HttpResponse;
class HttpRequest;
class Http;
class File;

struct HttpProgress
{
	virtual void operator()(int percent) {}
};

struct Url
{
	String protocol, host, path;
	int port;
	Url() : port(0) {}
};

Url ASL_API parseUrl(const String& url);

/**
\defgroup Global Global functions
@{
*/

/**
Encodes a string with percent encoding for use in a URL
*/
String ASL_API encodeUrl(const String& params);

/**
Decodes a string with percent encoding
*/
String ASL_API decodeUrl(const String& params);

/**@}*/


/**
Base class of HttpRequest and HttpResponse with common functionality.
*/

class ASL_API HttpMessage
{
	friend class Http;
public:
	HttpMessage();
	HttpMessage(const Dic<>& headers);
	HttpMessage(Socket& s);
	/**
	Returns the value of the specified header
	*/
	String operator[] (const String& header) const
	{
		return _headers.get(header, "");
	}
	/**
	Adds a message header with name `header` and value `value`
	*/
	void setHeader(const String& header, const String& value);
	/**
	Returns the value of the header named
	*/
	String header(const String& name) const;
	/**
	Returns true if the message includes the given header name
	*/
	bool hasHeader(const String& name) const;

	bool containsFile() const { return _fileBody; }

	const Dic<>& headers() const { return _headers; }

	Socket& socket() const
	{
		return *_socket;
	}

	void use(Socket& s)
	{
		_socket = &s;
	}
	/**
	Sets the body of the message as a binary blob.
	*/
	void put(const Array<byte>& data);
	/**
	Sets the body of the message as a text string.
	*/
	void put(const String& body);
	void put(const char* body) { put(Array<byte>((const byte*)body, (int)strlen(body))); }
	/**
	Sets the body of the message as a JSON document.
	*/
	void put(const Var& data);
	/**
	Sets the body of the message as a file.
	*/
	void put(const File& file);
	/**
	Returns the binary body of the message.
	*/
	const Array<byte>& body() const { return _body; }
	/**
	Returns the message body as text
	*/
	String text() const;
	/**
	Returns the message body interpreted as JSON
	*/
	Var json() const;

	/**
	\deprecated Use json()
	*/
	Var data() const { return json(); }

	void write();
	/**
	Sends the currently set headers and starts the message body.
	*/
	void sendHeaders();
	/**
	Writes the given text string to the message body.
	*/
	void write(const String& text);
	/**
	Writes the given buffer to the message body.
	*/
	int write(const char* buffer, int n);
	/**
	Sends the content of the given file in the message body
	*/
	void writeFile(const String& path);
	/**
	Sends the content of the given file as the message body and sets the content-length header
	*/
	void putFile(const String& path);

	operator String() const { return text(); }
	operator Var() const { return data(); }

protected:
	void readHeaders();
	void readBody();
	String _command;
	Dic<> _headers;
	Array<byte> _body;
	mutable Socket* _socket;
	HttpProgress* _progress;
	bool _fileBody;
	bool _chunked;
	bool _headersSent;
};


enum HttpMethod { HTTP_UNKNOWN, HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE, HTTP_OPTIONS };


/**
An HTTP request that servers can read from.

An HttpRequest has a method (such as GET or POST), optional headers, and optional body. The body can be a
String, an Array<byte> or a Var. In the case of a Var it will be encoded as JSON and the request content type
header automatically set.

\sa HttpServer
*/
class ASL_API HttpRequest : public HttpMessage
{
public:
	HttpRequest() { init(); }
	/**
	Constructs an HttpRequest with the given method
	*/
	HttpRequest(const String& method, const String& url) : _method(method), _url(url) { init(); }
	/**
	Constructs an HttpRequest with the given method and headers
	*/
	HttpRequest(const String& method, const String& url, const Dic<>& headers) : HttpMessage(headers), _method(method), _url(url) { init(); }
	/**
	Constructs an HttpRequest with the given method and body
	*/
	template<class T>
	HttpRequest(const String& method, const String& url, const T& data) : _method(method), _url(url) { put(data); init(); }
	/**
	Constructs an HttpRequest with the given method, headers and body
	*/
	template<class T>
	HttpRequest(const String& method, const String& url, const T& data, const Dic<>& headers) : HttpMessage(headers), _method(method), _url(url) { put(data); init(); }
	HttpRequest(Socket& s) : HttpMessage(s)
	{
		init();
		read();
	}
	~HttpRequest();
	void init()
	{
		_recursion = 0;
		_followRedirects = true;
	}
	void read();
	const String& resource() const
	{
		return _res;
	}
	const String& path() const
	{
		return _path;
	}
	/**
	Returns the HTTP method (GET, POST, etc)
	*/
	const String& method() const
	{
		return _method;
	}

	HttpMethod methodId() const;
	
	void setMethod(const String& m)
	{
		_method = m;
	}
	/**
	Returns the destination URL
	*/
	const String& url() const
	{
		return _url;
	}
	void setUrl(const String& url)
	{
		_url = url;
	}
	/**
	Returns true if the method is meth and the path matches pat.
	*/
	bool is(const char* meth, const String& pat)
	{
		return _method == meth && is(pat);
	}

	/**
	Returns true if the path matches pat. pat can be a fixed path name or a pattern
	including a `*` wildcard such as `"/api/clients/ *"`.
	*/
	bool is(const String& pat);
	/**
	After a call to is() returns the part of the path that substitutes the `*` in
	the pattern.
	*/
	const String& suffix()
	{
		return _argument;
	}
	/**
	Returns the address of the remote host (the client)
	*/
	const InetAddress& sender()
	{
		return _addr;
	}
	/**
	Returns the complete query string that follows the `?` character in the URL path
	*/
	const String& querystring() const
	{
		return _querystring;
	}
	/**
	Returns the query converted to a Dic, assuming that it consists of keys and values like
	`key1=value1&key2=value2`.
	*/
	const Dic<>& query();
	/**
	Returns the unescaped value associated with the named key in the query; For example if the query was
	`key1=value1&key2=value2%26`, then `request.query("key2")` would return "value1&".
	*/
	const String& query(const String& key);
	friend class HttpResponse;
	const Array<String>& parts() const;

	void setRecursion(int n) { _recursion = n; }

	int recursion() const { return _recursion; }

	void setFollowRedirects(bool enable) { _followRedirects = enable; }

	bool followRedirects() const { return _followRedirects; }

protected:
	String _method;
	String _url;
	String _res;
	InetAddress _addr;
	Array<String> _parts;
	String _path;
	String _querystring;
	String _fragment;
	Dic<> _query;
	String _argument;
	int _recursion;
	bool _followRedirects;
};

/**
An HTTP response that clients can read and servers write.
\sa HttpServer
\sa Http
*/
class ASL_API HttpResponse : public HttpMessage
{
public:
	/** Types of status codes */
	enum StatusType {
		OK = 2,           //!< Status 2xx OK
		REDIRECT = 3,     //!< Status 3xx Redirection
		CLIENT_ERROR = 4, //!< Status 4xx Client Error
		SERVER_ERROR = 5  //!< Status 5xx Server Error
	};

	HttpResponse();
	HttpResponse(const HttpRequest& r, const String& proto="HTTP/1.0", int code=200);
	/**
	Sets the response status code (such as 200 [default] for OK)
	*/
	void setProto(const String& p) { _proto = p; }
	/**
	Returns the protocol of the response (such as HTTP/1.1)
	*/
	String proto() const { return _proto; }
	void setCode(int code);
	/**
	Returns true if the response status code belongs to the type given (2xx, 3xx, 4xx or 5xx).
	*/
	bool is(StatusType code) const;
	/**
	Returns true if the response is OK (code 2xx)
	*/
	bool ok() const { return is(OK); }
	/**
	Returns the status code
	*/
	int code() const { return _code; }

protected:
	String _proto;
	int _code;
};


/**
This class contains the basic HTTP/HTTPS client functionality.

Requests are made with the static functions `get()`, `put()`, `post()`, etc. which will return an `HttpResponse` with
the response headers, body and status.

This code would get local wheather information in JSON format from a web service after first querying
our geographical location from another server. You need a registered user *key* to access this service.

~~~
auto loc = Http::get("http://ipinfo.io/").data()["loc"].toString().split(",");

auto response = Http::get("http://api.openweathermap.org/data/2.5/weather?lat=" + loc[0] + "&lon=" + loc[1] + "&APPID=" + key);

if (!response.ok())  // check if the request succeeded
	return;

Var data = response.data();
String city = data["name"];
float temperature = data["main"]["temp"];
float pressure = data["main"]["pressure"];
~~~

HTTPS client functionality currently does not check the server's certificate but the communication
is encrypted anyway. This would upload a file to a Dropbox app (provided we have an authorization key that goes in the
Authorization request header):

~~~
String content = "My New File!\n";

auto res = Http::post("https://content.dropboxapi.com/1/files_put/auto/myfile.txt", content, Dic<>("Authorization", "Bearer ..."));
~~~

Using IPv6 addresses is supported with square brackets in the host part `[ipv6]:port`:

~~~
auto res = Http::get("http://[::1]:80/path");
~~~
*/

class ASL_API Http
{
public:
	/** Sends an HTTP request and returns the response.
	*/
	static HttpResponse request(HttpRequest& req);

	/** Sends an HTTP GET request for the given url and returns the response. */
	static HttpResponse get(const String& url, const Dic<>& headers = Dic<>())
	{
		HttpRequest req("GET", url, headers);
		return request(req);
	}

	/** Sends an HTTP PUT request for the given url with the given data and returns the response. */
	template<class T>
	static HttpResponse put(const String& url, const T& body, const Dic<>& headers = Dic<>())
	{
		HttpRequest req("PUT", url, body, headers);
		return request(req);
	}

	/** Sends an HTTP POST request for the given url with the given data and returns the response. */
	template<class T>
	static HttpResponse post(const String& url, const T& body, const Dic<>& headers = Dic<>())
	{
		HttpRequest req("POST", url, body, headers);
		return request(req);
	}

	/** Sends an HTTP DELETE request for the given url and returns the response. */
	static HttpResponse delet(const String& url, const Dic<>& headers = Dic<>())
	{
		HttpRequest req("DELETE", url, headers);
		return request(req);
	}

	/** Sends an HTTP PATCH request for the given url with the given data and returns the response. */
	template<class T>
	static HttpResponse patch(const String& url, const T& body, const Dic<>& headers = Dic<>())
	{
		HttpRequest req("PATCH", url, body, headers);
		return request(req);
	}
};



}
#endif
