#include <asl/Process.h>
#include <asl/SHA1.h>
#include <asl/Shared.h>
#include <asl/Date.h>
#include <asl/util.h>
#include <stdio.h>
#include <asl/testing.h>

using namespace asl;

#define APPROX(x, y) (fabs(x - y) < 0.001)

#ifndef __ANDROID__

ASL_TEST(Process)
{
	{
		Process proc = Process::execute(Process::myPath(), "-subproc", 1);
		ASL_ASSERT(proc.exitStatus() == 3);
		//printf("<%s>\n", *proc.output());
		ASL_ASSERT(proc.output().trimmed() == "subprocess -subproc,1");
	}
	{
		Array<String> lines;
		Process proc;
		proc.run(Process::myPath(), array<String>("-subproc", 5, "a \"b\\"));
		while (1)
		{
			String line = proc.readOutputLine();
			if (line == "\n")
				break;
			lines << line;
		}
		proc.wait();
		//printf("<exitcode: %i>\n", proc.exitStatus());
		if (lines.length() == 0) {
			ASL_ASSERT(lines.length() > 0);
			return;
		}
		//printf("<%s>\n", *lines[0]);
		ASL_ASSERT(proc.exitStatus() == 4);
		ASL_ASSERT(lines[0] == "subprocess -subproc,5,a \"b\\");
	}
}

#endif

ASL_TEST(SHA1)
{
	SHA1::Hash h1 = SHA1::hash("abc");
	ASL_ASSERT(encodeHex(h1, 20) == "a9993e364706816aba3e25717850c26c9cd0d89d");
	SHA1::Hash h2 = SHA1::hash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
	ASL_ASSERT(encodeHex(h2, 20) == "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
}

//#define TRACE() for(int i=0; i<count; i++) printf(" "); printf("%s\n", __FUNCTION__)
#define TRACE() {}

ASL_SMART_CLASS(Shape, SmartObject)
{
	ASL_SMART_INNER_DEF(Shape)
	Shape_() { count++; TRACE(); }
	Shape_(const Shape_& s) { count++; TRACE(); }
	virtual ~Shape_() { TRACE(); count--; }
	virtual float area() const { return 0.0f; }
	static int count;
};

class Shape : public SmartObject
{
public:
	ASL_SMART_DEF(Shape, SmartObject)
	float area() const { return _()->area(); }
	static int& count() { return Shape_::count; }
};

int Shape_::count = 0;

ASL_SMART_CLASS(Circle, Shape)
{
	ASL_SMART_INNER_DEF(Circle)
	float _radius;
	Circle_() : _radius(0.0f) { TRACE(); }
	Circle_(const Circle_& c) : _radius(c._radius) { TRACE(); }
	Circle_(float r) : _radius(r) { TRACE(); }
	~Circle_() { TRACE(); }
	virtual float area() const { return (float)PI * sqr(_radius); }
};

class Circle : public Shape
{
public:
	ASL_SMART_DEF(Circle, Shape);
	Circle(float r) : ASL_SMART_INIT(r) {}
	float radius() const { return _()->_radius; }
	void setRadius(float r) { _()->_radius = r; }
};

// sparate intefcace and implementation
struct Rect_;

class Rect : public Shape
{
public:
	ASL_SMART_DECL(Rect, Shape)
	Rect(float w, float h);
};

struct Rect_ : public Shape_
{
	ASL_SMART_INNER_DECL(Rect);
	Rect_(float w, float h) { _w = w; _h = h; }
	float area() const { return _w * _h; }
	float _w, _h;
};

ASL_SMART_INNER_IMPL(Rect)

Rect::Rect(float w, float h) : ASL_SMART_INIT(w, h) {}

ASL_TEST(SmartObject)
{
	{
		float r = 2.0f;
		Circle circle;
		circle.setRadius(r);
		ASL_ASSERT(circle.radius() == r);
		ASL_ASSERT(APPROX(circle.area(), PI * sqr(r)));
		
		Shape shape = circle;
		ASL_ASSERT(shape.is(circle));
		ASL_ASSERT(shape.ptr() == circle.ptr());
		ASL_ASSERT(shape.is<Circle>());
		ASL_ASSERT(shape.as<Circle>().radius() == r);
		ASL_ASSERT(APPROX(shape.area(), PI * sqr(r)));

		Shape shape2;
		ASL_ASSERT(!shape2.is<Circle>());
		ASL_ASSERT(APPROX(shape2.area(), 0));

		Array<Shape> shapes;
		shapes << shape << shape2 << Circle(4.0f) << Rect(2, 3);

		ASL_ASSERT(Shape::count() == 4);

		ASL_ASSERT(shapes.last().area() == 6);

		Shape empty((Shape::Ptr)0);

		ASL_ASSERT(Shape::count() == 4);
		ASL_ASSERT(!empty);

		shapes.clear();

		ASL_ASSERT(Shape::count() == 2);

		shapes << shape.clone();

		ASL_ASSERT(Shape::count() == 3);
	}
	ASL_ASSERT(Shape::count() == 0);
}

ASL_TEST(Date)
{
	for (double t = -2214380800.0; t < 3102441200.0; t += Date::DAY/2)
	{
		Date d(t);
		DateData p = d.splitUTC();
		Date d2(Date::UTC, p.year, p.month, p.day, p.hours, p.minutes, p.seconds);
		ASL_ASSERT(fabs(d - d2) < 1);
	}
}
