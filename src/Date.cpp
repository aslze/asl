#include <asl/Date.h>
#include <asl/Map.h>
#include <asl/time.h>
#include <ctype.h>
#include <time.h>

namespace asl {

#if (defined( _MSC_VER ) && (_MSC_VER <= 1310))
#define ASL_API2
#else
#define ASL_API2 ASL_API
#endif

ASL_API2 const double Date::YEAR = 86400.0 * 365.25;
ASL_API2 const double Date::DAY = 86400;
ASL_API2 const double Date::HOUR = 3600;
ASL_API2 const double Date::MINUTE = 60;

Date::Date(const String& t)
{
	if (isalpha(t[0])) // HTTP like? "Thu, 18 May 2017 03:24:12 GMT"
	{
		Array<String> parts = t.split();
		if (parts.length() < 6) {
			_t = 0;
			return;
		}
		int d = parts[1];
		static Map<String, int> months = Map<String, int>
			("Jan", 1)("Feb", 2)("Mar", 3)("Apr", 4)("May", 5)("Jun", 6)("Jul", 7)("Aug", 8)("Sep", 9)("Oct", 10)("Nov", 11)("Dec", 12);
		int mo = months[parts[2]];
		int y = parts[3];
		String time = parts[4];
		Array<String> timeparts = time.split(':');
		int h = timeparts[0];
		int m = timeparts[1];
		int s = timeparts[2];
		construct(UTC, y, mo, d, h, m, s);
		return;
	}
	int i=0, j=0;
	j=t.indexOf('-');
	int y = t.substring(0,j);
	i=t.indexOf('-', j+1);
	int m = t.substring(j+1, i);
	j=t.indexOf('_', i+1);
	if(j==-1)
	{
		j=t.indexOf('T', i+1);
		if(j==-1) j=0;
	}
	int d = t.substring(i+1, j!=0? j : t.length());
	int h=0, mi=0, s=0;
	if(j!=0)
	{
		i=t.indexOf(':', j+1);
		if(i!=-1)
		{
			h = t.substring(j+1, i);
			j=t.indexOf(':', i+1);
			if(j!=-1)
			{
				mi = t.substring(i+1, j);
				s = t.substring(j+1);
			}
			else
			{
				mi = t.substring(i+1);
				s = 0;
			}
		}
	}
	construct(t.endsWith("Z")? UTC : LOCAL, y, m, d, h, mi, s);
}

static int parseSkipNumber(const char* &s)
{
	int val = atoi(s);
	while(isdigit(*s))
		s++;
	return val;
}

Date::Date(const String& str, const String& fmt)
{
	const char* f = fmt;
	const char* s = str;
	int year=0, month=0, day=0, hour=0, minute=0, second=0;

	while(char c = *f++)
	{
		switch(c)
		{
		case 'Y':
			year = parseSkipNumber(s);
			break;
		case 'D':
			day = parseSkipNumber(s);
			break;
		case 'M':
			month = parseSkipNumber(s);
			break;
		case 'h':
			hour = parseSkipNumber(s);
			break;
		case 'm':
			minute = parseSkipNumber(s);
			break;
		case 's':
			second = parseSkipNumber(s);
			break;
		default:
			if(c != '?' && *s != c) {
				_t = 0;
				return;
			}
			s++;
		}
	}
	construct(LOCAL, year, month, day, hour, minute, second);
}


Date::Date(int y, int m, int d, int h, int mn, int s)
{
	construct(LOCAL, y, m, d, h, mn, s);
}

Date::Date(Zone z, int y, int m, int d, int h, int mn, int s)
{
	construct(z, y, m, d, h, mn, s);
}

#define secsInDay        86400.0
#define daysInYearAve  365.2425
#define daysInYear(y)   ((y)%4 == 0 && ((y)%100 || ((y)%400 == 0))? 366 : 365)
#define timeFromYearAsDays(y)  (365*((y)-1970)+floor(((y)-1969)/4.0)-floor(((y)-1901)/100.0)+floor(((y)-1601)/400.0))
#define timeFromYear(y) (timeFromYearAsDays(y) * secsInDay)
#define isLeapYear(t)   (daysInYear(yearFromTime(t)) == 366)
#define dayWithinYear(t, year) (floor((t)/secsInDay) - timeFromYearAsDays(year))

static int yearFromTime(double t)
{
	static const int d4y = 365 * 4 + 1;     // 4 year block with 1 leap
	static const int d100y = d4y * 25 - 1;  // 100 year block (except multiples of 400)
	static const int d400y = 4 * d100y + 1; // 400 year block (one more leap for the first year in the block)

	int d = (int)floor(t *(1 / 86400.0)) + d400y * 4 + d100y + 1 + d100y * 2 + d4y - 1 + 16 * d4y + 2 * 365 + 1;
	//(1970 = 4 * 400 + 3 * 100 + 17 * 4 + 2)
	if (d > 695421 && d < 766645) // 1904 - 2099 : all d4y blocks
	{
		d -= 695421;
		int year = 1904;
		int k3 = (d / d4y); // num blocks 4y
		int j3 = k3 * d4y; // start of current 4y block
		year += k3 * 4;
		d -= j3;
		if (d >= 366 + 365)
			year += (d < 366 + 2 * 365) ? 2 : 3;
		else if (d >= 366)
			year += 1;
		return year;
	}

	int k1 = (d / d400y); // num blocks 400y
	int j1 = k1 * d400y; // start of current 400y block
	int year = k1 * 400;
	d = d - j1; // time relative to current 400y block
	int k2 = 0, j2 = 0;
	if (d > d100y + 1)
	{
		k2 = 1 + ((d - d100y - 1) / d100y);  // num blocks 100y
		j2 = d100y + 1 + (k2 - 1) * d100y;
	}
	year += k2 * 100;
	d = d - j2;
	int k3 = 0, j3 = 0;
	if (k2 == 0) // first 100y block -> all d4y
	{
		k3 = (d / d4y); // num blocks 4y
		j3 = k3 * d4y; // start of current 4y block
	}
	else if (d > d4y - 1) {
		k3 = 1 + ((d - d4y + 1) / d4y);
		j3 = d4y - 1 + (k3 - 1) * d4y;
	}
	d = d - j3; // time relative to current 4y block
	year += k3 * 4;
	if (k3 == 0 && k2 != 0) // all 365d
		year += (d / 365);
	else if (d >= 366 + 365)
		year += (d < 366 + 2 * 365) ? 2 : 3;
	else if (d >= 366)
		year += 1;
	return year;
}


static int month_days[][13]={
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
	{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

void Date::construct(Zone z, int year, int month, int day, int h, int m, int s)
{
	month--; // 1 = january
	bool leap = (daysInYear(year) == 366);
	double yearday = floor(timeFromYearAsDays(year));
	double monthday = month_days[leap][month];

	_t = (yearday + monthday + day - 1) * 86400.0;
	_t += h * 3600 + m * 60 + s;  // + ms * 0.001
	if (z != UTC) {
		double t = _t;
		for (int i = 0; i < 2; i++) {
			double dt3 = localOffset();
			_t = t - dt3;
		}
	}
}

#ifdef _MSC_VER
inline double round(double x)
{
	return floor(x+0.5);
}
#endif

DateData Date::calc(double t)
{
	DateData date;
	date.year = yearFromTime(t);
	int leap = isLeapYear(t)? 1 : 0;
	int yd = (int)dayWithinYear(t, date.year);

	for (int i = yd / 32; i < 13; i++)
	{
		if(yd < month_days[leap][i])
		{
			date.month = i;
			break;
		}
    }
	date.day = yd - month_days[leap][date.month - 1] + 1;

	double dt = ((t / 86400.0) - floor(t / 86400.0));
	int h = (int)floor(24*dt);
	int m = (int)floor((24*dt-h)*60);
	int s = (int)round(((24*dt-h)*60-m)*60.0);
	date.hours = h;
	date.minutes = m;
	date.seconds = s;
	date.weekDay = ((int)floor(t/86400.0)-3) % 7;
	return date;
}

String Date::toString(Date::Format fmt, bool utc) const
{
	DateData d = calc(_t + (utc? 0 : localOffset()));
	String s;
	switch(fmt)
	{
	case LONG:
		s = String(0, "%04i-%02i-%02iT%02i:%02i:%02i",
			d.year, d.month, d.day, d.hours, d.minutes, d.seconds);
		break;
	case SHORT:
		s = String(15, "%04i%02i%02iT%02i%02i%02i",
			d.year, d.month, d.day, d.hours, d.minutes, d.seconds);
		break;
	case DATE_ONLY:
		s = String(12, "%04i-%02i-%02i", d.year, d.month, d.day);
		break;
	case HTTP: {
		char buf[255];
		time_t now = (time_t)_t; //::time(0);
		struct tm tm = *gmtime(&now);
		strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S GMT", &tm);
		return buf;
		}
	}

	if (utc)
		s += 'Z';

	return s;
}

double Date::localOffset() const
{
	double t = _t;
	if (t < 0) // approximate offset before epoch
	{
		const double y = 86400 * daysInYearAve;
		t = t - floor(t / y - 2) * y;
	}
	time_t tm = (time_t)t;
	struct tm* tmL = localtime(&tm);
	if (!tmL)
		return 0;
	int hL = tmL->tm_hour;
	int dL = tmL->tm_yday;
	int yL = tmL->tm_year;
	struct tm* tmU = gmtime(&tm);
	int hU = tmU->tm_hour;
	int dU = tmU->tm_yday;
	int yU = tmU->tm_year;
	int o = hL - hU;
	if ((yL > yU && dL < dU) || (yL == yU && dL > dU))
		o += 24;
	else if ((yL < yU && dL > dU) || (yL == yU && dL < dU))
		o -= 24;
	return o * 3600;
}

#ifdef _WIN32
static inline double ft2t(const FILETIME& ft)
{
	LARGE_INTEGER t;
	t.HighPart = ft.dwHighDateTime;
	t.LowPart = ft.dwLowDateTime;
	return t.QuadPart*100e-9 - 11644473600.0;
}
#endif

Date Date::now()
{
#ifdef _WIN32
	FILETIME t;
	GetSystemTimeAsFileTime(&t);
	return Date(ft2t(t));
#else
	//return Date(::time(0));
	return Date(asl::now());
#endif
}

}
