#include <asl/StackTrace.h>
#include <asl/Process.h>
#include <asl/Path.h>

namespace asl
{
asl::Function<void> StackTrace::_onCrash;
asl::String         StackTrace::_message;
}

#if ((defined _WIN32 && defined _MSC_VER) || defined __linux__ || defined __APPLE__) && !defined(__ANDROID__)

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
	HANDLE thread = GetCurrentThread();
	if (!SymInitialize(process, NULL, TRUE))
	{
		_message = String::f("Fatal exception: 0x%08X\n", ep->ExceptionRecord->ExceptionCode);
		return EXCEPTION_EXECUTE_HANDLER;
	}

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

	_message.resize(3000);
	_message.clear();
	_message << String::f("Fatal exception: 0x%08X\n", ep->ExceptionRecord->ExceptionCode);

	for (int i = 0; i < 100; i++)
	{
		if (!StackWalk64(machine, process, thread, &frame, ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
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
				_message << String::f("%s() at %s:%lu\n", symbol->Name, *Path(line.FileName).name(), line.LineNumber);
			}
			else
			{
				_message << String::f("%s()\n", symbol->Name);
			}
		}
		else
		{
			_message << String::f("[0x%llx]\n", (ULong)addr);
		}
	}

	SymCleanup(process);

	if (_onCrash)
		_onCrash();

	return EXCEPTION_EXECUTE_HANDLER;
}
}

#else

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <dlfcn.h>
#endif

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
	String path = Process::myPath();
	args << "-o" << path << "-l" << String::f("%p", baseaddr);
	for (int i = 1; i < n; i++)
	{
		args << String::f("%p", frames[i]);
	}

	Process out = Process::execute("atos", args);
	if (out.success() && out.output())
	{
		_message << out.output();
	}
	else
	{
		path = path + ".dSYM/Contents/Resources/DWARF/" + Path(path).name();
		args[1] = path;
		Process out = Process::execute("atos", args);
		if (out.success() && out.output())
		{
			_message << out.output();
		}
	}
#else
	args << "-e" << Process::myPath() << "-f" << "-p" << "-C"; 
	for (int i = 1; i < n; i++)
	{
		args << String::f("%p", frames[i]);
	}

	Process out = Process::execute("addr2line", args);
	if (out.exitStatus() == 0)
	{
		_message << out.output();
	}
	else if (out.exitStatus() == 1) // try without -p for older versions of addr2line
	{
		args.remove(3);
		Process out = Process::execute("addr2line", args);
		_message << out.output();
	}

#endif
	if (_message.length() < 17)
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

#else // Unsupported OS/compiler

namespace asl
{
void StackTrace::onCrash(asl::Function<void> f)
{
	_onCrash = f;
	printf("StackTrace::onCrash() is not supported for this OS/compiler\n");
}
}

#endif
