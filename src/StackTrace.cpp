#include <asl/StackTrace.h>
#include <asl/Process.h>

namespace asl
{
asl::Function<void> StackTrace::_onCrash;
asl::String         StackTrace::_message;
}

#ifdef _WIN32
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

namespace asl
{

void StackTrace::onCrash(asl::Function<void> f)
{
	_onCrash = f;
	SetUnhandledExceptionFilter(crashHandler);
}

LONG StackTrace::crashHandler(EXCEPTION_POINTERS* ep)
{
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);

	CONTEXT* ctx = ep->ContextRecord;

#ifdef _M_X64
	DWORD        machine = IMAGE_FILE_MACHINE_AMD64;
	STACKFRAME64 frame = {};
	frame.AddrPC.Offset = ctx->Rip;
	frame.AddrFrame.Offset = ctx->Rbp;
	frame.AddrStack.Offset = ctx->Rsp;
#else
	DWORD        machine = IMAGE_FILE_MACHINE_I386;
	STACKFRAME64 frame = {};
	frame.AddrPC.Offset = ctx->Eip;
	frame.AddrFrame.Offset = ctx->Ebp;
	frame.AddrStack.Offset = ctx->Esp;
#endif
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;

	_message.resize(64 * 64);
	_message.clear();

	_message << String::f("Fatal exception: 0x%08X\n", ep->ExceptionRecord->ExceptionCode);

	for (int i = 0; i < 100; ++i)
	{
		if (!StackWalk64(machine, process, GetCurrentThread(), &frame, ctx, NULL, SymFunctionTableAccess64,
		                 SymGetModuleBase64, NULL))
			break;

		DWORD64 addr = frame.AddrPC.Offset;
		if (!addr)
			break;

		char         buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
		SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		DWORD64 displacement = 0;

		if (SymFromAddr(process, addr, &displacement, symbol))
		{
			IMAGEHLP_LINE64 line = {};
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			DWORD lineDisp = 0;

			if (SymGetLineFromAddr64(process, addr, &lineDisp, &line))
			{
				_message << String::f("%s() [%s:%lu]\n", symbol->Name, line.FileName, line.LineNumber);
			}
			else
			{
				_message << String::f("%s() + 0x%llx [0x%llx]\n", symbol->Name, (unsigned long long)displacement,
				                      (unsigned long long)addr);
			}
		}
		else
		{
			_message << String::f("[0x%llx]\n", (unsigned long long)addr);
		}
	}

	SymCleanup(process);

	if (_onCrash)
		_onCrash();

	return EXCEPTION_EXECUTE_HANDLER;
}
}

#elif defined(__APPLE__)

namespace asl
{

}

#else

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

namespace asl
{

void StackTrace::onCrash(Function<void> f)
{
	_onCrash = f;
	struct sigaction sa = { 0 };
	sa.sa_handler = segv_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESETHAND;
	sigaction(SIGTRAP, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
}

void StackTrace::segv_handler(int sig)
{
	void* frames[64];
	int   n = backtrace(frames, 64);

	_message = "Fatal exception\n";

	Array<String> args;

#if defined(__APPLE__)
	Dl_info info;
	dladdr((void*)&onCrash, &info);
	void* baseaddr = info.dli_fbase;
	args << "-o" << Process::myPath() << "-l" << String::f("%p", baseaddr);
	for (size_t i = 1; i < n; i++)
	{
		args << String::f("%p", frames[i]);
	}

	Process out = Process::execute("atos", args);
	if (out.exitStatus() == 0 && out.output())
	{
		_message << out.output();
	}
#else
	args << "-e" << Process::myPath() << "-f" << "-p" << "-C";
	for (size_t i = 1; i < n; i++)
	{
		args << String::f("%p", frames[i]);
	}

	Process out = Process::execute("addr2line", args);
	if (out.exitStatus() == 0 && out.output())
	{
		_message << out.output();
	}
#endif
	if (_message.length() < 20)
	{
		char** syms = backtrace_symbols(frames, n);
		for (int i = 1; i < n; i++)
		{
			_message << syms[i] << '\n';
		}

		free(syms);
	}

	if (_onCrash)
		_onCrash();

	_exit(128 + sig);
}
}
#endif
