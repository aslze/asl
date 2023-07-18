// Copyright(c) 1999-2023 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_XML_H
#define ASL_XML_H

#include <asl/Array.h>
#include <asl/Map.h>
#include <asl/String.h>

namespace asl {

static String xnoStr = "";

struct CSPair { const char* a; const char* b; };

class Xml;

class ASL_API NodeBase
{
protected:
	struct ASL_API _NodeBase
	{
		int rc;
		_NodeBase() :rc(0) {}
		virtual ~_NodeBase() {}
	};

	_NodeBase* _p;
	
public:
	typedef _NodeBase NType;

	NodeBase(){
		_p = new _NodeBase;
	}
	NodeBase(const NodeBase& n)
	{
		_p = n._p;
		if (_p)
			_p->rc++;
	}
	void unref()
	{
		if(_p && --_p->rc == 0) {
			delete _p;
		}
	}
	NodeBase& operator=(const NodeBase& n)
	{
		unref();
		_p = n._p;
		if (_p)
		_p->rc++;
		return *this;
	}
	~NodeBase()
	{
		unref();
	}

	NodeBase(_NodeBase* p) : _p(p)
	{
		if (_p)
			_p->rc++;
	}

	NodeBase clone() const
	{
		_NodeBase* p = new _NodeBase(*_p);
		return NodeBase(p);
	}

	bool isnull() const
	{
		return _p == 0;
	}

	bool operator!() const
	{
		return _p == 0;
	}

	operator bool() const
	{
		return _p != 0;
	}

	bool operator==(const NodeBase& n) const
	{
		return _p == n._p;
	}

	template<class T>
	T as()
	{
		return T(dynamic_cast<typename T::NType*>(_p));
	}
	template<class T>
	const T as() const
	{
		return T(dynamic_cast<typename T::NType*>(_p));
	}

	template<class T>
	bool is() const
	{
		return dynamic_cast<typename T::NType*>(_p) != 0;
	}
};


/*
Parses the given string as XML and returns the equivalent DOM tree.
\deprecated Use Xml::decode()
*/
ASL_DEPRECATED(Xml ASL_API decodeXML(const String& xml), "Use Xml::decode()");

/*
Encodes the given XML document as XML, with or without formatting.
\deprecated Use Xml::encode()
*/
ASL_DEPRECATED(String ASL_API encodeXML(const Xml& e, bool formatted = true), "Use Xml::encode()");


class XmlText;

/**
\defgroup XDL XML, XDL, and JSON
@{
*/

/**
This class represents an XML element and can be used to manipulate a document DOM tree.
The class allows accessing the element's tag, attributes and child elements.

Consider the following XML fragment.

~~~{.xml}
<html>
	<head>
		<meta charset="utf8"/>
		<meta name="author" content="John Doe"/>
	</head>
	<body>
		<h1>Hello</h1>
		<p class="main">world</p>
	</body>
</html>
~~~


__Parsing/reading__

This can be parsed from a string or read from a file:

~~~{.cpp}
Xml html = Xml::decode(xml);           // from a string

Xml html = Xml::read("document.xml");  // from a file
~~~


The content can be accessed easily using operator `()` for tags and `[]` for attributes.
For example, the charset attribute in the `<meta>` element can be read with:

~~~
String charset = html("head")("meta")["charset"];  // -> "utf8"
~~~

- `element("tag")` returns the first child of `element` that has the given tag
- `element("tag", 3)` would return the 4th child with that tag
- `element["attr"]` returns the value of the attribute `attr`.

The text content of the `<h1>` element would be retrieved like this:

~~~
String text = html("body")("h1").text();  // -> "Hello"
~~~

That text content of an element can be written with `.pu()`:

~~~
html("body").put("h1", "Bye");      // now it's <h1>Bye</h1>
~~~

We can iterate the children of the `<body>` element like this:

~~~
Xml body = html("body");

for(auto& e : body.children())
{
	e.setAttr("tag", e.tag());     // add an attribute with the tag name
}
~~~

Or only the children with a given tag:

~~~
for(auto& meta : html("head").children("meta"))
{
	if(meta.has("charset"))
		setCharset(meta["charset"]);
	else
		addMeta(meta["name"], meta["content"]);
}
~~~

Elements of the tree can be searched using selectors in the form of lambda predicates.
For example, this will return all the embedded PNG images (`<img>` elements with an `src` attribute ending in ".png").

~~~
auto elems = html.find([](const Xml& e) {
    return e.tag() == "img" && e["src"].endsWith(".png");
});
~~~


__Creating/writing__

The same example XML document DOM can be built in code:

~~~
Xml html = Xml("html")
    << (Xml("head")
        << Xml("meta", Map<>("charset", "utf8"))
        << Xml("meta", Map<>("name", "author")("content", "John Doe"))
    )
    << (Xml("body")
        << Xml("h1", "Hello")
        << Xml("p", Map<>("class", "main"), "world")
    );
~~~

Or in newer compilers (with initializer lists):

~~~
Xml html("html", {
    Xml("head", {
        Xml("meta", {{"charset", "utf8"}}),
        Xml("meta", {{"name", "author"}, {"content", "John Doe"}})
    }),
    Xml("body", {
        Xml("h1", "Hello"),
        Xml("p", {{"class", "main"}}, "world")
    })
});
~~~

And the original XML formatted document can be written like this:

~~~
String xml = Xml::encode(html);    // to a string
Xml::write(html, "document.xml");  // to a file
~~~

*/

class ASL_API Xml : public NodeBase
{
protected:

