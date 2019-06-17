// Copyright(c) 1999-2019 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_CONSOLE_H
#define ASL_CONSOLE_H

#include <asl/defs.h>
#ifdef _WIN32
#include <windows.h>
#endif

namespace asl {

/**
Helper class to control the text console, change output text color and cursor position.

~~~
Console console;
console.clear();
console.gotoxy(10, 5);
console.color(Console::BRED); // bright red
console.bgcolor(Console::GREEN);
cout << "Warning: " << warn_message << endl;
console.reset();
~~~
*/

class ASL_API Console
{
#ifdef _WIN32
	HANDLE _handle;
	int _w, _h, _fullh;
	WORD _defaultAttrib, _attrib;
#endif

public:
	/**
	Colors for text or background, there are normal and bright/bold versions (prefixed by a B):
	`RED, GREEN, BLUE, WHITE, MAGENTA, CYAN, YELLOW, BLACK, BRIGHT,
	BRED, BGREEN, BBLUE, BWHITE, BMAGENTA, BCYAN, BYELLOW, BBLACK`
	*/
	enum Color { DEFAULT, RED, GREEN, BLUE, WHITE, MAGENTA, CYAN, YELLOW, BLACK, BRIGHT=16,
		BRED, BGREEN, BBLUE, BWHITE, BMAGENTA, BCYAN, BYELLOW, BBLACK
	};
	struct Size {
		int w, h;
		bool operator!=(const Size& s) const { return w != s.w || h != s.h; }
	};

	Console();
	~Console();
	/** Sets the cursor position to coordinates `x`, `y` */
	void gotoxy(int x, int y);
	/** Clear the console */
	void clear();
	/** Sets the current text output color, or if no argument is given, the color will be the default terminal
	text color */
	void color(Color color = DEFAULT);
	/**
	Sets the current output background color
	*/
	void bgcolor(Color color = DEFAULT);
	/**
	Reverses text and background colors
	*/
	void inverse(bool on = true);
	/**
	Resets attributes to defaults
	*/
	void reset();
	/** Returns the size of the console window in characters */
	Size size();
};

extern ASL_API Console console;

// trick to get console constructed just by including this file

static Console _myconsole_ = console;
}
#endif
