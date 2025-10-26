#define SOCKET_CPP

#ifdef _WIN32
#define _WIN32_WINNT 0x0501
struct IUnknown; // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here" with /permissive-
#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef __has_include
#if __has_include(<afunix.h>) && !defined(ASL_SOCKET_LOCAL)
#define ASL_SOCKET_LOCAL
#endif
#endif
#ifdef ASL_SOCKET_LOCAL
#include <afunix.h>
#endif
#include <windows.h>
#else
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#ifndef ASL_SOCKET_LOCAL
#define ASL_SOCKET_LOCAL
#endif
#endif

#include <string.h>
#include <asl/Socket.h>

#ifndef ASL_NOEXCEPT
#define NET_ERROR(o) throw SocketException()
#else
#define NET_ERROR(o) asl::asl_die("Network error: " o, __LINE__)
#endif

#ifdef ASL_IPV6
#define AF_TYPE AF_UNSPEC
#else
#define AF_TYPE AF_INET
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

static void verbose_print(...) {}
//#define verbose_print printf

namespace asl {

enum SocketError
{
	SOCKET_OK,
	SOCKET_BAD_INIT,
	SOCKET_BAD_DNS,
	SOCKET_BAD_CONNECT,
	SOCKET_BAD_LINE,
	SOCKET_BAD_RECV,
	SOCKET_BAD_DATA,
	SOCKET_BAD_WAIT,
	SOCKET_BAD_BIND
};

static const char* messages[] = {
	"OK",
	"SOCKET_BAD_INIT",
	"SOCKET_BAD_DNS",
	"SOCKET_BAD_CONNECT",
	"SOCKET_BAD_LINE",
	"SOCKET_BAD_RECV",
	"SOCKET_BAD_DATA",
	"SOCKET_BAD_WAIT",
	"SOCKET_BAD_BIND"
};

Sockets& Sockets::operator<<(Socket& s)
{
	set << s;
	return *this;
}

int Sockets::waitInput(double t)
{
	fd_set rset;
	timeval to;
	to.tv_sec = (int)floor(t);
	to.tv_usec = (int)((t-floor(t))*1e6);
	FD_ZERO(&rset);
	int m=0;
	for(int i=0; i<set.length(); i++)
	{
		int sockh = set[i].handle();
		if (sockh < 0)
			return -1;
		if (sockh > m)
			m = sockh;
		FD_SET(sockh, &rset);
	}
	int r=select(m+1, &rset, 0, 0, &to);
	if(r<0)
		return -1;
	changed.clear();
	for (int j = 0; j < set.length(); j++)
	{
		int sockh = set[j].handle();
		if (sockh < 0)
			return -1;
		if (FD_ISSET(sockh, &rset))
			changed << set[j];
	}
	return changed.length();
}

bool Sockets::hasInput(const Socket& s)
{
	return set.contains(s);
}

void Sockets::close()
{
	foreach(Socket& s, set)
	{
		s.close();
	}
}

#ifdef _WIN32
static bool g_wsaStarted = false;

static void startWSA()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
		ASL_BAD_ALLOC();
	g_wsaStarted = true;
}
#endif

// InetAddress

inline int sockfamily(InetAddress::Type t)
{
	switch (t)
	{
	case InetAddress::IPv4: return AF_INET;
	case InetAddress::IPv6: return AF_INET6;
#ifdef ASL_SOCKET_LOCAL
	case InetAddress::LOCAL: return AF_UNIX;
#endif
	default:;
	}
	return AF_INET;
}

InetAddress::InetAddress(InetAddress::Type t)
{
	_type = t;
	int n = 0;
	switch (t)
	{
	case IPv4: n = sizeof(sockaddr_in); break;
	case IPv6: n = sizeof(sockaddr_in6); break;
#ifdef ASL_SOCKET_LOCAL
	case LOCAL: n = sizeof(sockaddr_un); break;
#endif
	default: ;
	}
	resize(n);
	memset(ptr(), 0, n);
}

