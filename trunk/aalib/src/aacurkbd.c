#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include "config.h"
#ifdef CURSES_KBDDRIVER
#ifdef USE_NCURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif
#ifdef GPM_MOUSEDRIVER
#include <gpm.h>
#endif
#include "aalib.h"
#include "aaint.h"
static int iswaiting;
static jmp_buf buf;
extern int __curses_is_up;
extern int __resized_curses;
#ifdef GPM_MOUSEDRIVER
extern int __curses_usegpm;
#endif
static int uninitcurses;
int __curses_x, __curses_y, __curses_buttons;
int __curses_keyboard;

#ifdef SIGWINCH
static void handler(int i)
{
	__resized_curses = 2;
	signal(SIGWINCH, handler);
	if (iswaiting)
		longjmp(buf, 1);
}
#endif
static int curses_init(struct aa_context *context, int mode)
{
	if (!__curses_is_up) {
		fflush(stdout);
		if ((initscr()) == NULL)
			return 0;
		__curses_is_up = 1;
		uninitcurses = 1;
	}
	__curses_keyboard = 1;
	cbreak();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
#ifdef SIGWINCH
	signal(SIGWINCH, handler);
#endif
#ifdef GPM_MOUSEDRIVER
	aa_recommendlowmouse("gpm");
#endif
	aa_recommendlowkbd ("linux");
	aa_recommendlowmouse("curses");
	return 1;
}
static void curses_uninit(aa_context * c)
{
	printf("uninitializing\n");
	keypad(stdscr, FALSE);
	nodelay(stdscr, FALSE);
#ifdef SIGWINCH
	signal(SIGWINCH, SIG_IGN);        /*this line may cause problem... */
#endif
	nocbreak();
	echo();
	__curses_keyboard = 0;
	nl();
	if (uninitcurses) {
		intrflush(stdscr, TRUE);
		wclear(stdscr);
		refresh();
		uninitcurses = 0;
		__curses_is_up = 0;
		endwin();
	}
}
static int curses_getchar(aa_context * c1, int wait)
{
	int retval = AA_UNKNOWN;
	static int delay = 0;
	static int lastchar = AA_UNKNOWN;
	static int newchar = AA_UNKNOWN;
	int c;
	if (newchar != AA_UNKNOWN) {
		retval = newchar;
		newchar = AA_UNKNOWN;
		lastchar = retval;
		return retval;
	}
	if (lastchar != AA_UNKNOWN) {
		if (++delay == 10) {
			delay = 0;
			retval = lastchar;
			lastchar = AA_UNKNOWN;
			return (retval | AA_RELEASE);
		}
	} else {
		delay = 0;
	}


	if (wait) {
		nodelay(stdscr, FALSE);
		setjmp(buf);
		iswaiting = 1;
	} else
		nodelay(stdscr, TRUE);
	if (__resized_curses == 2) {
		__resized_curses = 1;
		return (AA_RESIZE);
	}
#ifdef GPM_MOUSEDRIVER
	if (__curses_usegpm) {
		c = Gpm_Wgetch(stdscr);
	} else
#endif
		c = wgetch(stdscr);

	switch (c) {
	case 27:
		retval = AA_ESC;
		break;
	case ERR:
		return (AA_NONE);
		break;
	case KEY_LEFT:
		retval = (AA_LEFT);
		break;
	case KEY_RIGHT:
		retval = (AA_RIGHT);
		break;
	case KEY_UP:
		retval = (AA_UP);
		break;
	case KEY_DOWN:
		retval = (AA_DOWN);
		break;
	case 96:	// backtick `
		retval = 41;
		break;
#ifdef KEY_MOUSE
	case KEY_MOUSE:
#ifdef GPM_MOUSEDRIVER
		if (!__curses_usegpm)
#endif
		{
			MEVENT m;
			if (getmouse(&m) == OK) {
				__curses_x = m.x;
				__curses_y = m.y;
			}
			if (m.bstate & BUTTON1_PRESSED)
				__curses_buttons |= AA_BUTTON1;
			if (m.bstate & BUTTON1_RELEASED)
				__curses_buttons &= ~AA_BUTTON1;
			if (m.bstate & BUTTON2_PRESSED)
				__curses_buttons |= AA_BUTTON2;
			if (m.bstate & BUTTON2_RELEASED)
				__curses_buttons &= ~AA_BUTTON2;
			if (m.bstate & BUTTON3_PRESSED)
				__curses_buttons |= AA_BUTTON3;
			if (m.bstate & BUTTON3_RELEASED)
				__curses_buttons &= ~AA_BUTTON3;
		}
		return (AA_MOUSE);
		break;
#endif
	case KEY_BACKSPACE: //263
	case 127:
		retval = (AA_BACKSPACE);
		break;
	default:
		if (c > 0 && c < 127)
			retval = c;
		break;
	}

	if (retval != AA_UNKNOWN && lastchar != AA_UNKNOWN) {
		newchar = retval;
		retval = lastchar;
		lastchar = AA_UNKNOWN;
		return (retval | AA_RELEASE);
	}

	lastchar = retval;
	return (retval);
}


__AA_CONST struct aa_kbddriver kbd_curses_d =
{
	"curses", "Curses keyboard driver 1.0",
	0,
	curses_init,
	curses_uninit,
	curses_getchar,
};
#endif
