#ifndef NYAA_H
#define NYAA_H
#include "core.h"
#include <libxml/parser.h>

struct Category
{
    char * id;
    int sukebe;
    char * name;
};

struct Sort
{
    int id;
    int order;
    char * name;
};

extern struct Category nyaa_categories[];
extern int nyaa_categories_size;

int parse_nyaa(struct TorrentList * torrent_list, struct Search * search);
struct TorrentItem * parse_nyaa_item(xmlNodePtr node_ptr);

int download_nyaa(struct TorrentItem * item);

#endif