	struct ASL_API _Xml : public _NodeBase
	{
		String tag;
		Map<> attribs;
		Array<Xml> children;
		mutable _Xml* parent;
		_Xml() : parent(NULL) {}
		_Xml(const String& t) : tag(t), parent(NULL) {}
		virtual const String& text() const;
		virtual bool isText() const { return false; }
		virtual _Xml* clone(bool detach = true) const;
	};

	_Xml* _() { return (_Xml*)_p; }
	const _Xml* _() const { return (_Xml*)_p; }

	ASL_EXPLICIT operator int() const;
public:
	typedef _Xml NType;

	Xml() : NodeBase(new _Xml){
	}

	//Xml(_NodeBase* p) : NodeBase(p){}

	Xml(_Xml* p) : NodeBase(p)
	{
	}

	/**
	Constructs an element with the given tag.
	*/
	ASL_EXPLICIT Xml(const String& tag) : NodeBase(new _Xml(tag))
	{
	}

	/**
	Constructs an element with the given tag and attributes.
	*/
	ASL_EXPLICIT Xml(const String& tag, const Map<>& attrs) : NodeBase(new _Xml(tag))
	{
		_()->attribs = attrs;
	}

	/**
	Constructs an element with the given tag and value (text subelement).
	*/
	ASL_EXPLICIT  Xml(const String& tag, const String& val);

	ASL_EXPLICIT  Xml(const String& tag, const Array<Xml>& elems) : NodeBase(new _Xml(tag))
	{
		_()->children = elems;
	}
	
	ASL_EXPLICIT  Xml(const String& tag, const Map<>& attrs, const Array<Xml>& elems) : NodeBase(new _Xml(tag))
	{
		_()->attribs = attrs;
		_()->children = elems;
	}

	/**
	Constructs an element with the given tag, attributes and value (text subelement).
	*/
	ASL_EXPLICIT Xml(const String& tag, const Map<>& attrs, const String& val);

#ifdef ASL_HAVE_INITLIST
	ASL_EXPLICIT Xml(const String& tag, const std::initializer_list<Map<>::KeyVal>& attrs) : NodeBase(new _Xml(tag))
	{
		_()->attribs = attrs;
	}

#endif
	/**
	Returns a separate copy of this element with its children, and no parent
	*/
	Xml clone(bool detach = true) const { return _()->clone(); }

	/**
	Changes the tag name of this element
	\deprecated Why would anyone do this?
	*/
	ASL_DEPRECATED(void setTag(const String& tag), "Seems unneeded")
	{
		_()->tag = tag;
	}
	/**
	Returns the parent element of this element (a null Xml object if it this is the root)
	*/
	Xml parent() const
	{
		return _()->parent;
	}
	/**
	Returns the tag of this element.
	*/
	const String& tag() const
	{
		return _()->tag;
	}

	bool operator!() const
	{
		return _p == 0 || !_()->tag.ok();
	}

