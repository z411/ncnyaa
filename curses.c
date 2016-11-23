/* Copyright (c) 2016 z411, see LICENSE for details */

#include "curses.h"

#include <locale.h>
#include <signal.h>

#include "core.h"
#include "nyaa.h"

#define LIST_PAGE_SIZE  (LINES - 4)
#define COL_SIZE        10
#define COL_SEEDERS     4
#define COL_LEECHERS    4
#define COL_DOWNLOADS   6
#define COL_TITLE       (COLS-COL_SIZE-COL_SEEDERS-COL_LEECHERS-COL_DOWNLOADS)
#define MIN_COLS    54
#define MIN_LINES   10

static WINDOW *win_title;
static WINDOW *win_list;
static WINDOW *win_help;
static WINDOW *win_status;

static struct TorrentList torrent_list;
static int list_offset = 0;
static int choice = 0;

static struct Search search = { "", 0, SORT_DATE, ORDER_DESC };

int
init_curses()
{	
    setlocale(LC_ALL, "");
    torrent_list.size = 0;
    
    signal(SIGWINCH, handle_resize);
    
    initscr();
    
    if (COLS < MIN_COLS || LINES < MIN_LINES) {
        printf("Terminal too small.\n");
        endwin();
        return -1;
    }
    
    // Set default category if defined in config file
    if (config.category >= 0 && config.category <= nyaa_categories_size)
        search.category = config.category;
    
    cbreak();
    noecho();
    curs_set(0);
    
    start_color();
    use_default_colors();
    
    init_pair(1, COLOR_YELLOW, COLOR_BLUE);
    init_pair(2, COLOR_GREEN, -1); // Trusted torrent
    init_pair(3, COLOR_YELLOW, -1); // Remake torrent
    init_pair(4, COLOR_CYAN, -1); // A+ torrent
    init_pair(5, COLOR_RED, -1); // sukebei category
    
    refresh();
    
    win_title = newwin(1, COLS, 0, 0);
    win_list = newwin(LINES - 3, COLS, 1, 0);
    win_help = newwin(1, COLS, LINES-2, 0);
    win_status = newwin(1, COLS, LINES-1, 0);
    wbkgd(win_title, COLOR_PAIR(1));
    wbkgd(win_help, COLOR_PAIR(1));
    refresh_windows();
    
    // Event loop
    int ch;
    keypad(win_list, TRUE);
    while((ch = wgetch(win_list)) != 'q')
        key_event(ch);
    
    // Free memory and quit
    delwin(win_title);
    delwin(win_list);
    delwin(win_status);
    endwin();
    
	return 0;
}

void
key_event(int ch)
{
    switch(ch)
    {
        case KEY_UP:
        case 'k':
            move_choice(-1); break;
        case KEY_DOWN:
        case 'j':
            move_choice(1); break;
        case KEY_PPAGE:
        case 'u':
            move_choice(-10); break;
        case KEY_NPAGE:
        case 'b':
            move_choice(10); break;
        case 'o':
            cycle_sort(); break;
        case 'O':
            cycle_order(); break;
        case 'c':
            ask_categories(); break;
        case 's':
            ask_search(); perform_search(); break;
        case '.':
            perform_search(); break;
        case 'd':
        case 10:
            download_selected(); break;
    }
}

struct TorrentItem
*get_selected()
{
    return torrent_list.list[list_offset+choice];
}

void
write_item(int i)
{
    if(list_offset+i >= torrent_list.size) {
        mvwaddstr(win_list, i+1, 0, "None");
        return;
    }
    
    struct TorrentItem * item = torrent_list.list[list_offset+i];
    
    if (item->type == TYPE_TRUSTED) wattron(win_list, COLOR_PAIR(2));
    else if (item->type == TYPE_REMAKE) wattron(win_list, COLOR_PAIR(3));
    else if (item->type == TYPE_A) wattron(win_list, COLOR_PAIR(4));
    
    wmove(win_list, i+1, 0);
    
    waddnwstr(win_list, item->title_w, wcols(item->title_w, COL_TITLE - 1));
    wfillx(win_list, COL_TITLE);
    
    wprintw(win_list, "%.1lf %s", item->size, item->size_u);
    wfillx(win_list, COL_TITLE+COL_SIZE);
    
    wprintw(win_list, "%d", item->seeders);
    wfillx(win_list, COL_TITLE+COL_SIZE+COL_SEEDERS);
    
    wprintw(win_list, "%d", item->leechers);
    wfillx(win_list, COL_TITLE+COL_SIZE+COL_SEEDERS+COL_LEECHERS);
    
    wprintw(win_list, "%d", item->downloads);
    wfillx(win_list, COLS-1);
    
    if (item->type == TYPE_TRUSTED) wattroff(win_list, COLOR_PAIR(2));
    else if (item->type == TYPE_REMAKE) wattroff(win_list, COLOR_PAIR(3));
    else if (item->type == TYPE_A) wattroff(win_list, COLOR_PAIR(3));
}
    
