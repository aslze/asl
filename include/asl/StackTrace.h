// Copyright(c) 1999-2026 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_STACKTRACE_H
#define ASL_STACKTRACE_H

#include <asl/String.h>
#include <asl/util.h>

namespace asl
{

/**
 * Functionality to capture call stack traces on crashes. It allows setting a callback function that will be called when a crash
 * occurs, and provides a message with the stack trace information. Supports Windows, Linux, and macOS.
 * Programs must be compiled with debug info (Debug or RelWithDebInfo) to get function names, file names, and line numbers.
 * 
 * ```
 * StackTrace::onCrash([]() {
 *    ASL_LOG_E("%s", *StackTrace::message());
 * });
 * ```
 * 
 * The message is platform-dependent and may include function names, file names, and line numbers if available.
 * On Linux, it calls `addr2line` if available to get file and line information if the binary has debug symbols.
 * On macOS, it calls `atos` if available to get file and line information if the binary has debug symbols.
 */
class ASL_API StackTrace
{
public:
	/**
	 * Sets a callback function to be called when a crash occurs.
	 */
	static void onCrash(asl::Function<void> f);
	/**
	 * Returns the message containing the stack trace information after a crash has occurred.
	 */
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
