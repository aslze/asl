#define SOCKET_CPP

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0501
struct IUnknown; // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here" when using /permissive-
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#endif

#include <stdio.h>
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
//#define OLD_RESOLVE
#define AF_TYPE AF_INET
#endif

static void verbose_print(...) {}
//#define verbose_print printf

namespace asl {

Sockets& Sockets::operator<<(Socket& s)
{
	set << s;
	return *this;
}

int Sockets::waitInput(double t)
{
	fd_set rset;
	struct timeval to;
	to.tv_sec = (int)floor(t);
	to.tv_usec = (int)((t-floor(t))*1e6);
	FD_ZERO(&rset);
	int m=0;
	for(int i=0; i<set.length(); i++)
	{
		if (set[i].handle() < 0)
			return -1;
		if (set[i].handle() > m) m = set[i].handle();
		FD_SET(set[i].handle(), &rset);
	}
	int r=select(m+1, &rset, 0, 0, &to);
	if(r<0)
		return -1;
	changed.clear();
	for (int j = 0; j < set.length(); j++) {
		if (set[j].handle() < 0)
			return -1;
		if (FD_ISSET(set[j].handle(), &rset))
			changed << set[j];
	}
	return changed.length();
}

bool Sockets::hasInput(Socket& s)
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
#ifndef _WIN32
	case InetAddress::LOCAL: return AF_UNIX;
#endif
	default:;
	}
	return AF_INET;
}

InetAddress::InetAddress()
{
	_type = IPv4;
}

InetAddress::~InetAddress()
{
}

InetAddress::InetAddress(const String& host, int port)
{
	set(host, port);
}

InetAddress::InetAddress(const String& host)
{
	set(host);
}

InetAddress::InetAddress(InetAddress::Type t)
{
	_type = t;
	int n = 0;
	switch (t)
	{
	case IPv4: n = sizeof(sockaddr_in); break;
	case IPv6: n = sizeof(sockaddr_in6); break;
#ifndef _WIN32
	case LOCAL: n = sizeof(sockaddr_un); break;
#endif
	default: ;
	}
	data.resize(n);
	memset(data.ptr(), 0, n);
}

InetAddress::InetAddress(int port)
{
	set("", port);
}

InetAddress::InetAddress(const InetAddress& a):
	data(a.data), _type(a._type)
{
}

void InetAddress::operator=(const InetAddress& a)
{
	data = a.data;
	_type = a._type;
}

int InetAddress::port() const
{
	if (_type == IPv6) {
		sockaddr_in6* addr = (sockaddr_in6*)data.ptr();
		return ntohs(addr->sin6_port);
	}
	else if (_type == IPv4) {
		sockaddr_in* addr = (sockaddr_in*)data.ptr();
		return ntohs(addr->sin_port);
	}
	return 0;
}

InetAddress& InetAddress::setPort(int port)
{
	if (_type == IPv6) {
		sockaddr_in6* addr = (sockaddr_in6*)data.ptr();
		addr->sin6_port = htons(port);
	}
	else if (_type == IPv4) {
		sockaddr_in* addr = (sockaddr_in*)data.ptr();
		addr->sin_port = htons(port);
	}
	return *this;
}

