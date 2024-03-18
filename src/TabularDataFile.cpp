#include <asl/TabularDataFile.h>
#include <ctype.h>

#ifdef _MSC_VER
#pragma warning(disable : 26451 26812)
#endif

namespace asl
{
inline bool myisdigit(char c)
{
	return c >= '0' && c <= '9';
}

inline bool isnumber(const String& s, char dec)
{
	const char* p = s.data();
	int n = s.length();
	if (!myisdigit(p[0]) && p[0] != '-' && p[0] != dec)
		return false;
	for (int i = 1; i < n; i++)
	{
		char c = p[i];
		if (!myisdigit(c) && c != '-' && c != '+' && c != dec && c != 'e' && c != 'E')
			return false;
	}
	return true;
}

void TabularDataFile::init()
{
	_dataStarted = false;
	_separator = ',';
	_decimal = '.';
	_quote = '\"';
	_equote = String::repeat(_quote, 2);
	_flushEvery = 0;
	_rowIndex = 0;
	_quoteStrings = false;
}

TabularDataFile::TabularDataFile()
{
	init();
}

TabularDataFile::TabularDataFile(const String& filename)
{
	init();
	_name = filename;
}

TabularDataFile::TabularDataFile(const String& filename, const Array<String>& cols)
{
	init();
	_name = filename;
	columns(cols);
}

TabularDataFile& TabularDataFile::columns(int cols)
{
	_numCols = cols;
	return *this;
}

TabularDataFile& TabularDataFile::columns(const Array<String>& cols)
{
	if(_separator == '\0')
		_separator = ',';

	if(_columnNames.length() == 0)
	{
		_file.open(_name, File::WRITE);
		if(!_file)
		{
			return *this;
		}
		foreach(String& col, cols)
		{
			Array<String> parts = col.split(':');
			_columnNames << parts[0];
		}
	}
	if(_name.toLowerCase().endsWith(".arff"))
	{
		_quote = '\'';
		_equote = String::repeat(_quote, 2);
		int i1 = max(max(_name.lastIndexOf('/')+1, _name.lastIndexOf('\\')+1), 0);
		int i2 = _name.lastIndexOf('.');
		String rel = _name.substring(i1, i2);
		_file << "@relation " << rel << "\n\n";

		foreach(String& col, cols)
		{
			Array<String> parts = col.split(':');
			char typec = (parts.length()==1)? 'n' : parts[1][0];
			if(parts.length() > 1 && parts[1].length() > 1 && parts[1].contains('|'))
				typec = 'c';
			String type;
			switch(typec)
			{
			case 'n':
				type = "numeric";
				break;
			case 's':
				type = "string";
				break;
			case 'c':
				type = '{' + parts[1].replaceme('|', ',') + '}';
			}
			_file << "@attribute " << parts[0] << ' ' << type << '\n';
		}
		_file << "\n@data\n";
	}
	else
	{
		String header = _columnNames.join(_separator);
		_file << header;
	}
	return *this;
}

void TabularDataFile::readAs(const String& types)
{
	_types = types;
}

TabularDataFile& TabularDataFile::operator<<(const Var& x)
{
	bool rowFull = false;
	if (x == "\n" && _row.length() > 0)
		rowFull = true;
	else if (x.is(Var::ARRAY))
		_row = x.array();
	else
		_row << x;
	if (_row.length() == _columnNames.length() || rowFull)
	{
		String row;
		if(!_dataStarted)
		{
			row << '\n';
			_dataStarted = true;
		}
		for(int i=0; i<_row.length(); i++)
		{
			if(i>0)
				row << _separator;
			Var& item = _row[i];
			String value = item.ok() ? item.toString() : String();

			if (item.is(Var::NUMBER))
			{
				float y = item; // print nans empty
				if (y != y)
					continue;
			}
			if (_decimal != '.' && item.is(Var::NUMBER))
			{
				value.replaceme('.', _decimal);
				row << value;
			}
			else if (item.is(Var::STRING))
			{
				if (value.contains(_quote) || value.contains(_separator))
					row << _quote << value.replace(_quote, _equote) << _quote;
				else
					row << value;
			}
			else
				row << value;
		}
		row << '\n';
		_file << *row;
		if(++_rowIndex == _flushEvery)
		{
			_file.flush();
			_rowIndex = 0;
		}
		_row.clear();
	}
	return *this;
}
	
Array<Array<Var> > TabularDataFile::data()
{
	Array<Array<Var> > data;

	while(nextRow())
	{
		data << _row.clone();
	}
	return data;
}

bool TabularDataFile::readHeader()
{
	if (_file)
		return true;
	_file.open(_name, File::READ);
	if (!_file)
		return false;
	String& line = _currentLine;
	line = _file.readLine();
	
	if (line.length() >= 3 && line.startsWith("\xef\xbb\xbf")) // eat BOM
	    line = line.substr(3);
	
	if (line.contains(';'))
	{
		_separator = ';';
		_decimal = ',';
	}
	else if (line.contains(','))
		_separator = ',';
	else if (line.contains('\t'))
		_separator = '\t';

	Array<String> row = line.split(_separator);
	foreach2(int i, String& col, row)
	{
		if (isdigit(col[0]) || (col[0] == '-' && isdigit(col[1])))
		{
			_file.seek(0);
			col = i;
			break;
		}
	}
	_columnNames = row;
	return true;
}

enum State
{
	BASE,
	QUOTE,
	QUOTE2
};

bool TabularDataFile::nextRow()
{
	String& line = _currentLine;
	if(!_file)
	{
		if (!readHeader())
			return false;
	}
	if (_file.end() || !_file.readLine(line))
		return false;
	
	if (!_dataStarted && line.length() >= 3 && line.startsWith("\xef\xbb\xbf")) // eat BOM
		line = line.substr(3);
	
	Array<String>& row = _currentRowParts;

	row.clear();
	const char* p = line.data();
	char        c = ' ';
	State       state = BASE;
	String      value;
	while ((c = *p++))
	{
		switch (state)
		{
		case BASE:
			if (c == '"')
				state = QUOTE;
			else if (c == _separator)
			{
				row << value;
				value = "";
			}
			else
				value << c;
			break;
		case QUOTE:
			if (c != '"')
			{
				value << c;
			}
			else
				state = QUOTE2;
			break;
		case QUOTE2:
			if (c == '"')
			{
				value << c;
				state = QUOTE;
			}
			else if (c == _separator)
			{
				row << value;
				value = "";
				state = BASE;
			}
			else
				state = QUOTE;
			break;
		default:
			value << c;
		}
	}

	row << value;

	_row.clear();
	char decimal = _decimal;
	int ntypes = _types.length();
	
	foreach2(int i, String& v, row)
	{
		if(ntypes > i)
		{
			switch(_types[i])
			{
			case 'n':
				if (decimal != '.')
					v.replaceme(decimal, '.');
				_row << myatof(*v);
				break;
			case 's':
				_row << v;
				break;
			case 'i':
				_row << myatoi(*v);
				break;
			case 'h':
				_row << v.hexToInt();
				break;
			}
		}
		else
		{
			if (isnumber(v, decimal))
			{
				if (decimal != '.')
					v.replaceme(decimal, '.');
					_row << myatof(*v);
			}
			else
				_row << v;
		}
	}
	_dataStarted = true;
	return true;
}

Var TabularDataFile::operator[](int i) const
{
	if(i>=_row.length() || i < 0)
		return Var();
	return _row[i];
}

Var TabularDataFile::operator[](const String& col) const
{
	int i = _columnNames.indexOf(col);
	return i < 0? Var() : (*this)[i];
}

}