	ASL_EXPLICIT operator bool() const
	{
		return _p != 0 && _()->tag.ok();
	}

	bool isvalid() const
	{
		return  _p != 0 && tag().ok();
	}

	/**
	Returns the element's attributes
	*/
	Map<>& attribs()
	{
		return _()->attribs;
	}

	const Map<>& attribs() const
	{
		return _()->attribs;
	}

	/**
	Returns the i-th child element with the given tag.
	*/
	Xml operator()(const String& tag, int i = 0) const;
	
	/**
	Traverses all sub elements and executes the given function.
	*/
	template <class F>
	void traverse(const F& f)
	{
		f(*this);
		foreach(Xml& e, _()->children)
			e.traverse(f);
	}

	template <class F>
	void findAppend(const F& pred, Array<Xml>& a) const
	{
		foreach(const Xml& e, _()->children)
		{
			if (pred(e))
				a << e;
			e.findAppend(pred, a);
		}
	}

	/**
	Searches recursively and returns all sub elements that satisfy a condition given as a predicate.
	*/
	template <class F>
	Array<Xml> find(const F& pred) const
	{
		Array<Xml> a;
		findAppend(pred, a);
		return a;
	}

	/**
	Searches recursively and returns the first sub element that satisfies a condition given as a predicate.
	*/
	template <class F>
	Xml findOne(const F& pred) const
	{
		Xml item;
		foreach(const Xml& e, _()->children)
		{
			if (pred(e)) {
				item = e;
				return item;
			}
			item = e.findOne(pred);
			if (item)
				return item;
		}
		return item;
	}

	/**
	Returns the number of children with the given tag.
	*/
	int count(const String& tag) const;

	/**
	Removes the i-th child element
	*/
	void remove(int i)
	{
		if (i>=0 && i<_()->children.length())
			_()->children.remove(i);
	}

	/**
	Removes the element e if it is a child of this element
	*/
	void remove(const Xml& e);

	/**
	Inserts an element at position i as a child.
	*/
	void insert(int i, const Xml& e)
	{
		if (i < _()->children.length()) {
			e._()->parent = _();
			_()->children.insert(i, e);
		}
	}

	/**
	Returns the value of an attribute
	*/
	const String& operator[](const String& a) const
	{
		return _()->attribs.get(a, xnoStr);
	}

	const String& operator[](const char* a) const
	{
		return _()->attribs.get(a, xnoStr);
	}
	
	/**
	Sets the value of an attribute
	*/
	void setAttr(const String& attr, const String& val)
	{
		_()->attribs[attr] = val;
	}

	/**
	Returns true if the given attribute exists
	*/
	bool has(const String& attr) const
	{
		return _()->attribs.has(attr);
	}

	/**
	Removes the given attribute
	*/
	void removeAttr(const String& attr)
	{
		_()->attribs.remove(attr);
	}

	/**
	Sets the content of this element to the given text value.
	*/
	Xml& put(const String& value);

	/**
	Sets the content of this element to the given text value.
	\deprecated Use put()
	*/
	ASL_DEPRECATED(Xml& set(const String& value), "Use put()") { return put(value); }

	/**
	Sets the content of a named subelement creating it if it does not exist
	*/
	Xml& put(const String& name, const String& val);

	/**
	Sets the content of a named subelement creating it if it does not exist
	\deprecated Use put(), set() will probably set an attribute in the future
	*/
	ASL_DEPRECATED(Xml& set(const String& name, const String& val), "Use put(), set() will probably set an attribute in the future") { return put(name, val); }

	/**
	Removes all children
	*/
	void clear() { _()->children.clear(); }

	/**
	Appends an element as a child.
	*/
	Xml& operator<<(const Xml& e)
	{
		_()->children << e;
		e._()->parent = _();
		return *this;
	}

	/**
	Appends a text element as a child, or appends to the text if the last child is already a text element
	*/
	Xml& operator<<(const String& text);

	/**
	Returns this element's children elements.
	*/
	const Array<Xml>& children() const
	{
		return _()->children;
	}

	Array<Xml>& children()
	{
		return _()->children;
	}

