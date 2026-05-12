// Copyright(c) 1999-2026 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_RANDOM_H
#define ASL_RANDOM_H

#include <asl/defs.h>

namespace asl {

/**
A random number generator.

Generates uniformly distributed pseudo-random numbers, except in the `normal()` functions.

```
Random random;
int n = random(255);                // get an integer between 0 and 255
double x = random(-1.5, 1.5);       // get a number between -1.5 and +1.5
double y = random.normal(10, 0.75); // get a number from a normal distribution
```

The generator initially is seeded pseudorandomly. If you need a constant sequence you can create with
a false argument, to prevent this or call seed().

For compatibility with older code, there is a global `asl::random` object already random initialized
ready for use. But it is recommended to use new Random objects when separate sequences or multithreading
are needed.
*/
class ASL_API Random
{
	ULong _state[4];
	static const int _size;
public:
	ASL_EXPLICIT Random(bool autoseed = true, bool fast = true);
	/** Returns an integer pseudo-random number in the [0, 2^32-1] interval */
	unsigned get();

	ULong getLong();

	/** Returns a floating point random number in the [0, m] interval */
	double operator()(double m) { return m * 1.1102230246251565e-16 * (getLong() >> 11); } // 0x1.0p-53

	/** Returns a floating point random number in the [m, M] interval */
	double operator()(double m, double M) { return m + (*this)(M - m); }
	
	/** Returns a floating point random number in the [0, M] interval */
	float operator()(float m) { return (float)(*this)((double)m); }

	/** Returns a floating point random number in the [m, M] interval */
	float operator()(float m, float M) { return (float)(*this)((double)m, (double)M); }

	/** Returns an integer random number in the [0, M] interval */
	template<class T>
	T operator()(T m) { return (T)(*this)((double)m + 1); }

	/** Returns an integer random number in the [m, M] interval */
	template<class T>
	T operator()(T m, T M) { return (T)(*this)((double)m, (double)M + 1); }

	/** Returns a floating point random number with standard normal distribution */
	double normal() { double u = (*this)(1e-30, 1.0), v = (*this)(1e-30, 1.0); return sqrt(-2 * log(u))*cos(2 * PI * v); }

	/** Returns a floating point random number with normal distribution with given mean and standard deviation */
	double normal(double m, double s) { return m + s * normal(); }

	/** Returns a floating point random number with normal distribution with given mean and standard deviation */
	float normal(float m, float s) { return m + s * (float)normal(); }

	/** Returns true or false given a probability (by default it is 0.5, like flipping a coin) */
	bool coin(double p = 0.5) { return (*this)(1.0) < p; }

	/** Initializes the seed for the random functions */
	void seed(ULong s);

	/** Initializes the seed for the random functions randomly (set fast=false for a high quality random seed) */
	void init(bool fast = true);

	/** Fills a buffer with OS-provided random bytes or pseudo-random if that fails */
	static void getBytes(void* buffer, int n);

	/**
	Shuffles an array of elements in place (n items starting at a).
	*/
	template <typename T>
	void shuffle(T* a, int n)
	{
		while (n) {
			int i = int((*this)(1.0) * n--);
			swap(a[n], a[i]);
		}
	}

	/**
	Shuffles an array of elements in place.
	*/
	template <typename E>
	void shuffle(E& a) { shuffle(&a[0], a.length()); }
};

extern ASL_API Random random; //!< A global random number generator

}
#endif
