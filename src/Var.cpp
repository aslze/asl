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

#ifdef _MSC_VER
#pragma warning(disable : 26451 26495 26812)
#endif

const Var Var::none;

Var::Var(Type t)
{
	switch(_type=t)
	{
	case SSTRING: _ss[0] = '\0'; break;
	case STRING: NEW_STRING(_s); break;
	case ARRAY: NEW_ARRAY(_a); break;
	case OBJ: NEW_DIC(_o); break;
	default: break;
	}
}

void Var::copy(const Var& v)
{
	switch(_type) {
	case STRING:
		NEW_STRINGC(_s, v._s->length());
		memcpy(_s->data(), v._s->data(), v._s->length());
		break;
	case ARRAY:
		NEW_ARRAYC(_a, *v._a);
		break;
	case OBJ:
		NEW_DICC(_o, *v._o);
		break;
	default: break;
	}
}

Var::Var(unsigned y)
{
	if (y < 2147483648u) {
		_type = INT;
		_i = (int)y;
	}
	else {
		_type = NUMBER;
		_d = (double)y;
	}
}

Var::Var(Long y)
{
	_type=NUMBER;
	_d=(double)y;
}

Var::Var(ULong y)
{
	_type = NUMBER;
	_d = (double)y;
}

Var::Var(bool y)
{
	_type=BOOL;
	_b=y;
}

Var::Var(double x)
	: _type(NUMBER), _d(x)
{
}

Var::Var(const char* y)
{
	int len = (int)strlen(y);
	if(len < VAR_SSPACE) {
		_type=SSTRING;
		memcpy(_ss, y, len + 1);
	}
	else {
		_type=STRING;
		NEW_STRINGC(_s, len + 1);
		memcpy(_s->data(), y, len + 1);
	}
}

Var::Var(char y)
{
	_type=INT; // int o string?
	_i=y;
}

Var::Var(const String& k, const Var& x)
{
	_type = OBJ;
	NEW_DIC(_o);
	_o->set(k, x);
}

Var::operator double() const
{
	switch(_type) {
	case NUMBER:
	case FLOAT:
		return _d;
	case INT:
		return _i;
	case STRING:
		return atof(_s->data());
	case SSTRING:
		return atof(_ss);
	case NUL:
		return nan();
	default:
		return 0.0; // nan?
	}
}

Var::operator float() const
{
	switch(_type) {
	case NUMBER:
	case FLOAT:
		return (float)_d;
	case INT:
		return (float)_i;
	case STRING:
		return (float)atof(_s->data());
	case SSTRING:
		return (float)atof(_ss);
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
		return _i;
	case NUMBER:
	case FLOAT:
		return (int)_d;
	case STRING:
		return atoi(_s->data());
	case SSTRING:
		return atoi(_ss);
	default: break;
	}
	return 0;
}

Var::operator unsigned() const
{
	switch(_type) {
	case INT:
		return (unsigned)_i;
	case NUMBER:
	case FLOAT:
		return (unsigned)_d;
	case STRING:
		return (unsigned)atoi(_s->data());
	case SSTRING:
		return (unsigned)atoi(_ss);
	default: break;
	}
	return 0;
}

Var::operator Long() const
{
	switch (_type) {
	case INT:
		return _i;
	case NUMBER:
	case FLOAT:
		return (Long)_d;
	case STRING:
		return (Long)atoi(_s->data());
	case SSTRING:
		return (Long)atoi(_ss);
	default: break;
	}
	return 0;
}

Var::operator String() const
{
	if(_type==STRING)
		return _s->data();
	if(_type==SSTRING)
		return _ss;
	return toString();
}

Var::operator bool() const
{
	switch (_type) {
	case BOOL:
		return _b;
	case INT:
		return _i != 0;
	case NUMBER:
	case FLOAT:
		return _d != 0;
	case ARRAY:
	case OBJ:
		return true;
	case STRING:
		return _s->length() > 1;
	case SSTRING:
		return _ss[0] != 0;
	case NUL:
		return false;
	default: return false;
	}
}

const char* Var::operator*() const
{
	switch(_type)
	{
	case STRING:
		return (_s->data()); break;
	case SSTRING:
		return _ss; break;
	case ARRAY:
		return "[?]"; break;
	case OBJ:
		return "{?}"; break;
	case NUL:
		return "null"; break;
	case NONE:
		return ""; break;
	case BOOL:
		return _b ? "true" : "false"; break;
	default: return "?";
	}
}

String Var::toString() const
{
	String r(15, 0);
	switch(_type) {
	case INT:
		r.fix(snprintf(r.data(), r.cap(), "%i", _i));
		break;
	case FLOAT:
		r.resize(16);
		r.fix(snprintf(r.data(), r.cap(), "%.7g", _d));
		break;
	case NUMBER:
		r.resize(29);
		r.fix(snprintf(r.data(), r.cap(), "%.15g", _d));
		break;
	case BOOL:
		r=_b?"true":"false";
		break;
	case STRING:
		r=_s->data();
		break;
	case SSTRING:
		r=_ss;
		break;
	case ARRAY:
		r << '[' << _a->join(',') << ']';
		break;
	case OBJ:
		r << '{' << _o->join(',', '=') << '}';
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
	case STRING: DEL_STRING(_s); break;
	case ARRAY: DEL_ARRAY(_a); break;
	case OBJ: DEL_DIC(_o); break;
	default: break;
	}
	_type=NONE;
}

