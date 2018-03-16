#include <asl/SerialPort.h>


namespace asl {

SerialPort::SerialPort()
{
	_handle = 0;
}

SerialPort::~SerialPort()
{
	if(_handle != 0)
		close();
}

String SerialPort::readLine()
{
	char c;
	String s;
	while(read(&c, 1) > 0 && c != '\r' && c != '\n' && s.length() < 1024)
		s += c;
	return s;
}

SerialPort& SerialPort::operator<<(const String& x)
{
	write(x);
	return *this;
}

#ifdef _WIN32

bool SerialPort::open(const String& port)
{
	_handle = CreateFile("\\\\.\\" + port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if(_handle == INVALID_HANDLE_VALUE)
		return false;

	return true;
}

void SerialPort::config(int bps, const char* mode)
{
	if(strlen(mode) < 3)
	{
		config(bps, "8N1");
		return;
	}
	DCB serialConfig;
	serialConfig.DCBlength = sizeof(DCB);
	GetCommState(_handle, &serialConfig);

	char p = mode[1];
	char s = mode[2];
	bool Xonoff = (strlen(mode) > 3 && mode[3] == 'X');

	serialConfig.BaudRate = bps;
	serialConfig.fBinary = TRUE;
	serialConfig.fParity = (p!='N')? TRUE : FALSE;
	serialConfig.ByteSize = mode[0]-'0';
	serialConfig.Parity	= (p=='N')? NOPARITY: (p=='O')? ODDPARITY : (p=='E')? EVENPARITY :
		(p=='M')? MARKPARITY : (p=='S')? SPACEPARITY : NOPARITY;
	serialConfig.StopBits = (s=='1')? ONESTOPBIT : (s=='2')? TWOSTOPBITS : ONE5STOPBITS;
	serialConfig.fOutxCtsFlow = FALSE;
	serialConfig.fOutxDsrFlow = FALSE;
	serialConfig.fDtrControl = DTR_CONTROL_ENABLE;
	serialConfig.fDsrSensitivity = FALSE;
	serialConfig.fTXContinueOnXoff = TRUE;
	serialConfig.fOutX = Xonoff? TRUE : FALSE;
	serialConfig.fInX = Xonoff? TRUE : FALSE;
	serialConfig.fErrorChar = FALSE;
	serialConfig.fNull = FALSE;
	serialConfig.fRtsControl = RTS_CONTROL_ENABLE;
	serialConfig.fAbortOnError = FALSE;

	if(!SetCommState(_handle, &serialConfig))
	{
		CloseHandle(_handle);
		return;
	}

	COMMTIMEOUTS timeouts;
	GetCommTimeouts(_handle,&timeouts);
	timeouts.ReadTotalTimeoutConstant   = 400;
	timeouts.ReadIntervalTimeout        = 20;
	timeouts.ReadTotalTimeoutMultiplier = 0; //MAXDWORD;
	SetCommTimeouts(_handle, &timeouts);

	SetupComm(_handle, 4096, 1024);
}

void SerialPort::setTimeout(int ms)
{
	COMMTIMEOUTS timeouts;
	GetCommTimeouts(_handle,&timeouts);
	timeouts.ReadTotalTimeoutConstant   = ms;
	timeouts.ReadIntervalTimeout        = 10;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	SetCommTimeouts(_handle, &timeouts);
}

bool SerialPort::waitInput(int timeout)
{
	if(!SetCommMask(_handle, EV_RXCHAR | EV_BREAK | EV_ERR))
		return false;
	DWORD eventMask;

	COMMTIMEOUTS timeouts;
	GetCommTimeouts(_handle, &timeouts);
	timeouts.ReadTotalTimeoutConstant   = timeout;
	timeouts.ReadIntervalTimeout        = 100;
	timeouts.ReadTotalTimeoutMultiplier = 1;
	SetCommTimeouts(_handle, &timeouts);

	if(WaitCommEvent(_handle, &eventMask, NULL))
		return true;
	else
		return false;
}

int SerialPort::available()
{
	COMSTAT stat;
	if(ClearCommError(_handle, NULL, &stat))
		return stat.cbInQue;
	else
		return 0;
}

int SerialPort::write(const void* p, int n)
{
	DWORD written;
	WriteFile(_handle, p, n, &written, 0);
	return written;
}

int SerialPort::read(void *p, int n)
{
	DWORD received;
	ReadFile(_handle, p, n, &received, 0);
	return received;
}

void SerialPort::close()
{
	CloseHandle(_handle);
}

#else // Linux

}

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>

namespace asl {

bool SerialPort::open(const String& port)
{
	_handle = ::open(port, O_RDWR, S_IRUSR | S_IWUSR);

	if(_handle == 0)
		return false;

	return true;
}

void SerialPort::config(int bps, const char* mode)
{
	int bps0[20]={0, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400,
		460800, 500000, 576000, 921600, 1000000};
	int bps1[20]={B0, B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200,
		0, 0, 0, 0, 0};
#ifdef B230400
	bps1[11] = B230400;
#endif
#ifdef  B460800
	bps1[12] = B460800;
#endif
#ifdef  B500000
	bps1[13] = B500000;
#endif
#ifdef  B576000
	bps1[14] = B576000;
#endif
#ifdef  B921600
	bps1[15] = B921600;
#endif
#ifdef  B1000000
	bps1[16] = B1000000;
#endif
	for(int i=0; i<17; i++)
	{
		if(bps==bps0[i])
		{
			bps=bps1[i];
			break;
		}
	}
	
	struct termios tty;
	tcgetattr(_handle, &tty);

	cfsetospeed(&tty, (speed_t)bps);
	cfsetispeed(&tty, (speed_t)bps);
	
	unsigned bits = CS8;
	switch (mode[0]) // bits
	{
	case '5': bits = CS5; break;
  	case '6': bits = CS6; break;
  	case '7': bits = CS7; break;
  	case '8': default: bits = CS8; break;
	}

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | bits;
	tty.c_iflag &= ~(IGNBRK | INLCR | IGNCR | ICRNL | IXANY | INPCK | ISTRIP | IXON | IXOFF);
	tty.c_iflag |= (BRKINT | IGNPAR);
	tty.c_oflag &= ~OPOST;
	tty.c_lflag = ~(ICANON | ECHO | ISIG);
	tty.c_cflag |= CREAD | CLOCAL;
	tty.c_cflag &= ~CRTSCTS;

	if(mode[2] == '2')
		tty.c_cflag |= CSTOPB;
	else
		tty.c_cflag &= ~CSTOPB;
	
	tty.c_cc[VTIME] = 6;
	tty.c_cc[VMIN] = 1;

	bool Xonoff = (strlen(mode) > 3 && mode[3] == 'X');
	if (Xonoff)
		tty.c_iflag |= (IXON | IXOFF);
	else
		tty.c_iflag &= ~(IXON | IXOFF);
	
	tty.c_cflag &= ~(PARENB | PARODD);
	
	if (mode[1] == 'E') // parity
		tty.c_cflag |= PARENB;
	else if (mode[1] == 'O')
		tty.c_cflag |= PARODD;

	tcsetattr(_handle, TCSANOW, &tty);
}

int SerialPort::write(const void* p, int n)
{
	return ::write(_handle, p, n);
}

int SerialPort::read(void *p, int n)
{
	return ::read(_handle, p, n);
}

void SerialPort::close()
{
	::close(_handle);
}

#endif

}
