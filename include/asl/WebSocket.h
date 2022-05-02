// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_WEBSOCKETSERVER
#define ASL_WEBSOCKETSERVER
#include <asl/Socket.h>
#include <asl/Map.h>
#include <asl/String.h>
#include <asl/SocketServer.h>
#include <asl/Mutex.h>

namespace asl {

class Var;

struct WebSocketMsg
{
	friend class WebSocket;
	WebSocketMsg() {}
	WebSocketMsg(const Array<byte>& data) : _data(data) {}
	operator String() const;
	operator Var() const;
	operator Array<byte>() const { return _data; }
	int length() const { return _data.length(); }
	operator bool() const { return length() != 0; }
	bool operator!() const { return length() == 0; }
	const char* operator*() const;
	void append(const Array<byte>& data) { _data.append(data); }
private:
	WebSocketMsg& fix();
	Array<byte> _data;
};

/**
This class represents a WebSocket. A WebSocket can be used to connect to WebSocket server as a client
and send and receive messages (binary or text). Or it can be an incoming connection in a WebSocketServer.

~~~
WebSocket ws;
ws.connect("ws://some-websocketserver", 9000);
ws.send("Hello!");                     // send as text
ws.send(Var("type", "info")("n", 10)); // send as JSON
String msg = ws.receive();
ws.close();
~~~

To connect to a TLS secure server, use the "wss:" protocol:

~~~
ws.connect("wss://some-encrypted-websocketserver:443");
~~~
*/

class ASL_API WebSocket
{
public:
	enum FrameType { FRAME_CONT, FRAME_TEXT, FRAME_BINARY, FRAME_CLOSE=8, FRAME_PING, FRAME_PONG };
	/**
	Creates an unconnected WebSocket
	*/
	WebSocket();
	WebSocket(const Socket& s, bool isclient = true);
	/**
	Connecto to a WebSocket server at the given host and port (the port can be in the `host` string separated with ':')
	*/
	bool connect(const String& host, int port = 80);
	/**
	Closes this WebSocket
	*/
	void close();
	/**
	Receives a messege from the peer
	*/
	WebSocketMsg receive();

	void send(const byte* p, int len, FrameType = FRAME_TEXT);

	/**
	Sends a binary message to the peer
	*/
	void send(const Array<byte>& m) { send(m.ptr(), m.length(), FRAME_BINARY); }
	/**
	Sends a text message to the peer
	*/
	void send(const String& m) { send((byte*)*m, m.length()); }

	void send(const char* m) { send(String(m)); }
	/**
	Sends a Var as a text message by encoding to JSON
	*/
	void send(const Var& v);
	/**
	Waits for incoming data for a maximum time or a disconnection, returns true if something
	happened before timeout
	*/
	bool wait(double timeout = 5);

	/**
	Waits for incoming data for a maximum time or a disconnection, returns true only if there is
	incoming data
	*/
	bool waitData(double timeout = 5) { return wait() && !closed(); }

	/** Checks if there is some input available */
	bool hasInput();

	/** Returns the close status code if the socket was closed */
	int code() const { return _code; }

	/**
	Tests if this WebSocket is closed, possibly by the other end
	*/
	bool closed();
protected:
	Socket _socket;
	Array<byte> _buffer;
	bool _isClient;
	bool _closed;
	int _code;
	Random _random;
};

/**
This class can be used to create WebSocket servers.

To create a server, define a subclass and implement the `serve()` method. This function is called **in a new thread**
when an incoming WebSocket connection arrives.

~~~
class AWebSocketServer : public WebSocketServer
{
public:
	void serve(WebSocket& ws)
	{
		while (1) {
			ws.wait();
			if (ws.closed())
				break;
			String msg = ws.receive();
			ws.send("Hello " + msg);
		}
	}
};

AWebSocketServer wsserver;
wsserver.bind(9000);
wsserver.start();
~~~

Additionally, a WebSocket server can use the same port as an HttpServer. To do this, call the `link()`
function in the HTTP server and only start that one.

~~~
httpserver.link(wsserver);
httpserver.start();
~~~
*/

class ASL_API WebSocketServer: public SocketServer
{
	friend class HttpServer;
public:
	WebSocketServer();
	WebSocketServer(int port);
	/**
	Serves the incoming client websocket, implement this function in a subclass to define
	the behavior of this server.
	*/
	virtual void serve(WebSocket& s);
	/**
	Returns an array of currently connected client websockets
	*/
	const Array<WebSocket*>& clients() const { return _clients; }
	Mutex& mutex() { return _mutex; }
protected:
	Array<byte> readMessage();
private:
	void process(Socket& socket, const Dic<String>& headers);
	void serve(Socket client);
	Array<WebSocket*> _clients;
	Mutex _mutex;
};
}
#endif
