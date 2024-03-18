// Copyright(c) 1999-2022 aslze
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
SerialPort represents a serial port to communicate with.

~~~
SerialPort port;
if(!port.open("COM1"))     // or "/dev/ttyS0", "/dev/ttyUSB0" ...  on Linux
	return;
port.config(9600, "8N1");  // 9600 bps, 8 bits, no parity, 1 stop bit
port.setTimeout(0.2);
port << "COMMAND\n";
String answer = port.readLine();
while(1)
{
	if(port.waitInput())
	{
		if(port.error()) // the device disconnected
			break;
		String info = port.readLine();
	}
}
~~~
*/

class ASL_API SerialPort
{
	bool _error;
	HANDLE _handle;
	String _nl;
public:
	SerialPort();
	~SerialPort();
	/**
	Opens the port by name (Windows: `"COM..."`, Linux: `"/dev/..."`)
	*/
	bool open(const String& port);

	/**
	Sets the newline to expect by readLine(), normally one of "\n", "\r" or "\r\n".
	*/
	void setNewline(const String& nl) { _nl = nl; }

	/**
	Closes the port
	*/
	void close();
	/**
	Returns true if there were communication errors (possibly the device was disconnected)
	*/
	bool error() { return _error; }
	/**
	Sets the read timeout in seconds
	*/
	void setTimeout(double s);
	/**
	Configures the port with a bitrate and a string encoded as "BPS":
	B=data bits, P=parity (N/E/O), S=stop bits, plus an optional X for Xon/Xoff flow control
	*/
	bool config(int bps, const String& mode="8N1");
	/**
	Waits until there is data to read for a maximum time of `timeout` seconds (or the device disconnected),
	and return true if something happened before the timeout
	*/
	bool waitInput(double timeout = 10);
	/**
	Returns the number of bytes available for reading
	*/
	int available();
	/**
	Writes n bytes of buffer p to the port
	*/
	int write(const void* p, int n);
	/**
	Reads n bytes from the port into buffer p
	*/
	int read(void* p, int n);

	ByteArray read(int n);

	/**
	Writes the given string to the port
	*/
	int write(const String& s)
	{
		return write(*s, s.length());
	}
	/**
	Reads a text line from the port (up to a \\r or \\n character) and returns it not
	including the newline
	*/
	String readLine();

	SerialPort& operator<<(const String& x);
};

}
#endif