void Var::operator=(const Var& v)
{
	if(_type == STRING && v._type == STRING) {
		_s->resize(v._s->length());
		memcpy(_s->data(), v._s->data(), v._s->length());
		return;
	}
	if(_type == ARRAY && v._type == ARRAY) {
		(*_a) = (*v._a);
		return;
	}
	if(_type == OBJ && v._type == OBJ) {
		(*_o) = (*v._o);
		return;
	}
	
	if(!isPod())
		free();
	memcpy((byte*)this, &v, sizeof(v));
	switch(_type)
	{
	case STRING:
		NEW_STRINGC(_s, v._s->length());
		memcpy(_s->data(), v._s->data(), v._s->length());
		break;
	case ARRAY:
		NEW_ARRAYC(_a, *v._a);
		break;
	case OBJ:
		NEW_DICC(_o, *v._o);
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
	_d=x;
}

void Var::operator=(int x)
{
	if(_type == NONE){}
	else
		free();
	_type=INT;
	_i=x;
}

void Var::operator=(Long x)
{
	if(_type == NONE){}
	else
		free();
	_type=NUMBER;
	_d=(double)x;
}

void Var::operator=(float x)
{
	if(_type == NONE){}
	else
		free();
	_type=FLOAT;
	_d=x;
}

void Var::operator=(unsigned x)
{
	if(_type == NONE){}
	else
		free();
	if (x & 0x80000000) {
		_type = NUMBER;
		_d = (double)x;
	}
	else {
		_type = INT;
		_i = (int)x;
	}
}

void Var::operator=(bool x)
{
	if(_type == NONE){}
	else
		free();
	_type=BOOL;
	_b=x;
}

void Var::operator=(const char* x)
{
	int n = (int)strlen(x);
	if (_type == STRING) {
		_s->resize(n + 1);
		memcpy(_s->data(), x, n + 1);
	}
	else if(_type==SSTRING && n < VAR_SSPACE)
		memcpy(_ss, x, n + 1);
	else
	{
		free();
		if(n < VAR_SSPACE)
		{
			_type = SSTRING;
			memcpy(_ss, x, n + 1);
		}
		else
		{
			_type=STRING;
			NEW_STRINGC(_s, n + 1);
			memcpy(_s->data(), x, n + 1);
		}
	}
}

void Var::operator=(const String& x)
{
	int len = x.length();
	Type t = _type;
	if(t==NONE) {}
	else if(t==STRING) {
		_s->resize(len + 1);
		memcpy(_s->data(), *x, len + 1);
		return;
	}
	else if(t==SSTRING && len < VAR_SSPACE) {
		memcpy(_ss, *x, len + 1);
		return;
	}

	{
		if(t==NONE) {}
		else
			free();
		if(len < VAR_SSPACE)
		{
			_type = SSTRING;
			memcpy(_ss, *x, len + 1);
		}
		else
		{
			_type=STRING;
			NEW_STRINGC(_s, len + 1);
			memcpy(_s->data(), *x, len + 1);
		}
	}
}

const Var& Var::operator[](int i) const
{
	if(_type==ARRAY)
		return (*_a)[i];

	return none;
}

Var& Var::operator[](int i)
{
	if (_type == ARRAY)
	{
		if (i >= _a->length())
			_a->resize(i + 1);
		return (*_a)[i];
	}
	else if (_type == OBJ)
	{
		return (*_o)[String(i)];
	}
	else if (_type == NONE)
	{
		_type = ARRAY;
		NEW_ARRAY(_a);
		_a->resize(i + 1);
		return (*_a)[i];
	}
	return *this;
}

Var& Var::operator[](const String& k)
{
	if (_type == NONE)
	{
		NEW_DIC(_o);
		_type = OBJ;
		return (*_o)[k];
	}
	else if (_type == OBJ)
		return (*_o)[k];
	else if (_type == ARRAY)
		return (*_a)[k];
	asl_error("Var[String] on non object");
	return *this;
}

const Var& Var::operator[](const String& k) const
{
	if(_type==OBJ)
		return (*_o)[k];

	return none;
}

int Var::length() const
{
	switch(_type) {
	case ARRAY:
		return _a->length(); break;
	case OBJ:
		return _o->length(); break;
	case STRING:
		return _s->length()-1; break;
	case SSTRING:
		return (int)strlen(_ss); break;
	default:
		break;
	}
	return 0;
}

Var& Var::operator<<(const Var& x)
{
	if(_type==ARRAY)
		(*_a) << x;
	else if(_type==NONE)
	{
		_type=ARRAY;
		NEW_ARRAY(_a);
		(*_a) << x;
	}
	return *this;
}

Var& Var::extend(const Var& v)
{
	if (_type == NONE)
	{
		NEW_DIC(_o);
		_type = OBJ;
	}
	
	if (_type == OBJ)
	{
		foreach2 (String& k, Var & x, *v._o)
		{
			if (x.ok())
				(*_o)[k] = x;
		}
	}
	return *this;
}

Var Var::clone() const
{
	Var v(*this);
	switch (_type)
	{
	case STRING:
		v._s->dup();
		break;
	case ARRAY:
		v._a->dup();
		foreach(Var& x, *v._a)
			x = x.clone();
		break;
	case OBJ:
		v._o->dup();
		foreach(Var& x, *v._o)
			x = x.clone();
		break;
	default:
		break;
	}
	return v;
}

}
