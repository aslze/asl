#include <asl/StreamBuffer.h>
#include <asl/Socket.h>
#include <asl/Http.h>
#include <stdio.h>
#include <asl/testing.h>

ASL_TEST(HTTP)
{
#ifdef ASL_TLS
	asl::HttpResponse res = asl::Http::get("https://7-zip.org/");
	ASL_CHECK(res.code(), ==, 200);
#else
	ASL_ASSERT(true);
#endif
}
