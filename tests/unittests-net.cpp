#include <asl/Http.h>
#include <asl/HttpServer.h>
#include <asl/testing.h>
#include <stdio.h>

using namespace asl;

#ifdef ASL_TLS
ASL_TEST(HTTPS)
{
	HttpResponse res = Http::get("https://7-zip.org/");
	ASL_CHECK(res.code(), ==, 200);
}
#endif

class AslServer : public HttpServer
{
public:
	AslServer()
	{
		setRoot(".");
	}
	void serve(HttpRequest& request, HttpResponse& response)
	{
		if (request.is("GET", "/"))
		{
			String name = request.query("name");
			response.put("Hello "+ name +" from AslServer!");
		}
		else if (request.is("POST", "/post"))
		{
			String body = request.text();
			response.put("Received: " + body);
		}
		else
		{
			response.put("Not found");
			response.setCode(404);
		}
	}
};

ASL_TEST(HTTP)
{
	AslServer server;
	server.bind("127.0.0.1", 9001);
	server.start(true);

	sleep(0.2);
	HttpResponse res = Http::get("http://127.0.0.1:9001/?name=World");
	ASL_CHECK(res.code(), ==, 200);
	ASL_CHECK(res.text(), ==, "Hello World from AslServer!");

	res = Http::post("http://127.0.0.1:9001/post", "some-data");
	ASL_ASSERT(res.ok());
	ASL_CHECK(res.text(), ==, "Received: some-data");

	res = Http::get("http://127.0.0.1:9001/what");
	ASL_CHECK(res.code(), ==, 404);

	server.stop();
}
