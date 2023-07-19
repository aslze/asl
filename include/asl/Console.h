// Copyright(c) 1999-2023 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_CONSOLE_H
#define ASL_CONSOLE_H

#include <asl/String.h>
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

In modern consoles, color can be set as RGB components:

~~~
console.gotoxy(10, 5);
console.bgcolor(255, 10, 10);
cout << "Text on red";
~~~

If many different colors are to be printed in sequence, better performance is achieved by
forming a string with all color-changing terminal codes followed by the text, and then printing
this string.

~~~
console.setColorMode(2); // use mode 1 for older consoles not supporting full color
console.gotoxy(10, 5);
String s;
s << console.bg() << console.rgb(255, 10, 10) << "Red ";
s << console.bg() << console.rgb(10, 50, 215) << "blue.";
printf("%s\n", *s);
~~~

_(This is still work in progress)_
*/
class ASL_API Console
{
#ifdef _WIN32
	HANDLE _handle;
	HANDLE _hinput;
	int _w, _h, _fullh;
	int    _cp0;
	WORD _defaultAttrib, _attrib;
#endif
	int _colorMode; // 0: basic, 1: 256, 2: truecolor
	bool _colorChanged;

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
	
	/**
	On Windows, sets the console output code page (if 0 given, uses the system code page, if -1 uses the default console page)
	*/
	void setCP(int cp = 0);
	
	/**
	Sets the cursor position to coordinates `x`, `y`
	*/
	void gotoxy(int x, int y);

	/** Clear the console */
	void clear();
	
	/**
	 * Sets color mode: 1=256 colors (RGB 6x6x6), 2=true color (RGB 24bit) for RGB color setting
	 * functions (only for relatively modern consoles)
	 */
	void setColorMode(int mode) { _colorMode = mode; }
	
	/**
	Sets the current text output color, or if no argument is given, the color will be the default terminal
	text color
	*/
	void color(Color color = DEFAULT);
	
	/**
	Sets the current output background color
	*/
	void bgcolor(Color color = DEFAULT);
	
	/**
	* Sets current text color as RGB (if supported)
	*/
	void color(int r, int g, int b);

	/**
	* Sets current text color as RGB (if supported)
	*/
	void bgcolor(int r, int g, int b);

	/**
	 * Returns the prefix code to set a background color
	 */
	String bg() const { return "\033[48;"; }
	
	/**
	 * Returns the prefix code to set a foreground color
	 */
	String fg() const { return "\033[38;"; }

	/**
	 * Returns terminal code to set an RGB color (or the closest if full RGB is not supported)
	 */
	String rgb(int r, int g, int b) const;

	/**
	Reverses text and background colors
	*/
	void inverse(bool on = true);

	/**
	Shows or hides the cursor
	*/
	void showCursor(bool on);

	/**
	Resets attributes to defaults
	*/
	void reset();
	/**
	Returns the size of the console window in characters {width, height}
	*/
	Size size();
};

#ifndef __ANDROID__
extern ASL_API Console console;

// trick to get console constructed just by including this file

static Console _myconsole_ = console;
#endif
}
#endif
