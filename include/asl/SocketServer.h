// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SOCKETSERVER_H
#define ASL_SOCKETSERVER_H

#include <asl/Socket.h>

namespace asl {
	
struct SockServerThread;
struct SockClientThread;

/**
This is a reusable TCP socket server that listens to incoming connections and answers them concurrently (default) or sequentially.
To make an actual server, derive a class from it and implement the virtual function `serve()`. That function will
whenever a new connection arrives.

~~~
class HelloServer : public SocketServer
{
public:
	void serve(Socket client)
	{
		String name = client.readLine();
		client << "Hello " + name + "\n";
	}
};

HelloServer server;
server.bind(9000);
server.start();
~~~

The server can be bound to more than one local address, incuding a TCP port and a Local Unix socket path.

To add a TLS serving end point, use the `bindTLS` function, and set the certificate and key.

~~~
server.bindTLS(443);
server.useCert(cert, key);
~~~

For long-running connections, the recommended way of dealing with possible disconnections is this:

~~~
void serve(Socket client)
{
	while (!client.disconnected())
	{
		if (!client.waitData())
			continue;

		String line = client.readLine(); // or other read operations
		...
	}
}
~~~

Add `&& !_requestStop` to the while condition to let the service be stoppable by SocketServer::stop().

\ingroup Sockets
*/

class ASL_API SocketServer
{
	friend struct SockClientThread;;
	SockServerThread* _thread;
protected:
	Sockets _sockets;
	bool _requestStop;
	bool _sequential;
	bool _running;
	AtomicCount _numClients;
	String _socketError;
public:
	SocketServer();
	~SocketServer();
	/** Makes the server start listening. This function blocks by default. If the `nonblocking` argument is `true`
	the server will listen in its own thread and the call will not block. */
	void start(bool nonblocking=false);
	/** Assigns a Unix local socket to listen to (Only on Linux). */
	bool bindPath(const String& sname);
	/** Assigns a TCP port to listen to. */
	bool bind(int port) { return bind("0.0.0.0", port); }
	/** Assigns an interface IP address and TCP port to listen to. */
	bool bind(const String& ip, int port);
#ifdef ASL_TLS
	/**
	Assigns an interface IP address and TCP port to listen for TLS connections
	*/
	bool bindTLS(const String& ip, int port);
	/**
	Assigns a TCP port for TLS connections
	*/
	bool bindTLS(int port) { return bindTLS("0.0.0.0", port); }
	/**
	Sets the certificate and private key for the TLS server in PEM format.
	*/
	bool useCert(const String& cert, const String& key);
#endif
	/**
	This function is called in a new thread with each new incoming client connection. Implement it
	in a subclass to make a specific server.
	*/
	virtual void serve(Socket client) {}

	void startLoop();
	/**
	Requests the server to stop receiving connections, and waits for all clients to exit if sync is true.
	*/
	void stop(bool sync = false);
	/**
	Sets the mode of operation: sequential (connections will be handled in sequence) or concurrent (
	connections will be handled in parallel by starting a new thread each time); this must be called before calling
	`start()` to start the server.
	*/
	void setSequential(bool on) { _sequential = on; }
	/**
	Returns true if this server started and has not yet stopped or still has clients running
	*/
	bool running() const { return _running || _numClients != 0; }

	/**
	Returns a string representation of the last socket error (after a failed bind or use cert...)
	*/
	String socketError() const;
};
}

#endif
