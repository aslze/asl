#include <asl/Array.h>
#include <asl/Map.h>
#include <asl/HashMap.h>
#include <asl/Pointer.h>
#include <asl/Factory.h>
#include <asl/Thread.h>
#include <asl/Path.h>
#include <asl/Xml.h>
#include <asl/testing.h>
#include <stdio.h>

using namespace asl;

struct hasTag {
	String tag;
	hasTag(const String& t) : tag(t) {}
	bool operator()(const Xml& e) const { return e.tag() == tag; }
};

bool isancestor(const Xml& ancestor, const Xml& e)
{
	Xml p = e;
	while ((p = p.parent()))
	{
		if (p == ancestor)
			return true;
	}
	return false;
}

ASL_TEST(XML)
{
	String xml1 = "<?xml encoding='utf8' ?>\n<a x='1'><b y=\"2&amp;3\"><br /><c>x<!--comment--> &gt; &#x30; &#95;y</c><d g='3'></d></b></a>";
	Xml dom = Xml::decode(xml1);
	ASL_ASSERT(dom);
	ASL_ASSERT(dom.tag() == "a");
	ASL_ASSERT(dom["x"] == "1");
	ASL_ASSERT(dom("b")["y"] == "2&3");
	ASL_ASSERT(dom.child(0).child(2).has("g"));

	ASL_ASSERT(dom("b"));
	ASL_ASSERT(dom("b")("c"));
	ASL_ASSERT(!dom("B"));
	ASL_ASSERT(!dom("b")("C"));

	Array<Xml> elems = dom.find(hasTag("br"));

	ASL_ASSERT(elems.length() == 1 && elems[0].tag() == "br" && elems[0].numChildren() == 0);

	Xml elem = dom.findOne(hasTag("br"));

	ASL_ASSERT(elem && elem.tag() == "br" && elem.numChildren() == 0);

	Xml elem2 = dom.findOne(hasTag("body"));

	ASL_ASSERT(!elem2);

	String xml2 = Xml::encode(dom, false);
	
	ASL_ASSERT(xml2 == "<a x=\"1\"><b y=\"2&amp;3\"><br/><c>x &gt; 0 _y</c><d g=\"3\"/></b></a>");

	dom.removeAttr("x");
	ASL_ASSERT(!dom.has("x"));

	Xml html = Xml("html")
		<< (Xml("head")
			<< Xml("meta", Map<>("charset", "utf8")("lang", "es"))
		)
		<< (Xml("body")
			<< Xml("h1", "Hello")
			<< Xml("p", Dic<>("class", "main"), "world")
		);

	ASL_ASSERT(html.tag() == "html");

	html("head").put("link", "css1");
	html("head").put("meta", "css2");

	ASL_ASSERT(html("head").count("meta") == 1);
	ASL_ASSERT(html("head")("meta").text() == "css2");
	ASL_ASSERT(html("head")("link").text() == "css1");

	Xml head = html("head");
	ASL_ASSERT(head("meta")["lang"] == "es" && head("meta")["charset"] == "utf8");

	ASL_ASSERT(html("body")("p")["class"] == "main");

	html("body")("p") << "!";
	ASL_ASSERT(html("body").child(1).text() == "world!");

	Xml p = html("body")("p");
	Xml body = html("body");
	Xml body2 = body.clone();
	Xml p2 = body2("p");
	ASL_ASSERT(isancestor(html, p));
	ASL_ASSERT(isancestor(body, p));
	ASL_ASSERT(isancestor(body2, p2));
	ASL_ASSERT(!isancestor(html, p2));


	String xml3 = "<e a='q=\"s\"'><b/></e>";
	Xml e3 = Xml::decode(xml3);
	ASL_ASSERT(e3["a"] == "q=\"s\"");
	ASL_ASSERT(e3.count("b") == 1);
	e3.remove(e3("b"));
	ASL_ASSERT(e3.count("b") == 0);
	// test ATT SQ, ATT DQ

	Xml html2 = Xml::decode(
		"<html>"
			"<head>"
				"<meta charset='utf8'/>"
				"<link rel='stylesheet'/>"
				"<meta name='author' content='John Doe'/>"
			"</head>"
		"</html>"
	);

	String txt;
	foreach(Xml meta, html2("head").children("meta"))
	{
		if (meta.has("charset"))
			txt += meta["charset"];
		else
			txt += meta["name"] + meta["content"];
	}
	ASL_ASSERT(txt == "utf8authorJohn Doe");
#ifdef ASL_HAVE_RANGEFOR
	txt = "";
	for (Xml meta : html2("head").children("meta"))
	{
		if (meta.has("charset"))
			txt += meta["charset"];
		else
			txt += meta["name"] + meta["content"];
	}
	ASL_ASSERT(txt == "utf8authorJohn Doe");
#endif


#ifdef ASL_HAVE_INITLIST2
	Xml html3 = Xml("html", {
		Xml("head", {
			Xml("meta", {{"charset", "utf8"}, {"lang", "es"}})
		}),
		Xml("body", {
			Xml("h1", "Hello"),
			Xml("p", {{"class", "main"}}, "world")
		})
	});

	ASL_ASSERT(html3.tag() == "html");
	ASL_CHECK(html3.child(1).child(1).tag(), ==, "p");
	ASL_CHECK(html3.child(0).child(0).tag(), ==, "meta");
	ASL_CHECK(html3.child(0).child(0)["charset"], == , "utf8");
	ASL_CHECK(html3.child(1).child(1)["class"], ==, "main");
	ASL_CHECK(html3.child(1).child(1).text(), ==, "world");
#endif
}

