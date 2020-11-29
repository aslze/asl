#include <asl/Process.h>
#include <stdio.h>
#include <stdlib.h>
#include <asl/Path.h>

namespace asl {

Process Process::execute(const String& command, const Array<String>& args)
{
	Process p;
	p.run(command, args);
	int n, i = 0;
	char buffer[8000];
	while (p.running())
	{
		sleep((i++)%16 == 0 ? 0.001 : 0);
		if (p.outputAvailable() > 0)
		{
			n = p.readOutput(buffer, sizeof(buffer));
			p._output.append(buffer, n);
		}
		if (p.errorsAvailable() > 0)
		{
			n = p.readErrors(buffer, sizeof(buffer));
			p._errors.append(buffer, n);
		}
	}
	while (n = p.readOutput(buffer, sizeof(buffer)), n > 0)
		p._output.append(buffer, n);
	while (n = p.readErrors(buffer, sizeof(buffer)), n > 0)
		p._errors.append(buffer, n);
	p._exitstat = p.exitStatus();
	return p;
}

Process::Process(const Process& p) : _output(p._output), _errors(p._errors)
{
	_exitstat = p._exitstat;
	_pid = p._pid;
	_hasExited = p._hasExited;
	_ok = p._hasExited;
	_ready = false;
	_detached = p._detached;

	_stdin = _stdout = _stderr = 0;
	_pipe_in[0] = _pipe_out[0] = _pipe_err[0] = 0;
	_pipe_in[1] = _pipe_out[1] = _pipe_err[1] = 0;
}

String Process::readOutputLine()
{
	char c;
	String line;
	int n;
	while ((n = readOutput(&c, 1)) > 0)
	{
		if (c == '\n') {
			if (line[line.length() - 1] == '\r')
				line.resize(line.length() - 1);
			break;
		}
		line << c;
	}
	if (n <= 0 && line.length() == 0) {
		line = '\n';
	}
	return line;
}

String Process::myDir()
{
	return Path(myPath()).directory().string();
}

void Process::ignoreOutput()
{
	_detached = true;
}

}

#ifdef _WIN32

#include <asl/Process.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

namespace asl {

	static String joinCmdArgs(const Array<String> args)
	{
		String cmdline;
		foreach(const String & arg, args)
		{
			cmdline << '"';
			for (int i = 0; i < arg.length(); i++)
			{
				int nbs = 0;
				while (i < arg.length() && arg[i] == '\\')
				{
					++i;
					++nbs;
				}
				if (i == arg.length())
					cmdline << String::repeat('\\', 2 * nbs);
				else
					cmdline << String::repeat('\\', (arg[i] == '"') ? 2 * nbs + 1 : nbs) << arg[i];
			}
			cmdline << "\" ";
		}

		return cmdline;
	}

	String Process::env(const String& var)
	{
		String value;
		DWORD len = 15;
		do {
			value.resize(len, false);
			len = GetEnvironmentVariableA(var, value, value.length() + 1);
			if (!len)
				value = "";
		} while ((int)len > value.length() + 1);
		value.fix(len);
		return value;
	}

	void Process::setEnv(const String& var, const String& value)
	{
		SetEnvironmentVariableA(var, value);
	}

	Process::Process()
	{
		_ok = false;
		_ready = false;
		_exitstat = 0;
		_pid = -1;
		_detached = false;
		_hasExited = false;
		_stderr = _stdin = _stdout = 0;

		SECURITY_ATTRIBUTES sec;
		sec.nLength = sizeof(SECURITY_ATTRIBUTES);
		sec.bInheritHandle = TRUE;
		sec.lpSecurityDescriptor = NULL;

		if (!CreatePipe(&_pipe_out[0], &_pipe_out[1], &sec, 5000))
			return;
		if (!CreatePipe(&_pipe_in[0], &_pipe_in[1], &sec, 5000))
			return;
		if (!CreatePipe(&_pipe_err[0], &_pipe_err[1], &sec, 5000))
			return;
		_ready = true;
	}

	Process::~Process()
	{
		if (_ready) {
			if (!_ok)
			{
				CloseHandle(_pipe_in[0]);
				CloseHandle(_pipe_out[1]);
				CloseHandle(_pipe_err[1]);
			}
			CloseHandle(_pipe_in[1]);
			CloseHandle(_pipe_out[0]);
			CloseHandle(_pipe_err[0]);
		}
	}

	int Process::myPid()
	{
		return (int)GetCurrentProcessId();
	}

	String Process::myPath()
	{
		String path;
		GetModuleFileNameW(NULL, SafeString(path, 1000), 1000);
		return path;
	}

	String Process::loadedLibPath(const String& lib)
	{
		String path;
		HMODULE handle = GetModuleHandleW(lib);
		if(handle == 0 && (handle = GetModuleHandleW("lib" + lib)) == 0)
			return path;
		GetModuleFileNameW(handle, SafeString(path, 1000), 1000);
		return path;
	}

	void Process::makeDaemon()
	{
		// does this make sense on Windows?
	}

