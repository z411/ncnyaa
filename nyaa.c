/* Copyright (c) 2016 z411, see LICENSE for details */

#include "nyaa.h"

#include "net.h"
#include <string.h>

int nyaa_categories_size = 34;
struct Category nyaa_categories[] = {
    { "0_0",  0, "All categories" },
    { "1_0",  0, "Anime" },
    { "1_32", 0, "Anime - AMV" },
    { "1_37", 0, "Anime - English" },
    { "1_38", 0, "Anime - Non-English" },
    { "1_11", 0, "Anime - Raw" },
    { "3_0",  0, "Audio" },
    { "3_14", 0, "Audio - Lossless" },
    { "3_15", 0, "Audio - Lossy" },
    { "2_0",  0, "Literature" },
    { "2_12", 0, "Literature - English" },
    { "2_39", 0, "Literature - Non-English" },
    { "2_13", 0, "Literature - Raw" },
    { "5_0",  0, "Live" },
    { "5_19", 0, "Live - English" },
    { "5_22", 0, "Live - PVs" },
    { "5_21", 0, "Live - Non-English" },
    { "5_20", 0, "Live - Raw" },
    { "4_0",  0, "Pictures" },
    { "4_18", 0, "Pictures - Graphics" },
    { "4_17", 0, "Pictures - Photos" },
    { "6_0",  0, "Software" },
    { "6_23", 0, "Software - Applications" },
    { "6_24", 0, "Software - Games" },
    { "0_0",  1, "All categories" },
    { "7_0",  1, "Art" },
    { "7_25", 1, "Art - Anime" },
    { "7_33", 1, "Art - Doujinshi" },
    { "7_27", 1, "Art - Games" },
    { "7_26", 1, "Art - Manga" },
    { "7_28", 1, "Art - Pictures" },
    { "8_0",  1, "Real Life" },
    { "8_31", 1, "Real Life - Photobooks" },
    { "8_30", 1, "Real Life - Videos" }
};

int
parse_nyaa(struct TorrentList * torrent_list, struct Search * search)
{
    /* Get a Nyaa.se RSS and fill the torrent_list with the available torrents */
    char url[200];

    if (search->category >= 0 && search->category < nyaa_categories_size)
        snprintf(url, sizeof(url),
            "https://%s.nyaa.se/?page=rss&cats=%s&sort=%d&order=%d&term=%s",
            (nyaa_categories[search->category].sukebe) ? "sukebei" : "www",
            nyaa_categories[search->category].id,
            search->sort,
            search->order,
            url_encode(search->term));
    else
        snprintf(url, sizeof(url),
            "https://www.nyaa.se/?page=rss&term=%s",
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
        for (i = 0; i < 150 && cur != NULL; cur = cur->next)
            if (xmlStrEqual(cur->name, (const xmlChar *)"item"))
                torrent_list->list[i++] = parse_nyaa_item(cur);
        
        torrent_list->size = i;
    } else
        return 0;
    
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
            item->title = xmlNodeGetContent(cur);
            size = mbstowcs(NULL, item->title, xmlStrlen(item->title)) + 1;
            
            item->title_w = malloc(size * sizeof(wchar_t));
            mbstowcs(item->title_w, item->title, size);
        } else if (xmlStrEqual(cur->name, (const xmlChar *)"link")) {
            item->link = xmlNodeGetContent(cur);
        } else if (xmlStrEqual(cur->name, (const xmlChar *)"description")) {
            char * description = xmlNodeGetContent(cur);
            char * p;

            p = strtok(description, "-");
            sscanf(p, "%d seeder(s), %d leecher(s), %d download(s)",
                &item->seeders,
                &item->leechers,
                &item->downloads);
            
            p = strtok(NULL, "-");
            sscanf(p, " %lf %s", &item->size, item->size_u);
            
            p = strtok(NULL, "-");
            if (p != NULL) {
                if (strcmp(p, " Trusted") == 0)
                    item->type = TYPE_TRUSTED;
                else if (strcmp(p, " Remake") == 0)
                    item->type = TYPE_REMAKE;
                else if (strcmp(p, " A+ ") == 0)
                    item->type = TYPE_A;
                else
                    item->type = TYPE_UNKNOWN;
            } else {
                item->type = TYPE_NORMAL;
            }
            
            free(description);
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