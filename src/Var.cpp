#include <asl/Var.h>

namespace asl {

#ifndef ASL_VAR_STATIC
#define NEW_ARRAY(a) (a) = new Array<Var>
#define NEW_ARRAYC(a, x) (a) = new Array<Var>(x)
#define DEL_ARRAY(a) delete (a)
#define NEW_DIC(d) (d) = new HDic<Var>
#define NEW_DICC(d, x) (d) = new HDic<Var>(x)
#define DEL_DIC(d) delete (d)
#else
#define NEW_ARRAY(a) (a).construct()
#define NEW_ARRAYC(a, x) (a).construct(x)
#define DEL_ARRAY(a) (a).destroy()
#define NEW_DIC(d) (d).construct()
#define NEW_DICC(d, x) (d).construct(x)
#define DEL_DIC(d) (d).destroy()
#define NEW_STRING(s) (s).construct()
#define NEW_STRINGC(s, x) (s).construct(x)
#define DEL_STRING(s) (s).destroy()
#endif

//#define SHORT_FLOATS

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
		NEW_STRINGC(s, (*v.s).length());
		strcpy((*s).ptr(), (*v.s).ptr());
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
#ifdef SHORT_FLOATS
	if ((*(ULong*)&d & 0x7ff0000000000000) != 0x7ff0000000000000)
		(*(ULong*)&d) |= 1;
#endif
}

Var::Var(const char* y)
{
	if(strlen(y) < VAR_SSPACE) {
		_type=SSTRING;
		strcpy(ss, y);
	}
	else {
		_type=STRING;
		NEW_STRINGC(s, (int)strlen(y)+1);
		strcpy((*s).ptr(), y);
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
		return d;
	case INT:
		return i;
	case STRING:
		return atof((*s));
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
		return (float)d;
	case INT:
		return (float)i;
	case STRING:
		return (float)atof(*s);
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
		return (int)d;
	case STRING:
		return atoi(*s);
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
		return (unsigned)d;
	case STRING:
		return (unsigned)atoi(*s);
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
		return (Long)d;
	case STRING:
		return (Long)atoi(*s);
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
		return d != 0;
	case ARRAY:
	case DIC:
		return true;
	case STRING:
		return s->length() != 0;
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
		return (*s); break;
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
		r.fix(sprintf(r, "%i", i));
		break;
	case NUMBER:
		if((float)d == d)
		{
			r.resize(15);
			r.fix(sprintf(r, "%.7g", d));
		}
		else
		{
			r.resize(26);
			r.fix(sprintf(r, "%.16g", d));
		}
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
		(*s).resize((*v.s).length() + 1);
		strcpy((*s).ptr(), (*v.s).ptr());
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
		NEW_STRINGC(s, (*v.s).length());
		strcpy((*s).ptr(), (*v.s).ptr());
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
#ifdef SHORT_FLOATS
	if ((*(ULong*)&d & 0x7ff0000000000000) != 0x7ff0000000000000)
		(*(ULong*)&d) |= 1;
#endif
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
	_type=NUMBER;
	d=x;
}

void Var::operator=(unsigned x)
{
	if(_type == NONE){}
	else
		free();
	_type=INT;
	i=(int)x;
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
	if (_type == STRING) {
		(*s).resize((int)strlen(x) + 1);
		strcpy((*s).ptr(), x);
	}
	if(_type==SSTRING && strlen(x) < VAR_SSPACE)
		strcpy(ss, x);
	else
	{
		free();
		if(strlen(x) < VAR_SSPACE)
		{
			_type = SSTRING;
			strcpy(ss, x);
		}
		else
		{
			_type=STRING;
			NEW_STRINGC(s, (int)strlen(x) + 1);
			strcpy((*s).ptr(), x);
		}
	}
}

void Var::operator=(const String& x)
{
	int len = x.length();
	Type t = _type;
	if(t==NONE) {}
	else if(t==STRING) {
		(*s).resize(x.length() + 1);
		strcpy((*s).ptr(), x);
		return;
	}
	else if(t==SSTRING && len < VAR_SSPACE) {
		strcpy(ss, x);
		return;
	}

	{
		if(t==NONE) {}
		else
			free();
		if(len < VAR_SSPACE)
		{
			_type = SSTRING;
			strcpy(ss, x);
		}
		else
		{
			_type=STRING;
			NEW_STRINGC(s, x.length() + 1);
			strcpy((*s).ptr(), x);
		}
	}
}

Var& Var::operator[](int i) const
{
	if(_type==ARRAY)
	{
		return (*a)[i];
	}
	else if(_type==NONE) {
		((Var*)this)->_type=ARRAY;
		NEW_ARRAY( ((Var*)this)->a );
		((Var*)this)->a->resize(i+1);
		return (*a)[i];
	}
	return *(Var*)this;
}

Var& Var::operator[](int i)
{
	if(_type==ARRAY)
	{
		if(i >= a->length())
			a->resize(i+1);
		return (*a)[i];
	}
	else if(_type==NONE) {
		_type=ARRAY;
		NEW_ARRAY(a);
		a->resize(i+1);
		return (*a)[i];
	}
	return *(Var*)this;
}

Var& Var::operator[](const String& k)
{
	if(_type==NONE)
	{
		NEW_DIC(o);
		_type=DIC;
		return (*o)[k];
	}
	else if(_type==DIC)
		return (*o)[k];
	asl_error("Var[String]");
	return *(Var*)this;
}

Var& Var::operator[](const String& k) const
{
	if(_type==DIC)
		return (*o)[k];

	asl_error("Var[String]");
	return *(Var*)this;
}

/*
Var& Var::operator()(const String& k) const
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
