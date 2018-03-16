#undef __STRICT_ANSI__
#include <asl/Console.h>
#include <asl/defs.h>

namespace asl {


#ifndef __ANDROID__
ASL_API Console console;
#endif

#ifdef _WIN32

Console::Console()
{
#ifndef ASL_ANSI
	SetConsoleOutputCP(65001);
#endif
	_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(_handle, &info);
	_defaultAttrib = _attrib = info.wAttributes;
	_fullh = info.dwSize.Y;
	_h = info.srWindow.Bottom - info.srWindow.Top;
	_w = info.dwMaximumWindowSize.X;
}

Console::~Console()
{
	color();
	bgcolor();
}

void Console::gotoxy(int x, int y)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(_handle, &info);
	COORD coord = {(short)x, (short)(y+info.srWindow.Top)};
	SetConsoleCursorPosition(_handle, coord);
}

void Console::clear()
{
	COORD coord = {0, 0};
	DWORD n;
	size();
	FillConsoleOutputCharacterW(_handle, L' ', _w*_fullh, coord, &n);
	gotoxy(0, 0);
}

void Console::color(Color color)
{
	bool bright = (color & BRIGHT) != 0;
	color = (Color)(color & 0x0f);
	WORD attr = _attrib;
	if(color == DEFAULT)
		attr = _defaultAttrib;
	else {
		WORD red = color == RED || color == MAGENTA || color==YELLOW || color==WHITE;
		WORD green = color == GREEN || color == CYAN || color==YELLOW || color==WHITE;
		WORD blue = color == BLUE || color == CYAN || color==MAGENTA || color==WHITE;
		attr = (red? FOREGROUND_RED:0) | (green? FOREGROUND_GREEN:0) | (blue? FOREGROUND_BLUE:0);
	}
	if (bright)
		attr |= FOREGROUND_INTENSITY;
	_attrib = (_attrib & ~0x0f) | (attr & 0x0f);
	SetConsoleTextAttribute(_handle, _attrib);
}

void Console::bgcolor(Color color)
{
	WORD attr = _attrib;
	if (color == DEFAULT)
		attr = _defaultAttrib;
	else {
		WORD red = color == RED || color == MAGENTA || color == YELLOW || color == WHITE;
		WORD green = color == GREEN || color == CYAN || color == YELLOW || color == WHITE;
		WORD blue = color == BLUE || color == CYAN || color == MAGENTA || color == WHITE;
		attr = (red ? BACKGROUND_RED : 0) | (green ? BACKGROUND_GREEN : 0) | (blue ? BACKGROUND_BLUE : 0);
	}
	_attrib = (_attrib & ~0xf0) | (attr & 0xf0);
	SetConsoleTextAttribute(_handle, _attrib);
}

void Console::inverse(bool on)
{
	WORD a = _attrib, b;
	b = ((a & 0x0f) << 4) | ((a & 0xf0) >> 4);
	SetConsoleTextAttribute(_handle, on? b : a);
}

void Console::reset()
{
	_attrib = _defaultAttrib;
	SetConsoleTextAttribute(_handle, _attrib);
}

Console::Size Console::size()
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(_handle, &info);
	_fullh = info.dwSize.Y;
	_h = info.srWindow.Bottom - info.srWindow.Top;
	_w = info.dwMaximumWindowSize.X;
	Console::Size s = {_w, _h};
	return s;
}


#else

}

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>

namespace asl {

static void sigint_handler(int sig)
{
	exit(0);
}

Console::Console()
{
#ifndef __ANDROID__
	if (signal(SIGINT, sigint_handler) == SIG_ERR)
		printf("Error\n");
	if (signal(SIGTERM, sigint_handler) == SIG_ERR)
		printf("Error\n");
#endif
}

Console::~Console()
{
	color();
}

void Console::gotoxy(int x, int y)
{
	printf("\033[%i;%iH", y, x);
}

void Console::clear()
{
	printf("\033[2J");
	gotoxy(0, 0);
}

void Console::color(Color color)
{
	bool bright = (color & BRIGHT);
	color = (Color)(color & 0x0f);
	const char* attr =
		color==BLACK? "30":
		color==RED? "31":
		color==GREEN? "32":
		color==YELLOW? "33":
		color==BLUE? "34":
		color==MAGENTA? "35":
		color==CYAN? "36":
		color==WHITE? "37":
		bright? "1":
		"0";
	if(bright)
		printf("\033[1m");
	else
		printf("\033[22m");
	printf("\033[%sm", attr);
}

void Console::bgcolor(Color color)
{
	const char* attr =
		color==BLACK? "40":
		color==RED? "41":
		color==GREEN? "42":
		color==YELLOW? "43":
		color==BLUE? "44":
		color==MAGENTA? "45":
		color==CYAN? "46":
		color==WHITE? "47":
		"49";
	printf("\033[%sm", attr);
}

void Console::inverse(bool on)
{
	printf(on? "\033[7m" : "\033[0m");
}

void Console::reset()
{
	printf("\033[0m");
}

Console::Size Console::size()
{
	struct WinSize
	{
		unsigned short ws_row;
		unsigned short ws_col;
		unsigned short ws_xpixel;
		unsigned short ws_ypixel;
	};
	WinSize win;
	
	Console::Size s = {0, 0};

	if (ioctl(0, TIOCGWINSZ, &win) == 0)
	{
		s.h = win.ws_row;
		s.w = win.ws_col;
	}
	return s;
}

#endif

}

