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

bool MulticastSocket_::join(const InetAddress& a)
{
	init(a.type() != _family);
	setOption(SOL_SOCKET, SO_REUSEADDR, 1);

	if(!bind("0.0.0.0", a.port()))
		return false;

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
