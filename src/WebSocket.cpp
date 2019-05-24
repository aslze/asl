#include <asl/Socket.h>
#include <asl/Map.h>
#include <asl/WebSocket.h>
#include <asl/StreamBuffer.h>
#include <asl/SHA1.h>
#include <asl/util.h>
#include <asl/Http.h>
#include <asl/JSON.h>
#ifdef ASL_TLS
#include <asl/TlsSocket.h>
#endif
#include <ctype.h>

namespace asl {
	
static void DEBUG_LOG(...) {}
//#define DEBUG_LOG printf
//#define DEBUG_LOG


WebSocketMsg::operator String() const
{
	return String(_data);
}

WebSocketMsg::operator Var() const
{
	return Json::decode((const char*)_data.ptr());
}

const char* WebSocketMsg::operator*() const
{
	return (const char*)_data.ptr();
}

WebSocketMsg& WebSocketMsg::fix()
{
	_data << '\0';
	_data.resize(_data.length() - 1);
	return *this;
}

WebSocketServer::WebSocketServer()
{
	_requestStop = false;
}

WebSocketServer::WebSocketServer(int port)
{
	bind(port);
	_requestStop = false;
}

void WebSocketServer::serve(Socket client)
{
	String head = client.readLine();
	int i = head.indexOf(' ');
	if (i == -1)
		return;
	int j = head.indexOf(' ', i + 1);
	if (j == -1)
		return;
	String method = head.substring(0, i);
	String res = head.substring(i + 1, j);

	String line;
	Dic<String> headers;
	while (line = client.readLine(), line != "\r")
	{
		line = line.trim();
		int i = line.indexOf(':');
		if (i<0) {
			client.close();
			return;
		}
		String name = line.substring(0, i);
		String cname;
		bool capitalize = true;
		for (int i = 0; i < name.length(); i++)
		{
			cname << char(capitalize ? toupper(name[i]) : tolower(name[i]));
			capitalize = !isalnum(name[i]);
		}

		String value = (i < line.length() - 1) ? line.substring(i + 2) : String();
		headers[cname] = value;
	}

	DEBUG_LOG("%s\n\n\n", *headers.join("\n", ": "));

	process(client, headers);
}

void WebSocketServer::process(Socket& client, const Dic<String>& headers)
{
	if (!headers.has("Upgrade") || headers["Upgrade"] != "websocket" || !headers["Connection"].split(", ").contains("Upgrade"))
	{
		client << "HTTP/1.1 400 Bad request\r\n\r\nNot a WebSocket request";
		return;
	}

	String key = headers["Sec-Websocket-Key"];

	SHA1::Hash hash = SHA1::hash(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	String digest = encodeBase64(hash, hash.length());

	client << String(180, "HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: %s\r\n", *digest);

	if (headers.has("Sec-Websocket-Protocol"))
		client << "Sec-Websocket-Protocol: chat\r\n";
	client << "\r\n";

	WebSocket ws(client, false);
	{
		Lock l(_mutex);
		_clients << &ws;
	}
	serve(ws);
	{
		Lock l(_mutex);
		_clients.removeOne(&ws);
	}
	client.close();
}



void WebSocketServer::serve(WebSocket& ws)
{
}

WebSocket::WebSocket()
{
	_isClient = true;
	_closed = true;
	_code = 1000;
	_socket.setEndian(ENDIAN_BIG);
	_random.init();
}

WebSocket::WebSocket(const Socket& s, bool isclient):
	_socket(s),
	_isClient(isclient)
{
	_closed = false;
	_code = 1000;
	_socket.setEndian(ENDIAN_BIG);
	_socket.setBlocking(true);
	_random.init();
}

bool WebSocket::connect(const String& uri, int port)
{
	String path = "/";

	Url url = parseUrl(uri);
	if (port != 0 && url.port == 0)
		url.port = port;
	if (url.protocol == "wss")
	{
#ifdef ASL_TLS
		_socket.close();
		_socket = TlsSocket();
		_socket.setEndian(ENDIAN_BIG);
		_socket.setBlocking(true);
		if (url.port == 0) url.port = 443;
#else
		return false;
#endif
	}
	else
		if (url.port == 0) url.port = 80;

	if (!_socket.connect(url.host, url.port)) {
		//printf("Cannot connect to %s : %i\n", *url.host, url.port);
		return false;
	}

	byte key[16];
	for (int i = 0; i < 16; i++)
		key[i] = (byte)_random(255);

	String key64 = encodeBase64(key, 16);

	_socket << String(200, "GET %s HTTP/1.1\r\n"
		"Host: %s:%i\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: %s\r\n"
		"Sec-WebSocket-Protocol: chat\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"Pragma: no-cache\r\n\r\n", *url.path, *url.host, url.port, *key64);

	String line = _socket.readLine();
	int i = line.indexOf(' ');
	if (i == -1) {
		_socket.close();
		return false;
	}
	int j = line.indexOf(' ', i + 1);
	if (j == -1) {
		_socket.close();
		return false;
	}
	String protocol = line.substring(0, i);
	String status = line.substring(i + 1, j);

	if (status != "101") {
		_socket.close();
		return false;
	}

	Dic<String> headers;
	while (line = _socket.readLine(), line != "\r")
	{
		line = line.trim();
		int i = line.indexOf(':');
		if (i<0) {
			_socket.close();
			return false;
		}
		String name = line.substring(0, i);
		String value = (i < line.length() - 1) ? line.substring(i + 2) : String();
		headers[name] = value;
	}

	DEBUG_LOG("%s\n\n\n", *headers.join("\n", ": "));

	if (headers["Upgrade"] != "websocket" || !headers["Connection"].split(", ").contains("Upgrade"))
	{
		_socket.close();
		return false;
	}

	_closed = false;

	return true;
}

void WebSocket::close()
{
	_socket.close();
	_closed = true;
}

bool WebSocket::closed()
{
	if (_closed)
		return true;
	if (_socket.disconnected()) {
		_closed = true;
		_socket.close();
		return true;
	}
	return false;
}

WebSocketMsg WebSocket::receive()
{
	WebSocketMsg msg;
	bool haveMsg = false;
	while (!haveMsg)
	{
		Array<byte> buffer;
		byte b0, mlen;
		DEBUG_LOG("receive\n");
		if (closed()) {
			return msg.fix();
		}
		DEBUG_LOG("avail %i\n", _socket.available());
		_socket >> b0 >> mlen;
		DEBUG_LOG("%i %i\n", b0, mlen);
		if (closed()) {
			return msg.fix();
		}
		bool fin = !!(b0 & 0x80);
		int opcode = b0 & 0x0f;
		bool masked = !!(mlen & 0x80);
		int len = mlen & 0x7f;
		if (len == 126)
		{
			len = _socket.read<unsigned short>();
		}
		else if (len == 127)
			len = (int)_socket.read<Long>(); // what if length larger than int?

		unsigned mask = 0;
		if (masked)
			_socket >> mask;

		buffer.resize(buffer.length() + len);
		if (len > 0)
			_socket.read(buffer.ptr() + buffer.length() - len, len);

		DEBUG_LOG("frame: op %i fin %i len %i\n", opcode, fin ? 1 : 0, (int)len);

		if (masked)
		{
			swapBytes(mask);
			buffer.resize(buffer.length() + 4);
			buffer.resize(buffer.length() - 4);
			int n = buffer.length() / 4 + 1;
			for (int i = 0; i < n; i++)
			{
				((unsigned*)buffer.ptr())[i] ^= mask;
			}
		}

		switch (opcode)
		{
		case 0: // continuation
		case 1: // text
		case 2: // binary
			msg.append(buffer);
			break;
		case 8: // connection close
		{
			if (buffer.length() >= 2) {
				_code = (buffer[0] << 8) | buffer[1];
				buffer.remove(0, 2);
				msg = buffer;
			}
			haveMsg = true;
			_closed = true;
			_socket.close();
			break;
		}
		case 9: // ping
			send(buffer, buffer.length(), FRAME_PONG);
			break;
		case 10: // pong
			buffer.clear();
			break;
		}

		if (fin)
			haveMsg = true;
	}

	return msg.fix();
}

void WebSocket::send(const Var& v)
{
	send(Json::encode(v));
}

void WebSocket::send(const byte* p, int length, FrameType type)
{
	if (length <= 0 || _closed)
		return;
	byte opcode = (type == FRAME_TEXT) ? 1 : (type == FRAME_BINARY) ? 2 : (type == FRAME_PONG) ? 10 : (type == FRAME_PING) ? 9 : 8;
	byte b0 = 0x80 | opcode;
	byte masked = _isClient ? 0x80 : 0;
	StreamBuffer buf(ENDIAN_BIG);
	buf << b0;
	Long len = length;
	if (len < 126)
		buf << byte(masked | (byte)len);
	else if (len < (1 << 16))
		buf << byte(masked | (byte)126) << (unsigned short)len;
	else
		buf << byte(masked | (byte)127) << len;

	unsigned mask = 0;
	if (_isClient) {
		mask = _random.get();
		buf << mask;
	}
	Array<byte> data(p, length);
	if (mask != 0){
		swapBytes(mask);
		data.resize(data.length() + 4);
		data.resize(data.length() - 4);
		int n = data.length() / 4 + 1;
		for (int i = 0; i < n; i++)
		{
			((unsigned*)data.ptr())[i] ^= mask;
		}
	}
	if (!_closed || _socket.disconnected())
		_socket << *buf << data;
}

bool WebSocket::wait(double timeout)
{
	return _socket.waitInput(timeout);
}

bool WebSocket::hasInput()
{
	return _socket.available() > 0;
}

}
