#ifndef CURSES_H
#define CURSES_H
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#include <wchar.h>
#include <ncursesw/curses.h>

int init_curses();
void key_event(int ch);

struct TorrentItem * get_selected();

void write_item(int i);
void write_page();
void set_status(const char * msg);
void cycle_sort();
void cycle_order();
void ask_categories();
void ask_search();
void perform_search();
void download_selected();
void move_choice(int add);

void refresh_windows();
void rewrite_title();
void rewrite_help(int all);
void handle_resize();
void make_error();

int wcols(const wchar_t * str, int width);
void wfillx(WINDOW * win, int size);

#endif
