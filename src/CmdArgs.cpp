#include <asl/CmdArgs.h>
#include <asl/File.h>
#include <asl/TextFile.h>
#ifdef _WIN32
#include <shellapi.h>
#endif

namespace asl {

inline bool asl_isdigit(char c)
{
	return c >= '0' && c <= '9';
}

inline bool myisalpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

#if defined _WIN32 || defined __linux__

CmdArgs::CmdArgs(const String& spec)
{
#ifdef _WIN32
	int nArgs;
	LPWSTR* wargs = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (wargs == NULL)
		return;
	Array<String> a;
	for (int i = 0; i < nArgs; i++)
		a << wargs[i];

	LocalFree(wargs);
	parse(-nArgs, (char**)(a.with<const char*>()).data(), spec);
#else
	File file("/proc/self/cmdline", File::READ);
	if (!file)
		return;
	char buffer[256];
	Array<char> all;
	int n;
	while ((n = file.read(buffer, sizeof(buffer))) > 0)
	{
		all.append(Array<char>((char*)buffer, n));
		if (n <= sizeof(buffer))
			break;
	}
	if (all.length() == 0)
		return;
	int i = 0, j = 0;
	Array<String> a;
	while ((j = all.indexOf('\0', i)) != -1)
	{
		a << String(&all[i]);
		i = j + 1;
	}

	parse(a.length(), (char**)(a.with<const char*>()).data(), spec);
#endif
}
#endif


void CmdArgs::parse(int argc, char* argv[], const String& spec)
{
	int n = abs(argc);
	for (int i = 0; i < n; i++)
	{
#ifdef _WIN32
		_args << ((argc < 0) ? String(argv[i]) : String::fromLocal(argv[i]));
#else
		_args << argv[i];
#endif
	}
	Array<String> flags;
	Array<String> options;
	Array<String> parts = spec.split(',');
	foreach(const String & s, parts)
	{
		if (s.contains(':'))
			options << s.substring(0, s.indexOf(':'));
		else
			flags << s;
	}
	for (int i = 1; i < _args.length(); i++)
	{
		if ((_args[i].startsWith('-') && myisalnum(_args[i][1])) || (_args[i].startsWith("--") && myisalnum(_args[i][2])))
		{
			bool isflag = _args[i].endsWith('!');
			String opt = _args[i].substring(1, _args[i].length() - (isflag ? 1 : 0));
			if (!_unused.contains(opt))
				_unused << opt;
			bool nextIsOption = i < _args.length() - 1 && ((_args[i + 1].startsWith('-') && myisalpha(_args[i + 1][1])) || (_args[i + 1].startsWith("--") && myisalpha(_args[i + 1][2])));
			if ((i < _args.length() - 1 && !nextIsOption) && !flags.contains(opt) && !isflag)
			{
				_multi[opt] << _args[i + 1];
				_opts[opt] = _args[i + 1];
				i++;
			}
			else if (isflag || !options.contains(opt))
			{
				_opts[opt] = "1";
			}
			else // option without value!
				return;
		}
		else if (_args[i] != '-')
			_rest << _args[i];
	}
}

void CmdArgs::use(const String& opt) const
{
	_unused.removeOne(opt);
}

bool CmdArgs::is(const String& opt) const
{
	use(opt);
	return _opts.has(opt) && _opts[opt].isTrue();
}

bool CmdArgs::has(const String& opt) const
{
	use(opt);
	return _opts.has(opt);
}

String CmdArgs::operator[](const String& opt) const
{
	return (*this)(opt, "");
}

String CmdArgs::operator()(const String& opt, const String& def) const
{
	use(opt);
	return _opts.has(opt) ? _opts[opt] : def;
}

Array<String> CmdArgs::operator()(const String& opt) const
{
	use(opt);
	return _multi[opt];
}

String CmdArgs::operator[](int i) const
{
	return i < _rest.length() ? _rest[i] : String();
}

int CmdArgs::length() const
{
	return _rest.length();
}

Array<String> CmdArgs::rest() const
{
	return _rest;
}

Array<String> CmdArgs::all() const
{
	return _args;
}

}
