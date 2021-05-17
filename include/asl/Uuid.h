// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_UUID_H
#define ASL_UUID_H

#include <asl/String.h>

namespace asl {

/**
Represents an UUID which can be generated to be unique.

~~~
Uuid u = Uuid::generate();   // this is a binary ID (16 bytes)
String s = u;                // convert to a string representation
~~~

Or directly:

~~~
String u = Uuid::generate();
~~~
*/
class ASL_API Uuid
{
public:
	Uuid() {}
	/**
	Constructs an UUID from a string representation
	*/
	Uuid(const String& s);
	/**
	Returns a string representation of this UUID
	*/
	String operator*() const;
	operator String() const { return **this; }
	/**
	Returns the i-th byte of the identifier (out of 16)
	*/
	byte& operator[](int i) { return _u[i]; }
	byte operator[](int i) const { return _u[i]; }
	bool operator==(const Uuid& u) const { return memcmp(_u, u._u, 16) == 0; }
	bool operator<(const Uuid& u) const { return memcmp(_u, u._u, 16) < 0; };
	bool operator!=(const Uuid& u) const { return !(*this == u); }
	/**
	Generates an UUID (version 4).
	*/
	static Uuid generate();

private:
	byte _u[16];
};

/*
An UUID generator, currently generates UUIDs in the standard string representation.
*/
class ASL_API UuidGenerator
{
public:
	UuidGenerator();
	Uuid generate();
private:
	Random _random1;
	Random _random2;
};

}
#endif
