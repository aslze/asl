// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_HTTPSERVER
#define ASL_HTTPSERVER
#include <asl/Socket.h>
#include <asl/Map.h>
#include <asl/String.h>
#include <asl/SocketServer.h>
#include <asl/Http.h>

namespace asl {

class WebSocketServer;

/**
This class can be used to create application-specific HTTP servers.

An HTTP server is implemented by subclassing HttpServer and implementing its `serve()` method. That
method will be invoked whenever a new HTTP request is received. The handle method receives an HttpRequest
to read the request (its method, path, query, headers, body, etc.) and and an HttpResponse object to
write its response to (including status code, headers and body). The function can call `serveFile()` to serve
static files like a normal web server.

~~~
class RestServer : public HttpServer
{
public:
	RestServer()
	{
		setCrossDomain(true);
		setRoot("/www");
		bind(8000);
		bindTLS(443);        // this for TLS
		useCert(cert, key);  //
	}
	void serve(HttpRequest& request, HttpResponse& response)
	{
		if(request.is("GET", "/api/clients/ *"))
		{
			String clientID = request.suffix();
			response.put( Var("id", clientID)
			                 ("info", getInfo(clientID) )
			);
		}
		else if(request.is("GET", "/api/icon")
		{
			response.setHeader("Content-Type", "image/png");
			response.put( File("images/icon" + request.query("id") + ".png").content() );
		}
		else // serve static files
		{
			serveFile(request, response);
		}
	}
};
~~~

A server like that will respond to requests such as `/api/clients/132337` or `/api/icon?id=12`.

Each request is handled in a separate thread. So, you should probably use mutexes for synchronization.

*/

class ASL_API HttpServer: public SocketServer
{
public:
	HttpServer(int port = -1);

	/**
	* Sets the root directory from where files will be served by default
	*/
	void setRoot(const String& root);
	/**
	* Adds a mime type for a given extension for files served
	*/
	void addMimeType(const String& ext, const String& type);
	/**
	Implement this function in a subclass to create the server behavior.
	*/
	virtual void serve(HttpRequest& request, HttpResponse& response);

	virtual bool handleOptions(HttpRequest& request, HttpResponse& response);
	/**
	Adds a method to the list of allowed methods
	*/
	void addMethod(const String& verb);
	/**
	Enbles or disables cross-domain (CORS) support
	*/
	void setCrossDomain(bool on) { _cors = on; }

	/**
	Serves a static file from the configured root folder
	*/
	void serveFile(HttpRequest& request, HttpResponse& response);

	/**
	Links this socket with the given WebSocket server to process incoming WebSocket connections
	*/
	void link(WebSocketServer& wsserver) { _wsserver = &wsserver; }

protected:
	String _webroot;
	String _proto;
	Dic<> _mimetypes;
	String _methods;
	bool _cors;
	WebSocketServer* _wsserver;
private:
	void serve(Socket client);
};
}
#endif
