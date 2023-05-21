#include <asl/IniFile.h>
#include <asl/TextFile.h>

namespace asl {

IniFile::Section IniFile::Section::clone() const
{
	IniFile::Section s;
	s._title = _title;
	s._vars = _vars.clone();
	return s;
}

IniFile::IniFile(const String& name, bool shouldwrite)
{
	_modified = false;
	_filename = name;
	_currentTitle = "-";
	_ok = false;
	_shouldwrite = shouldwrite;
	TextFile file(name, File::READ);
	if(!file) {
		return;
	}
	_ok = true;
	while(!file.end())
	{
		String line=file.readLine();
		if (shouldwrite)
			_lines << line;
		if(!line.ok())
			continue;
		if(file.end())
			break;

		int i0 = 0;
		while (myisspace(line[i0]) && line[i0] != '\0')
			i0++;

		if (line[0] == '[')
		{
			int end = line.indexOf(']', 1);
			if(end < 0)
				continue;
			String name = line.substring(1, end);
			_currentTitle = name;
			_sections[_currentTitle] = Section(_currentTitle);
		}
		else if(line[i0] != '#' && line[i0] > 47 && line[i0] != ';')
		{
			int i = line.indexOf('=');
			if (i < 1)
			{
				if (_lines.length() > 0 && _lines.last().ok())
					_lines.resize(_lines.length() - 1);
				continue;
			}
			if (!_indent.ok())
				for (int i = 0; i < line.length(); i++)
				{
					if (myisspace(line[i]))
						_indent += line[i];
					else
						break;
				}
			String key = line.substring(0,i).trimmed();
			String value = line.substring(i+1).trimmed();
			key.replaceme('/', '\\');
			_sections[_currentTitle][key] = value;
		}
	}
	_currentTitle = "-";
	for(int i=_lines.length()-1; i>0 && _lines[i][0] == '\0'; i--)
	{
		_lines.resize(_lines.length()-1);
	}
 
}

IniFile::Section& IniFile::section(const String& name)
{
	_currentTitle = name;
	_sections[name]._title = name;
	return _sections[name];
}

String& IniFile::operator[](const String& name)
{
	int slash = name.indexOf('/');
	if(slash < 0)
	{
		_sections[_currentTitle]._title= _currentTitle;
		return _sections[_currentTitle][name];
	}
	else
	{
		String sec = name.substring(0, slash);
		Section& section = _sections[sec];
		if (!section._title.ok())
			section._title = sec;
		return section[name.substring(slash + 1)];
	}
}

const String IniFile::operator[](const String& name) const
{
	int slash = name.indexOf('/');
	if (slash < 0)
	{
		return _sections[_currentTitle].has(name) ? _sections[_currentTitle][name] : String();
	}
	else
	{
		String sec = name.substring(0, slash);
		if (!_sections.has(sec))
			return String();
		const Section& section = _sections[sec];
		sec = name.substring(slash + 1);
		return section.has(sec) ? section[sec] : String();
	}
}

const String IniFile::operator()(const String& name, const String& defaultVal) const
{
	return has(name)? (*this)[name] : defaultVal;
}

bool IniFile::has(const String& name) const
{
	int slash = name.indexOf('/');
	if(slash < 0)
	{
		return _sections[_currentTitle]._vars.has(name);
	}
	else
	{
		String sec = name.substring(0, slash);
		if(!_sections.has(sec))
			return false;
		return _sections[sec]._vars.has(name.substring(slash+1));
	}
}

void IniFile::write(const String& fname)
{
	Dic<Section> newsec = _sections.clone();
	foreach(Section & s, newsec)
	 	s = s.clone();

	Section* section = &_sections["-"];

	Array<String> oldlines = _lines.clone();

	foreach(String& line, _lines)
	{
		int i0 = 0;
		while (myisspace(line[i0]) && line[i0] != '\0')
			i0++;
		if (line[0] == '[')
		{
			int end = line.indexOf(']');
			if (end < 0)
				continue;
			String name = line.substring(1, end);
			_currentTitle = name;
			section = &_sections[name];
		}
		else if (line[i0] != '#' && line[i0] > 47 && line[i0] != ';')
		{
			int i = line.indexOf('=');
			if (i < 0)
				continue;
			String key = line.substring(0,i).trimmed();
			String value0 = line.substring(i+1).trimmed();
			const String& value1 = (*section)[key];
			line = _indent; line << key << '=' << value1;
			if(value0 != value1 && value1 != "")
			{
				_modified = true;
			}

			newsec[_currentTitle]._vars.remove(key);
		}
	}

	foreach(Section& section, newsec)
	{
		bool empty = true;
		foreach(String& value, section._vars)
		{
			if(value != "")
				empty = false;
		}
		if(empty)
			section._title = "";
	}

	foreach(Section& section, newsec)
	{
		if(section._vars.length()==0 || section._title == "")
			continue;
		int j=-1;
		for(int i=0; i<_lines.length(); i++)
		{
			if(section.title()=="-" || _lines[i]=='['+section.title()+']')
			{
				int k = (section.title()=="-")? 0 : 1;
				for(j=i+k; j<_lines.length() && _lines[j][0]!='['; j++)
				{}
				j--;
				if(j>0)
					while(_lines[j][0]=='\0') j--;
				j++;
				//_lines.insert(j, "");
				break;
			}
		}
		if(j==-1)
		{
			j=_lines.length()-1;
			if(j>0)
				while(_lines[j][0]=='\0') j--;
			j++;
			_lines.insert(j++, "");
			_lines.insert(j++, String(0, "[%s]", *section.title()));
		}

		foreach2(String& name, String& value, section._vars)
		{
			String line = _indent;
			line << name << "=" << value;
			_lines.insert(j++, line);
			_modified = true;
		}
	}
	
	if(_modified)
	{
		TextFile file ((fname.ok())? fname: _filename, File::WRITE);
		if(!file)
			return;
		foreach(String& line, _lines)
		{
			file.printf("%s\n", *line);
		}
		file.close();
	}

	_lines = oldlines;
}

IniFile::~IniFile()
{
	if(_shouldwrite)
		write(_filename);
}

int IniFile::arraysize(const String& name) const
{
	_currentTitle = name;
	return _sections[_currentTitle]["size"];
}

String IniFile::array(const String& name, int index) const
{
	String key = index + 1;
	key << '\\' << name;
	return _sections[_currentTitle][key];
}

Dic<> asl::IniFile::values() const
{
	Dic<> vals;
	foreach2 (String& secname, const Section& sec, _sections)
		foreach2 (String& k, const String& v, sec._vars)
			vals[secname + '/' + k] = v;
	return vals;
}

Dic<> asl::IniFile::values(const String& secname) const
{
	Dic<> vals;
	if (!_sections.has(secname))
		return vals;
	const Section& sec = _sections[secname];
	foreach2 (String& k, const String& v, sec._vars)
		vals[k] = v;
	return vals;
}

}
