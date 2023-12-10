#include <asl/Xdl.h>
#include <asl/TextFile.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>

#if defined(_MSC_VER) && _MSC_VER < 1800
#include <float.h>
#endif

#ifdef ASL_FAST_JSON
#define ASL_ATOF myatof
#else
#define ASL_ATOF atof
#endif

namespace asl {

struct XdlSink
{
	virtual ~XdlSink() {}
	virtual void write(String& s) {}
};

struct XdlSinkString : XdlSink
{
	String& str;
	XdlSinkString(String& s) : str(s) {}
	void write(String& s) {}
};

struct XdlSinkFile : XdlSink
{
	TextFile& file;
	XdlSinkFile(TextFile& f) : file(f) {}
	void write(String& s) { file << s; s = ""; }
};

enum StateN {
	NUMBER, INT, STRING, PROPERTY, IDENTIFIER,
	NUMBER_E, NUMBER_ES, NUMBER_EV, NUMBER_DOT, MINUS, WAIT_SEP,
	WAIT_EQUAL, WAIT_VALUE, WAIT_PROPERTY, WAIT_OBJ, QPROPERTY, ESCAPE, ERR, UNICODECHAR,
	WAIT_COMMA_OR_PROPERTY, WAIT_COMMA_OR_VALUE
};
enum ContextN {ROOT, ARRAY, OBJECT, COMMENT1, COMMENT, LINECOMMENT, ENDCOMMENT};

#define INDENT_CHAR '\t'

Var Xdl::decode(const String& xdl)
{
	XdlParser parser;
	return parser.decode(xdl);
}

Var Json::decode(const String& json)
{
	XdlParser parser;
	return parser.decode(json);
}

String Xdl::encode(const Var& data, int mode)
{
	XdlEncoder encoder;
	return encoder.encode(data, Json::Mode(mode));
}

String Json::encode(const Var& data, Json::Mode mode)
{
	return Xdl::encode(data, mode | Json::JSON);
}

Var Xdl::read(const String& file)
{
	XdlParser parser;
	TextFile tfile(file, File::READ);
	if (!tfile)
		return Var();
	int size = int(clamp(tfile.size(), 0ll, 100000ll));
	if (size == 0)
		return Var();
	Array<char> buffer(min(16382, size) + 1);
	byte bom[3];
	if(tfile.read(bom, 3) == 3 && !(bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf))
		tfile.seek(0);
	while (1)
	{
		int n = tfile.read(buffer.data(), buffer.length() - 1);
		buffer[n] = '\0';
		parser.parse(buffer.data());
		if (n < buffer.length() - 1)
			break;
	}
	parser.parse(" ");
	return parser.value();
}

bool Xdl::write(const Var& v, const String& file, int mode)
{
	XdlEncoder encoder;
	TextFile f(file, File::WRITE);
	if (!f)
		return false;
	encoder.use(new XdlSinkFile(f));
	encoder.encode(v, Json::Mode(mode));
	return true;
}

Var Json::read(const String& file)
{
	return Xdl::read(file);
}

bool Json::write(const Var& v, const String& file, Json::Mode mode)
{
	return Xdl::write(v, file, mode | Json::JSON);
}


inline void XdlParser::value_end()
{
	_state = (_context.top() == ROOT) ? WAIT_VALUE : WAIT_SEP;
	_buffer="";
}

void XdlParser::reset()
{
	_context.clear();
	_context << ROOT;
	_state = WAIT_VALUE;
	_buffer = "";
}

void XdlParser::parse(const char* s)
{
	if(_state == ERR)
		return;
	while(char c=*s++)
	{
		Context ctx = _context.top();
		if(!_inComment)
		{
			if(c=='/' && _state != STRING && _state != ESCAPE)
			{
				_inComment = true;
				_context << COMMENT1;
				continue;
			}
			goto NOCOMMENT;
		}
		switch(ctx)
		{
		case COMMENT1:
			_context.pop();
			if (c == '/')
				_context << LINECOMMENT;
			else if (c == '*')
				_context << COMMENT;
			else
				_state = ERR;
			break;
		case LINECOMMENT:
			if(c=='\n' || c=='\r')
			{
				_inComment = false;
				_context.pop();
			}
			break;
		case COMMENT:
			if(c=='*')
				_context << ENDCOMMENT;
			break;
		case ENDCOMMENT:
			_context.pop();
			if(c=='/') {
				_inComment = false;
				_context.pop();
				continue;
			}
			break;
		default:
			if(c=='/' && _state != STRING)
			{
				_inComment = true;
				_context << COMMENT1;
			}
			continue;
		}
		ctx = _context.top();
		if(ctx==COMMENT || ctx==LINECOMMENT || ctx==COMMENT1 || ctx==ENDCOMMENT)
		{
			_inComment = true;
			continue;
		}
		else
			_inComment = false;
		NOCOMMENT:
		switch(_state)
		{
		case MINUS:
			if (c >= '0' && c <= '9')
			{
				_state = INT;
				_buffer << c;
			}
			else
			{
				_state = ERR;
				return;
			}
			break;
		case INT:
			if(c>='0' && c<='9')
			{
				_buffer << c;
			}
			else if(c=='.')
			{
				_state = NUMBER_DOT;
				_buffer << c;
			}
			else if (c == 'e' || c == 'E')
			{
				_state = NUMBER_E;
				_buffer << c;
			}
			else if(_buffer != '-')
			{
				if (_buffer[0] == '-') // this block to check starting zero !
				{
					if (_buffer[1] == '0' && _buffer[2] != '\0')
						_state = ERR;
				}
				else if (_buffer[0] == '0' && _buffer[1] != '\0')
					_state = ERR;
				if (_state == ERR)
					return;

				if (_buffer.length() > 9) // check better if it fits in an int32
					new_number(ASL_ATOF(_buffer));
				else
					new_number(myatoiz(_buffer));
				value_end();
				s--;
			}
			else
			{
				_state = ERR;
				return;
			}
			break;
		case NUMBER_DOT:
			if (c >= '0' && c <= '9')
			{
				_state = NUMBER;
				_buffer << c;
			}
			else
			{
				_state = ERR;
				return;
			}
			break;
		case NUMBER_E:
			if (c == '-' || c == '+')
			{
				_state = NUMBER_ES;
				_buffer << c;
			}
			else if (c >= '0' && c <= '9')
			{
				_state = NUMBER_EV;
				_buffer << c;
			}
			else
			{
				_state = ERR;
				return;
			}
			break;
		case NUMBER_ES:
			if (c >= '0' && c <= '9')
			{
				_state = NUMBER_EV;
				_buffer << c;
			}
			else
			{
				_state = ERR;
				return;
			}
			break;
		case NUMBER_EV:
			if (c >= '0' && c <= '9')
			{
				_buffer << c;
			}
			else if (c == ',' || myisspace(c) || c == ']' || c == '}')
			{
#ifndef ASL_FAST_JSON
				for (char* p = _buffer.data(); *p; p++)
					if (*p == '.')
					{
						*p = _ldp;
						break;
					}
#endif
				new_number(ASL_ATOF(_buffer));
				value_end();
				s--;
			}
			else
			{
				_state = ERR;
				return;
			}
			break;
		case NUMBER:
			if(c >= '0' && c <= '9')
			{
				_buffer << c;
			}
			else if (c == 'e' || c == 'E')
			{
				_state = NUMBER_E;
				_buffer << c;
			}
			else if(c == ',' || myisspace(c) || c == ']' || c == '}')
			{
#ifndef ASL_FAST_JSON
				for(char* p = _buffer.data(); *p; p++)
					if (*p == '.')
					{
						*p = _ldp;
						break;
					}
#endif
				new_number(ASL_ATOF(_buffer));
				value_end();
				s--;
			}
			else
			{
				_state = ERR;
				return;
			}

			break;

		case STRING:
			if (c == '\\')
			{
				_state = ESCAPE;
				_prevState = STRING;
			}
			else if (c == '"')
			{
				new_string(_buffer);
				value_end();
			}
			else if (unsigned(c) < ' ') // disallow control chars in string
			{
				_state = ERR;
				return;
			}
			else
				_buffer << c;
			break;

		case PROPERTY:
			if (c == '=' || myisspace(c))
			{
				new_property(_buffer);
				s--;
				_state=WAIT_EQUAL;
				_buffer="";
			}
			else // not checking possible identifier chars !
				_buffer << c;
			break;

		case QPROPERTY: // JSON
			if (c == '\\')
			{
				_state = ESCAPE;
				_prevState = QPROPERTY;
			}
			else if (c != '"')
				_buffer << c;
			else
			{
				new_property(_buffer);
				_state = WAIT_EQUAL;
				_buffer = "";
			}
			break;

		case WAIT_COMMA_OR_VALUE:
			if (c == ',')
			{
				_state = WAIT_VALUE;
				break;
			} // No more break
		case WAIT_VALUE:
			if (c >= '0' && c <= '9')
			{
				_state = INT;
				_buffer << c;
			}
			else if (c == '-')
			{
				_state = MINUS;
				_buffer << c;
			}
			else if (c == '\"')
			{
				_state = STRING;
			}
			else if (c == '[')
			{
				begin_array();
				_context << ARRAY;
			}
			else if(c=='{')
			{
				begin_object(_buffer);
				_state=WAIT_PROPERTY;
				_context << OBJECT;
				_buffer="";
			}
			/*else if(c=='.') // not JSON
			{
				state=NUMBER;
				buffer += c;
			}*/
			else if (c == '}' && ctx == OBJECT) // only if we allow {}
			{
				_context.pop();
				value_end();
				end_object();
			}
			else if (myisalnum(c) || c == '_' || c == '$')
			{
				_state=IDENTIFIER;
				_buffer << c;
			}
			else if(c==']' && ctx==ARRAY)
			{
				_context.pop();
				value_end();
				end_array();
			}
			else if(!myisspace(c) /*&& c != ','*/)
			{
				_state = ERR;
				return;
			}
			break;
		case WAIT_SEP:
			if (c == ',')
				_state = (ctx == OBJECT) ? WAIT_PROPERTY : WAIT_VALUE;
			else if (c == '\n')
				_state = (ctx == OBJECT) ? WAIT_COMMA_OR_PROPERTY : WAIT_COMMA_OR_VALUE;
			else if (c == '}' && ctx == OBJECT)
			{
				_context.pop();
				value_end();
				end_object();
			}
			else if (c == ']' && ctx == ARRAY)
			{
				_context.pop();
				value_end();
				end_array();
			}
			else if (!myisspace(c))
			{
				_state = ERR;
				return;
			}
			break;
		case WAIT_OBJ:
			if (c == '{')
			{
				begin_object(_buffer);
				_state = WAIT_PROPERTY;
				_context << OBJECT;
				_buffer = "";
			}
			else if (!myisspace(c))
			{
				_state = ERR;
				return;
			}
			break;
		case WAIT_COMMA_OR_PROPERTY:
			if (c == ',')
			{
				_state = WAIT_PROPERTY;
					break;
			} // No more break
		case WAIT_PROPERTY:
			if(myisalnum(c)||c=='_'||c=='$')
			{
				_state=PROPERTY;
				_buffer << c;
			}
			else if(c=='"') // JSON
			{
				_state=QPROPERTY;
			}
			else if(c=='}')
			{
				_context.pop();
				value_end();
				end_object();
			}
			else if(!myisspace(c) /*&& c != ','*/ && c != '}')
			{
				_state = ERR;
				return;
			}
			break;
		case ESCAPE:
			if (c == '\\')
				_buffer << '\\';
			else if (c == '"')
				_buffer << '"';
			else if (c == 'n')
				_buffer << '\n';
			else if (c == '/')
				_buffer << '/';
			else if (c == 'r')
				_buffer << '\r';
			else if (c == 't')
				_buffer << '\t';
			else if (c == 'f')
				_buffer << '\f';
			else if (c == 'b')
				_buffer << '\b';
			else if (c == 'u')
			{
				_state = UNICODECHAR;
				break;
			}
			else
			{
				_state = ERR;
				break;
			}
			_state = _prevState;
			break;

		case IDENTIFIER:
			if(!myisalnum(c) && c != '_' && c != '.')
			{
				if(_buffer=="Y" || _buffer=="N" || _buffer=="false" || _buffer=="true" )
				{
					new_bool(_buffer=="true"||_buffer=="Y");
					value_end();
				}
				else if(_buffer=="null")
				{
					put(Var::NUL);
					value_end();
				}
				else
					_state = WAIT_OBJ;
				s--;
			}
			else
				_buffer << c;
			break;
		case WAIT_EQUAL:
			if(c == ':' || c == '=')
				_state=WAIT_VALUE;
			else if(!myisspace(c))
			{
				_state = ERR;
				return;
			}
			break;
		case UNICODECHAR:
			_unicode[(_unicodeCount++) % 4] = c;
			if (_unicodeCount == 4 || _unicodeCount == 8)
			{
				char unicode[5];
				memcpy(unicode, _unicode, 4);
				unicode[4] = '\0';
				wchar_t wchar = (wchar_t)strtoul(unicode, NULL, 16);
				
				if (_unicodeCount == 8)
				{
					wchar_t u16[3] = { _wchar, wchar, 0 };
					char ch[9];
					utf16toUtf8(u16, ch, 2);
					_buffer << ch;
					_unicodeCount = 0;
				}
				else if (_unicodeCount == 4) // first code
				{
					if (wchar < 0xd800 || wchar >= 0xdc00) // not first surrogate
					{
						wchar_t u16[2] = { wchar, 0 };
						char    ch[8];
						utf16toUtf8(u16, ch, 1);
						_buffer << ch;
						_unicodeCount = 0;
					}
					else
						_wchar = wchar;
				}
				_state = _prevState;
			}
			break;
		case ERR:
			break;
		}
//		printf("%c %i\n", c, state);
	}
}

XdlParser::XdlParser()
{
	lconv* loc = localeconv();
	_ldp = *loc->decimal_point;
	_context << ROOT;
	_state = WAIT_VALUE;
	_inComment = false;
	_lists << Var(Var::ARRAY);
	_prevState = _state;
	_unicode[0] = '\0';
	_unicodeCount = 0;
}

XdlParser::~XdlParser()
{
	while(_lists.length() > 0)
		_lists.pop();
}

Var XdlParser::value() const
{
	Var v;
	if (_state == ERR)
		return v;
	const Var& l = _lists[0];
	if(_context.top() == ROOT && _state == WAIT_VALUE && l.length() > 0)
		v = l[l.length()-1];
	return v;
}

Var XdlParser::decode(const char* s)
{
	parse(s);
	parse(" ");
	return value();
}


void XdlParser::begin_array()
{
	_lists << Var::ARRAY;
}

void XdlParser::end_array()
{
	Var v = _lists.top();
	_lists.pop();
	put(v);
}

void XdlParser::begin_object(const char* _class)
{
	_lists << Var(Var::OBJ);
	if(_class[0] != '\0')
		_lists.top()[ASL_XDLCLASS] = _class;
}

void XdlParser::end_object()
{
	Var v = _lists.top();
	_lists.pop();
	put(v);
}

void XdlParser::new_property(const String& name)
{
	_props << name;
}

void XdlParser::put(const Var& x)
{
	Var& top = _lists.top();
	switch(top.type())
	{
	case Var::ARRAY:
		top << x;
		break;
	case Var::OBJ: {
		top[_props.top()] = x;
		_props.pop();
		break;
	}
	default: break;
	}
}

XdlEncoder::XdlEncoder()
{
	_level = 0;
	_pretty = false;
	_json = false;
	_out.resize(512);
	_sep1 = ',';
	_sep2 = ',';
	_simple = false;
	_fmtF = "%.9g";
	_fmtD = "%.17g";
	_sink = new XdlSinkString(_out);
}

XdlEncoder::~XdlEncoder()
{
	delete _sink;
}

void XdlEncoder::use(XdlSink* sink)
{
	delete _sink;
	_sink = sink;
}

String XdlEncoder::encode(const Var& v, Json::Mode mode)
{
	_pretty = (mode & Json::PRETTY) != 0;
	_json = (mode & Json::JSON) != 0;
	_simple = (mode & Json::SIMPLE) != 0;
	_fmtF = _simple ? "%.7g" : "%.9g";
	_fmtD = _simple ? "%.15g" : "%.17g";
	if (mode & Json::SHORTF)
		_fmtD = _fmtF;
	if (_pretty)
		_sep1 = ", ";
	if (!_json && _pretty)
		_sep2 = "";
	reset();
	_encode(v);
	if (_pretty)
		_out += '\n';
	_sink->write(_out);
	return data();
}


void XdlEncoder::_encode(const Var& v)
{
	switch(v._type)
	{
	case Var::FLOAT:
		new_number((float)v.d);
		break;
	case Var::NUMBER:
		new_number(v.d);
		break;
	case Var::INT:
		new_number(v.i);
		break;
	case Var::STRING:
		new_string(v.s->data());
		break;
	case Var::SSTRING:
		new_string(v.ss);
		break;
	case Var::BOOL:
		new_bool(v.b);
		break;
	case Var::ARRAY: {
		begin_array();
		int n = v.length();
		const Var& v0 = n>0? v[0] : v;
		bool multi = (_pretty && (n > 10 || (n>0  && (v0.is(Var::ARRAY) || v0.is(Var::OBJ)))));
		if (_pretty && !multi && v0.is(Var::STRING))
		{
			for (int i = 0, m = 0; i < n; i++)
				if ((m += v[i].length()) > 100)
				{
					multi = true;
					break;
				}
		}
		bool big = false;
		if(multi)
		{
			big = n > 0 && (v0.is(Var::ARRAY) || v0.is(Var::DIC) || v0.is(Var::STRING));
			_indent = String::repeat(INDENT_CHAR, ++_level);
			_out << '\n' << _indent;
		}
		for(int i=0; i<v.length(); i++)
		{
			if(i>0) {
				if (multi && (big || (i % 16) == 0))
					_out << _sep2 << '\n' << _indent;
				else
					_out << _sep1;
			}
			_encode(v[i]);
		}
		if(multi) {
			_indent = String::repeat(INDENT_CHAR, --_level);
			_out << '\n' << _indent;
		}
		end_array();
		break;
		}
	case Var::OBJ: {
		const Var* cname = 0;
		if (!_json)
		{
			cname = v.getp(ASL_XDLCLASS);
			begin_object(cname ? **cname : "");
		}
		else
			begin_object("");
		int k = 0;
		if (_pretty)
			_indent = String::repeat(INDENT_CHAR, ++_level);

		foreach2(String& name, Var& value, v)
		{
			if(value.ok() && (_json || &value != cname))
			{
				if (k++ > 0)
					_out << _sep2;
				if (_pretty)
					_out << '\n' << _indent;

				new_property(name);
				_encode(value);
			}
		}
		if(_pretty) {
			_indent = String::repeat(INDENT_CHAR, --_level);
			_out << '\n' << _indent;
		}
		end_object();
		}
		break;
	case Var::NUL:
		_out << "null";
		break;
	case Var::NONE:
		_out << "null";
		break;
	}

	if (_out.length() > 16000)
		_sink->write(_out);
}

void XdlEncoder::put_separator()
{
	_out << ',';
}

void XdlEncoder::reset()
{
	_out = "";
}

void XdlEncoder::new_number(int x)
{
	int n = _out.length();
	_out.resize(n+11);
	_out.fix(n + myitoa(x, &_out[n]));
}

void XdlEncoder::new_number(double x)
{
	int n = _out.length();
#if defined(_MSC_VER) && _MSC_VER < 1800
	if (!_finite(x))
#else
	if (!isfinite(x))
#endif
	{
		if (x != x)
			_out << "null";
		else
			_out << ((x < 0)? "-1e400" : "1e400");
		return;
	}
	_out.resize(n + 26);
	_out.fix(n + snprintf(&_out[n], 27, _fmtD, x));

	// Fix decimal comma of some locales
#ifndef ASL_NO_FIX_DOT
	char* p = &_out[n];
	while (*p)
	{
		if (*p == ',') {
			*p = '.';
			break;
		}
		p++;
	}
#endif
}

void XdlEncoder::new_number(float x)
{
	int n = _out.length();
#if defined(_MSC_VER) && _MSC_VER < 1800
	if (!_finite(x))
#else
	if (!isfinite(x))
#endif
	{
		if (x != x)
			_out << "null";
		else
			_out << ((x < 0) ? "-1e400" : "1e400");
		return;
	}
	_out.resize(n + 16);
	_out.fix(n + snprintf(&_out[n], 17, _fmtF, x));

	// Fix decimal comma of some locales
#ifndef ASL_NO_FIX_DOT
	char* p = &_out[n];
	while (*p)
	{
		if (*p == ',') {
			*p = '.';
			break;
		}
		p++;
	}
#endif
}

void XdlEncoder::new_string(const char* x)
{
	_out << '\"';
	const char* p = x;
	while (char c = *p++)
	{
		switch (c)
		{
		case '\\':
			_out << "\\\\"; break;
		case '\"':
			_out << "\\\""; break;
		case '\n':
			_out << "\\n"; break;
		case '\r':
			_out << "\\r"; break;
		case '\t':
			_out << "\\t"; break;
		case '\f':
			_out << "\\f"; break;
		default:
			_out << c;
		}
	}
	_out << '\"';
}

void XdlEncoder::new_bool(bool x)
{
	if (_json)
		_out << (x ? "true" : "false");
	else
		_out << (x ? "Y" : "N");
}

void XdlEncoder::begin_array()
{
	_out << '[';
}

void XdlEncoder::end_array()
{
	_out << ']';
}

void XdlEncoder::begin_object(const char* _class)
{
	if(!_json)
		_out << _class;
	_out << '{';
}

void XdlEncoder::end_object()
{
	_out << '}';
}

void XdlEncoder::new_property(const String& name)
{
	if (_json)
	{
		new_string(name);
		_out << (_pretty ? ": " : ":");
	}
	else
		_out << name << '=';
}

}