void
write_page()
{
    werase(win_list);
    
    wattron(win_list, WA_BOLD);
    mvwprintw(win_list, 0, 0, "Title");
    mvwprintw(win_list, 0, COL_TITLE, "Size");
    mvwprintw(win_list, 0, COL_TITLE + COL_SIZE, "SE");
    mvwprintw(win_list, 0, COL_TITLE + COL_SIZE + COL_SEEDERS, "LE");
    mvwprintw(win_list, 0, COL_TITLE + COL_SIZE + COL_SEEDERS + COL_LEECHERS, "DLs");
    wattroff(win_list, WA_BOLD);
    
    if(torrent_list.size) {
        int i;
        for(i = 0; i < LIST_PAGE_SIZE && list_offset+i < torrent_list.size; i++)
        {
            if(i == choice)
                wattron(win_list, WA_STANDOUT);
            else
                wattroff(win_list, WA_STANDOUT);
            
            write_item(i);
        }
    }
    wattroff(win_list, WA_STANDOUT);
    wrefresh(win_list);
    
    rewrite_title();
}

void
set_status(const char * msg)
{
    werase(win_status);
    mvwaddstr(win_status, 0, 0, msg);
    wrefresh(win_status);
}

void
ask_categories()
{
    werase(win_list);
    
    wattron(win_list, WA_BOLD);
    mvwprintw(win_list, 0, 0, "Categories");
    wattroff(win_list, WA_BOLD);
    
    int i;
    int col = 0;
    int y = 1;
    
    for (i = 0; i < nyaa_categories_size; i++) {
        wmove(win_list, y++, col*28);
        if (nyaa_categories[i].sukebe) {
            wattron(win_list, COLOR_PAIR(5));
            wprintw(win_list, "%d (s) %s\n", i, nyaa_categories[i].name);
            wattroff(win_list, COLOR_PAIR(5));
        } else {
            wprintw(win_list, "%d %s\n", i, nyaa_categories[i].name);
        }
        
        if (getcury(win_list) >= getmaxy(win_list)-1) {
            col++;
            y = 1;
        }
    }

    wrefresh(win_list);

    set_status("Category number: ");
    
    char input[4];
    int i_input;
    int res;

    curs_set(1); echo();
    wgetnstr(win_status, input, sizeof(input)-1);
    res = sscanf(input, "%d", &i_input);
    curs_set(0); noecho();
    
    write_page();

    if (res != EOF && res && i_input >= 0 && i_input < nyaa_categories_size) {
        search.category = i_input;
        set_status("Searching in new category.");
    } else {
        set_status("Invalid category.");
    }
    rewrite_help(0);
    wrefresh(win_status);
}

void
cycle_sort()
{
    if (!search.sort || search.sort >= SORT_ALL-1)
        search.sort = 1;
    else
        search.sort++;
    
    rewrite_help(0);
}

void
cycle_order()
{
    search.order =
        (search.order == ORDER_ASC)
        ? ORDER_DESC : ORDER_ASC;
    rewrite_help(0);
}

void
ask_search()
{
    set_status("Search: ");
            
    curs_set(1); echo();
    wgetnstr(win_status, search.term, sizeof(search.term)-1);
    curs_set(0); noecho();
}

