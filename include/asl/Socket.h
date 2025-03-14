// Copyright(c) 1999-2024 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SOCKET_H
#define ASL_SOCKET_H

#include <asl/Array.h>
#include <asl/String.h>
#include <asl/Shared.h>

namespace asl {

struct SocketException : public Exception
{
};

struct SocketOptionException : public Exception
{
	const char* option;
	SocketOptionException(const char* opt) : option(opt) {}
};

class Socket;

class ASL_API Sockets
{
	Array<Socket> set, changed;
public:
	Sockets() {}
	int length() const { return set.length(); }
	Socket& operator[](int i) { return set[i]; }
	const Socket& operator[](int i) const { return set[i]; }
	Sockets& operator<<(Socket& s);
	int waitInput(double t=60);
	bool hasInput(const Socket& s);
	Socket& activeAt(int i) {return changed[i];}
	void close();
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812)
#endif
/**
Represents an endpoint of a socket which is ususally an IP address and a port. Both IPv4 and IPv6 are supported.
\ingroup Sockets
*/
class ASL_API InetAddress
{
public:
	enum Type { ANY, IPv4, IPv6, LOCAL };

	InetAddress() : _type(IPv4) {}
	ASL_EXPLICIT InetAddress(Type t);
	/**
	Creates an address with a name (to be looked up) and a port
	*/
	ASL_EXPLICIT InetAddress(const String& host, int port) : _type(IPv4) { set(host, port); }
	ASL_EXPLICIT InetAddress(const String& host_port) : _type(IPv4) { set(host_port); }

	void resize(int n) { _data.resize(n); }

	bool set(const String& host, int port);
	bool set(const String& host);
	/**
	Returns a string representation of this address
	*/
	String toString() const;
	bool operator==(const InetAddress& a) const;
	bool operator!=(const InetAddress& a) const {return !(*this==a);}
	/**
	Returns the port
	*/
	int port() const;
	InetAddress& setPort(int port);
	/**
	Returns the host IP as a string
	*/
	String host() const;
	byte* ptr() {return _data.data();}
	const byte* ptr() const {return _data.data();}
	int length() const {return _data.length();}
	/**
	Returns the type IPv4 or IPv6 of this address
	*/
	Type type() const { return _type; }
	/**
	Resolves a name and returns a list of addresses, possibly including IPv4 and IPv6, or translates a
	literal (such as "10.0.0.1" or "::1")
	*/
	static Array<InetAddress> lookup(const String& name);
protected:
	ByteArray _data;
	Type _type;
};

typedef InetAddress NetAddress;

ASL_SMART_CLASS(Socket, SmartObject)
{
	ASL_SMART_INNER_DEF(Socket);
	friend class Sockets;
	int _handle;
	enum { TCP, PACKET, LOCAL } _type;
	Endian _endian;
	InetAddress::Type _family;
	String _hostname;
	int _error;
	bool _blocking;
	virtual bool setOption(int level, int opt, const void* p, int n);
	bool init(bool force = false);
	Socket_();
	Socket_(int fd);
	virtual ~Socket_();
	template <class T>
	bool setOption(int level, int opt, const T& val) { return setOption(level, opt, &val, sizeof(T)); }
	void setBlocking(bool b) { _blocking = b; }
	virtual int handle() const { return _handle; }
	void setEndian(Endian e) { _endian = e; }
	virtual bool disconnected();
	virtual bool bind(const String& ip, int port);
	virtual bool bind(const String& path);
	virtual void listen(int n = 5);
	virtual Socket_* accept();
	InetAddress remoteAddress() const;
	InetAddress localAddress() const;
	virtual bool connect(const InetAddress& host);
	virtual void close();
	String readLine();
	virtual int available();
	virtual int read(void* data, int size);
	virtual int write(const void* data, int n);
	ByteArray read(int n = -1);
	void skip(int n);
	virtual bool waitInput(double timeout = 60);
	int error() const { return _error; }
	virtual String errorMsg() const;
};

/**
A communication socket for the TCP/IP protocol.
\ingroup Sockets
*/
class ASL_API Socket : public SmartObject
{
public:
	enum { TCP, PACKET, LOCAL };