String InetAddress::host() const
{
	if (_type == IPv6)
	{
		sockaddr_in6* addr = (sockaddr_in6*)data.ptr();
		unsigned short* ip = (unsigned short*)&addr->sin6_addr;
		return String(40, "%x:%x:%x:%x:%x:%x:%x:%x",
			bytesSwapped(ip[0]), bytesSwapped(ip[1]), bytesSwapped(ip[2]), bytesSwapped(ip[3]),
			bytesSwapped(ip[4]), bytesSwapped(ip[5]), bytesSwapped(ip[6]), bytesSwapped(ip[7]));
	}
	else if (_type == IPv4)
	{
		sockaddr_in* addr = (sockaddr_in*)data.ptr();
		byte* ip = (byte*)&addr->sin_addr;
		return String(15, "%i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
	}
#ifndef _WIN32
	sockaddr_un* addr = (sockaddr_un*)data.ptr();
	return addr->sun_path;
#else
	return "";
#endif
}

String InetAddress::toString() const
{
	int p = port();
	if (p == 0)
		return host();
	else if (_type == IPv4)
		return host() << ':' << port();
	else
		return (String) '[' << host() << "]:" << p;
}

bool InetAddress::operator==(const InetAddress& a)
{
	return _type == a._type && data == a.data;
}

Array<InetAddress> InetAddress::lookup(const String& host)
{
#ifdef _WIN32
	if (!g_wsaStarted) startWSA();
#endif
	Array<InetAddress> addresses;
	struct addrinfo hints;
	struct addrinfo* info;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = host.contains(':') ? AF_INET6 : AF_TYPE;
	hints.ai_socktype = 0;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	int s = getaddrinfo(host, NULL, &hints, &info);
	if (s != 0) {
		verbose_print("Cannot resolve %s\n", *host);
		return addresses;
	}
	for (struct addrinfo* ai = info; ai != NULL; ai = ai->ai_next) {
		InetAddress a;
		if (ai->ai_family == AF_INET)
			a._type = IPv4;
		else
			a._type = IPv6;
		a.data.resize((int)ai->ai_addrlen);
		memcpy(a.data.ptr(), ai->ai_addr, ai->ai_addrlen);
		if (!addresses.contains(a)) {
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
	if(host)
	{
		addrinfo hints, *info;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = host.contains(':') ? AF_INET6 : AF_TYPE;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;
		hints.ai_protocol = 0;
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;

		int s = getaddrinfo(host, NULL, &hints, &info);
		if (s != 0) {
			printf("Cannot resolve %s\n", *host);
			data.clear();
			return false;
		}
		addrinfo* inf = info;
		for (addrinfo* ai = info; ai != NULL; ai = ai->ai_next) {
			if (ai->ai_family == AF_INET) {
				inf = ai;
				break;
			}
		}
		data.resize((int)inf->ai_addrlen);
		memcpy(data.ptr(), inf->ai_addr, inf->ai_addrlen);
		if (inf->ai_family == AF_INET)
		{
			((sockaddr_in*)data.ptr())->sin_port = htons(port);
			_type = IPv4;
		}
		else if (inf->ai_family == AF_INET6)
		{
			((sockaddr_in6*)data.ptr())->sin6_port = htons(port);
			_type = IPv6;
		}
		freeaddrinfo(info);
	}
	else {
		data.resize(sizeof(sockaddr_in));
		sockaddr_in* addr = (sockaddr_in*)data.ptr();
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
		if (i > 0) {
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
	HostPort hp = parseHostPort(host);
	if(!hp.port && hp.host.contains('/'))
	{
#ifndef _WIN32
		data.resize(sizeof(sockaddr_un));
		sockaddr_un* a=(sockaddr_un*)data.ptr();
		a->sun_family=AF_UNIX;
		strcpy(a->sun_path, host.substring(0, min(107, host.length())));
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
	_error = false;
	_type = TCP;
	_blocking = true;
	_endian = ENDIAN_NATIVE;
}

Socket_::Socket_(bool)
{
	_handle = -1;
	_family = InetAddress::IPv4;
	_error = false;
	_type = TCP;
	_blocking = false;
	_endian = ENDIAN_NATIVE;
}

Socket_::Socket_(int fd)
{
	_handle = fd;
	_family = InetAddress::IPv4;
	_error = false;
	_type = TCP;
	_blocking = true;
	_endian = ENDIAN_NATIVE;
}

Socket_::~Socket_()
{
	close();
}

bool Socket_::init(bool force)
{
	if (_handle >= 0 && force)
		close();
	if (_handle < 0) {
		_handle = (int)socket(sockfamily(_family), _type == PACKET? SOCK_DGRAM : SOCK_STREAM, 0);
	}
	_error = _handle < 0;
	return !_error;
}

void Socket_::close()
{
	verbose_print("socket close %i\n", _handle);
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
	InetAddress here(ip, port);
	if (here.length() == 0)
		return false;

	init(_family != here.type());
	_family = here.type();
	if (!setOption(SOL_SOCKET, SO_REUSEADDR, 1))
		return false;

	if(::bind(_handle, (sockaddr*)here.ptr(), here.length())) {
		verbose_print("Can't bind to %s\n", *here.toString());
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
	_()->_hostname = host;
	for (int i = 0; i < addrs.length(); i++)
	{
		if (connect(addrs[i].setPort(port)))
			return true;
	}
	return false;
}

bool Socket_::connect(const InetAddress& addr)
{
	if (addr.length() == 0)
		return false;
	bool force = _family != addr.type();
	_family = addr.type();
	init(force);
	return ::connect(_handle, (sockaddr*)addr.ptr(), addr.length()) == 0;
}

InetAddress Socket_::remoteAddress() const
{
	InetAddress a(_family);
	socklen_t n = (socklen_t)a.length();
	getpeername(handle(), (sockaddr*)a.ptr(), (socklen_t*)&n);
	return a;
}

InetAddress Socket_::localAddress() const
{
	InetAddress a(_family);
	socklen_t n = (socklen_t)a.length();
	getsockname(handle(), (sockaddr*)a.ptr(), (socklen_t*)&n);
	return a;
}

bool Socket_::setOption(int level, int opt, const void* val, int n)
{
	init();
	int r = setsockopt(_handle, level, opt, (const char*)val, n);
	/*if (r < 0) {
		int e = h_errno;
		printf("sockopt %i fail %i\n", opt, e);
	}*/
	return r >= 0;
}

void Socket::enableBroadcast(bool on)
{
	setOption(SOL_SOCKET, SO_BROADCAST, on ? 1ul : 0);
}

int Socket_::available()
{
	if (_error || _handle < 0)
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
	if (available()>0 || waitInput()) {
		while (1) {
			int n = read(&c, 1);
			if (n <= 0 || c == '\n' || _error)
				break;
			if (s.length() > 8000) {
				_error = true;
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
	if(_blocking) {
	int s = 0;
	do {
#ifdef _WIN32
		int n = recv(_handle, (char*)data, size, 0);
#else
		int n = ::read(_handle, (char*)data, size);
#endif
		if (n <= 0) {
			_error = true;
			break;
		}
		data = (char*)data + n;
		s += n;
		size -= n;
	} while (s < size);
	return s;
	}
	else {
#ifdef _WIN32
		return recv(_handle, (char*)data, size, 0);
#else
		return ::read(_handle, data, size);
#endif
	}
}

int Socket_::write(const void* data, int n)
{
#ifndef _WIN32
	//int m = ::send(_handle, data, n, MSG_NOSIGNAL);
	int m = ::write(_handle, data, n);
	if (m != n) {
		verbose_print("socket %i wrote %i of %i\n", _handle, m, n);
	}
	return m;
#else
	return ::send(_handle, (char*)data, n, 0);
#endif
}

Array<byte> Socket_::read(int n)
{
	Array<byte> a((n < 0)? available() : n);
	a.resize(read(&a[0], a.length()));
	return a;
}

void Socket_::skip(int n)
{
	Array<byte> a(n);
	read(a.ptr(), a.length());
}

bool Socket_::disconnected()
{
	return _error || (waitInput(0) && available() <= 0);
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
		_error = true;
		return true;
	}
	fd_set rset;
	struct timeval to;
	to.tv_sec = (int)floor(t);
	to.tv_usec = (int)((t-floor(t))*1e6);
	FD_ZERO(&rset);
	FD_SET(handle(), &rset);
	if(select(handle()+1, &rset, 0, 0, &to) >= 0)
		return FD_ISSET(handle(), &rset)!=0;
	_error = true;
	return true;
}

// PacketSocket (UDP)

PacketSocket_::PacketSocket_() : Socket_(false)
{
	_type = PACKET;
	_family = InetAddress::IPv4;
#ifdef _WIN32
	if(!g_wsaStarted) startWSA();
#endif
}

PacketSocket_::PacketSocket_(int fd) : Socket_(fd)
{
	_type = PACKET;
}

PacketSocket_::~PacketSocket_()
{
}

String PacketSocket_::readLine()
{
	String s(1000, '\0');
	if (waitInput()) {
		read(&s[0], 1000);
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

#ifndef _WIN32

// LocalSocket (UNIX)

LocalSocket_::LocalSocket_() : Socket_(false)
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
	if(_pathname)
	{
		close();
		unlink(_pathname);
	}
}

bool LocalSocket_::bind(const String& name)
{
	_pathname=name;
	unlink(name);
	init();
	if(!setOption(SOL_SOCKET, SO_REUSEADDR, 1))
		return false;
	InetAddress here(name);
	if(::bind(_handle, (sockaddr*)here.ptr(), here.length()))
	{
		verbose_print("Can't bind to %s\n", *name);
		return false;
	}
	return true;
}

#endif

}

