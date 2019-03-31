#include <asl/CmdArgs.h>
#include <asl/File.h>
#include <asl/TextFile.h>
#ifdef _WIN32
#include <shellapi.h>
#endif

using namespace asl;

inline bool asl_isdigit(char c)
{
	return c >= '0' && c <= '9';
}

#if defined _WIN32 || defined __linux__

CmdArgs::CmdArgs(const String& spec)
{
#ifdef _WIN32
	int nArgs;
	LPWSTR* arglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	/*TextFile log("i:/log.txt", TextFile::APPEND);
	log << *(String)GetCommandLineW() << " " << (int)wcslen(GetCommandLineW()) << "\n";
	log.printf("args: %i\n", nArgs);*/
	if( NULL == arglist )
		return;
	Array<String> a;
	for (int i = 0; i < nArgs; i++)
	{
		a << arglist[i];
		//log << (int)arglist[i][12] << "\n";
		//log << *(String)arglist[i] << "\n";
	}

	LocalFree(arglist);
	*this = CmdArgs(-nArgs, (Array<char*>)a);
#else
	File file("/proc/self/cmdline", File::READ);
	if(!file)
		return;
	char buffer[256];
	Array<char> all;
	int n;
	while((n=file.read(buffer, sizeof(buffer))) > 0)
	{
		all.append(Array<char>((char*)buffer, n));
		if( n <= sizeof(buffer))
			break;
	}
	if(all.length() == 0)
		return;
	int i=0, j=0;
	Array<String> a;
	while((j = all.indexOf('\0', i)) != -1)
	{
		a << String(&all[i]);
		i = j+1;
	}
	*this = CmdArgs(a.length(), (Array<char*>)a, spec);
#endif
}
#endif

CmdArgs::CmdArgs(int argc, char* argv[], const String& spec)
{
	int n = abs(argc);
	for(int i=0; i < n; i++)
	{
		_args << ((argc < 0) ? String(argv[i]) : localToString(argv[i]));
	}
	Array<String> flags;
	Array<String> options;
	Array<String> parts = spec.split(',');
	foreach(const String& s, parts)
	{
		if(s.contains(':'))
			options << s.substring(0, s.indexOf(':'));
		else
			flags << s;
	}
	for(int i=1; i< _args.length(); i++)
	{
		if(_args[i].startsWith('-'))
		{
			bool isflag = _args[i].endsWith('!');
			String opt = _args[i].substring(1, _args[i].length() - (isflag ? 1 : 0));
			if (!_unused.contains(opt))
				_unused << opt;
			bool nextIsOption = i < _args.length() - 1 && (_args[i + 1].startsWith('-') && !asl_isdigit(_args[i + 1][1]));
			if ((i<_args.length() - 1 && !nextIsOption) && !flags.contains(opt) && !isflag)
			{
				_multi[opt] << _args[i+1];
				_opts[opt] = _args[i+1];
				i++;
			}
			else if(isflag || !options.contains(opt))
			{
				_opts[opt] = "1";
			}
			else // option without value!
				return;
			_rest.clear();
		}
		else
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
	return _opts.has(opt) && String("1|true|yes|y|on").contains(*_opts[opt].toLowerCase());
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
	return _opts.has(opt)? _opts[opt] : def;
}

Array<String> CmdArgs::operator()(const String& opt) const
{
	use(opt);
	return _multi[opt];
}

String CmdArgs::operator[](int i) const
{
	return _rest[i];
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
