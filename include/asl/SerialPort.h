// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SERIAL_H
#define ASL_SERIAL_H

#include <asl/String.h>
#ifdef _WIN32
#include <windows.h>
#else
#define HANDLE int
#endif


namespace asl {

/**
SerialPort represents a serial port to communicate with. Still quite simple but working.

~~~
SerialPort port;
if(!port.open("COM1"))     // or "/dev/ttyS0" on Linux
	return;
port.config(9600, "8N1");  // 9600 bps, 8 bits, no parity, 1 stop bit
port.setTimeout(200);
port << "COMMAND\n";
String answer = port.readLine();
~~~
*/

class ASL_API SerialPort
{
	HANDLE _handle;
public:
	SerialPort();
	~SerialPort();
	/** Opens the port by name (Win32: `"COM%i"`, Linux: `"/dev/..."`) */
	bool open(const String& port);
	/** Closes the port */
	void close();
	/**
	Sets the read timeout in ms
	*/
	void setTimeout(int ms);
	/**
	Configures the port with a bitrate and a string encoded as "BPS":
	B=data bits, P=parity (N/E/O), S=stop bits, plus an optional X for Xon/Xoff flow control */
	void config(int bps, const char* mode="8N1");
	/** waits until there is data to read for a maximum time of `timeout` (Currently not working on Windows) */
	bool waitInput(int timeout = 0);
	bool canRead(int timeout = 0) { return waitInput(timeout); }
	/** Returns the number of bytes available for reading */
	int available();
	/** Writes n bytes of buffer p to the port */
	int write(const void* p, int n);
	/** Reads n bytes from the port into buffer p */
	int read(void* p, int n);
	/** Writes the given string to the port */
	int write(const String& s)
	{
		return write(*s, s.length());
	}
	/** Reads a text line from the port (up to a \\r or \\n character) and returns it not
	including the newline */
	String readLine();

	SerialPort& operator<<(const String& x);
};

}
#endif
