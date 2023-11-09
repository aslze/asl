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

#include <stdlib.h>
#include <stdio.h>

namespace asl {

MulticastSocket_::MulticastSocket_()
{
}

bool MulticastSocket_::join(const InetAddress& a)
{
	init(a.type() != _family);
	if(!setOption(SOL_SOCKET, SO_REUSEADDR, 1))
		printf("MulticastSocket: Cannot set SO_REUSEADDR\n");

	bind("0.0.0.0", a.port());

	struct ip_mreq mr;
	mr.imr_multiaddr = ((sockaddr_in*)a.ptr())->sin_addr;
	mr.imr_interface.s_addr = INADDR_ANY;
	return setOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, mr);
}

bool MulticastSocket_::leave(const InetAddress& a)
{
	struct ip_mreq mr;
	mr.imr_multiaddr = ((sockaddr_in*)a.ptr())->sin_addr;
	mr.imr_interface.s_addr = INADDR_ANY;
	return setOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, mr); // seems to fail on WSL !!
}

void MulticastSocket_::multicast(const InetAddress& a, int ttl)
{
	connect(a);
	setOptions(true, ttl);
}

void MulticastSocket_::setOptions(bool loopback, int ttl)
{
	int c = loopback ? 1 : 0;
	if (!setOption(IPPROTO_IP, IP_MULTICAST_LOOP, c))
	{
		printf("MulticastSocket: Cannot set IP_MULTICAST_LOOP\n");
	}

	if (!setOption(IPPROTO_IP, IP_MULTICAST_TTL, ttl))
	{
		printf("MulticastSocket: Cannot set IP_MULTICAST_TTL\n");
	}
}

}
