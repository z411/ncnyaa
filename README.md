# ncnyaa
ncnyaa is a ncurses nyaa browser and downloader for your terminal.
It can search in different categories, download torrents and/or run a shell command
for a downloaded item.

![Sample](/sample.png?raw=true "ncnyaa")

# Requirements
* ncurses
* libxml2
* libcurl

# Compiling
After you've satisfied the requirements, just simply clone this repo
and run the Makefile:

    cd ncnyaa
    make

# Using
**Please check the configuration file (`config`) for customization.**

Then run the program in a terminal:

    ./ncnyaa

| Keybind | Action |
| --- | --- |
| j/k/Up/Down | Move up or down |
| u/b/PgUp/PgDn | Jump up or down |
| c | Change search category |
| o | Change sort type (Date, Size, Seeders, etc.) |
| O | Change sort order (Ascending/Descending) |
| s | Perform search |
| d/Enter | Download and/or run command (depending on config) |
| q | Quit |

# Authors/License
Copyright (c) 2016 z411 <z411@omaera.org>
Licensed under the MIT license; see LICENSE file for details.

