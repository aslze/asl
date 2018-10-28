#include <asl/Xdl.h>
#include <asl/TextFile.h>
#include <stdio.h>
#include <ctype.h>

#if defined(_MSC_VER) && _MSC_VER < 1800
#include <float.h>
#endif

namespace asl {

#define INDENT_CHAR ' '

Var decodeXDL(const String& xdl)
{
	XdlParser parser;
	return parser.decode(xdl);
}

Var decodeJSON(const char* json)
{
	XdlParser parser;
	return parser.decode(json);
}

String encodeXDL(const Var& data, bool pretty, bool json)
{
	XdlEncoder encoder;
	return encoder.encode(data, pretty, json);
}

Var decodeJSON(const String& json)
{
	return decodeXDL(json);
}

String encodeJSON(const Var& data, bool pretty)
{
	return encodeXDL(data, pretty, true);
}

Var Xdl::read(const String& file)
{
	return decodeXDL(TextFile(file).text());
}

bool Xdl::write(const String& file, const Var& v, bool pretty)
{
	return TextFile(file).put(encodeXDL(v, pretty));
}

Var Json::read(const String& file)
{
	return decodeJSON(TextFile(file).text());
}

bool Json::write(const String& file, const Var& v, bool pretty)
{
	return TextFile(file).put(encodeJSON(v, pretty));
}


inline void XdlParser::value_end()
{
	if(context.top()==OBJECT)
		state=WAIT_PROPERTY;
	else
		state=WAIT_VALUE;
	buffer="";
}

int myatoi(const char* s)
{
	int y = 0, sgn = 1;
	if(s[0] == '-') {sgn = -1; s++;}
	else if(s[0] == '+') s++;
	int c;
	while(c = *s++, c >= '0' && c <= '9')
		y=10 * y + (c - '0');
	return y*sgn;
}

int myatoiz(const char* s)
{
	int y = 0, sgn = 1;
	if (s[0] == '-') { sgn = -1; s++; }
	else if (s[0] == '+') s++;
	int c;
	while ((c = *s++))
		y = 10 * y + (c - '0');
	return y*sgn;
}

Long myatol(const char* s)
{
	Long y = 0, sgn = 1;
	if (s[0] == '-') { sgn = -1; s++; }
	else if (s[0] == '+') s++;
	int c;
	while (c = *s++, c >= '0' && c <= '9')
		y = 10 * y + (c - '0');
	return y*sgn;
}

inline double pow10(int x)
{
	static const double z[] = { 
		0.0000000001, 0.000000001, 0.00000001, 0.0000001, 0.000001, 0.00001, 0.0001, 0.001, 0.01, 0.1,
		1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0, 1000000000.0, 10000000000.0
	};
	if (x > -11 && x < 11)
		return z[x + 10];
	else
		return pow(10.0, x);
}

double myatof(const char* s)
{
	double y = 0;
	double m = 1;
	int exp = 0;
	if (s[0] == '-') { m = -1; s++; }
	const char* p = strchr(s, '.');
	if(p) {
		p++;
		while (*p != '\0' && *p!='e' && *p!='E') {
			exp--;
			p++;
		}
		p--;
	}
	if (!p) p = s;
	while(*p++)
		if(*p == 'e' || *p == 'E'){
			if(*(p+1) == '+')
				p++;
			exp += myatoiz(p+1);
			break;
		}
	while(int c=*s++)
	{
		if(c=='.') continue;
		if(c=='E' || c=='e') break;
		y = 10.0*y + (c-'0');
	}
	if(exp != 0)
		y *= pow(10.0, exp);
	return y * m;
}


/*
double myatof(const char* s)
{
	double y = 0;
	double m = 1;
	int exp = 0;
	int e = 0;
	if (s[0] == '-') { m = -1; s++; }
	const char* p = strchr(s, '.');
	if (p) {
		p++;
		while (*p != '\0' && *p != 'e' && *p != 'E') {
			//m *= 0.1;
			exp--;
			p++;
		}
		p--;
	}
	if(!p) p = s;
	while (*p++)
		if (*p == 'e' || *p == 'E'){
		if (*(p + 1) == '+')
			p++;
		exp += myatoi(p + 1);
		break;
		}
	long long y1 = 0;
	while (int c = *s++)
	{
		if (c == '.') continue;
		if (c == 'E' || c == 'e') break;
		y1 = 10*y1 + (c - '0');
	}
	y = double(y1) * pow10(exp);
	return y * m;
}
*/

int myitoa(int x, char* s)
{
	char ss[16];
	int i=0, j=0;
	if(x==0)
	{
		s[0]='0';
		s[1]='\0';
		return 1;
	}
	if (x < 0)
	{
		if (x == (-2147483647 - 1)) {
			strcpy(s, "-2147483648");
			return 11;
		}
		s[j++] = '-';
		x = -x;
	}
	while(x!=0) {
		ss[i++] = (x % 10) + '0';
		x = x/10;
	}
	while(i>=0)
		s[j++] = ss[--i];
	s[j-1] = '\0';
	return j-1;
}

int myltoa(Long x, char* s)
{
	char ss[32];
	int i = 0, j = 0;
	if (x == 0)
	{
		s[0] = '0';
		s[1] = '\0';
		return 1;
	}
	if (x<0)
	{
		s[j++] = '-';
		x = -x;
	}
	while (x != 0) {
		ss[i++] = (x % 10) + '0';
		x = x / 10;
	}
	while (i >= 0)
		s[j++] = ss[--i];
	s[j - 1] = '\0';
	return j - 1;
}

void XdlParser::parse(const char* s)
{
	if(state == ERR)
		return;
	while(char c=*s++)
	{
		Context ctx = context.top();
		if(!inComment)
		{
			if(c=='/' && state != STRING)
			{
				inComment = true;
				context << COMMENT1;
				continue;
			}
			goto NOCOMMENT;
		}
		switch(ctx)
		{
		case COMMENT1:
			context.pop();
			if(c=='/')
				context << LINECOMMENT;
			else if(c=='*')
				context << COMMENT;
			break;
		case LINECOMMENT:
			if(c=='\n' || c=='\r')
			{
				inComment = false;
				context.pop();
			}
			break;
		case COMMENT:
			if(c=='*')
				context << ENDCOMMENT;
			break;
		case ENDCOMMENT:
			context.pop();
			if(c=='/') {
				inComment = false;
				context.pop();
				continue;
			}
			break;
		default:
			if(c=='/' && state != STRING)
			{
				inComment = true;
				context << COMMENT1;
			}
			continue;
		}
		ctx = context.top();
		if(ctx==COMMENT || ctx==LINECOMMENT || ctx==COMMENT1 || ctx==ENDCOMMENT)
		{
			inComment = true;
			continue;
		}
		else
			inComment = false;
		NOCOMMENT:
		switch(state)
		{
		case INT:
			if(c>='0' && c<='9')
			{
				buffer+=c;
			}
			else if(c=='.' || c=='e' || c=='E')
			{
				state=NUMBER;
				buffer+=c;
			}
			else if(buffer != '-')
			{
				new_number(myatoiz(buffer));
				value_end();
				s--;
			}
			else
			{
				state = ERR;
				return;
			}
			break;

		case NUMBER:
			if((c>='0' && c<='9') || c=='-' || c=='.' || c=='e' || c=='E' || c=='+')
			{
				buffer+=c;
			}
			else if(buffer != '-')
			{
				new_number(myatof(buffer));
				value_end();
				s--;
			}
			else
			{
				state = ERR;
				return;
			}

			break;

		case STRING:
			if(c!='\"' && c!='\\')
				buffer+=c;
			else if(c=='\\')
			{
				state=ESCAPE;
			}
			else
			{
				new_string(buffer);
				value_end();
			}
			break;

		case PROPERTY:
			//if(!isalnum(c) && c!='_')
			if (c == '=' || isspace(c))
			{
				new_property(buffer);
				s--;
				state=WAIT_EQUAL;
				buffer="";
			}
			else
				buffer+=c;
			break;
		case QPROPERTY: // JSON
			if(c !='"')
				buffer+=c;
			else
			{
				new_property(buffer);
				//s--;
				state=WAIT_EQUAL;
				buffer="";
			}
			break;

		case WAIT_VALUE:
			if ((c >= '0' && c <= '9') || c == '-')
			{
				state = INT;
				buffer += c;
			}
			else if (c == '\"')
			{
				state = STRING;
			}
			else if (c == '[')
			{
				begin_array();
				context << ARRAY;
			}
			else if(c=='{')
			{
				begin_object(buffer);
				state=WAIT_PROPERTY;
				context << OBJECT;
				buffer="";
			}
			else if(c=='.')
			{
				state=NUMBER;
				buffer += c;
			}
			else if (c == '}' && ctx == OBJECT)
			{
				context.pop();
				value_end();
				end_object();
			}
			else if (isalnum(c) || c == '_' || c == '$')
			{
				state=IDENTIFIER;
				buffer += c;
			}
			else if(c==']' && ctx==ARRAY)
			{
				context.pop();
				value_end();
				end_array();
			}
			else if(!myisspace(c) && c != ',')
			{
				state = ERR;
				return;
			}
			break;
		case WAIT_PROPERTY:
			if(isalnum(c)||c=='_'||c=='$')
			{
				state=PROPERTY;
				buffer += c;
			}
			else if(c=='"') // JSON
			{
				state=QPROPERTY;
			}
			else if(c=='}')
			{
				context.pop();
				value_end();
				end_object();
			}
			else if(!myisspace(c) && c != ',' && c != '}')
			{
				state = ERR;
				return;
			}
			break;
		case ESCAPE:
			if(c=='\\')
			{
				buffer += '\\';
			}
			else if(c=='"')
				buffer += '"';
			else if(c=='n')
				buffer += '\n';
			else if(c=='/')
				buffer += '/';
			else if(c=='u')
			{
				state=UNICODE;
				unicodeCount = 0;
				break;
			}
			state=STRING;
			break;

		case IDENTIFIER:
			if(!isalnum(c) && c!='_')
			{
				if(buffer=="Y" || buffer=="N" || buffer=="false" ||buffer=="true" )
				{
					new_bool(buffer=="true"||buffer=="Y");
					value_end();
				}
				else if(buffer=="null")
				{
					put(Var::NUL);
					value_end();
				}
				else
					state=WAIT_VALUE;
				s--;
			}
			else
				buffer+=c;
			break;
		case WAIT_EQUAL:
			if(c=='=' || c==':')
				state=WAIT_VALUE;
			else if(!myisspace(c))
			{
				state = ERR;
				return;
			}
			break;
		case UNICODE:
			unicode[unicodeCount++] = c;
			if(unicodeCount == 4) // still only BMP!
			{
				unicode[4] = '\0';
				wchar_t wch[2] = {(wchar_t)strtoul(unicode, NULL, 16), 0};
				char ch[4];
				utf16toUtf8(wch, ch, 1);
				buffer += ch;
				state=STRING;
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
	context << ROOT;
	state = WAIT_VALUE;
	inComment = false;
	lists << Container(Var::ARRAY);
	lists.top().init();
}

XdlParser::~XdlParser()
{
	while(lists.length() > 0)
	{
		lists.top().free();
		lists.pop();
	}
}

Var XdlParser::value() const
{
	Var v;
	Array<Var>& l = *(Array<Var>*)lists[0].list;
	if(context.top() == ROOT && state == WAIT_VALUE && l.length() > 0)
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
	lists << Container(Var::ARRAY);
	lists.top().init();
}

void XdlParser::end_array()
{
	Var v = *(Array<Var>*)lists.top().list;
	lists.top().free();
	lists.pop();
	put(v);
}

void XdlParser::begin_object(const char* _class)
{
	lists << Container(Var::DIC);
	lists.top().init();
	if(_class[0] != '\0')
		(*(HDic<Var>*)lists.top().dict)[ASL_XDLCLASS]=_class;
}

void XdlParser::end_object()
{
	Var v = *(HDic<Var>*)lists.top().dict;
	lists.top().free();
	lists.pop();
	put(v);
}

void XdlParser::new_property(const char* name)
{
	props << (String)name;
}

void XdlParser::new_property(const String& name)
{
	props << name;
}

void XdlParser::put(const Var& x)
{
	Container& top = lists.top();
	switch(top.type)
	{
	case Var::ARRAY:
		(*(Array<Var>*)top.list) << x;
		break;
	case Var::DIC: {
		const String& name = props.top();
		(*(HDic<Var>*)top.dict).set(name, x);
		props.pop();
		break;
	}
	default: break;
	}
}

XdlWriter::XdlWriter()
{
	out.resize(512);
}

void XdlEncoder::_encode(const Var& v)
{
	switch(v._type)
	{
	case Var::NUMBER:
		new_number(v.d);
		break;
	case Var::INT:
		new_number(v.i);
		break;
	case Var::STRING:
		new_string((*v.s).ptr());
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
		bool multi = (pretty && (n > 10 || (n>0  && (v0.is(Var::ARRAY) || v0.is(Var::DIC)))));
		bool big = n>0 && (v0.is(Var::ARRAY) || v0.is(Var::DIC));
		if(multi)
		{
			out += '\n';
			indent = String(INDENT_CHAR, ++level);
			out += indent;
			//linestart = out.length;
		}
		for(int i=0; i<v.length(); i++)
		{
			if(i>0) {
				if(multi) {
					if(big || (i%16)==0) {
						if(json) out += ',';
						out += '\n';
						out += indent;
					}
					else put_separator();
				}
				else
					put_separator();
			}
			_encode(v[i]);
		}
		if(multi) {
			out += '\n';
			indent = String(INDENT_CHAR, --level);
			out += indent;
		}
		end_array();
		//if(multi) out += '\n';
		break;
		}
	case Var::DIC: {
		bool hasclass = v.has(ASL_XDLCLASS);
		if(hasclass)
			begin_object(v[ASL_XDLCLASS].toString());
		else
			begin_object("");
		int k = (hasclass && json)?1:0;
		indent = String(INDENT_CHAR, ++level);
		foreach2(String& name, Var& value, v)
		{
			if(!value.is(Var::NONE) && name!=ASL_XDLCLASS)
			{
				if(pretty) {
					if(json && k++!=0) out += ',';
					out += '\n';
					out += indent;
				}
				else if(k++>0) put_separator();
				new_property(name);
				_encode(value);
			}
		}
		if(pretty) {
			out += '\n';
			indent = String(INDENT_CHAR, --level);
			out += indent;
		}
		end_object();
		//if(pretty) out += '\n';
		}
		break;
	case Var::NUL:
		out += "null";
		break;
	case Var::NONE:
		break;
	}
}

void XdlWriter::put_separator()
{
	out += ',';
}

void XdlWriter::reset()
{
	out = "";
}

void XdlWriter::new_number(int x)
{
	int n = out.length();
	out.resize(n+11);
	out.fix(n + myitoa(x, &out[n]));
}

void XdlWriter::new_number(double x)
{
	int n = out.length();
#if defined(_MSC_VER) && _MSC_VER < 1800
	if (!_finite(x))
#else
	if (!isfinite(x))
#endif
	{
		if (x != x)
			out += "null";
		else
			out += (x < 0)? "-1e400" : "1e400";
		return;
	}
	if((float)x == x)
	{
		out.resize(n+16);
		out.fix(n+sprintf(&out[n], "%.9g", x));
	}
	else
	{
		out.resize(n+26);
		out.fix(n+sprintf(&out[n], "%.16g", x));
	}

	// Fix decimal comma of some locales
	char* p = &out[n];
	while (*p)
	{
		if (*p == ',') {
			*p = '.';
			break;
		}
		p++;
	}
}

void XdlWriter::new_string(const char* x)
{
	out += '\"';
	const char* p = x;
	while(char c = *p++) // optimizable: not appending chars directly
	{
		if(c=='\\')
			out += "\\\\";
		else if (c == '\"')
			out += "\\\"";
		else if (c == '\n')
			out += "\\n";
		else if (c == '\r')
			out += "\\r";
		else
			out += c;
	}
	out += '\"';
}


void XdlWriter::new_bool(bool x)
{
	if(json)
		out += (x)?"true":"false";
	else
		out += (x)?"Y":"N";
}

void XdlWriter::begin_array()
{
	out += '[';
}

void XdlWriter::end_array()
{
	out += ']';
}

void XdlWriter::begin_object(const char* _class)
{
	if(!json)
		out += _class;
	out += '{';
	if(json && _class[0] != '\0') {
		out += "\"_class\":\"";
		out += _class;
		out +="\"";
	}
}

void XdlWriter::end_object()
{
	out += '}';
}

void XdlWriter::new_property(const char* name)
{
	if(json) {
		out += '\"';
		out += name;
		out += "\":";
	}
	else {
		out += name;
		out += '=';
	}
}

void XdlWriter::new_property(const String& name)
{
	if(json) {
		out += '\"';
		out += name;
		out += "\":";
	}
	else {
		out += name;
		out += '=';
	}
}

}