int InetAddress::port() const
{
	if (_type == IPv6)
	{
		sockaddr_in6* addr = (sockaddr_in6*)ptr();
		return ntohs(addr->sin6_port);
	}
	else if (_type == IPv4)
	{
		sockaddr_in* addr = (sockaddr_in*)ptr();
		return ntohs(addr->sin_port);
	}
	return 0;
}

InetAddress& InetAddress::setPort(int port)
{
	if (_type == IPv6)
	{
		sockaddr_in6* addr = (sockaddr_in6*)ptr();
		addr->sin6_port = htons((unsigned short)port);
	}
	else if (_type == IPv4)
	{
		sockaddr_in* addr = (sockaddr_in*)ptr();
		addr->sin_port = htons((unsigned short)port);
	}
	return *this;
}

String InetAddress::host() const
{
	if (_type == IPv6)
	{
		sockaddr_in6* addr = (sockaddr_in6*)ptr();
		unsigned short* ip = (unsigned short*)&addr->sin6_addr;
		return String::f("%x:%x:%x:%x:%x:%x:%x:%x",
			ntohs(ip[0]), ntohs(ip[1]), ntohs(ip[2]), ntohs(ip[3]),
			ntohs(ip[4]), ntohs(ip[5]), ntohs(ip[6]), ntohs(ip[7]));
	}
	else if (_type == IPv4)
	{
		sockaddr_in* addr = (sockaddr_in*)ptr();
		byte* ip = (byte*)&addr->sin_addr;
		return String::f("%i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
	}
#ifdef ASL_SOCKET_LOCAL
	else if (_type == LOCAL)
	{
		sockaddr_un* addr = (sockaddr_un*)ptr();
	return addr->sun_path;
	}
#endif
	return String();
}

String InetAddress::toString() const
{
	int p = port();
	if (p <= 0)
		return host();
	else if (_type == IPv4)
		return host() << ':' << port();
	else
		return (String) '[' << host() << "]:" << p;
}

bool InetAddress::operator==(const InetAddress& a) const
{
	return _type == a._type && _data == a._data;
}

Array<InetAddress> InetAddress::lookup(const String& host)
{
#ifdef _WIN32
	if (!g_wsaStarted) startWSA();
#endif
	Array<InetAddress> addresses;
	addrinfo hints;
	addrinfo* info;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = host.contains(':') ? AF_INET6 : AF_TYPE;

	int s = getaddrinfo(host, NULL, &hints, &info);
	if (s != 0)
	{
		verbose_print("Cannot resolve %s\n", *host);
		return addresses;
	}
	for (const addrinfo* ai = info; ai; ai = ai->ai_next)
	{
		InetAddress a;
		if (ai->ai_family == AF_INET)
			a._type = IPv4;
		else
			a._type = IPv6;
		a.resize((int)ai->ai_addrlen);
		memcpy(a.ptr(), ai->ai_addr, ai->ai_addrlen);
		if (!addresses.contains(a))
		{
			if (a._type == IPv4)
				addresses.insert(0, a);
			else
				addresses << a;
		}
	}
	freeaddrinfo(info);
	return addresses;
}

bool InetAddress::set(const String& host, int port)
{
#ifdef _WIN32
	if (!g_wsaStarted) startWSA();
#endif
	if (host.ok())
	{
		addrinfo hints, *info;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = host.contains(':') ? AF_INET6 : AF_TYPE;
		hints.ai_socktype = SOCK_STREAM;

		int s = getaddrinfo(*host, NULL, &hints, &info);
		if (s != 0 || !info)
		{
			_type = ANY;
			resize(0);
			return false;
		}
		const addrinfo* inf = info;
		for (addrinfo* ai = info; ai; ai = ai->ai_next)
		{
			if (ai->ai_family == AF_INET)
			{
				inf = ai;
				break;
			}
		}
		resize((int)inf->ai_addrlen);
		memcpy(ptr(), inf->ai_addr, inf->ai_addrlen);
		if (inf->ai_family == AF_INET)
		{
			((sockaddr_in*)ptr())->sin_port = htons(port);
			_type = IPv4;
		}
		else if (inf->ai_family == AF_INET6)
		{
			((sockaddr_in6*)ptr())->sin6_port = htons(port);
			_type = IPv6;
		}
		freeaddrinfo(info);
	}
	else
	{
		resize(sizeof(sockaddr_in));
		sockaddr_in* addr = (sockaddr_in*)ptr();
		addr->sin_addr.s_addr = INADDR_ANY;
		addr->sin_family = AF_INET;
		addr->sin_port = htons(port);
	}
	return true;
}

struct HostPort
{
	String host, port;
};

HostPort parseHostPort(const String& u)
{
	HostPort hp;
	int hoststart = 0, hostend = u.length(), portstart = 0;
	if (u[0] == '[') // IPv6
	{
		hoststart++;
		hostend = u.lastIndexOf(']');
		if (hostend < 0)
			return hp;
		if (u[hostend + 1] == ':')
			portstart = hostend + 2;
	}
	else {
		int i = u.lastIndexOf(':');
		if (i > 1) {
			hostend = i;
			portstart = i + 1;
		}
	}
	hp.host = u.substring(hoststart, hostend);
	if (portstart > 0)
		hp.port = u.substring(portstart);
	return hp;
}

bool InetAddress::set(const String& host)
{
	String thehost;
	int c = host.indexOf(':');
	if (host[0] != '[' && c >= 0 && host.indexOf(':', c + 1) > 0)
		thehost << '[' << host << ']';
	else
		thehost = host;

	HostPort hp = parseHostPort(thehost);
	if (!hp.port.ok() && (hp.host.contains('/') || hp.host.contains('\\')))
	{
#ifdef ASL_SOCKET_LOCAL
		resize(sizeof(sockaddr_un));
		sockaddr_un* a=(sockaddr_un*)ptr();
		a->sun_family=AF_UNIX;
		memcpy(a->sun_path, *host, min((int)sizeof(a->sun_path), host.length() + 1));
		a->sun_path[sizeof(a->sun_path) - 1] = '\0';
		_type = LOCAL;
		return true;
#endif
	}
	return set(hp.host, hp.port);
}

// Socket

Socket_::Socket_()
{
#ifdef _WIN32
	if(!g_wsaStarted) startWSA();
#endif
	_handle = -1;
	_family = InetAddress::IPv4;
	_error = 0;
	_type = TCP;
	_blocking = true;
	_endian = ENDIAN_NATIVE;
}

Socket_::Socket_(int fd)
{
#ifdef _WIN32
	if (!g_wsaStarted)
		startWSA();
#endif
	_handle = fd;
	_family = InetAddress::IPv4;
	_error = 0;
	_type = TCP;
	_blocking = true;
	_endian = ENDIAN_NATIVE;
}

Socket_::~Socket_()
{
	Socket_::close();
}

bool Socket_::init(bool force)
{
	if (_handle >= 0 && force)
		close();
	if (_handle < 0) {
		_handle = (int)socket(sockfamily(_family), _type == PACKET? SOCK_DGRAM : SOCK_STREAM, 0);
	}
	_error = _handle < 0 ? SOCKET_BAD_INIT : 0;
	return !_error;
}

void Socket_::close()
{
	if(_handle >= 0)
#ifndef _WIN32
	::close(_handle);
#else
	closesocket(_handle);
#endif
	_handle = -1;
}

bool Socket_::bind(const String& name)
{
	HostPort hp = parseHostPort(name);
	return bind(hp.host, hp.port);
}

bool Socket_::bind(const String& ip, int port)
{
	if (port < 0 || port > 65535)
	{
		_error = SOCKET_BAD_BIND;
		return false;
	}

	InetAddress here(ip, port);
	if (here.length() == 0)
		return false;

	init(_family != here.type());
	_family = here.type();
	setOption(SOL_SOCKET, SO_REUSEADDR, 1);

	if(::bind(_handle, (sockaddr*)here.ptr(), here.length()))
	{
		verbose_print("Can't bind to %s\n", *here.toString());
		_error = SOCKET_BAD_BIND;
		return false;
	}
	return true;
}

void Socket_::listen(int n)
{
	::listen(_handle, n);
}

Socket_* Socket_::accept()
{
	return new Socket_((int)::accept(_handle, (sockaddr*)0, (socklen_t*)0));
}

bool Socket::connect(const String& host, int port)
{
	Array<InetAddress> addrs = InetAddress::lookup(host);
	if (addrs.length() == 0)
	{
		_()->_error = SOCKET_BAD_DNS;
		return false;
	}
	_()->_hostname = host;
	for (int i = 0; i < addrs.length(); i++)
	{
		if (connect(addrs[i].setPort(port)))
			return true;
	}
	_()->_error = SOCKET_BAD_CONNECT;
	return false;
}

bool Socket_::connect(const InetAddress& addr)
{
	if (addr.length() == 0)
	{
		_error = SOCKET_BAD_CONNECT;
		return false;
	}
	bool force = _family != addr.type();
	_family = addr.type();
	init(force);
	if (::connect(_handle, (const sockaddr*)addr.ptr(), addr.length()) == 0)
		return true;
	_error = SOCKET_BAD_CONNECT;
	return false;
}

InetAddress Socket_::remoteAddress() const
{
	InetAddress a(_family);
	socklen_t n = (socklen_t)a.length();
	if (getpeername(handle(), (sockaddr*)a.ptr(), &n))
		return InetAddress();
	return a;
}

InetAddress Socket_::localAddress() const
{
	InetAddress a(_family);
	socklen_t n = (socklen_t)a.length();
	getsockname(handle(), (sockaddr*)a.ptr(), &n);
	return a;
}

bool Socket_::setOption(int level, int opt, const void* val, int n)
{
	init();
	return setsockopt(_handle, level, opt, (const char*)val, n) == 0;
}

void Socket::enableBroadcast(bool on)
{
	setOption(SOL_SOCKET, SO_BROADCAST, on ? 1ul : 0);
}

int Socket_::available()
{
	if (_error != 0 || _handle < 0)
		return -1;
#ifndef _WIN32
	long n;
	if (ioctl(_handle, FIONREAD, &n) == 0)
#else
	unsigned long n;
	if (ioctlsocket(_handle, FIONREAD, &n) == 0)
#endif
		return (int)n;
	else
		return -1;
}

String Socket_::readLine()
{
	char c;
	String s;
	if (available()>0 || waitInput())
	{
		while (1)
		{
			int n = read(&c, 1);
			if (n <= 0 || c == '\n' || _error != 0)
				break;
			if (s.length() > 16000)
			{
				_error = SOCKET_BAD_LINE;
				s = "";
				break;
			}
			s += c;
		}
	}
	return s;
}

int Socket_::read(void* data, int size)
{
		int s = 0, size0 = size;
		do
		{
#ifdef _WIN32
		int n = recv(_handle, (char*)data, size, 0);
#else
		int n = ::read(_handle, (char*)data, size);
#endif
		if (!_blocking)
			return n;
			if (n <= 0)
			{
			_error = SOCKET_BAD_RECV;
			break;
		}
		data = (char*)data + n;
		s += n;
		size -= n;
		} while (s < size0);
	return s;
	}

int Socket_::write(const void* data, int size)
{
	if (size == 0)
		return 0;
	int s = 0, size0 = size;
	do
	{
#ifdef _WIN32
		int n = ::send(_handle, (const char*)data, size, 0);
#else
		int n = ::send(_handle, data, size, MSG_NOSIGNAL);
#endif
		if (!_blocking)
			return n;
		if (n < 0)
{
			_error = SOCKET_BAD_DATA;
			break;
	}
		data = (char*)data + n;
		s += n;
		size -= n;
	} while (s < size0);
	return s;
}

ByteArray Socket_::read(int n)
{
	ByteArray a((n < 0) ? available() : n);
	n = read(&a[0], a.length());
	return a.resize(max(0, n));
}

void Socket_::skip(int n)
{
	ByteArray a(n);
	read(a.data(), a.length());
}

bool Socket_::disconnected()
{
	return _handle < 0 || _error != 0 || (waitInput(0) && available() <= 0);
}

bool Socket_::waitInput(double t)
{
	if (_handle < 0)
		return false;
	int a = available();
	if (a > 0)
		return true;
	if (a < 0)
	{
		_error = SOCKET_BAD_DATA;
		return true;
	}
	fd_set rset;
	timeval to;
	to.tv_sec = (int)floor(t);
	to.tv_usec = (int)((t-floor(t))*1e6);
	FD_ZERO(&rset);
	FD_SET(handle(), &rset);
	if(select(handle()+1, &rset, 0, 0, &to) >= 0)
		return FD_ISSET(handle(), &rset)!=0;
	_error = SOCKET_BAD_WAIT;
	return true;
}

String Socket_::errorMsg() const
{
	return messages[_error];
}


// PacketSocket (UDP)

PacketSocket_::PacketSocket_()
{
	_type = PACKET;
	_blocking = false;
}

PacketSocket_::PacketSocket_(int fd) : Socket_(fd)
{
	_type = PACKET;
	_blocking = false;
}

String PacketSocket_::readLine()
{
	String s(1000, 0);
	if (waitInput())
	{
		int m = read(&s[0], 1000);
		if (m > 0)
			s[m - 1] = '\0';
		int n = s.indexOf('\n');
		if (n >= 0)
			s[n] = '\0';
	}
	return s.fix();
}

int PacketSocket_::readFrom(InetAddress& a, void* data, int n)
{
	if (_handle < 0)
		init();
	a = InetAddress(_family);
	socklen_t len = (socklen_t)a.length();
	int r= recvfrom(_handle, (char*)data, n, 0, (sockaddr*)a.ptr(), &len);
	return r;
}

void PacketSocket_::sendTo(const InetAddress& to, const void* data, int n)
{
	if (_handle < 0)
		init();
	sendto(_handle, (const char*)data, n, 0, (sockaddr*)to.ptr(), to.length());
}

// LocalSocket (UNIX)

LocalSocket_::LocalSocket_()
{
	_type = LOCAL;
	_family = InetAddress::LOCAL;
}

LocalSocket_::LocalSocket_(int fd) : Socket_(fd)
{
	_type = LOCAL;
	_family = InetAddress::LOCAL;
}

LocalSocket_::~LocalSocket_()
{
#ifdef ASL_SOCKET_LOCAL
	String path = localAddress().toString();
	if (path)
	{
		close();
#ifdef _WIN32
		DeleteFileW(path);
#else
		unlink(path);
#endif
	}
#endif
}

bool LocalSocket_::bind(const String& name)
{
#ifdef ASL_SOCKET_LOCAL
#ifdef _WIN32
	DeleteFileW(name);
#else
	unlink(name);
#endif
	init();
	InetAddress here(name);
	if (::bind(_handle, (sockaddr*)here.ptr(), sizeof(sockaddr_un)))
	{
		_error = SOCKET_BAD_BIND;
		return false;
	}
	return true;
#else
	asl_error("Unix sockets not supported");
	_error = SOCKET_BAD_BIND;
	return false;
#endif
}

Socket_* LocalSocket_::accept()
{
	return new LocalSocket_((int)::accept(_handle, (sockaddr*)0, (socklen_t*)0));
}

}

