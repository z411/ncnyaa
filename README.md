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
Please check the configuration file (`config`) for customization.

Run the program in a terminal:

    ./ncnyaa

Use `c`, `o`, `O` and `s` keys to change search category, change sort, change order or perform search, respectively.

You can use `j/k` or `Up/Down` keys to move, and the `d` or `Enter` key to download and/or run (depending on your config).

Press `q` to quit.

# Authors/License
Copyright (c) 2016 z411 <z411@omaera.org>
Licensed under the MIT license; see LICENSE file for details.