	ASL_SMART_DEF(Socket, SmartObject); // NOLINT
	ASL_EXPLICIT Socket(int fd) : ASL_SMART_INIT(fd) {}

	bool operator==(const Socket& s) const { return ptr() == s.ptr(); }
	
	void setBlocking(bool b)
	{
		_()->_blocking = b;
	}
	int handle() const
	{
		return _()->handle();
	}
	template <class T>
	bool setOption(int level, int opt, const T& val)
	{
		return _()->setOption(level, opt, &val, sizeof(T));
	}
	void setEndian(Endian e) { _()->_endian = e; }

	Endian endian() const { return (Endian)_()->_endian; }

	void enableBroadcast(bool on = true);
	/**
	Checks if the connection was lost.
	*/
	bool disconnected() { return _()->disconnected(); }

	/**
	Checks if the connection is open.
	*/
	bool connected() { return !disconnected(); }

	/**
	Binds this socket to the given IP address and port number.
	*/
	bool bind(const String& ip, int port) { return _()->bind(ip, port); }
	/**
	Binds this socket with the given port number to all IPv4 interfaces.
	*/
	bool bind(int port) { return bind("0.0.0.0", port); }

	bool bind(const String& name) { return _()->bind(name); }
	/**
	Makes this socket listen to incoming connections
	*/
	void listen(int n = 5) { _()->listen(n); }
	/**
	For a listening socket, waits for an incoming connection and returns a socket to communicate
	with the remote peer.
	*/
	Socket accept() { return _()->accept(); }
	/**
	Returns the adress of the remote peer for a connected socket.
	*/
	InetAddress remoteAddress() const { return _()->remoteAddress(); }
	/**
	Returns the adress of this connected socket if bound.
	*/
	InetAddress localAddress() const { return _()->localAddress(); }
	/**
	Connects this socket to the given host name and port, trying all addresses assigned to that name until one succeeds
	*/
	bool connect(const String& host, int port);
	/**
	Connects this socket to the given address given as a string "host:port" or "[ipv6]:port"
	*/
	bool connect(const String& host) { return connect(InetAddress(host)); }
	bool connect(const InetAddress& host) { return _()->connect(host); }
	bool connect(const char* host) { return connect(String(host)); }
	/**
	Closes this socket.
	*/
	void close() { _()->close(); }
	/**
	Reads a line of text from the socket (keep in mind this is not buffered, can be a bit inefficient, and there is a length limit of 8000 bytes)
	*/
	String readLine() { return _()->readLine(); }
	/**
	Returns the number of bytes available for reading without blocking
	*/
	int available() { return _()->available(); }
	/**
	Reads `size` bytes from the socket into the bufffer pointed to by `data`.
	*/
	int read(void* data, int size) { return _()->read(data, size); }
	/**
	Writes `n` bytes from the bufffer pointed to by `data` to the socket.
	*/
	int write(const void* data, int n) { return _()->write(data, n); }
	/**
	Writes the byte array to the socket and returns the number of bytes actually sent.
	*/
	int write(const ByteArray& data) { return _()->write(data.data(), data.length()); }
	/**
	Reads n bytes and returns them as an array of bytes, or reads all available bytes if no argument is given.
	*/
	ByteArray read(int n = -1) { return _()->read(n); }

	/**
	Skips (reads and discards) the next n bytes from the socket.
	*/
	void skip(int n) { _()->skip(n); }
	/**
	Waits until there is incoming data in the socket or it is disconnected for a maximum time, and
	returns true if some of that happened before timeout
	*/
	bool waitInput(double timeout = 2) { return _()->waitInput(timeout); }

