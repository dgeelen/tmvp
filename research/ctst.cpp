#include <ncurses.h>

int main()
{
	initscr();
	printw("---NCurses Test---\n");

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
		printw("Error: no pallete support\n");
		//getch();
		//endwin();
		//return 0;
	};

	COLORS = 16;
	// Set up color pair
	init_pair(1, COLOR_BLACK , 0);
	init_pair(2, COLOR_BLACK , 1);
	init_pair(3, COLOR_BLACK , 2);
	init_pair(4, COLOR_BLACK , 3);
	init_pair(5, COLOR_BLACK , 4);
	init_pair(6, COLOR_BLACK , 5);
	init_pair(7, COLOR_BLACK , 6);
	init_pair(8, COLOR_BLACK , 7);

	attrset(COLOR_PAIR( 1));
	printw("This line = 1=\n");
	attrset(COLOR_PAIR( 2));
	printw("This line = 2=\n");
	attrset(COLOR_PAIR( 3));
	printw("This line = 3=\n");
	attrset(COLOR_PAIR( 4));
	printw("This line = 4=\n");
	attrset(COLOR_PAIR( 5));
	printw("This line = 5=\n");
	attrset(COLOR_PAIR( 6));
	printw("This line = 6=\n");
	attrset(COLOR_PAIR( 7));
	printw("This line = 7=\n");
	attrset(COLOR_PAIR( 8));
	printw("This line = 8=\n");
/*
	attrset(COLOR_PAIR( 1) | A_BOLD);
	printw("This line =b1=\n");
	attrset(COLOR_PAIR( 2) | A_BOLD);
	printw("This line =b2=\n");
	attrset(COLOR_PAIR( 3) | A_BOLD);
	printw("This line =b3=\n");
	attrset(COLOR_PAIR( 4) | A_BOLD);
	printw("This line =b4=\n");
	attrset(COLOR_PAIR( 5) | A_BOLD);
	printw("This line =b5=\n");
	attrset(COLOR_PAIR( 6) | A_BOLD);
	printw("This line =b6=\n");
	attrset(COLOR_PAIR( 7) | A_BOLD);
	printw("This line =b7=\n");
	attrset(COLOR_PAIR( 8) | A_BOLD);
	printw("This line =b8=\n");
*/
	attrset(COLOR_PAIR( 1) | A_BLINK);
	printw("This line =b1=\n");
	attrset(COLOR_PAIR( 2) | A_BLINK);
	printw("This line =b2=\n");
	attrset(COLOR_PAIR( 3) | A_BLINK);
	printw("This line =b3=\n");
	attrset(COLOR_PAIR( 4) | A_BLINK);
	printw("This line =b4=\n");
	attrset(COLOR_PAIR( 5) | A_BLINK);
	printw("This line =b5=\n");
	attrset(COLOR_PAIR( 6) | A_BLINK);
	printw("This line =b6=\n");
	attrset(COLOR_PAIR( 7) | A_BLINK);
	printw("This line =b7=\n");
	attrset(COLOR_PAIR( 8) | A_BLINK);
	printw("This line =b8=\n");

	refresh();		/* Print it on to the real screen */
	getch();			/* Wait for user input */

	endwin();			/* End curses mode			*/

	return 0;
}
