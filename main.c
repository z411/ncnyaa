/* Copyright (c) 2016 z411, see LICENSE for details */

#include "core.h"
#include "curses.h"

int
main()
{
    if(load_config("config")) {
        init_curses();
        cleanup();
    }
}
