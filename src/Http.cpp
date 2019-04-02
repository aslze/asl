#include <asl/Socket.h>
#include <asl/Map.h>
#include <asl/File.h>
#include <asl/IniFile.h>
#include <asl/Http.h>
#include <asl/JSON.h>
#include <asl/TlsSocket.h>
#include <ctype.h>

namespace asl {

String decodeUrl(const String& q0)
{
	String q;
	for (int i = 0; i < q0.length(); i++)
	{
		char c = q0[i];
		if (c == '%')
		{
			if (i > q0.length() - 2)
				break;
			int val = q0.substring(i + 1, i + 3).hexToInt();
			q << (char)val;
			i += 2;
			continue;
		}
		q << c;
	}
	return q;
}

String encodeUrl(const String& q0)
{
	String q;
	for (int i = 0; i < q0.length(); i++)
	{
		byte c = *(byte*)&q0[i];
		if (!isalnum(c) && c != '-' && c != '.' && c != '~' && c != '/' && c != ':')
			q << String(3, "%%%0X", (int)c);
		else
			q << (char)c;
	}
	return q;
}

Dic<> decodeUrlParams(const String& querystring)
{
	Dic<> query;
	Dic<> q = split(querystring.replace('+', ' '), '&', '=');
	foreach2(String& k, const String& v, q)
		query[decodeUrl(k)] = decodeUrl(v);
	return query;
}

Url parseUrl(const String& url)
{
	Url u;
	int hoststart = 0;
	int i = url.indexOf("://");
	if (i > 0) {
		u.protocol = url.substring(0, i);
		hoststart = i + 3;
	}

	int pathstart = url.indexOf('/', hoststart);
	if (pathstart < 0)
		pathstart = url.length();
	u.host = url.substring(hoststart, pathstart);
	u.path = url.substring(pathstart);
	if (u.path == "")
		u.path = '/';
	int hostend = pathstart, portstart = 0;
	if (url[hoststart] == '[') // IPv6
	{
		hoststart++;
		hostend = url.indexOf(']', hoststart);
		if (hostend < 0)
			return Url();
		if (url[hostend + 1] == ':')
			portstart = hostend + 2;
	}
	else {
		int i = url.indexOf(':', hoststart);
		if (i >= 0 && i < pathstart) {
			hostend = i;
			portstart = i + 1;
		}
	}
	u.host = url.substring(hoststart, hostend);
	u.port = (portstart == 0)? 0 : (int)url.substring(portstart, pathstart);
	return u;
}


HttpMessage::HttpMessage() : _progress(NULL), _fileBody(false), _chunked(false)
{
	_headersSent = false;
}

HttpMessage::HttpMessage(const Dic<>& headers) : _headers(headers), _progress(NULL), _fileBody(false), _chunked(false)
{
	_headersSent = false;
}

HttpMessage::HttpMessage(Socket& s) : _socket(&s), _progress(NULL), _fileBody(false), _chunked(false)
{
	_headersSent = false;
}

String HttpMessage::text() const
{
	return String(_body);
}

Var HttpMessage::json() const
{
	String str = _body;
	Var data = Json::decode(str);
	return data.ok() ? data : Var(decodeUrlParams(str));
}

void HttpMessage::put(const String& body)
{
	_body = Array<byte>((byte*)*body, body.length());
	setHeader("Content-Length", _body.length());
}

void HttpMessage::put(const Array<byte>& data)
{
	_body = data;
	setHeader("Content-Length", _body.length());
}

void HttpMessage::put(const Var& body)
{
	put(Json::encode(body));
	setHeader("Content-Type", "application/json");
}

void HttpMessage::put(const File& body)
{
	put(body.path());
	_fileBody = true;
}

void HttpMessage::setHeader(const String& header, const String& value)
{
	String name;// = capitalize(header);
	bool capitalize = true;
	for (int i = 0; i < header.length(); i++)
	{
		name << char(capitalize ? toupper(header[i]) : tolower(header[i]));
		capitalize = !isalnum(header[i]);
	}
	_headers[name] = value;
}

String HttpMessage::header(const String& name) const
{
	return _headers.has(name) ? _headers[name] : String();
}

bool HttpMessage::hasHeader(const String& name) const
{
	return _headers.has(name);
}

void HttpMessage::readHeaders()
{
	String headerName, headerValue, line;

	while (line = _socket->readLine(), line != "\r")
	{
		if (isspace(line[0])) // multiline
		{
			setHeader(headerName, headerValue + line.trimmed());
			continue;
		}
		line.trim();
		int i = line.indexOf(':');
		if (i<0) {
			_socket->close();
			return;
		}
		headerName = line.substring(0, i);
		headerValue = (i < line.length() - 1) ? line.substring(i + 2) : String();
		setHeader(headerName, headerValue);
	}
}

void HttpMessage::readBody()
{
	int size = hasHeader("Content-Length") ? (int)header("Content-Length") : 0;

	//int totalsize = size;
	int currentsize = 0;

	bool chunked = header("Transfer-Encoding") == "chunked"; // Handle specially!!

	_body.resize(size);
	_body.clear();

	_socket->setBlocking(true);

	bool end = false;

	if (hasHeader("Content-Length")) {
		if (header("Content-Length") == "0")
			return;
	}
	else if(!chunked)
		return;

	while (!end)
	{
		int av = _socket->available();
		if (av < 0 || !_socket->waitInput()) {
			break;
		}
		byte buffer[16384];
		int maxToRead = _socket->available(), bytesRead = 0;
		if (chunked)
		{
			String chunkSize = _socket->readLine();
			maxToRead = chunkSize.hexToInt();
			if (maxToRead == 0)
				end = true;
		}
		while (maxToRead > 0) {
			bytesRead = _socket->read(buffer, min(maxToRead, (int)sizeof(buffer)));
			if (bytesRead <= 0) {
				return;
			}
			currentsize += bytesRead;
			//_progress(currentsize, totalsize);
			_body.append(buffer, bytesRead);
			maxToRead -= bytesRead;
			if (size) {
				size -= bytesRead;
				if (size <= 0) {
					return;
				}
			}
		}
		//if (maxToRead == 0) // ANDROID ?
		//	break;

		if (chunked)
		{
			if (_socket->read(buffer, 2) < 2) // skip crlf
				break;
		}
	}
}

HttpRequest::~HttpRequest()
{
	//_socket->close();
}

HttpResponse Http::request(HttpRequest& request)
{
	Socket socket((Socket::Ptr)NULL);
	HttpResponse response;

	Url url = parseUrl(request.url());
	if (url.protocol == "https")
	{
#ifdef ASL_TLS
		socket = TlsSocket();
		if (url.port == 0) url.port = 443;
#else
		return response;
#endif
	}
	else {
		socket = Socket();
		if (url.port == 0) url.port = 80;
	}

	response.use(socket);
	request.use(socket);

	if (!socket.connect(url.host, url.port)) {
		//printf("Cannot connect to %s : %i\n", *url.host, url.port);
		socket.close();
		return response;
	}
	
	if (request.body().length() != 0) {
		request.setHeader("Content-Length", request.body().length());
	}

	String title;
	title << request.method() << ' ' << url.path << " HTTP/1.1\r\nHost: " << url.host << ':' << url.port;
	request._command = title;
	request.sendHeaders();

	if (request.body().length() != 0)
		request.write();
	
	String line = socket.readLine();
	if (!line) {
		socket.close();
		return response;
	}
	Array<String> parts = line.split();
	if (parts.length() < 2) {
		socket.close();
		return response;
	}

	response.setProto(parts[0]);
	response.setCode(parts[1]);

	response.readHeaders();

	int code = response.code();

	if (request.followRedirects())
	{
		if (code == 301 || code == 302 || code == 307 || code == 308) // 303 ?
		{
			socket.close();
			String url = response.header("Location");
			HttpRequest req(request.method(), url, request.headers());
			int n = request.recursion() + 1;
			if (n < 4) {
				req.setRecursion(n);
				return Http::request(req);
			}
			else {
				response.setCode(421);
				return response;
			}
		}
	}

	response.readBody();

	socket.close();
	return response;
}


void HttpRequest::read()
{
	_addr = _socket->remoteAddress();
	_command = _socket->readLine();
	if (_socket->error() || ! _command)
		return;
	int i = _command.indexOf(' ');
	if(i==-1)
		return;
	int j = _command.indexOf(' ', i+1);
	if(j==-1)
		return;
	_method = _command.substring(0,i);
	_res = _command.substring(i+1, j);

	readHeaders();
	readBody();
	/*
	if(_body.length() > 0) // dump
	{
		File("bb.txt").put(_body);
	}
	*/
	int pathend = _res.length();
	int h = _res.indexOf('#');
	if(h > 0)
	{
		pathend = h;
		_fragment = _res.substring(h+1);
	}
	int q = _res.indexOf('?');
	if(q > 0)
	{
		_querystring = _res.substring(q+1, h>0? h : pathend);
		pathend = q;
	}
	_path = _res.substring(0, pathend);
	_parts = _path.split('/');
	if(_parts.length() > 0) {
		if(_parts.last() == "")
			_parts.remove(_parts.length()-1);
		if(_parts.length()>0 && _parts[0] == "")
			_parts.remove(0);
	}
}

const Dic<>& HttpRequest::query()
{
	if(_querystring.length() != 0 && _query.length() == 0)
	{
		Dic<> q = split(_querystring.replaceme('+', ' '), '&', '=');
		foreach2(String& k, const String& v, q)
			_query[decodeUrl(k)] = decodeUrl(v);
	}
	return _query;
}

const String& HttpRequest::query(const String& key)
{
	return query()[key];
}

const Array<String>& HttpRequest::parts() const
{
	return _parts;
}

bool HttpRequest::is(const String& pat)
{
	int i = pat.indexOf('*');
	if(i<0) {
		_argument = "";
		return _path == pat;
	}
	else if(/*_path.length() >= i &&*/ _path.startsWith(pat.substring(0, i))) {
		_argument = _path.substring(i);
		return true;
	}
	else {
		_argument = "";
		return false;
	}
}

HttpResponse::HttpResponse()
{
	_proto = "HTTP/1.0";
	_headersSent = true;
	setCode(0);
}

HttpResponse::HttpResponse(const HttpRequest& r, const String& proto, int code):
	HttpMessage(*r._socket)
{
	_proto = proto;
	_headersSent = false;
	setCode(200);
}

void HttpResponse::setCode(int code)
{
	_code = code;
	_command = String(0, "%s %i %s", *_proto, code, (code==200)? "OK" : "Not found");
}

bool HttpResponse::is(HttpResponse::StatusType code) const
{
	return _code / 100 == code;
}


void HttpMessage::sendHeaders()
{
	String s;
	s << _command << "\r\n";
	foreach2(String& name, String& value, _headers)
	{
		s << name << ": " << value << "\r\n";
	}
	s << "\r\n";
	*_socket << s;
	_headersSent = true;
	_chunked = !_headers.has("Content-Length");
}

void HttpMessage::write()
{
	if (!_headersSent)
		sendHeaders();
	if (_chunked)
		*_socket << String("%x\r\n", _body.length());
	*_socket << _body;
	if (_chunked)
		*_socket << "\r\n";
}

void HttpMessage::write(const String& text)
{
	if(!_headersSent)
		sendHeaders();
	if (_chunked)
		*_socket << String("%x\r\n", text.length());
	*_socket << text;
	if (_chunked)
		*_socket << "\r\n";
}

int HttpMessage::write(const char* buffer, int n)
{
	if(!_headersSent)
		sendHeaders();
	if (_chunked)
		*_socket << String("%x\r\n", n);
	int m = _socket->write(buffer, n);
	if (_chunked)
		*_socket << "\r\n";
	return m;
}

void HttpMessage::writeFile(const String& path)
{
	File file(path, File::READ);
	if (!file)
		return;
	if (!_headersSent)
		sendHeaders();
	int n = 1;
	while(n > 0)
	{
		char buf[16384];
		n = file.read(buf, sizeof(buf));
		if (n>0)
			if (write(buf, n) < 0)
				break;
	};
}

void HttpMessage::putFile(const String& path)
{
	setHeader("Content-Length", File(path).size());
	writeFile(path);
}

}
