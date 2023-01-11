#include <asl/Array.h>
#include <asl/TextFile.h>
#include <asl/Directory.h>
#include <asl/Console.h>
#include <asl/Log.h>
#include <asl/CmdArgs.h>
#include <asl/Http.h>
#include <asl/util.h>
#include <asl/Path.h>
#include <asl/Xdl.h>
#include <math.h>
#include <stdio.h>

/*
This sample shows a few functionalities. See comments in each block.
*/

using namespace asl;

// Prints a string with given text and background colors

void printc(const String& s, Console::Color c, Console::Color bg)
{
	Console console;
	console.bgcolor(bg);
	console.color(c);
	printf("%s", *s);
}

int main(int narg, char* argv[])
{
	// Log a message to the console and to a file:

	ASL_LOG_I("Starting demo");

	// Extraction of Path parts:

	Path p = "c:\\dir1/dir2/file.jpg";

	printf("path: %s\n", *p);
	printf("dir: %s\n", *p.directory());
	printf("name: %s\n", *p.name());
	printf("ext: %s\n", *p.extension());
	printf("is JPEG: %s\n", p.hasExtension("jpeg|jpg")? "Y" : "N");

	File f(p);
	printf("dir: %s\n", *f.directory());

	printf("dir:\n%s\n%s\n", *File("a.txt").directory(), *Path("a.txt").directory());

	Path p1 = "../a/b.h";
	printf("absolute (%s) -> %s\n", *p1, *p1.absolute());
	
	Path p2 = "c:/a/b/c/d/../../e/f//file.h";
	printf("%s = %s\n", *p2, *p2.absolute());

#if 1

	// HTTP client request to a Web Service to get your IP info including geographic location:

	Var info = Http::get("http://ipinfo.io/").json();
	Array<double> location = info["loc"].string().split(",");
	printf("IP info:\n%s\n", *Xdl::encode(info, true));
	printf("You are at latitude %f longitude %f\n", location[0], location[1]);
#endif

#if 1

	// File system operations:

	// Create a file and a subdirectory (with Unicode characters)

	TextFile("newdata.txt").write("Hello\n");
	String subdir = "subdir-Ñλ€и";
	Directory::create(subdir);

	// copy the new file to the subdirectory:

	if(!Directory::copy("newdata.txt", subdir))
		printf("err\n");

	// copy and move files:

	Directory::copy("newdata.txt", "newdata2.txt");
	Directory::move("newdata2.txt", subdir);

	ASL_LOG_W("A subdirectory and files were created. Press a key to delete them.");

	getchar(); // wait a Return key press (in the mean time you can check that the files were created)

	// Remove the files and directories created:

	Directory::remove(subdir + "/newdata.txt");
	Directory::remove(subdir + "/newdata2.txt");
	Directory::remove(subdir);
#endif

#if 1

	// Print to the console with text and background colors:

	Console console;
	for (int i = 1; i < 9; i++)
	{
		printc("Hello   ", Console::Color(i), Console::DEFAULT);
		printc("Hello   ", Console::WHITE, Console::Color(i));
		printc("Hello   ", Console::Color(i | Console::BRIGHT), Console::DEFAULT);
		printf("\n");
	}
	console.color();
	printc("Hello RED\n", Console::BRED, Console::BLACK);
	printc("Hello red\n", Console::RED, Console::BLACK);
	
	console.color();
	console.inverse();
	printf("inverse ");
	console.inverse(false);
	printf("normal\n");
#endif
}
