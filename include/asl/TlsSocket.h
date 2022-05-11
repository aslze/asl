// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined(ASL_TLSSOCKET_H) && defined(ASL_TLS)
#define ASL_TLSSOCKET_H

#include <asl/Array.h>
#include <asl/String.h>
#include <asl/Socket.h>

namespace asl {

class TlsSocket;
struct TlsCore;

ASL_SMART_CLASS(TlsSocket, Socket)
{
	ASL_SMART_INNER_DEF(TlsSocket);
	TlsCore* _core;
	bool setOption(int level, int opt, const void* p, int n);
	TlsSocket_();
	~TlsSocket_();
	int handle() const;
	bool bind(const String& ip, int port);
	void listen(int n = 5);
	Socket_* accept();
	bool connect(const InetAddress& host);
	void close();
	int available();
	int read(void* data, int size);
	int write(const void* data, int n);
	bool waitInput(double timeout = 60);
	String errorMsg() const;
	bool useCert(const String& cert);
	bool useKey(const String& key);
};

/**
A TLS secure socket. Derives from Socket and has the same interface.
\ingroup Sockets
*/
class ASL_API TlsSocket : public Socket
{
public:
	ASL_SMART_DEF(TlsSocket, Socket);
	/**
	Sets the certificate to use by this socket (needs a subsequent call to useKey)
	*/
	bool useCert(const String& cert)
	{
		return _()->useCert(cert);
	}
	/**
	Sets the private key to use by this socket
	*/
	bool useKey(const String& key)
	{
		return _()->useKey(key);
	}
};

}
#endif

