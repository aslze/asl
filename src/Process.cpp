#include <asl/Process.h>
#include <stdio.h>
#include <stdlib.h>
#include <asl/Path.h>

namespace asl {

Process Process::execute(const String& command, const Array<String>& args)
{
	Process p;
	p.run(command, args);
	int n;
	char buffer[1000];
	while (n = p.readOutput(buffer, sizeof(buffer)), n>0)
	{
		p._output.append(buffer, n);
	}
	while (n = p.readErrors(buffer, sizeof(buffer)), n>0)
	{
		p._errors.append(buffer, n);
	}
	p._exitstat = p.wait();

	return p;
}

Process::Process(const Process& p) : _output(p._output), _errors(p._errors)
{
	_exitstat = p._exitstat;
	_pid = p._pid;
	_hasExited = p._hasExited;
	_ok = p._hasExited;
	_ready = false;

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
			CloseHandle(_pipe_in[0]);
			CloseHandle(_pipe_out[0]);
			CloseHandle(_pipe_err[0]);
			CloseHandle(_pipe_in[1]);
			CloseHandle(_pipe_out[1]);
			CloseHandle(_pipe_err[1]);
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
		// tiene esto sentido en Windows?
	}

	void Process::run(const String& command, const Array<String>& args)
	{
		if(!_ready)
			return;

		_hasExited=false;
		STARTUPINFO startInfo;
		ZeroMemory(&startInfo, sizeof(STARTUPINFO));
		startInfo.cb = sizeof(STARTUPINFO);
		startInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startInfo.wShowWindow = SW_HIDE;
		startInfo.hStdInput = !_detached ? _pipe_in[0] : 0;
		startInfo.hStdOutput = !_detached ? _pipe_out[1] : 0;
		startInfo.hStdError = !_detached ? _pipe_err[1] : 0;

		String commandline;
		commandline << '\"' << command << '\"';
		foreach(const String& arg, args)
			commandline << " \"" << arg.replace('\"', "\\\"") << "\"";

		PROCESS_INFORMATION procInfo;
		_ok = CreateProcess(NULL,
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
			_pid = procInfo.dwProcessId;
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
#include <asl/File.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
//#include <stdlib.h>
//#include <dlfcn.h>
#include <stdint.h>
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
#else
	//String link(15, "/proc/%i/exe", (int)getpid());
	String link = "/proc/self/exe";
	char buffer[1024];
	int n = readlink(link, buffer, 1023);
	if (n != -1) 
	{
		buffer[n] = '\0';
		return buffer;
	}
	return "";
#endif
}

String Process::loadedLibPath(const String& lib)
{
	TextFile mapfile(String(15, "/proc/%i/maps", (int)getpid()), File::READ);
	if(!mapfile)
		return "";
	
	String name1 = '/' + lib + ".so";
	String name2 = "/lib" + lib + ".so";

	while(!mapfile.end())
	{
		String line = mapfile.readLine();
		Array<String> parts = line.split();
		if(parts.last().contains(name1) || parts.last().contains(name2))
			return parts.last();
	}
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

