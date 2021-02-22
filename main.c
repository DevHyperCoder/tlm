/*
 * TLM - Terminal Login Manager
 *
 * A NCurses interface with a suckless mindset. Edit the source code!
 * Edit the CONFIG section to match your taste
 *
 * Uses NCurses forms to make a simple and minimal form to enter the username and password. After PAM verification, `start_x_cmd` is executed. 
 *
 * Author: DevHyperCoder
 * */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ncurses.h>
#include <form.h>
#include <unistd.h>

#include "pam.h"
#include "utils.h"
#include "log.h"

// NCurses forms
static FORM *form;
static FIELD *fields[5];
static WINDOW *win_body, *win_form;

static pid_t child_pid;
static pid_t x_server_pid;

// CONFIG
char *start_x_cmd = "$HOME/.local/bin/x";
char *username_label =  "Username:";
char *password_label =  "Password:";

// Default username to be injected 
char *username_def =  "";

static void start_x_server() {
    x_server_pid = fork();
    if(x_server_pid ==0) {
        char cmd[64];
        snprintf(cmd,sizeof(cmd),"%s",start_x_cmd);
        execl("/bin/sh", "/bin/sh", "-c",cmd,NULL);
        printf("Fail");
        exit(1);
    } else {
        sleep(1);
    }
}

static void handle_login(FORM *form){
        form_driver(form, REQ_VALIDATION);

        char *username = trim_whitespace(field_buffer(fields[1], 0));
        char *password = trim_whitespace(field_buffer(fields[3], 0));

        if(login(username,password,&child_pid)) {
            start_x_server();
        }
}

static void driver(int ch) {
    /*
     * Here is where the keyboard input is handled
     * Change if necessary
     * NCurses Key definitions
     *
     * 10 is the ASCI for newline `\n`
     * 127 is the ASCI for DEL (Not the delete key)
     * These values are left here for some systems that do not work with the KEY_ENTER and KEY_BACKSPACE definitions
     * */
    switch (ch) {
    case KEY_ENTER:
    case 10:
        handle_login(form);
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

    set_field_buffer(fields[0], 0,username_label);
    set_field_buffer(fields[1], 0, username_def);
    set_field_buffer(fields[2], 0,password_label);
    set_field_buffer(fields[3], 0, "");

    set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
    set_field_opts(fields[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[3], O_VISIBLE | O_EDIT | O_ACTIVE);

    set_field_back(fields[1], A_UNDERLINE);
    set_field_back(fields[3], A_UNDERLINE);
    
    // Form init
    form = new_form(fields);
    assert(form != NULL);
    set_form_win(form, win_form);
    set_form_sub(form, derwin(win_form, 18, 76, 1, 1));
    post_form(form);

    refresh();
    wrefresh(win_body);
    wrefresh(win_form);

    // Key grabber
    while ((ch = getch()) != KEY_F(1))
        driver(ch);

    // Cleanup
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