	struct ChildrenEnumerator
	{
		typedef ChildrenEnumerator Enumerator;
		String _tag;
		Array<Xml>& _children;
		int i;
		Enumerator all() const { return *(Enumerator*)this; }
		ChildrenEnumerator(Xml& e, const String& tag) : _tag(tag), _children(e.children()), i(0) { if (_children[i].tag() != _tag) ++(*this); }
		void operator++() { do i++; while (i < _children.length() && _children[i].tag() != _tag); }
		Xml& operator*() { return _children[i]; }
		Xml* operator->() { return &(_children[i]); }
		operator bool() const { return i < _children.length(); }
		bool operator!=(const Enumerator& e) const { return (bool)*this; }
	};

	/**
	Returns an enumerator of the children elements with the given tag.
	*/
	ChildrenEnumerator children(const String& tag)
	{
		return ChildrenEnumerator(*this, tag);
	}

	ChildrenEnumerator children(const String& tag) const
	{
		return ChildrenEnumerator(*(Xml*)this, tag);
	}

	/**
	Returns the number of child elements.
	*/
	int numChildren() const
	{
		return _()->children.length();
	}

	/**
	Returns the i-th child element.
	*/
	Xml& child(int i)
	{
		return _()->children[i];
	}

	const Xml& child(int i) const
	{
		return _()->children[i];
	}

	/**
	Returns true if this is a text element.
	*/
	bool isText() const { return _()->isText(); }

	/**
	Returns the text content of this element (the first child text element).
	*/
	const String& text() const { return _()->text(); }

	/**
	Returns the trimmed content of this element converted to a given type; As in `elem.value<int>()`
	*/
	template<class T>
	T value(T deflt = T()) const
	{
		String t = text().trimmed();
		return t.ok() ? (T)t: deflt;
	}

	bool value(bool deflt = false) const
	{
		String t = text().trimmed();
		return t.ok() ? t.isTrue() : deflt;
	}

	/**
	Reads and decodes a file as XML and returns its root element
	*/
	static Xml read(const String& file);

	/**
	Writes an XML document to a file
	*/
	static bool write(const Xml& e, const String& file);

	/**
	Writes an XML document to a file
	\deprecated Use Xml::write(xml, file)
	*/
	static ASL_DEPRECATED(bool write(const String& file, const Xml& e), "Use Xml::write(xml, file)")
	{
		return write(e, file);
	}

	/**
	Parses the given string as XML and returns the equivalent DOM tree.
	*/
	static Xml decode(const String& xml);
	/**
	Encodes the given XML document as XML, with or without formatting.
	*/
	static String encode(const Xml& e, bool formatted = true);
};

#ifdef ASL_HAVE_RANGEFOR

inline
Xml::ChildrenEnumerator begin(const Xml::ChildrenEnumerator& a) { return a.all(); }

inline
Xml::ChildrenEnumerator end(const Xml::ChildrenEnumerator& a) { return a.all(); }

#endif

class ASL_API XmlText : public Xml
{
protected:

	struct ASL_API _XmlText : public _Xml
	{
		String _text;
		_XmlText() {}
		_XmlText(const char* t) : _text(t) {}
		_XmlText(const String& t) : _text(t) {}
		const String& text() const { return _text; }
		bool isText() const { return true; }
		_Xml* clone(bool detach = true) const { return new _XmlText(_text); }
	};

	_XmlText* _() { return (_XmlText*)_p; }
	const _XmlText* _() const { return (_XmlText*)_p; }

public:
	typedef _XmlText NType;

	XmlText() : Xml(new _XmlText){
	}
	
	XmlText(const String& t) : Xml(new _XmlText(t))
	{
	}

	XmlText(const char* t) : Xml(new _XmlText(t))
	{
	}

	XmlText(_XmlText* p) : Xml(p)
	{
	}

	void append(const String& txt)
	{
		return _()->_text += txt;
	}

	const String& text() const
	{
		return _()->_text;
	}
};


class ASL_API XmlCodec
{
	String _xml;
	bool _formatted;
	int _level;
public:
	XmlCodec()
	{
		_formatted = true;
		_level = 0;
	}

	void setFormatted(bool on) { _formatted = on; }

	const String& text() const { return _xml; }

	void escape(const String& s);

	void encode(const Xml& e);
};


/**@}*/

}

#endif
