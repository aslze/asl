#ifndef ASL_QUEUE_H
#define ASL_QUEUE_H

#include <asl/Array.h>

namespace asl {

/**
A simple queue of items. There are two operations: put() adds an element at the end of
the queue, and get() gets the first element and removes it. Method length() returns the current
number of items.

~~~
Queue<int> queue;
queue.put(1);
queue.put(2);
int x1 = queue.get(); // gets 1
int x2 = queue.get(); // gets 2
~~~

You can also use operators << and >>:

~~~
Queue<int> queue;
queue << 1 << 2;
int x1, x2;
if(queue.length() >= 2)
	queue >> x1 >> x2;
~~~
*/

template <class T>
class Queue: public Array<T>
{
public:
	Queue& operator>>(T& x)
	{
		x = (*this)[0];
		this->remove(0);
		return *this;
	}
	/**
	Appends an item at the end
	*/
	void put(const T& x) {(*this)<< x;}
	/**
	Gets and removes the item at the start
	*/
	T get() {T y; (*this)>> y; return y;}
};

}
#endif
