#include <asl/Socket.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

namespace asl {

bool MulticastSocket_::join(const InetAddress& a, int interfac)
{
	init(a.type() != _family);

	socklen_t n = (socklen_t)a.length();
	if (getsockname(handle(), (sockaddr*)a.ptr(), &n))
	{
	if(!bind("0.0.0.0", a.port()))
		return false;
	}

	if (_family == InetAddress::IPv6)
	{
		ipv6_mreq mr;
		mr.ipv6mr_multiaddr = ((sockaddr_in6*)a.ptr())->sin6_addr;
		mr.ipv6mr_interface = interfac;
		return setOption(IPPROTO_IP, IPV6_ADD_MEMBERSHIP, mr);
	}
	else
	{
		ip_mreq mr;
	mr.imr_multiaddr = ((sockaddr_in*)a.ptr())->sin_addr;
		mr.imr_interface.s_addr = interfac;
	return setOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, mr);
}
}

bool MulticastSocket_::leave(const InetAddress& a, int interfac)
{
	if (_family == InetAddress::IPv6)
	{
		ipv6_mreq mr;
		mr.ipv6mr_multiaddr = ((sockaddr_in6*)a.ptr())->sin6_addr;
		mr.ipv6mr_interface = interfac;
		return setOption(IPPROTO_IP, IPV6_DROP_MEMBERSHIP, mr);
	}
	else
	{
		ip_mreq mr;
	mr.imr_multiaddr = ((sockaddr_in*)a.ptr())->sin_addr;
		mr.imr_interface.s_addr = interfac;
	return setOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, mr); // seems to fail on WSL !!
}
}

bool MulticastSocket_::setLoop(bool loopback)
{
	int c = loopback ? 1 : 0;
	return setOption(IPPROTO_IP, IP_MULTICAST_LOOP, c);
	}

bool MulticastSocket_::setTTL(int ttl)
	{
	return setOption(IPPROTO_IP, IP_MULTICAST_TTL, ttl);
}

}
