// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_MUTEX_H
#define ASL_MUTEX_H

#include <asl/defs.h>

#ifndef _WIN32
#include <pthread.h>
#include <semaphore.h>
#include <errno.h> // ETIMEDOUT
#include <unistd.h>
#endif

namespace asl {

#ifdef _WIN32

/**
A mutex that can be locked or unlocked to protect concurrent access to a resource.

However it is recommended to use Lock instead, which is automatically unlocked on scope exit.
\ingroup Threading
*/
class Mutex
{
	CRITICAL_SECTION _mutex;
public:
	Mutex()
	{
		InitializeCriticalSection(&_mutex);
	}
	~Mutex()
	{
		DeleteCriticalSection(&_mutex);
	}
	/**
	Locks the mutex. If it was already locked by another thread, the function
	will wait until it is unlocked.
	*/
	void lock()
	{
		EnterCriticalSection(&_mutex);
	}
	bool trylock()
	{
#if defined (_MSC_VER) && _MSC_VER >= 1500
		return TryEnterCriticalSection(&_mutex) != 0;
#else
		return true;
#endif
	}
	/**
	Unlocks the mutex so other threads can use the protected resource.
	*/
	void unlock()
	{
		LeaveCriticalSection(&_mutex);
	}
	friend class Condition;
};

/**
A semaphore allows synchronizing access to shared resources and signaling
observers when they can use the resource. An observer calls `wait()` to wait
for the producer to provide something. At that time the producer calls `post()`
on the same semaphore to signal that.
\ingroup Threading
*/

class Semaphore
{
	HANDLE _semaphore;
public:
	Semaphore(int count=0)
	{
		_semaphore=CreateSemaphore(0, count, 0x7fffffff, 0);
	}
	~Semaphore()
	{
		CloseHandle(_semaphore);
	}
	/**
	Wake up a waiting thread and increase the semaphore count by n
	*/
	void post(int n = 1)
	{
		ReleaseSemaphore(_semaphore, n, 0);
	}
	/**
	Wait for the semaphore to signal
	*/
	void wait()
	{
		WaitForSingleObject(_semaphore, INFINITE);
	}
	//bool trywait() {if(sem_trywait(&sem)) return false;}
	//int value() {int i; sem_getvalue(&sem, &i); return i;}
};

/**
A *condition variable* allows one or more threads to wait until a condition is made true by another thread. The condition
or watched resource itself must be protected by a mutex and this mutex given to the condition variable. This is how this is done:

We have the ready flag to wait until it is true:

~~~
bool ready = false;
Mutex mutex;
Condition condition(mutex);
~~~

One thread waits for it to become true like this:

~~~
mutex.lock();
while (!ready)
	condition.wait();
mutex.unlock();
~~~

Another thread, when it is ready signals it like this:

~~~
mutex.lock();
ready = true;
condition.signal();
mutex.unlock();
~~~
\ingroup Threading
*/

class Condition
{
	HANDLE _cond;
	Mutex* _mutex;
public:
	Condition()
	{
		_cond = CreateEvent(0, TRUE, FALSE, 0);
	}
	/**
	Constructs a condition variable with a mutex to use
	*/
	Condition(Mutex& m)
	{
		_cond = CreateEvent(0, TRUE, FALSE, 0);
		use(m);
	}
	~Condition()
	{
		CloseHandle(_cond);
	}
	/**
	Sets mutex to use if it was not given in the constructor
	*/
	void use(Mutex& m)
	{
		_mutex = &m;
	}
	/**
	Wake up waiting threads to signal a condition change
	*/
	void signal()
	{
		PulseEvent(_cond);
	}
	/**
	Wait for the condition change to be signaled
	*/
	void wait()
	{
		_mutex->unlock();
		WaitForSingleObject(_cond, INFINITE);
		_mutex->lock();
	}
	/**
	Wait for the condition change to be signaled up to a timeout and return true if there was no signal
	*/
	bool wait(double timeout)
	{
		_mutex->unlock();
		bool ret = WaitForSingleObject(_cond, (unsigned)(1000 * timeout)) == WAIT_TIMEOUT;
		_mutex->lock();
		return ret;
	}
};

#else // !_WIN32

static pthread_mutex_t mutex_init = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_init = PTHREAD_COND_INITIALIZER;

class Mutex
{
	pthread_mutex_t _mutex;
public:
	Mutex(): _mutex(mutex_init)
	{}
	~Mutex()
	{
		pthread_mutex_destroy(&_mutex);
	}
	void lock()
	{
		pthread_mutex_lock(&_mutex);
	}
	bool trylock()
	{
		return pthread_mutex_trylock(&_mutex) == 0;
	}
	void unlock()
	{
		pthread_mutex_unlock(&_mutex);
	}
	friend class Condition;
};

#ifndef __APPLE__

class Semaphore
{
	sem_t _sem;
public:
	Semaphore(int count=0)
	{
		sem_init(&_sem, 0, count);
	}
	~Semaphore()
	{
		sem_destroy(&_sem);
	}
	void post()
	{
		sem_post(&_sem);
	}
	void post(int n)
	{
		for(int i=0; i<n; i++)
			sem_post(&_sem);
	}
	void wait()
	{
		sem_wait(&_sem);
	}
	bool trywait()
	{
		return sem_trywait(&_sem) == 0;
	}
	int value()
	{
		int i;
		sem_getvalue(&_sem, &i);
		return i;
	}
};

#elif 0

class Semaphore
{
	sem_t* _sem;
	char _name[32];
public:
	Semaphore(int count=0)
	{
		sprintf(_name, "/%x-%p", (int)getpid(), this);
		_sem = sem_open(_name, O_CREAT, 0700, 0);
	}
	~Semaphore()
	{
		sem_close(_sem);
		sem_unlink(_name);
	}
	void post()
	{
		sem_post(_sem);
	}
	void post(int n)
	{
		for(int i=0; i<n; i++)
			sem_post(_sem);
	}
	void wait()
	{
		sem_wait(_sem);
	}
	bool trywait()
	{
		return sem_trywait(_sem) == 0;
	}
	int value()
	{
		return 0;
	}
};

#else

}

