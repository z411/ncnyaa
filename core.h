#ifndef CORE_H
#define CORE_H
#define _VERSION "0.1"

#include <stddef.h>

struct Config
{
    int download;
    char * path;
    char * cmd;
    int category;
};

enum TorrentType
{
    TYPE_UNKNOWN,
    TYPE_NORMAL,
    TYPE_TRUSTED,
    TYPE_REMAKE,
    TYPE_A
};

enum SortType
{
    SORT_NONE,
    SORT_DATE,
    SORT_SEEDERS,
    SORT_LEECHERS,
    SORT_DOWNLOADS,
    SORT_SIZE,
    SORT_NAME,
    SORT_ALL
};

enum SortOrder
{
    ORDER_NONE,
    ORDER_DESC,
    ORDER_ASC
};

struct Search
{
    char term[30];
    int category;
    enum SortType sort;
    enum SortOrder order;
};

struct TorrentItem
{
    short i;
    const char * title;
    wchar_t * title_w;
    const char * link;
    int seeders;
    int leechers;
    int downloads;
    double size;
    char size_u[5];
    enum TorrentType type;
};

struct TorrentList
{
    size_t size;
    size_t capacity;
    struct TorrentItem * list[150];
};

extern struct Config config;

void run_command(const char * cmd, struct TorrentItem * item);
void apply_config_opt(char * key, char * val);
int load_config(const char * name);
int parse_config(const char * path);
void cleanup();

void strip(char * str);

#endif
