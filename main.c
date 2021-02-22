#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ncurses.h>
#include <form.h>
#include <unistd.h>

#include "pam.h"
#include "utils.h"
#include "log.h"

static FORM *form;
static FIELD *fields[5];
static WINDOW *win_body, *win_form;

static pid_t child_pid;
static pid_t x_server_pid;

static void start_x_server() {
    x_server_pid = fork();
    if(x_server_pid ==0) {
        char cmd[64];
        snprintf(cmd,sizeof(cmd),"/home/devhypercoder/.local/bin/x");
        execl("/bin/sh", "/bin/sh", "-c",cmd,NULL);
        printf("Fail");
        exit(1);
    } else {
        sleep(1);
    }
}


static void driver(int ch) {
    switch (ch) {
    case KEY_ENTER:
    case 10:
        form_driver(form, REQ_VALIDATION);
        char *username = trim_whitespace(field_buffer(fields[1], 0));
        char *password = trim_whitespace(field_buffer(fields[3], 0));

        if(login(username,password,&child_pid)) {
            start_x_server();
        }

        refresh();
        pos_form_cursor(form);
        break;

    case KEY_DOWN:
        form_driver(form, REQ_NEXT_FIELD);
        form_driver(form, REQ_END_LINE);
        break;

    case KEY_UP:
        form_driver(form, REQ_PREV_FIELD);
        form_driver(form, REQ_END_LINE);
        break;

    case KEY_LEFT:
        form_driver(form, REQ_PREV_CHAR);
        break;

    case KEY_RIGHT:
        form_driver(form, REQ_NEXT_CHAR);
        break;

    case KEY_BACKSPACE:
    case 127:
        form_driver(form, REQ_DEL_PREV);
        break;

    case KEY_DC:
        form_driver(form, REQ_DEL_CHAR);
        break;

    default:
        form_driver(form, ch);
        break;
    }

    wrefresh(win_form);
}

int main() {
    int ch;

    // Init ncurses
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    win_body = newwin(24, 80, 0, 0);
    assert(win_body != NULL);
    win_form = derwin(win_body, 20, 78, 3, 1);
    assert(win_form != NULL);
    mvwprintw(win_body, 1, 2, "TLM");

    // Fields
    fields[0] = new_field(1, 10, 0, 0, 0, 0);
    fields[1] = new_field(1, 40, 0, 10, 0, 0);
    fields[2] = new_field(1, 10, 2, 0, 0, 0);
    fields[3] = new_field(1, 40, 2, 10, 0, 0);
    fields[4] = NULL;
    assert(fields[0] != NULL && fields[1] != NULL && fields[2] != NULL && fields[3] != NULL);

    set_field_buffer(fields[0], 0, "Username:");
    set_field_buffer(fields[1], 0, "");
    set_field_buffer(fields[2], 0, "Password:");
    set_field_buffer(fields[3], 0, "");

    set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
    set_field_opts(fields[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[3], O_VISIBLE | O_EDIT | O_ACTIVE);

    set_field_back(fields[1], A_UNDERLINE);
    set_field_back(fields[3], A_UNDERLINE);

    form = new_form(fields);
    assert(form != NULL);
    set_form_win(form, win_form);
    set_form_sub(form, derwin(win_form, 18, 76, 1, 1));
    post_form(form);

    refresh();
    wrefresh(win_body);
    wrefresh(win_form);

    while ((ch = getch()) != KEY_F(1))
        driver(ch);

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    free_field(fields[1]);
    free_field(fields[2]);
    free_field(fields[3]);
    delwin(win_form);
    delwin(win_body);
    endwin();

    return 0;
}
