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
	return false;
}
#endif

bool SocketServer::bindPath(const String& sname)
{
#ifndef _WIN32
	LocalSocket userver;
	if(userver.bind(sname))
	{
		userver.listen(5);
		_sockets << userver;
		return true;
	}
#endif
	return false;
}

void SocketServer::startLoop()
{
	int n;
	_running = true;
	do
	{
		if ((n = _sockets.waitInput(10)) > 0)
		{
			for (int i = 0; i < n; i++)
			{
				Socket client = _sockets.activeAt(i).accept();
				if (_sequential) {
					serve(client);
					client.close();
				}
				else
					new SockClientThread(this, client);
			}
		}
		if(_requestStop || n < 0)
		{
			_running = false;
			return;
		}
		
	}
	while(true);
}

void SocketServer::start(bool nonblocking)
{
	if(nonblocking) {
		_thread = new SockServerThread(this);
		_thread->start();
	}
	else
		startLoop();
}

void SocketServer::stop()
{
	_requestStop = true;
	_sockets.close();
}

#ifdef ASL_TLS
void SocketServer::useCert(const String& cert, const String& key)
{
	for (int i = 0; i < _sockets.length(); i++)
	{
		if (TlsSocket s = _sockets[i].as<TlsSocket>())
		{
			if (!s.useCert(cert))
				printf("bad cert: %s\n", *cert);
			if (!s.useKey(key))
				printf("bad key: %s\n", *key);
		}
	}
}
#endif

}