	/**
	Waits until there is incoming data in the socket or it is disconnected for a maximum time, and
	returns true only if there is data to read (false may mean no data or disconnection)
	*/
	bool waitData(double timeout = 2) { return waitInput(timeout) && !disconnected(); }
	/**
	Returns true if there was some communication error in this socket
	*/
	int error() const { return _()->_error; }

	/**
	Returns a string representation of the last socket error
	*/
	String errorMsg() const { return _()->errorMsg(); }

	/**
	Writes variable x to the socket respecting endianness in binary form
	*/
	template<class T>
	Socket& operator<<(const T& x)
	{
		T y = (endian() == ASL_OTHER_ENDIAN) ? bytesSwapped(x) : x;
		write(&y, sizeof(x));
		return *this;
	}

	/**
	Reads variable x from the socket respecting endianness in binary form
	*/
	template<class T>
	Socket& operator>>(T& x)
	{
		read(&x, sizeof(x));
		if (endian() == ASL_OTHER_ENDIAN)
			swapBytes(x);
		return *this;
	}

	Socket& operator>>(char& x)
	{
		read(&x, sizeof(x));
		return *this;
	}

	Socket& operator>>(byte& x)
	{
		read(&x, sizeof(x));
		return *this;
	}

	template<class T>
	Socket& operator<<(const Array<T>& x)
	{
		if (endian() == ASL_OTHER_ENDIAN)
		{
			foreach(const T& y, x)
				*this << y;
		}
		else
			write(&x[0], x.length());
		return *this;
	}

	Socket& operator<<(const Array<byte>& x)
	{
		write(&x[0], x.length());
		return *this;
	}

	Socket& operator<<(const char* x)
	{
		write(x, (int)strlen(x));
		return *this;
	}

	Socket& operator<<(const String& x)
	{
		write(*x, x.length());
		return *this;
	}
	/**
	Reads n bytes from the socket a string
	*/
	String readString(int n)
	{
		String s(n, 0);
		n = read(&s[0], n);
		if (n >= 0)
			s[n] = '\0';
		return s.fix();
	}

	/**
	Reads a string from the socket that is preceded by its length as an int32 (the sender must send the byte length before)
	\deprecated Too specific
	*/
	ASL_DEPRECATED(Socket& operator>>(String& x), "Use your own logic to read strings")
	{
		int n = 0;
		*this >> n;
		x = readString(n);
		return *this;
	}

	template <class T>
	T read() { T x; *this >> x; return x; }
};

ASL_SMART_CLASS(PacketSocket, Socket)
{
	ASL_SMART_INNER_DEF(PacketSocket);
	PacketSocket_();
	PacketSocket_(int fd);
	String readLine();
	void sendTo(const InetAddress& addr, const void* data, int n);
	int readFrom(InetAddress& addr, void* data, int n);
};

/**
A communication socket for UDP/IP protocol.
\ingroup Sockets

A listening side binds to a port and receives packets. The readFrom() functions also
identifies the sender so you can reply.

```
PacketSocket socket;
socket.bind(port);
InetAddress sender;
auto data = socket.readFrom(sender);
socket.sendTo(sender, String("hi"));
```

The sender just sends packets to an endpoint:

```
PacketSocket socket;
socket.sendTo(InetAddress("localhost", port), data);
```
*/
class ASL_API PacketSocket : public Socket
{
public:
	ASL_SMART_DEF(PacketSocket, Socket);
	ASL_EXPLICIT PacketSocket(int fd) : ASL_SMART_INIT(fd) {}

	ASL_DEPRECATED(String readLine(), "") { return _()->readLine(); }

	/**
	Sends `n` bytes from `data` as a packet to the address `addr`.
	*/
	void sendTo(const InetAddress& addr, const void* data, int n)
	{
		_()->sendTo(addr, data, n);
	}

	/**
	Sends a data packet to the address `addr`.
	*/
	void sendTo(const InetAddress& addr, const ByteArray& data) { sendTo(addr, data.data(), data.length()); }