void
perform_search()
{
    if (!search.term) return;
    
    set_status("Getting torrent list...");
    
    torrent_list.size = 0;
    write_page();
    
    if(parse_nyaa(&torrent_list, &search)) {
        list_offset = 0;
        choice = 0;
    
        write_page();
        set_status("Ready.");
    }
    else
        set_status("Error with search.");
}

void
download_selected()
{
    if(!torrent_list.size) return;

    if (config.download) {
        set_status("Downloading...");
        download_nyaa(get_selected());
    }

    if (config.cmd != NULL) {
        set_status("Running command...");
        endwin();
        run_command(config.cmd, get_selected());
        refresh();
    }

    set_status("Ready.");
}

void
move_choice(int add)
{
    if(!torrent_list.size) return;
    
    write_item(choice);
    
    choice += add;
    if(choice < 0) {
        if(list_offset > 0) {
            // Go to previous page
            list_offset -= LIST_PAGE_SIZE;
            if(list_offset < 0) list_offset = 0;
            choice = LIST_PAGE_SIZE+choice;
            write_page();
        } else { choice = 0; }
    } else if(list_offset+choice >= torrent_list.size) {
        choice = torrent_list.size - list_offset - 1;
    } else if(choice > LIST_PAGE_SIZE-1) {
        // Go to next page
        list_offset += LIST_PAGE_SIZE;
        choice -= LIST_PAGE_SIZE;
        write_page();
    }
    
    wattron(win_list, WA_STANDOUT);
    write_item(choice);
    wattroff(win_list, WA_STANDOUT);
    
    wrefresh(win_list);
}

void
rewrite_title()
{
    werase(win_title);
    if (torrent_list.size)
        mvwprintw(win_title, 0, 0, "ncnyaa v%s - %s (%d/%d)",
	    _VERSION,
            search.term,
            list_offset / LIST_PAGE_SIZE + 1,
            torrent_list.size / LIST_PAGE_SIZE + 1);
    else
        mvwprintw(win_title, 0, 0, "ncnyaa v%s", _VERSION);
    wrefresh(win_title);
}

void
rewrite_help(int all)
{
    if (all) {
        werase(win_help);
        wmove(win_help, 0, 0);
        waddstr(win_help, "c:Cat s:Search d:DL+Run o/O:Sort q:Quit");
    }
    
    wmove(win_help, 0, COLS - 12);
    
    if(search.category < 10) waddch(win_help, ' ');
    wprintw(win_help, "%d/", search.category);
    switch (search.sort) {
        case SORT_DATE:
            waddstr(win_help, "Date"); break;
        case SORT_SEEDERS:
            waddstr(win_help, "SE  "); break;
        case SORT_LEECHERS:
            waddstr(win_help, "LE  "); break;
        case SORT_DOWNLOADS:
            waddstr(win_help, "DLs "); break;
        case SORT_SIZE:
            waddstr(win_help, "Size"); break;
        case SORT_NAME:
            waddstr(win_help, "Name"); break;
    }
    waddch(win_help, '/');
    waddstr(win_help, (search.order == ORDER_ASC) ? "Asc " : "Desc");
    
    wrefresh(win_help);
}

void
refresh_windows()
{
    write_page();   
    rewrite_help(1);
    set_status("Ready.");
}

void
handle_resize()
{
    endwin();
    refresh();
    clear();
    
    if (COLS >= MIN_COLS && LINES >= MIN_LINES) {
        mvwin(win_help,   LINES-2, 0);
        mvwin(win_status, LINES-1, 0);
    
        wresize(win_title,  1, COLS);
        wresize(win_list,   LINES-3, COLS);
        wresize(win_help,   1, COLS);
        wresize(win_status, 1, COLS);
        
        refresh_windows();
    } else {
        make_error("Terminal too small.");
    }
}

void
make_error(char * str)
{
    mvaddstr(0, 0, str);
    refresh();
}

int
wcols(const wchar_t * str, int width)
{
    size_t i;
	int remained_len = width;
    
	for (i = 0; str[i] != L'\0'; i++)
	{
        int cols = wcwidth(str[i]);
        remained_len -= (cols > 1) ? cols : 1;
        
		if (remained_len < 0)
			return i;
	}
    return width;
}

void
wfillx(WINDOW * win, int size)
{
    while(getcurx(win) < size)
        waddch(win, ' ');
}
