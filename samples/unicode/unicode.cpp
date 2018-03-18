#include <asl/String.h>
#include <asl/File.h>
#include <asl/TabularDataFile.h>
#include <asl/CmdArgs.h>
#include <asl/Http.h>
#include <stdio.h>

using namespace asl;

/*
This program is used to generate the src/unicodedata.cpp file. It contains lookup tables to
help convert UTF8 to uppercase or lowercase. Tables are generated from the Unicode data tables
which can automatically downloaded from:
http://www.unicode.org/Public/9.0.0/ucd/UnicodeData.txt
Columns: 0 code, 12 upper, 13 lower

If the program is run with no arguments it will load ./UnicodeData.txt if present.
If it is run with an argument, it will load that file or download the URL if it starts with http:
If it is run with "-www" it will download and process the above URL
*/

double processFile(const String& fname)
{
	TabularDataFile file(fname);
	file.readAs("hssisssssssshhh");
	Array<Array<Var> > all = file.data();

	if(all.length() == 0)
	{
		printf("Can't find %s\n", *fname);
		return 0;
	}
	
	TextFile src("unicodedata.cpp", File::WRITE);
	
	src << "\n\n";
	src << "namespace asl { \n";

	src << "char toUppercaseU8[]=\"";
	for(int i=0, j=0; i<1415; i++, j++)
	{
		int code = all[i][0];
		while(code != j)
		{
			src.printf("\\xc9\\x83");
			j++;
		}
		int uc = all[i][12];
		if (uc > 0x4015) {
			wchar_t w[] = { (wchar_t)code, 0 };
			wchar_t wuc[] = { (wchar_t)uc, 0 };
			printf("Overflow %x %s uppercase %s\n", code, *String(w), *String(wuc));
		}
		if (uc == 0 || uc > 0x4015)
			uc = code;
		wchar_t wuc[] = {(wchar_t)uc, 0};
		char uc8[3] = {0, 0, 0};
		utf16toUtf8(wuc, uc8, 3);
		if((i % 16)==0)
			src << "\"\n\t\"";
		src.printf("\\x%02x\\x%02x", (unsigned char)uc8[0], (unsigned char)uc8[1]);
	}
	src << "\";\n";

	src << "char toLowercaseU8[]=\"";
	for(int i=0, j=0; i<1415; i++, j++)
	{
		int code = all[i][0];
		while(code != j)
		{
			src.printf("\\xc9\\x83");
			j++;
		}
		int lc = all[i][13];
		if (lc > 0x4015) {
			wchar_t w[] = { (wchar_t)code, 0 };
			wchar_t wuc[] = { (wchar_t)lc, 0 };
			printf("Overflow %x %s lowercase %s\n", code, *String(w), *String(wuc));
		}

		if (lc == 0 || lc > 0x4015)
			lc = code;
		wchar_t wlc[] = { (wchar_t)lc, 0 };
		char lc8[3] = {0, 0, 0};
		utf16toUtf8(wlc, lc8, 3);
		if((i % 16)==0)
			src << "\"\n\t\"";
		src.printf("\\x%02x\\x%02x", (unsigned char)lc8[0], (unsigned char)lc8[1]);
	}
	src << "\";\n";

	src << "}\n";
	return 0;
}


int main(int argc, char* argv[])
{
	CmdArgs args(argc, argv);
	String file = (args.length()==0)? "UnicodeData.txt" : args[0];
	String url = "http://www.unicode.org/Public/9.0.0/ucd/UnicodeData.txt";
	
	if (args.length() > 0 && args[0].startsWith("http:"))
	{
		url = args[0];
	}
	if (args.has("www") || args[0].startsWith("http:"))
	{
		printf("Downloading %s\n", *url);
		file = "unicodedata.txt";
		HttpResponse res = Http::get(url);
		if (res.ok()) {
			File(file).put(res.body());
		}
		else
		{
			printf("Cannot download file\n");
			return 1;
		}
	}
	processFile(file);
}
