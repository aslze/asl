// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_THREAD_H
#define ASL_THREAD_H

#include <asl/defs.h>
#include <asl/time.h>
#include <asl/Array.h>
#include "Mutex.h"

#ifdef _WIN32
#include <process.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>

namespace asl {

class ThreadAttrib
{
	pthread_attr_t _a;
public:
	ThreadAttrib() {pthread_attr_init(&_a);}
	ThreadAttrib& stackSize(int s)
	{
		pthread_attr_setstacksize(&_a, (size_t)s);
		return *this;
	}
	int stackSize()
	{
		int s;
		pthread_attr_getstacksize(&_a, (size_t*)&s);
		return s;
	}
	pthread_attr_t* operator*()
	{
		return &_a;
	}
};
}
#endif

#if defined(ASL_HAVE_LAMBDA) && !(defined(_MSC_VER) && _MSC_VER <= 1600)
#define ASL_EXP_THREADING
#endif

/**
\defgroup Threading Threading
*/

namespace asl {

class ThreadAttrib;
template<class T>
class ThreadGroup;
class Thread;

/**
The Thread class represents an execution thread. To create threads, derive a class from Thread and reimplement
the `run()` function. That is what objects of the new class will execute in parallel when the `start()` function is called.

~~~
struct Worker : public Thread
{
	void run()
	{
		... // what the function will do in a parallel thread
	}
};

Worker worker;
worker.start();
...
worker.join();
~~~

In compilers supporting *lambdas* threads can now be started with a lambda without defining a
specific Thread subclass. These threads can get input data or keep state using variables captured
by the lambda.

~~~
Thread thread([=]() {
	... // what the thread does
});
...
thread.join();
~~~

See also Mutex, Semaphore and Lock.
\ingroup Threading
*/
class Thread
{
#ifdef _WIN32
	#define ASL_THREADFUNC_API __stdcall
	#define ASL_THREADFUNC_RET unsigned int
	typedef unsigned(__stdcall *Function)(void*);
	typedef HANDLE Handle;
#else
	#define ASL_THREADFUNC_API
	#define ASL_THREADFUNC_RET void*
	typedef void* (*Function)(void*);
	typedef pthread_t Handle;
#endif
	Handle _thread;
private:
	struct Context {
		void* f;
		Thread* t;
		Handle h;
		int i;
	};

