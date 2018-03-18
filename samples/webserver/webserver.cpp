#include <asl/HttpServer.h>
#include <asl/IniFile.h>
#include <asl/TextFile.h>

/*
This is a very simple web server.
In www.ini we can configure the document root directory (index.html will be served by default) and
the TCP port.
If TLS support was built, this will also serve HTTPS on port 443. By default it will use a dummy certificate,
but you can also use an actual certificate/key. In that case, set the cert and key values in www.ini to the
path of the files containing them.
*/

using namespace asl;

int main(int argc, char* argv[])
{
	IniFile config("www.ini");
	String webroot = config("webroot", ".");
	int port = config("port", 8000);

	HttpServer server;

	server.setRoot(webroot);
	server.bind(port);

#ifdef ASL_TLS

	server.bindTLS(443);

	if (config.has("key"))
	{
		server.useCert(TextFile(config["cert"]).text(), TextFile(config["key"]).text());
	}

#endif

	// a few well known types are already defined
	server.addMimeType("dae", "model/vnd.collada+xml");

	server.start();
}


