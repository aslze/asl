// Copyright(c) 1999-2026 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_STACKTRACE_H
#define ASL_STACKTRACE_H

#include <asl/String.h>
#include <asl/util.h>

namespace asl
{

class ASL_API StackTrace
{
public:
	static void onCrash(asl::Function<void> f);
	static asl::String message() { return _message; }

private:
	static asl::Function<void> _onCrash;
	static asl::String         _message;

#ifdef _WIN32
	static LONG WINAPI crashHandler(EXCEPTION_POINTERS* ep);
#else
	static void segv_handler(int sig);
#endif
};

}

#endif