	void run(Function f, void* arg)
	{
#ifdef _WIN32
		if (_thread != 0)
			CloseHandle(_thread);
		unsigned int n;
		if (!(_thread = (HANDLE)_beginthreadex(NULL, 0, f, arg, 0, &n)))
		{
			ASL_BAD_ALLOC();
		}
	}
#else
		int n;
		if((n = pthread_create(&_thread, 0, f, arg)))
		{
			ASL_BAD_ALLOC();
		}
	}
	void run(Function f, ThreadAttrib& a , void* arg=0)
	{
		int n;
		if((n = pthread_create(&_thread, *a, f, arg)))
		{
			ASL_BAD_ALLOC();
		}
	}
#endif
	static ASL_THREADFUNC_RET ASL_THREADFUNC_API begin(void* p)
	{
		Thread* t = (Thread*)p;
		t->run();
		return 0;
	}
#ifdef ASL_EXP_THREADING
	template<class Func>
	static void ASL_THREADFUNC_API beginf(void* f)
	{
		(*((Func*)f))();
	}
	template<class Func>
	static void ASL_THREADFUNC_API beginf1(void* p)
	{
		if(!p) return;
		Context* s = (Context*)p;
		(*((Func*)s->f))(s->i);
	}
#endif
public:
	Thread()
	{
		_thread = 0;
	}
	Thread(const Thread& t) : _thread(t._thread)
	{
		const_cast<Thread&>(t)._thread = 0;
	}
	void operator=(const Thread& t)
	{
		_thread = t._thread;
		const_cast<Thread&>(t)._thread = 0;
	}
	virtual ~Thread()
	{
		if (_thread != 0) {
#ifdef _WIN32
			CloseHandle(_thread);
#else
			pthread_detach(_thread);
#endif
		}
	}
	/** The thread procedure. Reimplement this function to create new threads */
	virtual void run()
	{}
	/** Starts a new thread by calling run() in parallel */
	void start()
	{
		run(Thread::begin, this);
	}
	/** Cancels a thread */
	void kill()
	{
#ifdef _WIN32
		TerminateThread(_thread, 0);
#elif !defined(__ANDROID__)
		pthread_cancel(_thread);
#endif
	}
	/** Waits for this thread to end */
	void join()
	{
#ifdef _WIN32
		WaitForSingleObject(_thread, INFINITE);
#else
		void* ret;
		pthread_join(_thread, &ret);
		_thread = 0;
#endif
	}
	/**
	Returns the number of logical processors or cores
	*/
	static int numProcessors()
	{
#ifdef _WIN32
		SYSTEM_INFO info;
		GetSystemInfo( &info );
		return info.dwNumberOfProcessors;
#else
		return sysconf(_SC_NPROCESSORS_ONLN);
#endif
	}
#ifdef ASL_EXP_THREADING
	/**
	Starts a thread given a function or lambda expression as the run() function.
	*/
	template<class F>
	Thread(const F& f)
	{
		*this = start(f);
	}
	template<class Func>
	static Thread start(const Func& f)
	{
		Thread t;
		t.run((Function)Thread::beginf<Func>, (void*)&f);
		return t;
	}
	/**
	Emulates an OpenMP *parallel for* by running function `f` several times in parallel. Function `f` must
	receive an integer argument which will get the values from `i0` up to but not including `i1`.
	If the argument `wait` is true, the call is blocking and will wait for all executions to end.
	
	This is equivalent to running:

	~~~
	for(int i=i0; i<i1; i++)
		f(i);
	~~~
	
	but with all iterations run in parallel threads.
	Needs lambda support.
	*/
	template<class F>
	static void parallel_for(int i0, int i1, const F& f, bool wait=true)
	{
		Array<Thread> threads;
		Array<Context> contexts(i1-i0);
		contexts.clear();
		for(int i=i0; i<i1; i++)
		{
			Thread t;
			Context s = { &f, i, &t, 0};
			contexts << s;
			t.run((Function)Thread::beginf1<F>, (void*)&contexts.last());
			threads << t;
		}
		if(wait)
			foreach(Thread& t, threads)
				t.join();
	}
	/**
	Runs the two functions/lambdas in parallel and returns when both are finished.
	*/
	template<class F1, class F2>
	static void parallel_invoke(const F1& f1, const F2& f2)
	{
		Thread t2(f2);
		f1();
		t2.join();
	}
	/**
	Runs the 3 functions/lambdas in parallel and returns when all are finished.
	*/
	template<class F1, class F2, class F3>
	static void parallel_invoke(const F1& f1, const F2& f2, const F3& f3)
	{
		Array<Thread> threads;
		Thread t2(f2), t3(f3);
		f1();
		threads << t2 << t3;
		foreach(Thread& t, threads)
			t.join();
	}
	/**
	Runs the 4 functions/lambdas in parallel and returns when all are finished.
	*/
	template<class F1, class F2, class F3, class F4>
	static void parallel_invoke(const F1& f1, const F2& f2, const F3& f3, const F4& f4)
	{
		Array<Thread> threads;
		Thread t2(f2), t3(f3), t4(f4);
		f1();
		threads << t2 << t3 << t4;
		foreach(Thread& t, threads)
			t.join();
	}
#endif
};


/**
A ThreadGroup is a set of threads that start at the same time and can be waited for termination.

**experimental: API may change**

In this example there is an array of `task`s to do. Each `Worker` is a thread that performs a subset
of the tasks: one does the first half and the other the second half.

~~~
ThreadGroup<Worker> threads;
threads << Worker(tasks.slice(0, tasks.length()/2));
threads << Worker(tasks.slice(tasks.length()/2, tasks.length()));
threads.start();
threads.join();
~~~
*/

template<class Thread>
class ThreadGroup
{
public:
	Array<Thread> _threads;
public:
	ThreadGroup() {}
	/** Add a thread to the group */
	ThreadGroup& operator<<(const Thread& thread)
	{
		_threads << thread;
		return *this;
	}
	/** Start all threads */
	void start()
	{
		foreach(Thread& t, _threads)
			t.start();
	}
	/** Wait for all threads to finish */
	void join()
	{
		foreach(Thread& t, _threads)
			t.join();
	}
};

}
#endif
