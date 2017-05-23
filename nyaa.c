/* Copyright (c) 2016 z411, see LICENSE for details */

#include "nyaa.h"

#include "net.h"
#include <string.h>

int nyaa_categories_size = 34;
struct Category nyaa_categories[] = {
    { "0_0",  0, "All categories" },
    { "1_0",  0, "Anime" },
    { "1_1", 0, "Anime - AMV" },
    { "1_2", 0, "Anime - English" },
    { "1_3", 0, "Anime - Non-English" },
    { "1_4", 0, "Anime - Raw" },
    { "2_0",  0, "Audio" },
    { "2_1", 0, "Audio - Lossless" },
    { "2_2", 0, "Audio - Lossy" },
    { "3_0",  0, "Literature" },
    { "3_1", 0, "Literature - English" },
    { "3_2", 0, "Literature - Non-English" },
    { "3_3", 0, "Literature - Raw" },
    { "4_0",  0, "Live" },
    { "4_1", 0, "Live - English" },
    { "4_2", 0, "Live - PVs" },
    { "4_3", 0, "Live - Non-English" },
    { "4_4", 0, "Live - Raw" },
    { "5_0",  0, "Pictures" },
    { "5_1", 0, "Pictures - Graphics" },
    { "5_2", 0, "Pictures - Photos" },
    { "6_0",  0, "Software" },
    { "6_1", 0, "Software - Applications" },
    { "6_2", 0, "Software - Games" },
    { "0_0",  1, "All categories" },
    { "1_0",  1, "Art" },
    { "1_1", 1, "Art - Anime" },
    { "1_2", 1, "Art - Doujinshi" },
    { "1_3", 1, "Art - Games" },
    { "1_4", 1, "Art - Manga" },
    { "1_5", 1, "Art - Pictures" },
    { "2_0",  1, "Real Life" },
    { "2_1", 1, "Real Life - Photobooks" },
    { "2_2", 1, "Real Life - Videos" }
};

char nyaa_sorts[][10] = {"none", "id", "seeders", "leechers", "downloads", "size", "", ""};
char nyaa_orders[][5] = {"none", "desc", "asc"};

int
parse_nyaa(struct TorrentList * torrent_list, struct Search * search)
{
    /* Get a Nyaa.se RSS and fill the torrent_list with the available torrents */
    char url[200];

    if (search->category >= 0 && search->category < nyaa_categories_size)
        snprintf(url, sizeof(url),
            "https://%s.nyaa.si/?page=rss&c=%s&s=%s&o=%s&q=%s",
            (nyaa_categories[search->category].sukebe) ? "sukebei" : "www",
            nyaa_categories[search->category].id,
            nyaa_sorts[search->sort],
            nyaa_orders[search->order],
            url_encode(search->term));
    else
        snprintf(url, sizeof(url),
            "https://www.nyaa.si/?page=rss&q=%s",
            url_encode(search->term));
    
    struct MemoryChunk * mem = download_to_mem(url);
    if (mem == NULL)
        return 0;
    
    xmlDocPtr doc = parse_rss(mem);
    
    free(mem->memory);
    free(mem);
    
    //xmlDocPtr doc = xmlReadFile("test.rss", NULL, 0);
    
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (cur == NULL)
        return 0;
    
    /* Only proceed if it's a valid RSS (root element is rss) */
    if (xmlStrEqual(cur->name, (const xmlChar *)"rss")) {
        cur = cur->children->children;
        
        /* Fill the torrent_list with pointers to Nyaa items */
        int i;
        for (i = 0; i < 150 && cur != NULL; cur = cur->next) {
            if (xmlStrEqual(cur->name, (const xmlChar *)"item")) {
                torrent_list->list[i++] = parse_nyaa_item(cur);
            }
        }
        torrent_list->size = i;
    } else {
        return 0;
    }
    
    xmlFreeDoc(doc);
    
    return 1;
}

struct TorrentItem
*parse_nyaa_item(xmlNodePtr node_ptr)
{
    /* Parses a single RSS item containing a Nyaa torrent */
    xmlNodePtr cur;
    struct TorrentItem * item = malloc(sizeof(struct TorrentItem));
    
    size_t size;
    for (cur = node_ptr->children; cur; cur = cur->next) {
        if (xmlStrEqual(cur->name, (const xmlChar *)"title")) {
            item->title = (const char*)xmlNodeGetContent(cur);
            size = mbstowcs(NULL, item->title, strlen(item->title)) + 1;
            
            item->title_w = malloc(size * sizeof(wchar_t));
            mbstowcs(item->title_w, item->title, size);
        } else if (xmlStrEqual(cur->name, (const xmlChar *)"link")) {
            item->link = (const char*)xmlNodeGetContent(cur);
        } else if (xmlStrEqual(cur->name, (const xmlChar *)"seeders")) {
            sscanf((char*)xmlNodeGetContent(cur), "%d", &item->seeders);
        } else if (xmlStrEqual(cur->name, (const xmlChar *)"leechers")) {
            sscanf((char*)xmlNodeGetContent(cur), "%d", &item->leechers);
        } else if (xmlStrEqual(cur->name, (const xmlChar *)"downloads")) {
            sscanf((char*)xmlNodeGetContent(cur), "%d", &item->downloads);
        } else if (xmlStrEqual(cur->name, (const xmlChar *)"size")) {
            sscanf((char*)xmlNodeGetContent(cur), "%lf %s", &item->size, item->size_u);
        }
    }
    
    return item;
}

int
download_nyaa(struct TorrentItem * item)
{
    char fname[256];
    
    if (config.path != NULL && config.path[0] != '\0')
        snprintf(fname, sizeof(fname), "%s/%s.torrent", config.path, item->title);
    else
        snprintf(fname, sizeof(fname), "%s.torrent", item->title);
    
    int res = download_to_file(item->link, fname);
    setenv("FILE", fname, 1);
    
    return res;
}
