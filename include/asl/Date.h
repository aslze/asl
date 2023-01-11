// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_DATE_H
#define ASL_DATE_H

#include <asl/String.h>

namespace asl {

/**
A struct containing all components of a Date after calling Date::split().

Components are: year, month, day, hours, minutes, seconds, weekDay
*/
struct DateData
{
	int year, month, day;
	int hours, minutes, seconds;
	int weekDay;
};

/**
This class represents a point in time, both a date and a time. It can be used to store date-time values, add or
subtract time, compare times and convert to/from string representations. Time is stored as the number of seconds (with fractions)
from 1970-01-01 0:00:00 UTC ignoring leap seconds.

~~~
Date today = Date::now();                     // the current date and time
String stoday = today.toString();             // an ISO-8601 formatted representation
String compact = today.toString(Date::SHORT); // without '-' or ':' symbols

Date fileTime = File("x.txt").lastModified();
double age = today - fileTime;                // file modified 'age' seconds ago

Date future("1/05/2030 12:30", "D/M/Y?h:m");  // formatted parsing
~~~

Dates can be created by their components, either as local time or UTC time:

~~~
Date d1(2016, 12, 31, 12, 30, 00); // Local 2016-12-31 12:30:00
Date d2(Date::UTC, 2016, 12, 31, 12, 30, 00); // UTC 2016-12-31 12:30:00Z
~~~

And its components (year, month, etc.) can be extracted with the functions `year()`, `month()`, `day()`, etc. These always
return them in local time. But if many components or UTC are needed, it is faster to use the `split()` or `splitUTC()` functions.
These two return all components in a struct.

~~~
int year = d2.year(), month = d2.month(), day = d2.day(), hours = d2.hours();

DateData d = d2.split();
int y = d.year, month = d.month, day = d.day, hours = d.hours;
~~~
*/
class ASL_API Date
{
public:
	enum Format {LONG, SHORT, DATE_ONLY, HTTP, FULL};
	enum Zone {UTC, LOCAL};
	Date() {}
	Date(double t) : _t(t) {}
	Date(const Date& d) : _t(d._t) {}
	/**
	Constructs a Date from a string representation in ISO-8601 format ("yyyy-mm-ddThh:mm:ss") or in RFC 1123
	format (like HTTP, "Thu, 18 May 2017 03:24:12 GMT"). The last time components can be
	omitted and will be zero.
	*/
	Date(const String& s);
	/**
	Constructs a Date from the string `s` using `fmt` as format specification. The format can
	include characers `Y`, `M`, `D`, `h`, `m`, `s` as place holders for year, month, day, hour, minute, second.
	Any other characters will be matched literally except character '?', which matches any character.
	*/
	Date(const String& s, const String& fmt);
	/**
	Creates a date-time from the given components, from year to seconds.
	*/
	Date(int y, int m, int d, int h=0, int mn=0, int s=0);
	/**
	Creates a date-time from the given components, from year to seconds, in LOCAL or UTC time, depending on specified zone
	*/	Date(Zone z, int y, int m, int d, int h = 0, int mn = 0, int s = 0);
	/**
	Returns a string representation of this date in ISO-8601 format
	*/
	operator String() const {return toString();}

	String toString(Format f, bool utc) const;

	/**
	Returns a string representation of this date in ISO-8601 format. The format argument is one of
	`LONG` (default): full ISO-8601, `SHORT`: compact ISO-8601 (no separators), `DATE_ONLY`: just the date
	part with separators, `HTTP` HTTP long format in GMT.
	*/
	String toString(Format f = LONG) const { return toString(f, false); }
	/**
	Returns a string representation of this date in UTC in ISO-8601 format. The format argument is one of
	`LONG` (default): full ISO-8601, `SHORT`: compact ISO-8601 (no separators), `DATE_ONLY`: just the date
	part with separators.
	*/
	String toUTCString(Format f = LONG) const { return toString(f, true); }
	/**
	Returns the time value of this date-time (in seconds)
	*/
	double time() const {return _t;}

	/**
	Splits this date into components year, month, day, hour, etc in local time
	*/
	DateData split() const { return calc(_t + localOffset()); }

	/**
	Splits this date into components year, month, day, hour, etc in UTC time zone
	*/
	DateData splitUTC() const { return calc(_t); }

	/**
	Returns the year of this date
	*/
	int year() const { return split().year; }
	/**
	Returns the month of this date
	*/
	int month() const { return split().month; }
	/**
	Returns the day of this date
	*/
	int day() const { return split().day; }
	/**
	Returns the hours of this date-time
	*/
	int hours() const { return split().hours; }
	/**
	Returns the minutes of this date-time
	*/
	int minutes() const { return split().minutes; }
	/**
	Returns the seconds of this date-time
	*/
	int seconds() const { return split().seconds; }
	/**
	Returns the week day number of this date (0=Sunday)
	*/
	int weekDay() const { return split().weekDay; }
	/**
	Compares two date-times for equality within a millisecond
	*/
	bool operator==(const Date& d) const {return fabs(_t - d._t) < 0.001;}
	bool operator!=(const Date& d) const {return !(*this == d);}
	/**
	Compares two date-times returning true if this is before date `d`
	*/
	bool operator<(const Date& d) const {return _t < d._t;}
	bool operator<=(const Date& d) const { return _t <= d._t; }
	bool operator>(const Date& d) const { return _t > d._t; }
	/**
	Adds `dt` seconds to this date-time and returns the resulting future date-time
	*/
	Date operator+(double dt) const {return Date(_t + dt);}
	/**
	Subtracts `dt` seconds to this date-time and returns the resulting past date-time
	*/
	Date operator-(double dt) const {return Date(_t - dt);}
	/**
	Returns the time difference between two date times in seconds
	*/
	double operator-(const Date& d) const {return (_t - d._t);}
	/**
	Returns the local time offset
	*/
	double localOffset() const;
	/**
	Returns the current date-time
	*/
	static Date now();
	static const double DAY;
	static const double HOUR;
	static const double YEAR;
	static const double MINUTE;
private:
	double _t;
	void construct(Zone z, int year, int month, int day, int h = 0, int m = 0, int s = 0);
	static DateData calc(double t);
};

inline Date::Format operator|(Date::Format a, Date::Format b)
{
	return Date::Format(int(a) | int(b));
}

}
#endif
