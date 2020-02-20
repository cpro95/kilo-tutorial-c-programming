/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/
struct termios orig_termios;

/*** terminal ***/
void die(const char *s)
{
	perror(s);
	exit(1);
}

void disableRawMode()
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode()
{
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
		die("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = orig_termios;

	/*
		disable list
		IXON : Ctrl-S, Ctrl-Q 
		ICRNL : Ctrl-M (carriage return and new line: CRNL)
		BRKINT : break condition
		INPCK : parity checking
		ISTRIP : 8th bit to strip
	*/
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	/*
		disable list
		OPOST: "\n" => "\r\n" fix
	*/
	raw.c_oflag &= ~(OPOST);

	/*
		disable list
		CS8 : setting character size to 8bit 
	*/
	raw.c_cflag |= ~(CS8);

	/* 
	 disable list
	 ECHO : print to terminal when each key you typed
	 IEXTEN : Ctrl-V
	 ICANON : canonical mode
	 ISIG : Ctrl-C, Ctrl-Z
	*/
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	/*
	VMIN and VTIME come from <termios.h>.
	They are indexes into the c_cc field, which stands for “control characters”,
	an array of bytes that control various terminal settings.
	The VMIN value sets the minimum number of bytes of input needed before read() can return.
	We set it to 0 so that read() returns as soon as there is any input to be read.
	The VTIME value sets the maximum amount of time to wait before read() returns.
	It is in tenths of a second, so we set it to 1/10 of a second, or 100 milliseconds.
	If read() times out, it will return 0, which makes sense because its usual return value is the number of bytes read.
	*/
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
}

/*** init ***/

int main(void)
{
	enableRawMode();

	while (1)
	{
		char c = '\0';

		// In Cygwin, when read() times out it returns -1 with an errno of EAGAIN, instead of just returning 0 like it’s supposed to.
		// To make it work in Cygwin, we won’t treat EAGAIN as an error.
		if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
		if (iscntrl(c))
		{
			printf("%d\r\n", c);
		}
		else
		{
			printf("%d ('%c')\r\n", c, c);
		}
		if (c == 'q')
			break;
	}

	return 0;
}
