#include <stdio.h>
#include "log.h"
#include <ncurses.h>

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    mvprintw(0,0, "TLM - Temrinal Login Manager");
    mvprintw(1,0, "Press any key to exit!");

    getch();
    endwin();
    return 0;

}
