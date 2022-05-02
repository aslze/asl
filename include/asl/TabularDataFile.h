// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_TABDATAFILE_H
#define ASL_TABDATAFILE_H

#include <asl/TextFile.h>
#include <asl/Array.h>
#include <asl/String.h>
#include <asl/Var.h>

namespace asl {

/**
This class allows reading/writing CSV files and writing ARFF files.
Files have an optional header with column names, and data rows that can contain
numbers (integer or floating point) or strings. When reading, the class tries to infer if there is a header and
what separator and decimal symbol are used.

A data row is written when it has as many elements as the header indicates. Make sure you match written data
to specified columns!

~~~{.cpp}
TabularDataFile file("data.csv");

file.columns("i,x,cos,sign");

for(int i=0; i<100; i++)
{
	float x = 0.1*i, y = cos(x);
	file << i << x << y << ((y<0)? "neg" : "pos");
}
~~~

Reading the file would be done as follows. This class will try to detect the separator used in the
read file (if ',', ';' or 'tab'), and will try to detect if there is a header line (when no numbers are found in it).

~~~{.cpp}
TabularDataFile file("data.csv");

while(file.nextRow())
{
	int i = file[0];         // Note you can use field indices
	float x = file["x"];     // or column names
	float y = file["sin"];
	String sign = file[3];
}
~~~

Or just as:

~~~{.cpp}
TabularDataFile file("data.csv");
Array<Array<Var>> dataset = file.data();
~~~

If a file uses a specific format that autodetection can't handle, use `readAs()` to sepecify column types.
That includes the ability to read numbers as hexadecimal. For example:

~~~{.cpp}
file.readAs("hsn");
~~~

will interpret a row like `20,20,20` as hexadecimal 0x20 (32), string "20" and number 20.0.

For writing ARFF files, just set a file name with a `.arff` extension and specify columns with optional types
(default is numeric). For nominal types, write classes separated with |, for strings use 's':

~~~{.cpp}
TabularDataFile file("data.arff");  // relation name will be "data"

file.columns("i,x,cos,sign:neg|pos,name:s");
~~~
*/

class ASL_API TabularDataFile
{
public:
	TabularDataFile();
	/**
	Creates a data file with name `filename` for reading or writing
	*/
	TabularDataFile(const String& filename);
	/**
	Creates a data file with name `filename` for writing with the given column names
	*/
	TabularDataFile(const String& filename, const Array<String>& cols);

	/**
	Defines the columns for the data with a comma separated list of column names
	*/
	TabularDataFile& columns(const String& cols) { return columns(cols.split(',')); }
	/**
	Defines the columns for the data
	*/
	TabularDataFile& columns(const Array<String>& cols);
	
	bool ok() {return !!_file;}

	bool readHeader();
	/**
	Returns the number of columns
	*/
	int numColumns() const
	{
		return _columnNames.length();
	}
	
	/**
	Returns the column names
	*/
	const Array<String>& columns()
	{
		if (_columnNames.length() == 0 && !_file)
			readHeader();
		return _columnNames;
	}
	/**
	Set field separator (',' by default)
	*/
	void setSeparator(char s)
	{
		_separator = s;
	}
	/**
	Set decimal separator (`dot` by default)
	*/
	void setDecimal(char d)
	{
		_decimal = d;
	}
	/**
	Makes string values be quoted when writing
	*/
	void useQuotes()
	{
		_quoteStrings = true;
	}
	/**
	Forces a real disk write every `nrows` number of rows.
	*/
	void flushEvery(int nrows)
	{
		_flushEvery = nrows;
	}
	/**
	Sets the types to read for each column as a string with chars:
	i:int, n:number, h:hex, s:string
	*/
	void readAs(const String& types);
	/**
	Appends one data item to the current row, and writes the row if it reaches the
	number of columns defined.
	*/
	TabularDataFile& operator<<(const Var& x);

	/**
	Returns the whole file contents as a matrix (array of arrays)
	*/
	Array<Array<Var> > data();
	/**
	Reads the next row and returns true if it succeded
	*/
	bool nextRow();
	/**
	Returns the value for column index `i` in the current row
	*/
	Var operator[](int i) const;
	/**
	Returns the value for column named `col` in the current row
	*/
	Var operator[](const String& col) const;
	/**
	*/
	const Array<Var>& row() const
	{
		return _row;
	}
protected:
	void init();
	mutable TextFile _file;
	Array<String> _columnNames;
	Array<Var> _row;
	String _currentLine;
	Array<String> _currentRowParts;
	String _name;
	String _types;
	char _separator, _decimal, _quote;
	bool _quoteStrings;
	bool _dataStarted;
	int _numCols, _currCol;
	int _flushEvery, _rowIndex;
};

}
#endif
