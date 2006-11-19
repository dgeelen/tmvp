#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include "config.h"
#ifdef CURSES_MOUSEDRIVER
#ifdef USE_NCURSES
#ifdef RENAMED_NCURSES
#include <curses.h>
#else
#include <ncurses.h>
#endif
#else
#include <curses.h>
#endif
#include "aalib.h"
#include "aaint.h"
extern int __curses_is_up;
extern int __curses_keyboard;
extern int __curses_x, __curses_y, __curses_buttons;

static int curses_init(struct aa_context *context, int mode)
{
	if (!__curses_is_up || !__curses_keyboard)
		return 0;

	if (!mousemask( /*ALL_MOUSE_EVENTS */ BUTTON1_PRESSED | BUTTON1_RELEASED | BUTTON2_PRESSED | BUTTON2_RELEASED | BUTTON3_PRESSED | BUTTON3_RELEASED | REPORT_MOUSE_POSITION, NULL))
		return 0;

	mouseinterval(0);

	return 1;
}

static void curses_uninit(aa_context * c)
{
	mousemask(0, NULL);
}

static void curses_mouse(aa_context * c, int *x, int *y, int *b)
{
	// bit of a hack here to work around a bug in aalib sdl driver
	// TODO: should really be fixed there and not here...
	static int prev_x = -1, prev_y = -1, prev_b = 0;
	static int count = 10;
	*x = __curses_x;
	*y = __curses_y;
	*b = prev_b;
	if (*x == prev_x && *y == prev_y) {
		if (--count == 0) {
			count = 10;
			*b = __curses_buttons;
		};
	} else {
		count = 10;
	}

//	fprintf(stderr, "curses: (%i,%i,%i)\n", *x, *y, *b);
	prev_x = *x;
	prev_y = *y;
	prev_b = *b;
}

__AA_CONST struct aa_mousedriver mouse_curses_d =
{
	"curses", "Curses mouse driver 1.0",
	0,
	curses_init,
	curses_uninit,
	curses_mouse,
	NULL,
};
#endif