#include <dispatch/dispatch.h>

namespace asl {

class Semaphore
{
	dispatch_semaphore_t _sem;
public:
	Semaphore(int count=0)
	{
		_sem = dispatch_semaphore_create(count);
	}
	~Semaphore()
	{
		dispatch_release(_sem);
	}
	void post()
	{
		dispatch_semaphore_signal(_sem);
	}
	void post(int n)
	{
		for(int i=0; i<n; i++)
			post();
	}
	void wait()
	{
		dispatch_semaphore_wait(_sem, DISPATCH_TIME_FOREVER);
		
		//dispatch_semaphore_wait(_sem, dispatch_time(DISPATCH_TIME_NOW, timeout); // ns
	}
	bool trywait()
	{
		return false;
	}
	int value()
	{
		return 0;
	}
};
#endif

class Condition
{
	pthread_cond_t _cond;
	Mutex* _mut;
public:
	Condition(): _cond(cond_init)
	{
	}
	Condition(Mutex& m): _cond(cond_init)
	{
		use(m);
	}
	~Condition()
	{
		pthread_cond_destroy(&_cond);
	}
	void use(Mutex& m)
	{
		_mut = &m;
	}
	void signal()
	{
		pthread_cond_broadcast(&_cond);
	}
	void wait()
	{
		pthread_cond_wait(&_cond, &_mut->_mutex);
	}
	bool wait(double timeout)
	{
		double t = now()+timeout;
		struct timespec to;
		to.tv_sec = (time_t)floor(t);
		to.tv_nsec = (long)((t-floor(t))*1e9);
		int r = pthread_cond_timedwait(&_cond, &_mut->_mutex, &to);
		return r == ETIMEDOUT;
	}
};

#endif // Linux

/**
A Lock is an automatic locker/unlocker of a Mutex. Locks the mutex on construction and unlocks on destruction.

~~~
Mutex mutex;
void do_something_sync()
{
	Lock lock(mutex);
	do_something();
}
~~~
\ingroup Threading
*/

class Lock
{
	Mutex& _m;
public:
	Lock(Mutex& m): _m(m)
	{
		_m.lock();
	}
	~Lock()
	{
		_m.unlock();
	}
};


/**
%Atomic version of another type.

~~~
Atomic<double> value = 0;
value += 1.5;
~~~

Variable `value` can be read or written from different threads without interfering.

Basic operators of the underlying type are exposed (+=, -=, *=, /=, ==, ...). For other operations you can cast to
the underlying type or use operator `~`. Or you can use operator `*` to get a reference to the internal variable
(with no synchronization). In that case you can use the objet's mutex to protect access:

~~~
int appendAtomic(Atomic<Array<int>>& list, double number)
{
	Lock _(list);
	*list << number;
	return list->length();
}
~~~
\ingroup Threading
*/

template<class T>
class Atomic
{
public:
	Atomic() {}

