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

#ifndef ASL_NOEXCEPT
#define BAD_OPTION(o) throw SocketOptionException(o)
#else
#define BAD_OPTION(o) asl::asl_die("Cannot set socket option " o, __LINE__)
#endif

namespace asl {

MulticastSocket_::MulticastSocket_()
{
}

void MulticastSocket_::join(const InetAddress& a)
{
	int on = 1;
	init(a.type() != _family);
	if(!setOption(SOL_SOCKET, SO_REUSEADDR, on))
		printf("Cannot set socket option SO_REUSEADDR\n");

	bind("0.0.0.0", a.port());

	struct ip_mreq mr;
	mr.imr_multiaddr = ((sockaddr_in*)a.ptr())->sin_addr;
	mr.imr_interface.s_addr = INADDR_ANY;
	if (!setOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, mr))
	{
		BAD_OPTION("IP_ADD_MEMBERSHIP");
	}
}

void MulticastSocket_::leave(const InetAddress& a)
{
	struct ip_mreq mr;
	mr.imr_multiaddr = ((sockaddr_in*)a.ptr())->sin_addr;
	mr.imr_interface.s_addr = INADDR_ANY;
	if (!setOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, mr))
	{
		//BAD_OPTION("IP_DROP_MEMBERSHIP"); // seems to fail on WSL !!
	}
}

void MulticastSocket_::multicast(const InetAddress& a, int ttl)
{
	connect(a);
	setOptions(true, ttl);
}

void MulticastSocket_::setOptions(bool loopback, int ttl)
{
	char c=loopback? 1:0;
	// in Win32 the loopback can't be disabled
	if (!setOption(IPPROTO_IP, IP_MULTICAST_LOOP, c))
	{
		/*if (c == 0)
			// trying to disable loopback
		*/
		printf("Cannot set socket option IP_MULTICAST_LOOP\n");
	}
	u_char t = ttl; //(ttl > 255) ? 255 : (ttl < 0) ? 0 : ttl;
	if (!setOption(IPPROTO_IP, IP_MULTICAST_TTL, t))
	{
		BAD_OPTION("IP_MULTICAST_TTL");
	}
}

}
