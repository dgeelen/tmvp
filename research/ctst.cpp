#include <ncurses.h>

int main()
{
	initscr();			/* Start curses mode 		  */
	start_color();

	printw("---NCurses Display---\n");
	printw("\n");
	printw("real colors: %i\n", COLORS);
	printw("Can do 'pallete': %s\n", can_change_color() ? "yes" : "no");

	init_color(COLOR_RED, 500, 0, 500);
	init_color(COLOR_CYAN, 500, 500, 0);

	init_color(COLOR_RED+8, 1000, 0, 1000);
	init_color(COLOR_CYAN+8, 10000, 1000, 0);


	init_pair(1, COLOR_CYAN, COLOR_RED);

	attrset(COLOR_PAIR(0) );


	printw("Hello World !!!\n");	/* Print Hello World		  */
	attrset(COLOR_PAIR(1) );
	printw("normal: Hello World !!!\n");	/* Print Hello World		  */

	attrset(COLOR_PAIR(1) | A_BLINK);
	printw("blink: Hello World !!!\n");	/* Print Hello World		  */

	attrset(COLOR_PAIR(1) | A_BOLD);
	printw("bold: Hello World !!!\n");	/* Print Hello World		  */

	attrset(COLOR_PAIR(1) | A_BLINK | A_BOLD);
	printw("b&b: Hello World !!!\n");	/* Print Hello World		  */

	refresh();			/* Print it on to the real screen */
	getch();			/* Wait for user input */
	endwin();			/* End curses mode		  */

	return 0;
}
