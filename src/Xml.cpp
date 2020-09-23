#include <asl/Xml.h>
#include <asl/Stack.h>
#include <asl/TextFile.h>
#include <stdio.h>

#define INDENT_CHAR '\t'

namespace asl {

Xml Xml::read(const String& file)
{
	return decodeXML(TextFile(file).text());
}

bool Xml::write(const String& file, const Xml& e)
{
	return TextFile(file).put(encodeXML(e, true));
}

Xml::Xml(const String& tag, const String& val) : NodeBase(new _Xml(tag))
{
	_()->children << XmlText(val);
}

Xml::Xml(const String& tag, const Map<>& attrs, const String& val) : NodeBase(new _Xml(tag))
{
	_()->attribs = attrs;
	_()->children << XmlText(val);
}

Xml::_Xml* Xml::_Xml::clone(bool detach) const
{
	_Xml* e = new _Xml(tag);
	e->attribs = attribs.clone();

	for (int i = 0; i < children.length(); i++)
	{
		e->children << children[i].clone(false);
		e->children.last()._()->parent = e;
	}
	return e;
}

Xml Xml::operator()(const String& tag, int i) const
{
	int n = 0;
	foreach(Xml& e, _()->children)
	{
		if (e.tag() == tag && n++ == i)
			return e;
	}
	return Xml();
}

int Xml::count(const String& tag) const
{
	int n = 0;
	foreach(Xml& e, _()->children)
	{
		if (e.tag() == tag)
			n++;
	}
	return n;
}

void Xml::remove(const Xml& e)
{
	for (int i = 0; i < numChildren(); i++)
	{
		if (child(i)._p == e._p) {
			remove(i);
			e._()->parent = NULL;
			return;
		}
	}
}


Xml& Xml::set(const String& value)
{
	_()->children.clear();
	_()->children << XmlText(value);
	return *this;
}

Xml& Xml::set(const String& name, const String& val)
{
	Xml e = (*this)(name);
	if (e)
		e.set(val);
	else
		(*this) << Xml(name, val);
	return *this;
}

const String& Xml::_Xml::text() const
{
	if (children.length() > 0 /* && children[0].isText()*/)
		return children[0].text();
	return xnoStr;
}

Xml& Xml::operator<<(const String& t)
{
	_Xml* e = _();
	if (e->children.length() > 0 && e->children.last().isText())
		e->children.last().as<XmlText>().append(t);
	else
		e->children << XmlText(t);
	return *this;
}

Xml decodeXML(const String& x)
{
	if (x == "")
		return Xml(0);
	Dic<char> entities;
	entities["amp"] = '&';
	entities["apos"] = '\'';
	entities["gt"] = '>';
	entities["lt"] = '<';
	entities["quot"] = '\"';

	String b, ref, atname;
	Stack<Xml> elems;
	elems << Xml();
	enum State {
		FREE, TAG_START, TAG_END, TAG, WAIT_ATT, ATT_NAME, WAIT_EQUAL, WAIT_ATTVAL,
		ATT_VAL, ATT_VALSQ, SLASH, TAG_EXCLAM, COMMENT_START2, COMMENT, COMMENT_END1, COMMENT_END2,
		REF_START, CHAR_REF, DEF, TAG_QUES
	};
	State state = FREE;
	State lastState = FREE;
	char* p = x;
	int anglecount = 0;
	
	if (x.startsWith("<?xml"))
		p += x.indexOf("?>") + 2;

	// markupdecl: <!DOCTYPE, <!ENTITY, <!ATTLIST, <?PI
		
	while (char c = *p++)
	{
		//printf("[%c]: ", c);

		switch (state)
		{
		case FREE:
			switch (c)
			{
			case '<': {
				for (int i = 0; i < b.length(); i++)
					if (b[i] != ' ' && b[i] != '\n' && b[i] != '\r' && b[i] != '\t') {
						elems.top() << XmlText(b);
						break;
					}
				}
				b = "";
				state = TAG_START;
				break;
			case '&':
				state = REF_START;
				break;
			default:
				b << c;
				break;
			}
			break;
		case TAG_START:
			switch (c)
			{
			case '/':
				state = TAG_END;
				break;
			case '!':
				state = TAG_EXCLAM;
				break;
			case '?':
				state = TAG_QUES;
				break;
			default:
				state = TAG;
				b = c;
				break;
			}
			break;
		case TAG:
			switch (c)
			{
			case '>':
				elems.push(Xml(b));
				b = "";
				state = FREE;
				break;
			case '/':
				elems.push(Xml(b));
				b = "";
				state = SLASH;
				break;
			case ' ': case '\t': case '\r': case '\n':
				elems.push(Xml(b));
				b = "";
				state = WAIT_ATT;
				break;
			default:
				b << c;
				break;
			}
			break;
		case TAG_END:
			switch (c)
			{
			case '>':
				if (b != elems.top().tag())
					return Xml(0);
				{
					Xml e = elems.popget();
					elems.top() << e;
				}
				state = FREE;
				b = "";
				break;
			default:
				b << c;
				break;
			}
			break;
		case WAIT_ATT:
			switch (c)
			{
			case '>':
				state = FREE;
				b = "";
				break;
			case '/':
				state = SLASH;
				break;
			case '\"': // sure??
				state = ATT_VAL;
				lastState = ATT_VAL;
				b = "";
				break;
			case '\'':
				state = ATT_VALSQ;
				lastState = ATT_VALSQ;
				b = "";
				break;
			case ' ': case '\t': case '\n': case '\r':
				break;
			default:
				state = ATT_NAME;
				b = c;
				break;
			}
			break;
		case ATT_NAME:
			switch (c)
			{
			case ' ': case '\t': case '\r': case '\n':
				atname = b;
				b = "";
				state = WAIT_EQUAL;
				break;
			case '=':
				atname = b;
				b = "";
				state = WAIT_ATTVAL;
				break;
			default:
				b << c;
			}
			break;
		case WAIT_EQUAL:
			switch (c)
			{
			case '=':
				state = WAIT_ATTVAL;
				break;
			}
			break;
		case WAIT_ATTVAL:
			switch (c)
			{
			case '\"':
				state = ATT_VAL;
				lastState = ATT_VAL;
				b = "";
				break;
				// disallow non white
			case '\'':
				state = ATT_VALSQ;
				lastState = ATT_VALSQ;
				b = "";
				break;
				// disallow non white
			}
			break;

		case ATT_VAL:
			switch (c)
			{
			case '\"':
				elems.top().setAttr(atname, b);
				state = WAIT_ATT;
				lastState = FREE;
				b = "";
				break;
			case '&':
				state = REF_START;
				break;
			default:
				b << c;
				break;
			}
			break;

		case ATT_VALSQ:
			switch (c)
			{
			case '\'':
				elems.top().setAttr(atname, b);
				state = WAIT_ATT;
				lastState = FREE;
				b = "";
				break;
			case '&':
				state = REF_START;
				break;
			default:
				b << c;
				break;
			}
			break;
		case SLASH:
			switch (c)
			{
			case '>':
				{
				Xml e = elems.popget();
				elems.top() << e;
				}
				state = FREE;
				b = "";
				break;
			}
			break;
		case REF_START:
			state = CHAR_REF;
			ref = c;
			break;
		case CHAR_REF:
			if (c == ';')
			{
				if (ref[0] == '#')
				{
					int code = (ref[1] == 'x') ? (int)ref.substring(2).hexToInt() : (int)ref.substring(1);
					int wch[2] = { code, 0 };
					char bytes[5];
					utf32toUtf8(wch, bytes, 1);
					b << bytes;
				}
				else
					b << entities.get(ref, '?');
				state = lastState;
				ref = "";
			}
			else
			ref << c;
			break;
		case TAG_EXCLAM:
			if (c == '-')
				state = COMMENT_START2;
			else { // only D,E,A
				state = DEF;
				b = c;
			}
			break;
		case TAG_QUES:
			if (c == '>') // should be ?>
				state = FREE;
			break;
		case COMMENT_START2:
			if (c == '-')
				state = COMMENT;
			else
				state = FREE;
			break;
		case COMMENT:
			if (c == '-')
				state = COMMENT_END1;
			break;
		case COMMENT_END1:
			if (c == '-')
				state = COMMENT_END2;
			else
				state = COMMENT;
			break;
		case COMMENT_END2:
			if (c == '>')
				state = FREE;
			else
				state = COMMENT;
			break;
		case DEF:
			if (myisspace(c)) {
				// b = markup (DOCTYPE, ENTITY, ELEMENT, ATTLIST)
			}
			else if (c == '<')
				anglecount++;
			else if (c == '>') {
				if (anglecount == 0) {
					state = FREE;
					b = "";
				}
				else
					anglecount--;
			}
			else
				b << c;
			break;
		}
	}
	return (elems.top().numChildren() == 1)? elems.top().child(0) : Xml(0);
}


void XmlCodec::escape(const String& s)
{
	char* p = s;
	while (char c = *p++)
	{
		switch (c)
		{
		case '&': _xml << "&amp;"; break;
		case '<': _xml << "&lt;"; break;
		case '>': _xml << "&gt;"; break;
		case '\'': _xml << "&apos;"; break;
		case '\"': _xml << "&quot;"; break;
		default: _xml << c;
		}
	}
}

void XmlCodec::encode(const Xml& e)
{
	if (e.isnull())
		return;
	if (e.isText()) {
		escape(e.text());
		return;
	}
	// write prolog?
	// if (!e.parent()) _xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	if(_formatted)
		for (int i = 0; i < _level; i++) _xml << INDENT_CHAR;
	_level++;
	_xml << '<' << e.tag();
	const Map<>& attribs = e.attribs();
	if (attribs.length() > 0)
	{
		foreach2(String& name, String& value, attribs)
		{
			_xml << ' ' << name << "=\"";
			escape(value);
			_xml << '\"';
		}
	}
	if (e.numChildren() == 0) {
		_xml << "/>";
		if (_formatted)
			_xml << '\n';
		_level--;
	}
	else
	{
		_xml << '>';
		if (_formatted && !e.child(0).isText())
			_xml << '\n';

		for (int i = 0; i < e.numChildren(); i++)
			encode(e.child(i));

		_level--;
		if (_formatted && !e.children().last().isText())
			for (int i = 0; i < _level; i++) _xml << ' ';
		_xml << "</" << e.tag() << '>';
		if (_formatted)
			_xml << '\n';
	}
}

String encodeXML(const Xml& e, bool formatted)
{
	XmlCodec c;
	c.setFormatted(formatted);
	c.encode(e);
	return c.text();
}

}