	void Process::run(const String& command, const Array<String>& args)
	{
		if (!_ready)
			return;

		_hasExited = false;
		STARTUPINFOW startInfo;
		ZeroMemory(&startInfo, sizeof(startInfo));
		startInfo.cb = sizeof(startInfo);
		startInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startInfo.wShowWindow = SW_HIDE;
		startInfo.hStdInput = !_detached ? _pipe_in[0] : 0;
		startInfo.hStdOutput = !_detached ? _pipe_out[1] : 0;
		startInfo.hStdError = !_detached ? _pipe_err[1] : 0;

		String cmd = command;
		if (command.endsWith('*')) {
			startInfo.wShowWindow = SW_SHOWNORMAL;
			cmd = cmd.substr(0, cmd.length() - 1);
		}
	
		String commandline;
		commandline << '"' << cmd << "\" " << joinCmdArgs(args);

		PROCESS_INFORMATION procInfo;
		_ok = CreateProcessW(NULL,
			commandline,            // application name
			NULL,                   // process security attributes
			NULL,                   // primary thread security attributes
			TRUE,                   // handles are inherited
			CREATE_NEW_CONSOLE,     // creation flags DETACHED_PROCESS
			NULL,                   // use parent's environment
			NULL,
			&startInfo,
			&procInfo) != 0;

		if (_ok)
		{
			_pid = procInfo.dwProcessId;
			CloseHandle(procInfo.hProcess);
			CloseHandle(procInfo.hThread);
		}
		else
		{
			_pid = -1;
			return;
		}
		CloseHandle(_pipe_in[0]);
		CloseHandle(_pipe_out[1]);
		CloseHandle(_pipe_err[1]);
		_stdout = _pipe_out[0];
		_stderr = _pipe_err[0];
		_stdin = _pipe_in[1];
	}

	int Process::outputAvailable()
	{
		if (!_ready)
			return 0;
		DWORD n;
		return PeekNamedPipe(_stdout, 0, 0, 0, &n, 0) ? n : 0;
	}

	int Process::errorsAvailable()
	{
		if (!_ready)
			return 0;
		DWORD n;
		return PeekNamedPipe(_stderr, 0, 0, 0, &n, 0) ? n : 0;
	}

	int Process::readOutput(void* p, int n)
	{
		if(!_ready)
			return 0;
		DWORD read;
		int _ok = ReadFile(_stdout, p, n, &read, NULL);
		return _ok? read : -1;
	}

	int Process::readErrors(void* p, int n)
	{
		if(!_ready)
			return 0;
		DWORD  read;
		int _ok = ReadFile(_stderr, p, n, &read, NULL);
		return _ok? read : -1;
	}

	int Process::writeInput(const void* p, int n)
	{
		if(!_ready)
			return 0;
		DWORD written;
		WriteFile(_stdin, p, n, &written, NULL);
		return written;
	}

	void Process::signal(int s)
	{
		//kill(_pid, s);
	}

	int Process::wait()
	{
		if(!_ready)
			return 0;
		HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _pid);
		WaitForSingleObject(process, INFINITE);
		DWORD exitCode;
		GetExitCodeProcess(process, &exitCode);
		CloseHandle(process);
		_hasExited = true;
		_exitstat = exitCode;
		return _exitstat;
	}

	bool Process::finished()
	{
		if(!_ready)
			return true;

		if (_pid != -1)
		{
			DWORD exitCode = 1;
			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _pid);

			if (!process || (process && GetExitCodeProcess(process, &exitCode)))
			{
				_exitstat = exitCode;
			}
			_hasExited = (exitCode!=STILL_ACTIVE);
			CloseHandle(process);
		}
		return _hasExited;
	}

	bool Process::started()
	{
		return _pid != -1;
	}

}

#else
//#include <asl/File.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#include <stdio.h>
//#include <dlfcn.h>
#include <stdint.h>
#endif
#if __FreeBSD__
#include <sys/sysctl.h>
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined (__bsdi__) || defined (__DragonFly__)
#define ASL_OS_BSD
#endif

