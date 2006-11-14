#include <ncurses.h>

int main()
{
	initscr();			/* Start curses mode 		  */
	start_color();

	printw("---NCurses Display---\n");
	printw("\n");
	printw("real colors: %i\n", COLORS);
	printw("Can do 'pallete': %s\n", can_change_color() ? "ÿes" : "no");

	init_color(COLOR_RED, 500, 0, 500);
	init_color(COLOR_CYAN, 500, 0, 500);

	init_color(COLOR_RED, 700, 0, 700);

	init_pair(1, COLOR_CYAN, COLOR_RED);

	attrset(COLOR_PAIR(0) );


	printw("Hello World !!!\n");	/* Print Hello World		  */
	attrset(COLOR_PAIR(1) ;

	printw("Hello World !!!\n");	/* Print Hello World		  */
	refresh();			/* Print it on to the real screen */
	getch();			/* Wait for user input */
	endwin();			/* End curses mode		  */

	return 0;
}