	void sendTo(const InetAddress& addr, const String& data) { sendTo(addr, data.data(), data.length()); }

	/**
	Reads an incoming packet into the buffer `data` of size `n`, writes the address
	of the sending peer and returns the actual packet size.
	*/
	int readFrom(InetAddress& addr, void* data, int n)
	{
		return _()->readFrom(addr, data, n);
	}

	/**
	Reads an incoming packet of at most n bytes and returns it, and gets the address
	of the sending peer.
	*/
	ByteArray readFrom(InetAddress& addr, int n = 1000)
	{
		ByteArray data(n);
		n = _()->readFrom(addr, data.data(), n);
		return n > 0? data.resize(n) : data.resize(0);
	}
};

ASL_SMART_CLASS(LocalSocket, Socket)
{
	ASL_SMART_INNER_DEF(LocalSocket);
	LocalSocket_();
	LocalSocket_(int fd);
	~LocalSocket_();
	bool bind(const String& name);
	Socket_* accept();
};

/**
A Local or Unix socket for inter-process communication within a machine.

Works similar to a TCP socket but it is bound to a local path in the file system.

Used to be Unix/Linux-only but is also supported on recent Windows 10+ versions (need to
enable CMAKE_SOCKET_LOCAL on Windows)

```
LocalSocket server;
server.bind("/dir/comm.sock");
server.listen();
LocalSocket client = server.accept();
...

LocalSocket client;
client.connect("/dir/comm.sock");
client << "Hello\n";
```
\ingroup Sockets
*/
class ASL_API LocalSocket : public Socket
{
public:
	ASL_SMART_DEF(LocalSocket, Socket);
	ASL_EXPLICIT LocalSocket(int fd) : ASL_SMART_INIT(fd) {}
};

ASL_SMART_CLASS(MulticastSocket, PacketSocket)
{
	ASL_SMART_INNER_DEF(MulticastSocket);
	bool join(const InetAddress& a, int interfac);
	bool leave(const InetAddress& a, int interfac);
	bool setLoop(bool on);
	bool setTTL(int n);
};

/**
A communication socket for multicast UDP/IP protocol.

A socket can send packets to a multicast group and port, and another socket can join the group
so it receives packets sent to it.

\ingroup Sockets

```
InetAddress group("224.0.1.1", 18000);
```

A listening side would bind to the port on a local interface, join the group, receive packets and then optionally leave (on socket destruction
the group is automatically left):

```
MulticastSocket socket;
socket.bind(group.port());
socket.join(group);
InetAddress sender;
socket.readFrom(sender, data);
socket.leave(group);
```

And a sending side would start a multicast session to the same group and send packets to it:

```
MulticastSocket socket;
socket.sendTo(group, data);
```
*/
class ASL_API MulticastSocket : public PacketSocket
{
public:
	ASL_SMART_DEF(MulticastSocket, PacketSocket);
	
	/** Joins a multicast group. Packets sent to the group's address will be received */
	bool join(const InetAddress& a, int interfac = 0)
	{
		return _()->join(a, interfac);
	}
	/** Leaves a multicast group and stops receiving packets from it. */
	bool leave(const InetAddress& a, int interfac = 0)
	{
		return _()->leave(a, interfac);
	}

	/**
	Enables receiving packets in the loopback interface
	*/
	bool setLoop(bool on) { return _()->setLoop(on); }

	/**
	Sets the multicast socket's TTL value (default 1)
	*/
	bool setTTL(int n) { return _()->setTTL(n); }

	/** Starts a multicast session for group address `a`. Packets sent will be received by
	all sockets that join the group.
	\deprecated Use .connect() or .sendTo()
	*/
	ASL_DEPRECATED(void multicast(const InetAddress& a, int ttl = 1), "Use .connect() or .sendTo()")
	{
		connect(a);
		setLoop(true);
		setTTL(ttl);
	}

	bool setOptions(bool loop, int ttl)
	{
		return setLoop(loop) && setTTL(ttl);
	}
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

#endif
