// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#include <asl/Socket.h>
#include <asl/Map.h>
#include <asl/File.h>
#include <asl/SocketServer.h>
#include <asl/HttpServer.h>
#include <asl/WebSocket.h>

namespace asl {

bool verbose = false;

HttpServer::HttpServer(int port):
	_proto("HTTP/1.1"),
	_methods("GET, POST, OPTIONS, PUT, DELETE, PATCH, HEAD")
{
	_requestStop = false;
	if (port >= 0)
		bind(port);
	_wsserver = NULL;
	_cors = false;
	_mimetypes = String(
		"css:text/css,"
		"gif:image/gif,"
		"htm:text/html,"
		"html:text/html,"
		"jpeg:image/jpeg,"
		"jpg:image/jpeg,"
		"js:application/javascript,"
		"json:application/json,"
		"png:image/png,"
		"txt:text/plain,"
		"mp4:video/mp4,"
		"ogv:video/ogg,"
		"webm:video/webm,"
		"xml:text/xml"
		).split(',', ':');
}

void HttpServer::addMimeType(const String& ext, const String& type)
{
	_mimetypes[ext] = type;
}

void HttpServer::serve(Socket client)
{
	double t1 = now();
	while(!client.disconnected() && now() - t1 < 10.0 && !_requestStop)
	{
		if (!client.waitData(5))
			continue;

		HttpRequest request(client);
		if (client.error())
			break;

		String hconn = request.header("Connection").toLowerCase();

		if (request.header("Upgrade") == "websocket" && _wsserver)
		{
			if(verbose) printf("handing over to ws\n");
			_wsserver->process(client, request.headers());
			return;
		}
		HttpResponse response(request);
		response.put("");
		if (_cors && request.hasHeader("Origin"))
		{
			response.setHeader("Access-Control-Allow-Origin", request.header("Origin"));
			response.setHeader("Access-Control-Allow-Credentials", "true");
		}
		if (!handleOptions(request, response))
		{
			serve(request, response);
			if (response.code() == 405)
				response.setHeader("Allow", _methods);

			if (response.containsFile())
			{
				File file(response.text());
				if (!file.exists())
				{
					response.setCode(404);
					response.setHeader("Content-Type", "text/html");
					response.put("<h1>Error</h1><p>File <b>" + file.name() + "</b> not found</p>");
					continue;
				}

				String mime = _mimetypes.get(file.extension(), "text/plain");
				response.setHeader("Date", Date::now().toString(Date::HTTP));
				response.setHeader("Content-Type", mime);
				if (hconn == "keep-alive")
					response.setHeader("Connection", "keep-alive");
				if (!response.hasHeader("Cache-Control"))
					response.setHeader("Cache-Control", "max-age=60, public");
				if (request.hasHeader("Range"))
				{
					String range = request.header("Range");
					if (range.startsWith("bytes=") && !range.contains(',')) // no multiple ranges
					{
						Array<String> parts = range.substr(6).split('-');
						int begin = parts[0];
						int end = parts[1];
						response.setCode(206);
						response.setHeader("Content-Range", "");
						response.putFile(file.path(), begin, end);
					}
				}
				else
					response.putFile(file.path());
				if (response.header("Content-Range").contains('*'))
				{
					response.setCode(416);
					response.write();
				}
			}
			else
				response.write();
		}
		
		if ((request.protocol() == "HTTP/1.0" && hconn != "keep-alive") || hconn == "close")
			break;
	}
}

void HttpServer::setRoot(const String& root)
{
	_webroot = root;
}

void HttpServer::serveFile(HttpRequest& request, HttpResponse& response)
{
	if (request.method() == "GET")
	{
		String path = Url::decode(request.path());

		if (path.endsWith("/"))
			path += "index.html";

		String localpath = _webroot + path;
		File file(localpath);
		if (file.isDirectory())
		{
			response.setCode(301);
			response.setHeader("Location", "http://" + request.header("Host") + path+'/');
		}
		else if (file.exists())
		{
			if (request.hasHeader("If-Modified-Since"))
			{
				Date ifdate = request.header("If-Modified-Since");
				if (file.lastModified() <= ifdate + 1.0) {
					response.setCode(304);
					return;
				}
			}
			response.setHeader("Last-Modified", file.lastModified().toString(Date::HTTP));
			response.put(file);
		}
		else
		{
			response.setCode(404);
			response.setHeader("Content-Type", "text/html");
			response.put("<h1>Not found</h1>");
		}
	}
	else
	{
		response.setCode(501);
		response.setHeader("Content-Type", "text/html");
		response.put("<h1>Not implemented</h1>");
	}
}

void HttpServer::addMethod(const String& verb)
{
	if (_methods == "")
		_methods = "GET, POST, OPTIONS, PUT, DELETE, PATCH, HEAD";
	Array<String> methods = _methods.split(", ");
	if (!methods.contains(verb))
		methods << verb;
	_methods = methods.join(", ");
}

bool HttpServer::handleOptions(HttpRequest& request, HttpResponse& response)
{
	if (request.method() == "OPTIONS")
	{
		if (request.hasHeader("Origin"))
			response.setHeader("Access-Control-Allow-Methods", _methods);
		if (request.hasHeader("Access-Control-Request-Headers"))
			response.setHeader("Access-Control-Allow-Headers", request.header("Access-Control-Request-Headers"));
		response.setHeader("Allow", _methods);
		response.setHeader("Content-Length", "0");
		response.setCode(200);
		response.write();
		return true;
	}
	else
		return false;
}

void HttpServer::serve(HttpRequest& request, HttpResponse& response)
{
	if (_webroot.ok())
		serveFile(request, response);
}

}