struct Animal
{
	static int count;
	Animal() { count++; }
	Animal(const Animal& a) { count++; }
	virtual ~Animal() { count--; }
	virtual Animal* clone() const = 0;
	virtual String speak() = 0;
};

int Animal::count = 0;

struct Cat : public Animal
{
	Animal* clone() const { return new Cat(*this); }
	String speak() { return "Meow!"; }
};

ASL_FACTORY_REGISTER(Animal, Cat)

struct Dog : public Animal
{
	Animal* clone() const { return new Dog(*this); }
	String bark() { return "Woof!"; }
	String speak() { return bark(); }
};

ASL_FACTORY_REGISTER(Animal, Dog)

struct Device
{
	virtual bool enable() { return false; }
};

struct Motor: public Device
{
	bool enable() { return move(); }
	bool move() { return true; }
};

ASL_TEST(Factory)
{
	{
		Array<String> catalog = Factory<Animal>::catalog();
		ASL_ASSERT(catalog.length() == 2);
		ASL_ASSERT(catalog.contains("Cat"));
		ASL_ASSERT(catalog.contains("Dog"));

		Shared<Animal> animal = Factory<Animal>::create("Dog");

		ASL_ASSERT(animal->speak() == "Woof!");

		ASL_ASSERT(!animal.as<Cat>());
		ASL_ASSERT(animal.as<Dog>()->bark() == "Woof!");

		Shared<Animal> another = animal;
		Shared<Animal> dolly = animal.clone();
		ASL_ASSERT(dolly->speak() == "Woof!");

		Shared<Device> dev = new Motor();
		ASL_ASSERT(dev->enable());
		ASL_ASSERT(dev.as<Motor>() && dev.as<Motor>()->move());
	}
	ASL_ASSERT(Animal::count == 0);
}

ASL_TEST(Path)
{
	Path path("c:\\a/b.h");
	ASL_ASSERT(path);
	ASL_ASSERT(path.extension() == "h");
	ASL_ASSERT(path.name() == "b.h");
	ASL_ASSERT(path.noExt().name() == "b");
	ASL_ASSERT(path.nameNoExt() == "b");
	ASL_ASSERT(path.noExt().string() == "c:/a/b");
	ASL_ASSERT(path.hasExtension("H|cpp"));
	ASL_ASSERT(path.directory().string() == "c:/a");
	ASL_ASSERT(path.directory() / "c.d" == "c:/a/c.d");
	Path rel("/a/b/../c/d/e/../../f");
	rel.removeDDots();
	ASL_ASSERT(rel == "/a/c/f");
	ASL_ASSERT(Path("/a/b//c/d/../../e").equals("/a/b/e"));
	ASL_ASSERT(Path("a/b.png").noExt() + ".jpg" == "a/b.jpg");
	Path empty;
	ASL_ASSERT(!empty);

}

