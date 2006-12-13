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

	cbreak();
	noecho();
	keypad(stdscr, TRUE);


	int mm = BUTTON1_PRESSED | BUTTON1_RELEASED | BUTTON2_PRESSED | BUTTON2_RELEASED | BUTTON3_PRESSED | BUTTON3_RELEASED | REPORT_MOUSE_POSITION;
	//mm = ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION;
	mm |= BUTTON2_CLICKED | BUTTON4_CLICKED; // scroll wheel in putty
	printw("Desired mouse mask  : %x\n", mm);
	mm = mousemask(mm, NULL);
	printw("Supported mouse mask: %x\n", mm);

	mouseinterval(0);


	int ch;

	do {
		refresh();		/* Print it on to the real screen */
		ch = wgetch(stdscr);		/* Wait for user input */
		printw("getch() = %i\n", ch);

		MEVENT e;
#ifdef KEY_MOUSE
		if ((ch == KEY_MOUSE) && (getmouse(&e) == OK)) {
			printw("mouse event at (%i.%i):%i\n",e.x, e.y, e.bstate);
					if(e.bstate & BUTTON1_CLICKED)
							printw("Left 1 CLICKED\n");
					if(e.bstate & BUTTON1_RELEASED)
							printw("Left 1 Released\n");
					if(e.bstate & BUTTON2_CLICKED)
							printw("Left 2 CLICKED\n");
					if(e.bstate & BUTTON2_RELEASED)
							printw("Left 2 Released\n");
					if(e.bstate & BUTTON3_CLICKED)
							printw("Left 3 CLICKED\n");
					if(e.bstate & BUTTON3_RELEASED)
							printw("Left 3 Released\n");
					if(e.bstate & BUTTON4_CLICKED)
							printw("Left 4 CLICKED\n");
					if(e.bstate & BUTTON4_RELEASED)
							printw("Left 4 Released\n");
#endif
		}
	} while (ch != 'x');

	endwin();			/* End curses mode			*/

	return 0;
}
