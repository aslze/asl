// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_STACK_H
#define ASL_STACK_H

#include <asl/Array.h>

namespace asl {

/**
This class represents a stack of elements of any type.
Adding or removing elements on top can be made with the functions `push` (add an item on top),
`pop` (remove topmost item) and `popget` (return the topmost item and remove it).

The `top()` function returns a reference to the topmost item.
\ingroup Containers
*/

template <class T>
class Stack: public Array<T>
{
public:
	Stack()
	{}
	Stack(const Stack& s): Array<T>((Array<T>&)s)
	{}
	Stack& operator>>(T& x)
	{
		int lenMinusOne = this->length()-1;
		x = (*this)[lenMinusOne];
		Array<T>::resize(lenMinusOne);
		return *this;
	}
	/** Pushes an item on top of the stack */
	void push(const T& x) {(*this)<< x;}
	/** Removes the topmost item of the stack */
	void pop() {Array<T>::resize(this->length()-1);}
	/** Removes the topmost item and returns it */
	T popget() {T y; (*this)>> y; return y;}
	/** Returns a reference to the topmost item in the stack. */
	T& top() const {return (T&)(*this)[this->length()-1];}
	T& top() {return (T&)(*this)[this->length()-1];}
};

}
#endif