ASL_TEST(HashMap)
{
	HashDic<int> dic;
	for(int i=0; i<10; i++)
		dic[10-i] = i;
	ASL_ASSERT(dic.length() == 10);

	for(int i=0; i<10; i++)
		ASL_ASSERT(dic[10-i] == i);
	dic.clear();

	ASL_ASSERT(dic.length() == 0);

	HashMap<int, float> m;
	m[100] = 125.0f;
	m[2000] = -128.5f;

	HashMap<int, float> m2 = m.clone();

	ASL_ASSERT(m2 == m);

	m2[100] = 5.5f;

	ASL_ASSERT(m2 != m);
}

String join1(const Dic<String>& a)
{
	return a.join(",", ":");
}

String join(const Dic<int>& a)
{
	return a.join(",", ":");
}

ASL_TEST(Map)
{
	Map<int, String> numbers;
	numbers[12] = "twelve";
	numbers[-2] = "minus two";
	numbers[100] = "one hundred";

	ASL_ASSERT(!numbers.find(1));
	
	ASL_ASSERT(*numbers.find(12) == "twelve");
	
	ASL_ASSERT(numbers.has(12));

	Dic<String> snumbers = numbers;

	ASL_ASSERT(numbers.get(12, "?") == "twelve");
	ASL_ASSERT(numbers.get(5, "?") == "?");

	ASL_ASSERT(snumbers["12"] == numbers[12] && snumbers["-2"] == numbers[-2] && snumbers["100"] == numbers[100]);

	ASL_ASSERT(numbers.join(",", "=") == "-2=minus two,12=twelve,100=one hundred");

#ifdef ASL_HAVE_INITLIST
	ASL_ASSERT(join1({{"a", "1"}, {"b", "2"}}) == "a:1,b:2");
#endif

	Dic<> snumbers2 = split("-2=minus two,12=twelve,100=one hundred", ",", "=");
	ASL_ASSERT(snumbers == snumbers2);
	snumbers2.remove("12");
	ASL_ASSERT(snumbers != snumbers2);

#ifdef ASL_HAVE_RANGEFOR
	int sum = 0;
	String all;
	for (auto e : numbers)
	{
		sum += e.key;
		all += e.value;
	}
	ASL_ASSERT(sum == 110);
	ASL_ASSERT(all == "minus twotwelveone hundred");

#ifdef ASL_HAVE_RANGEFOR_CXX17
	sum = 0;
	all = "";
	for (auto& [k, v] : numbers)
	{
		sum += k;
		all += v;
	}
	ASL_ASSERT(sum == 110);
	ASL_ASSERT(all == "minus twotwelveone hundred");
#endif
#endif
}

ASL_TEST(StaticSpace)
{
	StaticSpace<String> ss;
	ss.construct();
	(*ss) = " Alvaro\t";
	(*ss) = ss->trim();
	ASL_ASSERT( *ss == "Alvaro" );
	ss.destroy();
}

struct AThread : public Thread
{
	static AtomicCount n;
	static const int N = 20000;
	void run()
	{
		for (int i = 0; i < N; i++)
			++n;

		for (int i = 0; i < N; i++)
			--n;
	}
};

template <class T>
struct A2Thread : public Thread
{
	static T n;
	static const int N = 5000;
	void run()
	{
		for (int i = 0; i < N; i++)
			n += 2;

		sleep(0.001);

		for (int i = 0; i < N; i++)
			--n;
	}
};

AtomicCount AThread::n = 0;

template<class T>
T A2Thread<T>::n = 0;

ASL_TEST(AtomicCount)
{
	AThread::n = 0;
	A2Thread< Atomic<double> >::n = 0;
	ThreadGroup<AThread> threads1;
	for (int i = 0; i < 30; i++)
		threads1 << AThread();
	threads1.start();
	int n = AThread::n;
	threads1.join();
	ASL_ASSERT(AThread::n == 0);

	typedef Atomic<double> ItemType;
	ThreadGroup<A2Thread<ItemType> > threads2;
	for (int i = 0; i < 20; i++)
		threads2 << A2Thread<ItemType>();
	threads2.start();
	sleep(0.001);
	double bn = A2Thread<ItemType>::n;
	threads2.join();
	ASL_ASSERT(A2Thread<ItemType>::n == ItemType(20.0 * A2Thread<ItemType>::N));
}

