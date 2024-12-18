#include <asl/Socket.h>
#include <asl/Var.h>
#include <asl/Map.h>
#include <asl/File.h>
#include <asl/Http.h>
#include <asl/JSON.h>
#include <asl/TlsSocket.h>
#include <ctype.h>

#define SEND_BLOCK_SIZE 128000
#define RECV_BLOCK_SIZE 16000

#ifdef _MSC_VER
#pragma warning(disable : 26451 26812)
#endif

namespace asl {

String Url::decode(const String& q0)
{
	String q;
	char   b[3];
	b[2] = '\0';
	for (int i = 0; i < q0.length(); i++)
	{
		char c = q0[i];
		if (c == '%')
		{
			if (i > q0.length() - 2)
				break;
			b[0] = q0[i + 1];
			b[1] = q0[i + 2];
			q << (char)strtoul(b, NULL, 16);
			i += 2;
		}
		else
		q << c;
	}
#ifdef ASL_ANSI
	return utf8ToLocal(q);
#else
	return q;
#endif
}

inline bool isanyof(char c, const char* chars)
{
	char k;
	while ((k = *chars++))
		if (k == c)
			return true;
	return false;
}

inline char hexNibble(int x)
{
	const char h[] = "0123456789ABCDEF";
	return h[x];
}

String Url::encode(const String& q0_, bool component)
{
#ifdef ASL_ANSI
	String q0 = localToUtf8(q0_);
#else
	const String& q0 = q0_;
#endif

	String q(q0.length(), 0);
	for (int i = 0; i < q0.length(); i++)
	{
		byte c = *(byte*)&q0[i];
		if (!isalnum(c) && !isanyof(c, component ? "-_.!~*'()" : "-_.!~*'();/?:@&=+$,#"))
			q << '%' << hexNibble(c >> 4) << hexNibble(c & 0x0f);
		else
			q << (char)c;
	}
	return q;
}

String Url::params(const Dic<>& q)
{
	Dic<> d;
	foreach2(String& k, const String& v, q)
		d[Url::encode(k, true)] = Url::encode(v, true);
	return d.join('&', '=');
}

Dic<> Url::parseQuery(const String& querystring)
{
	Dic<> query;
	Dic<> q = querystring.replace('+', ' ').split('&', '=');
	foreach2(String& k, const String& v, q)
		query[Url::decode(k)] = Url::decode(v);
	return query;
}

Url::Url(const String& url)
{
	int hoststart = 0;
	int i = url.indexOf("://");
	if (i > 0) {
		protocol = url.substring(0, i);
		hoststart = i + 3;
	}

	int pathstart = url.indexOf('/', hoststart);
	if (pathstart < 0)
		pathstart = url.length();
	host = url.substring(hoststart, pathstart);
	path = url.substring(pathstart);
	if (path == "")
		path = '/';
	int hostend = pathstart, portstart = 0;
	if (url[hoststart] == '[') // IPv6
	{
		hoststart++;
		hostend = url.indexOf(']', hoststart);
		if (hostend < 0)
		{
			*this = Url();
			return;
		}
		if (url[hostend + 1] == ':')
			portstart = hostend + 2;
	}
	else {
		int j = url.indexOf(':', hoststart);
		if (j >= 0 && j < pathstart) {
			hostend = j;
			portstart = j + 1;
		}
	}
	host = url.substring(hoststart, hostend);
	port = (portstart == 0)? 0 : (int)url.substring(portstart, pathstart);
}

struct HttpSinkArray : public HttpSink
{
	ByteArray* a;
	HttpSinkArray(ByteArray& a) : a(&a) {}
	int write(byte* p, int n)
	{
		a->append(p, n);
		return n;
	}
	void use(HttpMessage* m)
	{
		a = (ByteArray*)&m->body();
	}
	void init(int n)
	{
		a->reserve(n);
	}
};

struct HttpSinkFile : public HttpSink
{
	File* file;
	HttpSinkFile(File& f) : file(&f) {}
	int write(byte* p, int n)
	{
		return file->write(p, n);
	}
};

HttpMessage::HttpMessage() : _proto("HTTP/1.1"), _socket(NULL), _fileBody(false), _chunked(false)
{
	_sink = new HttpSinkArray(_body);
	_headersSent = false;
	_status = new HttpStatus;
	memset(&*_status, 0, sizeof(*_status));
}

String HttpMessage::text() const
{
	return String(_body);
}

Var HttpMessage::json() const
{
	String str = _body;
	Var data = Json::decode(str);
	return data.ok() ? data : Var(Url::parseQuery(str));
}

void HttpMessage::put(const ByteArray& data)
{
	_body = data;
	_fileBody = false;
	setHeader("Content-Length", _body.length());
}

void HttpMessage::put(const Var& body)
{
	if (header("Content-Type") == "application/x-www-form-urlencoded")
	{
		Dic<> dic;
		foreach2(String & k, Var & v, body)
			dic[Url::encode(k, true)] = Url::encode(v, true);
		put(dic.join('&', '='));
	}
	else
	{
		put(Json::encode(body));
		setHeader("Content-Type", "application/json");
	}
}

void HttpMessage::put(const File& body)
{
	put(body.path());
	_fileBody = true;
}

String capitalized(const String& name) 
{
	String cname = name;
	char*  pname = cname.data();
	bool capitalize = true;
	for (int i = 0; i < cname.length(); i++)
	{
		pname[i] = capitalize ? toupper(pname[i]) : tolower(pname[i]);
		capitalize = pname[i] == '-';
	}

	return cname;
}

void HttpMessage::setHeader(const String& name, const String& value)
{
	String cname = capitalized(name);
	if (!value.ok())
		_headers.remove(cname);
	else
		_headers[cname] = value;
}

String HttpMessage::header(const String& name) const
{
	String cname = capitalized(name);
	const String* pvalue = _headers.find(cname);
	return pvalue ? *pvalue : String();
}

bool HttpMessage::hasHeader(const String& name) const
{
	return _headers.has(capitalized(name));
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
	int size = header("Content-Length");

	int currentsize = 0;

	bool chunked = header("Transfer-Encoding") == "chunked"; // Handle specially!!

	_sink->init(size);

	_socket->setBlocking(true);

	bool end = false;

	if (hasHeader("Content-Length")) {
		if (header("Content-Length") == "0")
			return;
	}
	else if(!chunked)
		return;

	_status->totalReceive = size;
	_status->received = 0;

	while (!end)
	{
		int av = _socket->available();
		if (av < 0 || !_socket->waitInput(10)) {
			break;
		}
		byte buffer[RECV_BLOCK_SIZE];
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
			_status->received = currentsize;
			_sink->write(buffer, bytesRead);
			if(_progress)
				_progress(*_status);
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
	//printf("readbody end\n");
}

HttpResponse Http::request(HttpRequest& request)
{
	Socket socket((Socket::Ptr)NULL);
	HttpResponse response(request);
	response.setCode(0);

	Url url(request.url());
	bool hasPort = url.port != 0;

	if (url.protocol == "https")
	{
#ifdef ASL_TLS
		socket = TlsSocket();
		if (!hasPort) url.port = 443;
#else
		response.setSockError("SOCKET_NO_TLS_AVAILABLE");
		return response;
#endif
	}
	else {
		socket = Socket();
		if (!hasPort) url.port = 80;
	}

	response.use(socket);
	request.use(socket);

	if (!socket.connect(url.host, url.port)) {
		//printf("Cannot connect to %s : %i\n", *url.host, url.port);
		socket.close();
		response.setSockError(socket.errorMsg());
		return response;
	}
	
	if (request.body().length() != 0) {
		request.setHeader("Content-Length", request.body().length());
	}

	String title;
	title << request.method() << ' ' << url.path << " HTTP/1.1\r\nHost: " << url.host;
	if (hasPort)
		title << ':' << url.port;
	request._command = title;

	if (!request.write())
	{
		response.setSockError(socket.errorMsg());
		return response;
	}
	
	String line = socket.readLine();
	if (!line.ok()) {
		socket.close();
		response.setSockError(socket.errorMsg());
		return response;
	}
	Array<String> parts = line.split();
	if (parts.length() < 2) {
		socket.close();
		response.setSockError(socket.errorMsg());
		return response;
	}

	response.setProto(parts[0]);
	response.setCode(parts[1]);
	response.readHeaders();

	int code = response.code();

	if (request.followRedirects() && (code == 301 || code == 302 || code == 307 || code == 308)) // 303 ?
	{
		socket.close();
		String loc = response.header("Location");
		HttpRequest req(request);
		req.setUrl(loc);
		Http::Progress progress = req._progress;
		req.onProgress(progress);
		int n = request.recursion() + 1;
		if (n < 4) {
			req.setRecursion(n);
			return Http::request(req);
		}
		else {
			response.setCode(421);
			response.setSockError("Too many redirects");
			return response;
		}
	}

	response.onProgress(request._progress);
	response.useSink(request._sink);

	response.readBody();

	socket.close();
	return response;
}


void HttpRequest::read()
{
	_addr = _socket->remoteAddress();
	_command = _socket->readLine();
	if (_socket->error() || !_command.ok())
		return;
	int i = _command.indexOf(' ');
	if (i == -1)
		return;
	int j = _command.indexOf(' ', i + 1);
	if (j == -1)
		return;
	_method = _command.substring(0,i);
	_res = _command.substring(i + 1, j);
	_proto = _command.substring(j + 1).trim();

	readHeaders();
	
	if (header("Expect") == "100-continue")
	{
		if ((Long)header("Content-Length") < 128000000)
			*_socket << "HTTP/1.1 100 Continue\r\n\r\n";
		else
			*_socket << "HTTP/1.1 417 Too big\r\n\r\n";
	}

	readBody();
	/*
	if(_body.length() > 0) // dump
	{
		File("bb.txt").put(_body);
	}
	*/
	int pathend = _res.length();
	int h = _res.indexOf('#');
	if (h > 0)
	{
		pathend = h;
		_fragment = _res.substring(h + 1);
	}
	int q = _res.indexOf('?');
	if (q > 0)
	{
		_querystring = _res.substring(q + 1, h > 0 ? h : pathend);
		pathend = q;
	}

	_path = Url::decode(_res.substring(0, pathend));

	if(_path.contains(".."))
		_path = _path.replace("..", "");

	_path.split('/', _parts);
	if (_parts.length() > 0)
	{
		if (_parts.last() == "")
			_parts.removeLast();
		if (_parts.length() > 0 && _parts[0] == "")
			_parts.remove(0);
	}
}

const Dic<>& HttpRequest::query()
{
	if(_querystring.length() != 0 && _query.length() == 0)
		_query = Url::parseQuery(_querystring);

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
	_headersSent = true;
	setCode(0);
}

HttpResponse::HttpResponse(const HttpRequest& r)
{
	_socket = r._socket;
	_status = r._status;
	_status->sent = 0;
	_status->received = 0;
	if (r._proto == "HTTP/1.0")
		_proto = r._proto;
	_headersSent = false;
	setCode(200);
}

void HttpResponse::setCode(int code)
{
	_code = code;
	String msg;
	if (code == 200)
		msg = "OK";
	else if (code == 404)
		msg = "Not Found";
	else if (code == 206)
		msg = "Partial Content";
	else if (code >= 500)
		msg = "Server error";
	else if (code >= 400)
		msg = "Request error";
	else if (code >= 300)
		msg = "Redirect";
	else
		msg = "OK";

	_command = String::f("%s %i %s", *_proto, code, *msg);
}

bool HttpResponse::is(HttpResponse::StatusType code) const
{
	return _code / 100 == code;
}

String HttpResponse::socketError() const
{
	return _socketError;
}

void HttpMessage::useSink(const Shared<HttpSink>& s)
{
	_sink = s;
	_sink->use(this);
}

bool HttpMessage::sendHeaders()
{
	String s;
	s << _command << "\r\n";
	foreach2(String& name, String& value, _headers)
	{
		s << name << ": " << value << "\r\n";
	}
	s << "\r\n";
	int sent = _socket->write(*s, s.length());
	if (sent < s.length())
		return false;
	_headersSent = true;
	String contentlength = header("Content-Length");
	_chunked = !contentlength.ok();
	_status->totalSend = _chunked ? 0 : int(contentlength);
	return true;
}

bool HttpMessage::write()
{
	if (_fileBody)
		return putFile(_body);
	else
		return write((const char*)_body.data(), _body.length()) > 0;
}

void HttpMessage::write(const String& text)
{
	write(*text, text.length());
}

int HttpMessage::write(const char* buffer, int n)
{
	if (!_headersSent)
		if (!sendHeaders())
			return false;
	int sent = n == 0 ? 1 : 0;
	while (n > 0)
	{
		int m = min(n, SEND_BLOCK_SIZE);
		if (_chunked)
			*_socket << String::f("%x\r\n", m);
		int written = _socket->write(buffer, m);
		if (written != m)
			return sent;
		_status->sent += written;
		if (_progress)
			_progress(*_status);

		sent += written;
		if (_chunked)
			*_socket << "\r\n";
		n -= m;
		buffer += m;
	}
	return sent;
}

void HttpMessage::writeFile(const String& path, int begin, int end)
{
	File file(path, File::READ);
	if (!file)
		return;
	if (!_headersSent)
		sendHeaders();
	int n = 1;
	file.seek(begin);
	Long size = file.size();
	if (begin != end)
		size = end - begin + 1;
	int bytesSent = 0;
	//HttpStatus status;
	//status.sent = 0;
	//status.totalSend = (int)size;
	while(n > 0 && bytesSent < (int)size)
	{
		char buf[RECV_BLOCK_SIZE];
		n = file.read(buf, min((int)sizeof(buf), (int)size - bytesSent));
		if (n > 0) {
			int w = 0;
			if ((w = write(buf, n)) < 0)
			{
				break;
			}
			bytesSent += n;
		}
	};
}

bool HttpMessage::putFile(const String& path, int begin, int end)
{
	File file(path);
	if (!file.exists())
	{
		printf("file to upload not found %s\n", *path);
		put("");
		write();
		return false;
	}
	if (begin == 0 && end == 0 && !hasHeader("Content-Range"))
		setHeader("Content-Length", file.size());
	else
	{
		Long size = file.size();
		if (end == 0)
			end = int(size - 1);
		if (end <= begin || begin < 0 || end > size)
		{
			setHeader("Content-Length", "0");
			setHeader("Content-Range", String::f("bytes */%lli", size));
			return false;
		}
		setHeader("Content-Length", end - begin + 1);
		setHeader("Content-Range", String::f("bytes %i-%i/%lli", begin, end, size));
	}

	bool multipart = header("Content-Type") == "multipart/form-data";

	String boundary;

	if (multipart)
	{
		boundary = "-----------";
		for (int i = 0; i < 64; i++)
			boundary += (char)random('0', '9');

		String head = "--" + boundary + "\r\n" +
			"Content-Disposition: form-data; name=\"files\"; filename=\"" + file.name() + "\"\r\n" +
			"Content-Type: application/octet-stream\r\n\r\n";

		setHeader("Content-Length", int(header("Content-Length")) + head.length() + boundary.length() + 8);
		setHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

		write(head);
	}
	writeFile(path, begin, end);
	
	if (multipart)
	{
		write("\r\n--" + boundary + "--\r\n");
	}

	return true;
}

bool Http::download(const String& url, const String& path, const Function<void, const HttpStatus&>& f, const Dic<>& headers)
{
	File file(path, File::WRITE);
	if (!file)
		return false;
	HttpRequest req("GET", url, headers);
	req.onProgress(f);
	req.useSink(new HttpSinkFile(file));
	HttpResponse res = request(req);
	return res.ok();
}

bool asl::Http::upload(const String& url, const String& path, const Dic<>& headers, const Function<void, const HttpStatus&>& f)
{
	if (!File(path).isFile())
		return false;
	HttpRequest req("POST", url, File(path), headers);
	if (!req.hasHeader("Content-Type"))
		req.setHeader("Content-Type", "multipart/form-data");
	req.onProgress(f);
	HttpResponse res = request(req);
	return res.ok();
}

}
