#include <asl/SocketServer.h>
#include <asl/Thread.h>
#ifdef ASL_TLS
#include <asl/TlsSocket.h>
#endif
#include <stdio.h>

namespace asl {

struct SockClientThread : public Thread
{
	SocketServer* _server;
	Socket _client;

	SockClientThread(SocketServer* svr, const Socket& cli):
		_server(svr), _client(cli)
	{
		start();
	}
	void run()
	{
		_server->serve(_client);
		_client.close();
		--_server->_numClients;
		delete this;
	}
};

struct SockServerThread : public Thread
{
	SocketServer* _server;

	SockServerThread(SocketServer* svr):
		_server(svr)
	{
	}
	void run()
	{
		_server->startLoop();
	}
};

SocketServer::SocketServer()
{
	_thread = NULL;
	_requestStop = false;
	_sequential = false;
	_running = false;
	_numClients = 0;
}

SocketServer::~SocketServer()
{
	if(_thread) {
		_thread->kill();
		delete _thread;
	}
}

bool SocketServer::bind(const String& ip, int port)
{
	Socket server;
	if(server.bind(ip, port))
	{
		server.listen(40);
		_sockets << server;
		return true;
	}
	_socketError = server.errorMsg();
	return false;
}

#ifdef ASL_TLS
bool SocketServer::bindTLS(const String& ip, int port)
{
	TlsSocket server;
	if(server.bind(ip, port))
	{
		server.listen(40);
		_sockets << server;
		return true;
	}
	_socketError = server.errorMsg();
	return false;
}
#endif

bool SocketServer::bindPath(const String& sname)
{
#if !defined(_WIN32) || defined(ASL_SOCKET_LOCAL)
	LocalSocket server;
	if(server.bind(sname))
	{
		server.listen(5);
		_sockets << server;
		return true;
	}
	_socketError = server.errorMsg();
#else
	(void)sname;
	_socketError = "SOCKET_BAD_BIND";
#endif
	return false;
}

void SocketServer::startLoop()
{
	int n;
	do
	{
		if ((n = _sockets.waitInput(2)) > 0)
		{
			for (int i = 0; i < n; i++)
			{
				Socket client = _sockets.activeAt(i).accept();
				++_numClients;
				if (_sequential) {
					serve(client);
					client.close();
					--_numClients;
				}
				else
					new SockClientThread(this, client);
			}
		}
		if(_requestStop || n < 0)
		{
			_running = false;
			break;
		}
		
	}
	while(true);
}

void SocketServer::start(bool nonblocking)
{
	_running = true;

	if(nonblocking) {
		_thread = new SockServerThread(this);
		_thread->start();
	}
	else
		startLoop();
}

void SocketServer::stop(bool sync)
{
	_requestStop = true;
	
	if (sync)
	{
		do {
			sleep(0.1);
		} while (_running || _numClients > 0);
	}
}

String SocketServer::socketError() const
{
	return _socketError;
}

#ifdef ASL_TLS
bool SocketServer::useCert(const String& cert, const String& key)
{
	for (int i = 0; i < _sockets.length(); i++)
	{
		if (TlsSocket s = _sockets[i].as<TlsSocket>())
		{
			if (!s.useCert(cert) || !s.useKey(key))
			{
				_socketError = s.errorMsg();
				return false;
			}
		}
	}
	return true;
}
#endif

}