	Atomic(const Atomic& x)
	{
		_x = x;
	}

	Atomic(const T& x)
	{
		_x = x;
	}

	Atomic& operator=(const T& x)
	{
		Lock _(_mutex);
		_x = x;
		return *this;
	}

	/**
	Returns a reference to the internal value (not synchronized)
	*/
	T& operator*()
	{
		return (T&)_x;
	}

	const T& operator*() const
	{
		return (const T&)_x;
	}

	/**
	Returns the internal mutex for synchronization
	*/
	Mutex& mutex()
	{
		return _mutex;
	}

	operator Mutex&() const
	{
		return _mutex;
	}

	/**
	Returns the value of this variable as a copy
	*/
	T operator~() const
	{
		Lock _(_mutex);
		return _x;
	}

	/**
	Returns the value of this variable as a copy
	*/
	operator T() const
	{
		return ~(*this);
	}

	/**
	Allows calling member functions of the internal object
	*/
	const T* operator->() const
	{
		Lock _(_mutex);
		return (const T*)&_x;
	}

	T* operator->()
	{
		Lock _(_mutex);
		return (T*)&_x;
	}

	bool operator!()
	{
		Lock _(_mutex);
		return !_x;
	}

	operator bool() const
	{
		Lock _(_mutex);
		return (bool)_x;
	}

	bool operator==(const T& x) const
	{
		Lock _(_mutex);
		return _x == x;
	}

	bool operator != (const T& x) const
	{
		Lock _(_mutex);
		return _x != x;
	}

	bool operator < (const T& x) const
	{
		Lock _(_mutex);
		return _x < x;
	}

	bool operator <= (const T& x) const
	{
		Lock _(_mutex);
		return _x <= x;
	}

	bool operator >(const T& x) const
	{
		Lock _(_mutex);
		return _x > x;
	}

	bool operator >= (const T& x) const
	{
		Lock _(_mutex);
		return _x >= x;
	}

	T operator-() const
	{
		Lock _(_mutex);
		return -_x;
	}

	T operator++()
	{
		Lock _(_mutex);
		++_x;
		return _x;
	}

	T operator++(int)
	{
		Lock _(_mutex);
		T x = _x;
		++_x;
		return x;
	}

	T operator--()
	{
		Lock _(_mutex);
		--_x;
		return _x;
	}

	T operator--(int)
	{
		Lock _(_mutex);
		T x = _x;
		--_x;
		return x;
	}

	Atomic& operator+=(const T& x)
	{
		Lock _(_mutex);
		_x += x;
		return *this;
	}

	Atomic& operator-=(const T& x)
	{
		Lock _(_mutex);
		_x -= x;
		return *this;
	}

	Atomic& operator*=(const T& x)
	{
		Lock _(_mutex);
		_x *= x;
		return *this;
	}

	Atomic& operator/=(const T& x)
	{
		Lock _(_mutex);
		_x /= x;
		return *this;
	}

private:
	T _x;
	mutable Mutex _mutex;
};


}

#endif
