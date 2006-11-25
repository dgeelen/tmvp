#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include "config.h"
#ifdef CURSES_DRIVER
#ifdef USE_NCURSES
#ifdef RENAMED_NCURSES
#include <curses.h>
#else
#include <ncurses.h>
#endif
#else
#include <curses.h>
#endif
#ifdef GPM_MOUSEDRIVER
#include <gpm.h>
#endif
#include "aalib.h"
#include "aaint.h"
__AA_CONST struct aa_driver curses_d;
int __curses_is_up;
int __resized_curses;
static int uninitcurses;
static int ncattr[256];

extern int r_pal[16];

static int curses_init(__AA_CONST struct aa_hardware_params *p, __AA_CONST void *none, struct aa_hardware_params *dest, void **param)
{
	int a;
	if (!__curses_is_up) {
		fflush(stdout);
		if ((initscr()) == NULL)
			return 0;

		//fprintf(stderr, "has_color: %i\n", has_colors());
		start_color();
		__curses_is_up = 1;
		uninitcurses = 1;
	}
	dest->font=NULL;
	dest->supported=AA_NORMAL_MASK;
#ifdef HAVE_TERMATTRS
	a = termattrs();
	if (a & A_DIM)
		dest->supported |= AA_DIM_MASK;
	if (a & A_BOLD)
		dest->supported |= AA_BOLD_MASK;
	if (a & A_BOLD)
		dest->supported |= AA_BOLDFONT_MASK;
	if (a & A_REVERSE)
		dest->supported |= AA_REVERSE_MASK;
#else
	dest->supported |= AA_REVERSE_MASK | AA_DIM_MASK | AA_BOLD_MASK | AA_BOLDFONT_MASK;
#endif
	intrflush(stdscr, FALSE);
	aa_recommendlowkbd("curses");

	int fg, bg, i;
	for (fg = 0; fg < 8; ++fg)
		for (bg = 0; bg < 8; ++bg)
		{
			int pairnum = (7-fg) + (bg * 8);
			init_pair(pairnum , fg, bg);
			ncattr[(fg  )+((bg  )*16)] = COLOR_PAIR(pairnum);
			ncattr[(fg+8)+((bg  )*16)] = COLOR_PAIR(pairnum) | A_BOLD;
			ncattr[(fg  )+((bg+8)*16)] = COLOR_PAIR(pairnum)          | A_BLINK;
			ncattr[(fg+8)+((bg+8)*16)] = COLOR_PAIR(pairnum) | A_BOLD | A_BLINK;
		}

	/*
	if (can_change_color()) {
		refresh();

		for (i = 1; i < 16; ++i) {
			int r = (i%8)*32;
			int g = i < 8 ? 0 : (i%8)*32;
			int b = 0;

			if (i < COLORS)
				init_color(i, (r*999)/255, (g*999)/255, (b*999)/255);
			else
				fprintf(stdout, "\033]P%1x%02x%02x%02x", i, r, g, b);

			r_pal[i] = (r << 16) | (g << 8) | (b << 0);
		}
	}
	*/

	return 1;
}
static void curses_uninit(aa_context * c)
{
	wclear(stdscr);
	intrflush(stdscr, TRUE);
#if 0
	nocbreak();
	echo();
	nl();
#endif
	refresh();
	if (uninitcurses) {
		uninitcurses = 0;
		__curses_is_up = 0;
		endwin();
	}
}
static void curses_getsize(aa_context * c, int *width, int *height)
{
	if (__resized_curses)
		curses_uninit(c), curses_init(&c->params, NULL,&c->driverparams, NULL), __resized_curses = 0;
	*width = stdscr->_maxx + 1;
	*height = stdscr->_maxy + 1;
#ifdef GPM_MOUSEDRIVER
	gpm_mx = *width;
	gpm_my = *height;
#endif

}
static void curses_setattr(aa_context * c, int attr)
{
	//fprintf(stderr, "setting attr: %i,%i\n", attr, ncattr[attr]);
	attrset(ncattr[attr]);
	return;
	switch (attr) {
	case AA_NORMAL:
		attrset(A_NORMAL);
		break;
	case AA_DIM:
		attrset(A_DIM);
		break;
	case AA_BOLD:
		attrset(A_BOLD);
		break;
	case AA_BOLDFONT:
		attrset(A_BOLD);
		break;
	case AA_REVERSE:
		attrset(A_REVERSE);
		break;
	case AA_SPECIAL:
		attrset(A_REVERSE);
		break;
	}
}
static void curses_print(aa_context * c, __AA_CONST char *text)
{
	addstr(text);
}
static void curses_flush(aa_context * c)
{
	int i;
	if (can_change_color()) {
		for (i = 1; i < 16; ++i) {
			int r = (r_pal[i] >> 16) & 0xFF;
			int g = (r_pal[i] >>  8) & 0xFF;
			int b = (r_pal[i] >>  0) & 0xFF;

			if (i < COLORS) {
				init_color(i, (r*999)/255, (g*999)/255, (b*999)/255);
			} else {
				char buf[21];
				snprintf(buf, 20, "\033]P%1x%02x%02x%02x", i, r, g, b);
				putp(buf);
			}
		}
	}

	refresh();
}
static void curses_gotoxy(aa_context * c, int x, int y)
{
	move(y, x);
}
static void curses_cursor(aa_context * c, int mode)
{
#ifdef HAVE_CURS_SET
	curs_set(mode);
#endif
}



__AA_CONST struct aa_driver curses_d =
{
	"curses", "Curses driver 1.0",
	curses_init,
	curses_uninit,
	curses_getsize,
	curses_setattr,
	curses_print,
	curses_gotoxy,
	/*curses_getchar,
	   NULL, */
	curses_flush,
	curses_cursor,
};
#endif
