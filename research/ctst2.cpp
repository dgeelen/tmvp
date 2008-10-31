#include <ncurses.h>
//#include <ncurses/term.h>
#include <locale.h>
#include "stdio.h"

int main(int argc, char** argv)
{
	initscr();
	printw("---NCurses Image Test---\n");

	if (!has_colors()) {
		printw("Error: no color support\n");
		getch();
		endwin();
		return 0;
	};

	start_color();

	if (COLORS != 8) {
		printw("Error: need 8 colors for this test\n");
		getch();
		endwin();
		return 0;
	};

	if (!can_change_color()) {
		printw("Warning: no pallete support\n");
	};

	// opening /proc/blinkbit thingy if it exist
	FILE *fbb = fopen("/proc/blinkbit", "wb");
	if (fbb) {
			fputc('0', fbb); // disable blinking
			fclose(fbb);
	}

//	setlocale(LC_ALL, "");

	unsigned short int image[4000];
	unsigned char pal[48];
	int ncattr[256];

	FILE *fp = fopen(argv[0], "rb");
	if (!fp) {
			printw("Error: image.aap not found\n");
			getch();
			endwin();
			return 0;
	}

	fread(image, sizeof(*image), 4000, fp);
	fread(pal, sizeof(*pal), 48, fp);

	fclose(fp);

	if (can_change_color()) {
		for (int i = 0; i < 8; ++i)
			init_color(i, pal[3*i+0] * 1000 / 64, pal[3*i+1] * 1000 / 64, pal[3*i+2] * 1000 / 64);
		COLORS = 16;
		for (int i = 8; i < 16; ++i)
			init_color(i, pal[3*i+0] * 1000 / 64, pal[3*i+1] * 1000 / 64, pal[3*i+2] * 1000 / 64);
		COLORS = 8;
	}

	for (int fg = 0; fg < 8; ++fg)
		for (int bg = 0; bg < 8; ++bg)
		{
			int pairnum = (7-fg) + (bg * 8);
			init_pair(pairnum , fg, bg);
			ncattr[(fg  )+((bg  )*16)] = COLOR_PAIR(pairnum);
			ncattr[(fg+8)+((bg  )*16)] = COLOR_PAIR(pairnum) | A_BOLD;
			ncattr[(fg  )+((bg+8)*16)] = COLOR_PAIR(pairnum)          | A_BLINK;
			ncattr[(fg+8)+((bg+8)*16)] = COLOR_PAIR(pairnum) | A_BOLD | A_BLINK;
		}

	for (int y =0; y < 50; ++y)
	{
		move(y, 0);
		for (int x =0; x < 80; ++x)
		{
			unsigned char ch = image[y*80 + x] & 0xFF;
			unsigned char at = image[y*80 + x] >> 8;
			addch(ch | ncattr[at]);
		}
	}

	refresh();		/* Print it on to the real screen */
	getch();			/* Wait for user input */

	endwin();			/* End curses mode			*/

	return 0;
}
