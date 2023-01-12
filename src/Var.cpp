#include <asl/Var.h>

namespace asl {

#ifndef ASL_VAR_STATIC
#define NEW_ARRAY(a) (a) = new Array<Var>
#define NEW_ARRAYC(a, x) (a) = new Array<Var>(x)
#define DEL_ARRAY(a) delete (a)
#define NEW_DIC(d) (d) = new VDic<Var>
#define NEW_DICC(d, x) (d) = new VDic<Var>(x)
#define DEL_DIC(d) delete (d)
#else
#define NEW_ARRAY(a) (a).construct()
#define NEW_ARRAYC(a, x) (a).construct(x)
#define DEL_ARRAY(a) (a).destroy()
#define NEW_DIC(d) (d).construct()
#define NEW_DICC(d, x) (d).construct(x)
#define DEL_DIC(d) (d).destroy()
#define NEW_STRING(s) (s).construct()
#define NEW_STRINGC(s, n) (s).construct(asl::Array<char>(n))
#define DEL_STRING(s) (s).destroy()
#endif

const Var Var::none;

Var::Var(Type t)
{
	switch(_type=t)
	{
	case SSTRING: ss[0] = '\0'; break;
	case STRING: NEW_STRING(s); break;
	case ARRAY: NEW_ARRAY(a); break;
	case DIC: NEW_DIC(o); break;
	default: break;
	}
}

void Var::copy(const Var& v)
{
	switch(_type) {
	case STRING:
		NEW_STRINGC(s, v.s->length());
		memcpy(s->ptr(), v.s->ptr(), v.s->length());
		break;
	case ARRAY:
		NEW_ARRAYC(a, *v.a);
		break;
	case DIC:
		NEW_DICC(o, *v.o);
		break;
	default: break;
	}
}

Var::Var(unsigned y)
{
	if (y < 2147483648u) {
		_type = INT;
		i = (int)y;
	}
	else {
		_type = NUMBER;
		d = (double)y;
	}
}

Var::Var(Long y)
{
	_type=NUMBER;
	d=(double)y;
}

Var::Var(ULong y)
{
	_type = NUMBER;
	d = (double)y;
}

Var::Var(bool y)
{
	_type=BOOL;
	b=y;
}

Var::Var(double x)
	: _type(NUMBER), d(x)
{
}

Var::Var(const char* y)
{
	int len = (int)strlen(y);
	if(len < VAR_SSPACE) {
		_type=SSTRING;
		memcpy(ss, y, len + 1);
	}
	else {
		_type=STRING;
		NEW_STRINGC(s, len + 1);
		memcpy(s->ptr(), y, len + 1);
	}
}

Var::Var(char y)
{
	_type=INT; // int o string?
	i=y;
}

Var::Var(const String& k, const Var& x)
{
	_type = DIC;
	NEW_DIC(o);
	o->set(k, x);
}

Var::operator double() const
{
	switch(_type) {
	case NUMBER:
	case FLOAT:
		return d;
	case INT:
		return i;
	case STRING:
		return atof(s->ptr());
	case SSTRING:
		return atof(ss);
	case NUL:
		return nan();
	default:
		return 0.0;
	}
	return 0.0; // NaN ?
}

Var::operator float() const
{
	switch(_type) {
	case NUMBER:
	case FLOAT:
		return (float)d;
	case INT:
		return (float)i;
	case STRING:
		return (float)atof(s->ptr());
	case SSTRING:
		return (float)atof(ss);
	case NUL:
		return nan();
	default: break;
	}
	return 0.0; // NaN ?
}

Var::operator int() const
{
	switch(_type) {
	case INT:
		return i;
	case NUMBER:
	case FLOAT:
		return (int)d;
	case STRING:
		return atoi(s->ptr());
	case SSTRING:
		return atoi(ss);
	default: break;
	}
	return 0;
}

Var::operator unsigned() const
{
	switch(_type) {
	case INT:
		return (unsigned)i;
	case NUMBER:
	case FLOAT:
		return (unsigned)d;
	case STRING:
		return (unsigned)atoi(s->ptr());
	case SSTRING:
		return (unsigned)atoi(ss);
	default: break;
	}
	return 0;
}

Var::operator Long() const
{
	switch (_type) {
	case INT:
		return i;
	case NUMBER:
	case FLOAT:
		return (Long)d;
	case STRING:
		return (Long)atoi(s->ptr());
	case SSTRING:
		return (Long)atoi(ss);
	default: break;
	}
	return 0;
}

Var::operator String() const
{
	if(_type==STRING)
		return (*s).ptr();
	if(_type==SSTRING)
		return ss;
	return toString();
}

Var::operator bool() const
{
	switch (_type) {
	case BOOL:
		return b;
	case INT:
		return i != 0;
	case NUMBER:
	case FLOAT:
		return d != 0;
	case ARRAY:
	case DIC:
		return true;
	case STRING:
		return s->length() > 1;
	case SSTRING:
		return ss[0] != 0;
	case NUL:
		return false;
	default: return false;
	}
	return false;
}

bool Var::isTrue() const
{
	return (bool)*this;
}

Var::operator const char*() const
{
	switch(_type)
	{
	case STRING:
		return (s->ptr()); break;
	case SSTRING:
		return ss; break;
	case ARRAY:
		return "[?]"; break;
	case DIC:
		return "{?}"; break;
	default: return "?";
	}
	return "?";
}

String Var::toString() const
{
	String r(15, 0);
	r[0]='\0';
	switch(_type) {
	case INT:
		r.fix(snprintf(r, r.cap(), "%i", i));
		break;
	case FLOAT:
		r.resize(16);
		r.fix(snprintf(r, r.cap(), "%.7g", d));
		break;
	case NUMBER:
		r.resize(29);
		r.fix(snprintf(r, r.cap(), "%.15g", d));
		break;
	case BOOL:
		r=b?"true":"false";
		r.fix();
		break;
	case STRING:
		r=(*s).ptr();
		break;
	case SSTRING:
		r=ss;
		break;
	case ARRAY:
		r = '[' + a->join(',') + ']';
		break;
	case DIC:
		r = '{' + o->join(',', '=') + '}';
		break;
	case NUL:
		r="null";
		break;
	default:
		r = "?";
		break;
	}
	return r;
}

void Var::free()
{
	switch(_type) {
	case STRING: DEL_STRING(s); break;
	case ARRAY: DEL_ARRAY(a); break;
	case DIC: DEL_DIC(o); break;
	default: break;
	}
	_type=NONE;
}

void Var::operator=(const Var& v)
{
	if(_type == STRING && v._type == STRING) {
		//(*s) = (*v.s);
		s->resize(v.s->length());
		memcpy(s->ptr(), v.s->ptr(), v.s->length());
		return;
	}
	if(_type == ARRAY && v._type == ARRAY) {
		(*a) = (*v.a);
		return;
	}
	if(_type == DIC && v._type == DIC) {
		(*o) = (*v.o);
		return;
	}
	//if(_type!=NONE)
	if(!isPod())
		free();
	memcpy(this, &v, sizeof(v));
	switch(_type)
	{
	case STRING:
		NEW_STRINGC(s, v.s->length());
		memcpy(s->ptr(), v.s->ptr(), v.s->length());
		break;
	case ARRAY:
		NEW_ARRAYC(a, *v.a);
		break;
	case DIC:
		NEW_DICC(o, *v.o);
		break;
	default: break;
	}
}

void Var::operator=(double x)
{
	if(_type == NONE){}
	else
		free();
	_type=NUMBER;
	d=x;
}

void Var::operator=(int x)
{
	if(_type == NONE){}
	else
		free();
	_type=INT;
	i=x;
}

void Var::operator=(Long x)
{
	if(_type == NONE){}
	else
		free();
	_type=NUMBER;
	d=(double)x;
}

void Var::operator=(float x)
{
	if(_type == NONE){}
	else
		free();
	_type=FLOAT;
	d=x;
}

void Var::operator=(unsigned x)
{
	if(_type == NONE){}
	else
		free();
	if (x & 0x80000000) {
		_type = NUMBER;
		d = (double)x;
	}
	else {
		_type = INT;
		i = (int)x;
	}
}

void Var::operator=(bool x)
{
	if(_type == NONE){}
	else
		free();
	_type=BOOL;
	b=x;
}

void Var::operator=(const char* x)
{
	int n = (int)strlen(x);
	if (_type == STRING) {
		(*s).resize(n + 1);
		memcpy((*s).ptr(), x, n + 1);
	}
	else if(_type==SSTRING && n < VAR_SSPACE)
		memcpy(ss, x, n + 1);
	else
	{
		free();
		if(n < VAR_SSPACE)
		{
			_type = SSTRING;
			memcpy(ss, x, n + 1);
		}
		else
		{
			_type=STRING;
			NEW_STRINGC(s, n + 1);
			memcpy((*s).ptr(), x, n + 1);
		}
	}
}

void Var::operator=(const String& x)
{
	int len = x.length();
	Type t = _type;
	if(t==NONE) {}
	else if(t==STRING) {
		(*s).resize(len + 1);
		memcpy((*s).ptr(), *x, len + 1);
		return;
	}
	else if(t==SSTRING && len < VAR_SSPACE) {
		memcpy(ss, *x, len + 1);
		return;
	}

	{
		if(t==NONE) {}
		else
			free();
		if(len < VAR_SSPACE)
		{
			_type = SSTRING;
			memcpy(ss, *x, len + 1);
		}
		else
		{
			_type=STRING;
			NEW_STRINGC(s, len + 1);
			memcpy((*s).ptr(), *x, len + 1);
		}
	}
}

const Var& Var::operator[](int i) const
{
	if(_type==ARRAY)
		return (*a)[i];

	return none;
	/*else if (_type == NONE)
	{
		((Var*)this)->_type=ARRAY;
		NEW_ARRAY( ((Var*)this)->a );
		((Var*)this)->a->resize(i+1);
		return (*a)[i];
	}
	return *(Var*)this;*/
}

Var& Var::operator[](int i)
{
	if (_type == ARRAY)
	{
		if (i >= a->length())
			a->resize(i + 1);
		return (*a)[i];
	}
	else if (_type == OBJ)
	{
		return (*o)[String(i)];
	}
	else if (_type == NONE)
	{
		_type = ARRAY;
		NEW_ARRAY(a);
		a->resize(i + 1);
		return (*a)[i];
	}
	return *this;
}

Var& Var::operator[](const String& k)
{
	if (_type == NONE)
	{
		NEW_DIC(o);
		_type = DIC;
		return (*o)[k];
	}
	else if (_type == DIC)
		return (*o)[k];
	else if (_type == ARRAY)
		return (*a)[k];
	asl_error("Var[String] on non object");
	return *this;
}

const Var& Var::operator[](const String& k) const
{
	if(_type==DIC)
		return (*o)[k];

	return none;
}

/*
const Var& Var::operator()(const String& k) const
{
	if(_type==DIC)
		return (*o)[k];
	asl_error("Var(String)");
	return *(Var*)this;
}
*/

int Var::length() const
{
	switch(_type) {
	case ARRAY:
		return a->length(); break;
	case DIC:
		return o->length(); break;
	case STRING:
		return s->length()-1; break;
	case SSTRING:
		return (int)strlen(ss); break;
	default:
		break;
	}
	return 0;
}

Var& Var::operator<<(const Var& x)
{
	if(_type==ARRAY)
		(*a) << x;
	else if(_type==NONE)
	{
		_type=ARRAY;
		NEW_ARRAY(a);
		(*a) << x;
	}
	return *this;
}

Var Var::clone() const
{
	Var v(*this);
	switch (_type)
	{
	case STRING:
		v.s->dup();
		break;
	case ARRAY:
		v.a->dup();
		foreach(Var& x, *v.a)
			x = x.clone();
		break;
	case DIC:
		v.o->dup();
		foreach(Var& x, *v.o)
			x = x.clone();
		break;
	default:
		break;
	}
	return v;
}

}
