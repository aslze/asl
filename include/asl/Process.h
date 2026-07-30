// Copyright(c) 1999-2026 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_PROCESS
#define ASL_PROCESS

#include <asl/String.h>
#include <asl/Map.h>
#include <asl/Directory.h>

namespace asl {

#ifdef _WIN32
static const char pathsSeparator = ';';
#else
static const char pathsSeparator = ':';
#endif

class ASL_API Process;

/**
 * A process environment variables that works in a case-insensitive way on Windows and case-sensitive on other platforms.
 */
class ASL_API ProcEnv
{
	Dic<> _env;
	Dic<> _case;

public:
	/**
	 * Returns the value of the environment variable `key`
	 */
	const String& operator[](const String& key) const;
	/**
	 * Sets the value of the environment variable `key` to `value`
	 */
	void set(const String& key, const String& value);
	/**
	 * Removes the environment variable `key` from this environment
	 */
	void remove(const String& key);
	/**
	 * Returns the number of environment variables in this environment
	 */
	int length() const { return _env.length(); }
	/**
	 * Returns true if this environment has no variables
	 */
	bool                      empty() const { return length() == 0; }
	const Dic<>&              map() const { return _env; }
	typedef Dic<>::Enumerator Enumerator;
	Enumerator                all() const { return _env.all(); }
};

#ifdef ASL_HAVE_RANGEFOR

inline Array<Map<>::KeyVal>::Enumerator begin(const ProcEnv& a)
{
	return a.map().kv().all();
}

inline Array<Map<>::KeyVal>::Enumerator end(const ProcEnv& a)
{
	return a.map().kv().all();
}
#endif

/**
Process information, including its PID and the path of its executable file, returned by Process::list().

```
auto procs = Process::list();
for (const auto& info : procs)
    printf("Process %i: %s\n", info.pid(), *info.name());
```
*/
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
Process start parameters, including environment variables and working directory. The `detached` flag indicates that we are not interested
in the process output and the process can continue running even if the parent process ends (must be set before run()).
The `showWindow` flag on Windows indicates that the process should show its window (if it is a GUI application) or
console (if it is a console application). The `env` field contains the environment variables to be used by the subprocess,
and the `workingDir` field contains the directory where the process will start.
*/
struct ProcParams
{
	bool detached;
	bool showWindow;
	ProcEnv env;
	String  workingDir;
	ProcParams() : detached(false), showWindow(false) {}
	ProcParams(const ProcEnv& e, const String& wd = String()) : detached(false), showWindow(false), env(e), workingDir(wd) {}
	ProcParams(const String& wd, const ProcEnv& e = ProcEnv()) : detached(false), showWindow(false), env(e), workingDir(wd) {}
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

In Windows you can append a '\*' to program names (e.g. "notepad.exe*") or set `showWindow` in the process parameters to show their window
if they are Win32 GUI apps. Or to show their console. Otherwise they run in the background with no window.

__Warning__: If a process is run with the run() method and we will not read its output, the process can hang if it writes
a lot to its output stream (because it will fill a buffer that no one will free). To avoid that, call detach() before calling
run().

You can set the environment variables to be used by the subprocess with setSubprocessEnvironment() before calling run(),
and set the directory where the process will start with setStartDirectory().
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
	ProcParams _params;

	static int exec(const String& command, const Array<String>& args = Array<String>(), const ProcEnv& env = ProcEnv());

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
	void detach();
	
	ASL_DEPRECATED(void ignoreOutput(), "Use .detach()") { detach(); }
	
	/**
	Sets the starting directory of the new process
	*/
	void setStartDirectory(const String& dir) { _params.workingDir = dir; }

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
	void run(const String& command, const Array<String>& args = Array<String>());

	void run(const String& command, const String& arg1)
	{
		run(command, array<String>(arg1));
	}

	ASL_DEPRECATED(void run(const String& command, const String& arg1, const String& arg2), "Pass arguments as array")
	{
		run(command, array<String>(arg1, arg2));
	}

	ASL_DEPRECATED(void run(const String& command, const String& arg1, const String& arg2, const String& arg3), "Pass arguments as array")
	{
		run(command, array<String>(arg1, arg2, arg3));
	}

	ASL_DEPRECATED(void run(const String& command, const String& arg1, const String& arg2, const String& arg3, const String& arg4), "Pass arguments as array")
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
	Sets the parameters to be used by the subprocess when run. This includes environment variables, working directory, and
	other options.
	*/
	void use(const ProcParams& params) { _params = params; }
	/**
	Sets the environment variables to be used by the subprocess when run
	*/
	void setSubprocessEnvironment(const ProcEnv& env) { _params.env = env; }

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
	static ProcEnv environment();
	/**
	Executes `command` with optional arguments, environment vars and a start directory, and returns the process'
	data (including output (written to *stdout* and *stderr*);
	*/
	static Process execute(const String& command, const Array<String>& args = Array<String>(),
	                       const ProcParams& env = ProcParams());

	static Process execute(const String& command, const String& arg1)
	{
		return execute(command, array<String>(arg1));
	}

	static ASL_DEPRECATED(Process execute(const String& command, const String& arg1, const String& arg2),
	                      "Pass arguments as array")
	{
		return execute(command, array<String>(arg1, arg2));
	}

	static ASL_DEPRECATED(Process execute(const String& command, const String& arg1, const String& arg2, const String& arg3),
	                      "Pass arguments as array")
	{
		return execute(command, array<String>(arg1, arg2, arg3));
	}

	static ASL_DEPRECATED(Process execute(const String& command, const String& arg1, const String& arg2,
	                                      const String& arg3, const String& arg4),
	                      "Pass arguments as array")
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
