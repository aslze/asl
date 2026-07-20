// Copyright(c) 1999-2026 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_PROCESS
#define ASL_PROCESS

#include <asl/String.h>
#include <asl/Map.h>

namespace asl {

class ASL_API Process;

class ASL_API ProcessInfo
{
	friend class Process;
	int    _pid;
	String _path;

public:
	ProcessInfo(int pid = 0) : _pid(pid) {}
	int    pid() const { return _pid; }
	String name() const;
	String path() const { return _path; }
};

/**
A class allowing running subprocesses and communicating with them through stdin/stdout/stderr. The
class allows keeping the subprocess running and writing to its input and reading from its output
as that is produced. This example would run a subprocess, write one line to its input, and
read its output line by line while it runs.

All output reading functions (readOutput(), readOutputLine(), ...) are blocking and will wait until
there is something to read from the process.

~~~
Process proc;
proc.run("program.exe");
proc.writeInput("Hello\n");
while(true) {
	String line = proc.readOutputLine();
	if(line == "\n")
		break;
	printf("Subprocess wrote %s\n", *line);
}
~~~

A **shorthand** function allows executing a program and, after it finishes, getting its full output,
errors and exitcode.

~~~
Process p = Process::execute("ipconfig");
if( p.success() )
	text = p.output();
~~~

In Windows you can append a '\*' to program names (e.g. "notepad.exe*") to show their window if they are Win32 GUI apps. Or to
show their console. Otherwise they run in the background with no window.

__Warning__: If a process is run with the run() method and we will not read its output, the process can hang if it writes
a lot to its output stream (because it will fill a buffer that no one will free). To avoid that, call detach() before calling
run().
*/

class ASL_API Process
{
	int _pid;
	bool _hasExited, _ok, _ready;
	int _exitstat;
#ifdef _WIN32
	typedef HANDLE PipeHandle;
	HANDLE _hProcess;
#else
	typedef int PipeHandle;
#endif
	PipeHandle _pipe_out[2], _pipe_in[2], _pipe_err[2];
	PipeHandle _stdin, _stdout, _stderr;
	String _output, _errors;
	bool _detached;

	static int exec(const String& command, const Array<String>& args = Array<String>(), const Dic<>& env = Dic<>());

public:
	Process();
	Process(const Process& p);
	~Process();

	/**
	Returns this process' ID (PID)
	*/
	int pid() { return _pid; }

	/**
	Reads `n` bytes of the process' *stdout* into a buffer pointed to by `p`
	*/
	int readOutput(void* p, int n);
	/**
	Reads `n` bytes of the process' *stderr* into a buffer pointed to by `p`
	*/
	int readErrors(void* p, int n);
	/**
	Writes `n` bytes into the process' *stdin* from a buffer pointed to by `p`
	*/
	int writeInput(const void* p, int n);
	/**
	Writes a string to the process' *stdin*
	*/
	void writeInput(const String& s) { writeInput(*s, s.length()); }
	/**
	Reads one text line from the process' *stdout* or a "\n" if the process ended
	*/
	String readOutputLine();
	/**
	Indicates that we are not interested in the process' output (must be called this before run()), and the
	subprocess can continue running if the parent process ends.
	*/
	void detach() { ignoreOutput(); }
	
	void ignoreOutput();

	/**
	Returns the number of bytes that can be read from the process' standard output
	*/
	int outputAvailable();

	/**
	Returns the number of bytes that can be read from the process' standard errors
	*/
	int errorsAvailable();

	/**
	Returns the standard output of a subprocess as a string (if executed with Process::execute())
	*/
	const String& output() const { return _output; }

	/**
	Returns the standard errors of a subprocess as a string (if executed with Process::execute())
	*/
	const String& errors() const { return _errors; }
	/**
	Returns the current process identifier (PID)
	*/
	static int myPid();
	/**
	Returns the full path of the executable file of the current process
	*/
	static String myPath();
	/**
	Returns the directory containing the executable file of the current process
	*/
	static String myDir();
	/**
	Returns the full path of the shared library named `lib` (without extension) loaded in the current
	process.
	*/
	static String loadedLibPath(const String& lib);
	static void makeDaemon();
	/**
	Starts executing a program with optional command line arguments and environment variables.
	*/
	void run(const String& command, const Array<String>& args = Array<String>(), const Dic<>& env = Dic<>());

	void run(const String& command, const String& arg1)
	{
		run(command, array<String>(arg1));
	}

	void run(const String& command, const String& arg1, const String& arg2)
	{
		run(command, array<String>(arg1, arg2));
	}

	void run(const String& command, const String& arg1, const String& arg2, const String& arg3)
	{
		run(command, array<String>(arg1, arg2, arg3));
	}

	void run(const String& command, const String& arg1, const String& arg2, const String& arg3, const String& arg4)
	{
		run(command, array<String>(arg1, arg2, arg3, arg4));
	}

	/**
	Tests if the process object and its pipes were created successfully
	*/
	bool ready() const
	{
		return _ready;
	}
	/**
	Returns true if the process executed correctly (exited with zero status)
	*/
	bool success()
	{
		return started() && finished() && exitStatus() == 0;
	}
	/** Tests if the subprocess has started successfully */
	bool started();
	/** Tests if the subprocess has finished */
	bool finished();
	/** Tests if the subprocess has not finished */
	bool running() {return !finished();}
	void signal(int s);
	/** Waits for the subprocess to exit */
	int wait();
	/** Returns the exit code of the process, if finished */
	int exitStatus() { return _exitstat; }

	/**
	Gets the value of an environment variable.
	*/
	static String env(const String& var);
	/**
	Sets the value of an environment variable.
	*/
	static void setEnv(const String& var, const String& value);

	/**
	Returns a dictionary of all environment variables of the current process.
	*/
	static Dic<> environment();
	/**
	Executes `command` with optional arguments and environment vars, and returns the process' data (including output (written to *stdout* and *stderr*);
	Add a '*' at the end of the command name to show the program's window in case of Win32 apps.
	*/
	static Process execute(const String& command, const Array<String>& args = Array<String>(),
	                       const Dic<>& env = Dic<>());

	static Process execute(const String& command, const String& arg1)
	{
		return execute(command, array<String>(arg1));
	}

	static Process execute(const String& command, const String& arg1, const String& arg2)
	{
		return execute(command, array<String>(arg1, arg2));
	}

	static Process execute(const String& command, const String& arg1, const String& arg2, const String& arg3)
	{
		return execute(command, array<String>(arg1, arg2, arg3));
	}

	static Process execute(const String& command, const String& arg1, const String& arg2, const String& arg3, const String& arg4)
	{
		return execute(command, array<String>(arg1, arg2, arg3, arg4));
	}

	/**
	Returns a list of all running processes in the system, with their PID and executable path.
	*/
	static Array<ProcessInfo> list();

	/**
	Kills the process with the given PID. Returns true if the process was killed successfully.
	*/
	static bool kill(int pid);

	/**
	Kills all processes with the given name. Returns the number of processes killed.
	*/
	static int killAll(const String& name);
};

}
#endif
