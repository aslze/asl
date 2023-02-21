#include <asl/HttpServer.h>
#include <asl/TextFile.h>
#include <asl/IniFile.h>
#include <asl/Directory.h>
#include <asl/CmdArgs.h>
#include <asl/Xdl.h>
#include <asl/Thread.h>
#include <asl/WebSocket.h>
#include <asl/Process.h>
#include <ctype.h>

using namespace asl;

/*
This sample shows the creation of custom HTTP and WebSocket servers. Both servers listen
on the same port using the link() function. 

The sample serves a small web page that will display a ball moving in a trajectory
given by the server live. That is, the server will provide new (x, y) positions
continuously. And they can be provided either by HTTP (with repeated requests) or
by a WebSocket. The trajectory is circular with configurable radius and speed.

To start the servers run this program without arguments. On Linux, cd to the build directory
of this sample (cd samples/http-websocket) and run from there (../../bin/http). If TLS is
enabled, the server will also be started on port 443 using HTTPS (so you will need to run
with `sudo`).

Then open http://localhost:9000 in a browser, or https://localhost

Click [Run] to start requesting/receiving positions from the server. Initially, HTTP requests
will be used to query new positions every 50 ms. Click [WebSocket] to change to use a WebSocket
to receive positions.

Clicking on the background will set new motion radius and speed as the horizontal and vertical
click point coordinates, respectively. These are sent to the server using HTTP or WebSocket
messages (whatever mode is active).

The program can also be started as a client to test GET and POST requests. Have the server running
and then run the program with either -get or -post arguments.
*/


/**
This class is our HTTP server, implementing a simple REST-like service.

GET /pos will return a new position as a JSON object {"x": xcoord, "y": ycoord}
PUT /conf will set a new radius and speed given in the request body {"r": radius, "v": speed}
*/

class Server : public HttpServer
{
public:
	static float _r, _v; // radius and speed of trajectory
	Server();
	virtual void serve(HttpRequest& request, HttpResponse& response);
};

float Server::_r;
float Server::_v;

Server::Server()
{
	_r = 100;
	_v = 0.3f;
	setCrossDomain(true); // so it can be accessed by JS in other web pages
}

void Server::serve(HttpRequest& request, HttpResponse& response)
{
	if (request.is("GET", "/pos"))
	{
		double t = asl::now();
		response.put(Var("x", 250 + _r*cos(_v*t))
		                ("y", 250 + _r*sin(_v*t)));
		return;
	}
	else if (request.is("PUT", "/conf"))
	{
		Var data = request.json();
		_r = data["r"] | _r;
		_v = data["v"] | _v;
		response.put(Var("status", "OK"));
		return;
	}
	
	// for other paths, serve static files:

	serveFile(request, response);
}


/**
This is our WebSocket server. The broadcast function sends a JSON message to
all connected clients containing a new particle position. It is called later
every 15 ms. The server can receive messages containing a new radius and speed.
*/
class MyWebSocketServer : public WebSocketServer
{
public:
	void serve(WebSocket& ws)
	{
		printf("New client: %i\n", clients().length());
		while (!ws.closed() && !_requestStop)
		{
			if (!ws.waitData())
				continue;
			Var msg = ws.receive(); // receive message and extract radius and speed
			if (msg.ok())
			{
				Server::_r = msg["r"] | Server::_r;
				Server::_v = msg["v"] | Server::_v;
				ws.send(Var("status", "ok"));
			}
		}
		printf("Client out: %i (code %i)\n", clients().length()-1, ws.code());
	}

	/**
	Send new position to all connected clients as JSON object {"op": "pos", "x": x, "y": y}
	*/
	void broadcast()
	{
		Lock _(mutex());
		float v = Server::_v, r = Server::_r;
		double t = asl::now();
		Var msg = Var("op", "pos")("x", 250 + r * cos(v*t))("y", 250 + r * sin(v*t));
		foreach(WebSocket* ws, clients())
		{
			ws->send(msg);
		}
	}
};

int main(int argc, char* argv[])
{
	CmdArgs args(argc, argv);

	int port = 9000;
	int httpsport = 443;
	String webroot = Process::myDir();

	// read configuration file

	IniFile config("httpws.ini");
	if (config.ok())
	{
		webroot = config("webroot", webroot);
		port = config("port", port);
		httpsport = config("httpsport", httpsport);
	}

	// run as server if -server option given or run with no arguments

	bool isserver = args.has("server") || args.all().length() == 1;

	// create servers:

	Server server;          // HTTP server
	MyWebSocketServer wss;  // WebSocket server

	if (isserver)
	{
		server.setRoot(webroot);
		server.bind(port);

#ifdef ASL_TLS
		server.bindTLS(httpsport); // bind HTTPS port
		if (config.has("key"))
		{
			server.useCert(TextFile(config["cert"]).text(), TextFile(config["key"]).text());
		}
#endif

		server.start(true); // start HTTP server (true to start listening in new thread, non-blocking)

		server.link(wss); // link WebSocket server to HTTP server

		while (1)
		{
			sleep(0.015);

			wss.broadcast(); // send position to all clients
		}
	}


	// Running with arguments makes the program act as a client and benchmark:

	/*
	Run with -get to test HTTP GET. Other arguments are:
	-n <int> number of requests (default: 5000)
	-host <string> host to connect to (default: http://localhost:9000)
	-print print responses
	*/

	if (args.has("get"))
	{
		double t1 = now();
		int n = args("n", 5000), nok = 0;
		String host = args("host", "http://localhost:" + String(port));
		bool print = args.has("print");
		for (int i = 0; i < n; i++)
		{
			HttpResponse res = Http::get(host + "/pos");
			Var data = res.json();
			if (print)
				printf("[%i] %i  %s\n", i, res.code(), *Xdl::encode(data));
			if (res.code() == 200)
				nok++;
		}
		double t2 = now();
		printf("%f s (%.0f req/s) (%i ok)\n", t2 - t1, n/(t2-t1), nok);
		return 0;
	}

	/*
	Run with -post to test HTTP POST. Other arguments are:
	-n <int> number of requests (default: 5000)
	-size <int> size of request body to post (default: 20000 bytes)
	-host <string> host to connect to (default: http://localhost:9000)
	-print print responses
	*/

	else if (args.has("post"))
	{
		double t1 = now();
		int n = args("n", 5000);
		int m = args("size", 20000);
		String host = args("host", "http://localhost:" + String(port)); 
		int nok = 0;
		bool print = args.has("print");
		for (int i = 0; i < n; i++)
		{
			HttpResponse res = Http::post(host + "/blob?n=" + String(m), String::repeat('x', m));
			Var data = res.json();
			if (print)
				printf("[%i] %i  %s\n", i, res.code(), *Xdl::encode(data));
			if (res.code() == 200)
				nok++;
		}
		double t2 = now();
		printf("%f s (%.0f req/s) (%i ok)\n", t2 - t1, n / (t2 - t1), nok);
		return 0;
	}
	//else
	//	while (1) { sleep(10); }

	return 0;
}