namespace asl {

String Process::env(const String& var)
{
	const char* value = getenv(var);
	return String(value? value : "");
}

void Process::setEnv(const String& var, const String& value)
{
	setenv(var, value, 1);
}

Process::Process()
{
	_hasExited = false;
	_ready = false;
	_pid = -1;
	_detached = false;
	if(pipe(_pipe_in)==-1 || pipe(_pipe_out)==-1 || pipe(_pipe_err)==-1)
	{
		return;
	}
	_stdin = _pipe_in[1];
	_stdout = _pipe_out[0];
	_stderr = _pipe_err[0];
	_ready = true;
}

Process::~Process()
{
	if (_ready) {
		close(_pipe_in[0]);
		close(_pipe_out[0]);
		close(_pipe_err[0]);
		close(_pipe_in[1]);
		close(_pipe_out[1]);
		close(_pipe_err[1]);
	}
}

int Process::myPid()
{
	return (int)getpid();
}

String Process::myPath()
{
#ifdef __APPLE__
	char buffer[PATH_MAX];
	char buffer2[PATH_MAX];
	char* path = buffer;
	uint32_t size = (uint32_t)sizeof(buffer);
	if (_NSGetExecutablePath(path, &size) < 0)
		return "";
	char* resolved = realpath(path, buffer2);
	return buffer2;
#elif defined(__FreeBSD__)
	int mib[4];
	size_t len = 255;
	char path[255];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	sysctl(mib, 4, path, &len, NULL, 0);
	return path;
#else
	char buffer[1024];
	int n = readlink("/proc/self/exe", buffer, 1023); // linux
	if(n == -1)
		n = readlink("/proc/curproc/exe", buffer, 1023); // netbsd
	if(n == -1)
		n = readlink("/proc/curproc/file", buffer, 1023); // freebsd, old openbsd,
	if (n != -1) 
	{
		buffer[n] = '\0';
		return String(buffer);
	}
	return "";
#endif
}

String Process::loadedLibPath(const String& lib)
{
	//TextFile mapfile("/proc/self/maps", File::READ);
	FILE* mapfile = fopen("/proc/self/maps", "rt"); // linux
	if(!mapfile)
		mapfile = fopen("/proc/curproc/map", "rt"); // freebsd
	if(!mapfile)
		return "";
	
	String name1 = '/' + lib + ".so";
	String name2 = "/lib" + lib + ".so";
	String line(1000, 1000);

	while(!feof(mapfile))
	{
		//String line = mapfile.readLine();
		char* s = fgets(SafeString(line), 1000, mapfile);
		if(!s) break;
		Array<String> parts = line.split();
		for(int i=0; i<parts.length(); i++)
		if(parts[i].contains(name1) || parts[i].contains(name2))
		{
			fclose(mapfile);
			return parts[i];
		}
	}
	fclose(mapfile);
	return "";
}

void Process::makeDaemon()
{
	int _pid = fork();
	switch (_pid)
	{
	case -1: // error
		break;

	case 0: // child
        break;

	default: // parent
		_exit(0); // child inherited by init process
	}
}

void Process::run(const String& command, const Array<String>& args)
{
	if(!_ready)
		return;
	_hasExited=false;
	switch (_pid = fork())
	{
	case -1: // error
		_ready=false;
		break;

	case 0: // child
		if(!_detached) {
			dup2(_pipe_err[1], 2);
			dup2(_pipe_out[1], 1);
			dup2(_pipe_in[0], 0);
			close(_pipe_in[1]);
			close(_pipe_out[0]);
			close(_pipe_err[0]);
		}
		else {
			close(1);
			close(2);
			close(0);
			//::signal (SIGHUP, SIG_IGN);
			pid_t sid = setsid();
		}
		exec(command, args);
		_exit(0);
        break;

	default: // parent
		if(!_detached) {
			close(_pipe_in[0]);
			close(_pipe_out[1]);
			close(_pipe_err[1]);
		}
	}
}

int Process::outputAvailable()
{
	if (!_ready)
		return 0;
	long n;
	return (ioctl(_stdout, FIONREAD, &n) == 0) ? (int)n : 0;
}

int Process::errorsAvailable()
{
	if (!_ready)
		return 0;
	long n;
	return (ioctl(_stderr, FIONREAD, &n) == 0) ? (int)n : 0;
}

int Process::readOutput(void* p, int n)
{
	if(!_ready)
		return 0;
	return read(_stdout, p, n);
}

int Process::readErrors(void* p, int n)
{
	if(!_ready)
		return 0;
	return read(_stderr, p, n);
}

int Process::writeInput(const void* p, int n)
{
	if(!_ready)
		return 0;
	return write(_stdin, p, n);
}

void Process::signal(int s)
{
	kill(_pid, s);
}

int Process::wait()
{
	if(!_ready)
		return 0;
	int stat;
	if(_hasExited)
		return 0;
	if(waitpid(_pid, &stat, 0)<=0)
	{
		_ready = false;
		return 0;
	}
	_hasExited = true;
	_exitstat = WEXITSTATUS(stat);
	return _exitstat;
}

bool Process::finished()
{
	if(!_ready)
		return true;
	int stat, p;
	if(_hasExited)
		return true;
	if((p = waitpid(_pid, &stat, WNOHANG))<0)
		return true;
	if(p == 0)
		return false;
	bool end = WIFEXITED(stat);
	if(end)
		_exitstat = WEXITSTATUS(stat);
	_hasExited=true;
	return true;
}

int Process::exec(const String& command, const Array<String>& args)
{
	Array<const char*> argv;
	argv << command;
	for(int i=0; i<args.length(); i++)
		argv << args[i];
	argv << 0;
	return execv(command, (char* const*)argv.ptr());
}

bool Process::started()
{
	return !finished()? (_pid != -1) : (_pid != -1 && _exitstat != 126 && _exitstat != 127);
}

}

#endif

