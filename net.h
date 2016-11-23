#ifndef NET_H
#define NET_H
#include <curl/curl.h>
#include <libxml/parser.h>

struct MemoryChunk
{
  char * memory;
  size_t size;
};

struct MemoryChunk * download_to_mem(const char * url);
int download_to_file(const char * url, const char * out_filename);

char * url_encode(const char * str);
extern size_t mem_write_callback(void *contents, size_t size, size_t nmemb, void *userp);

xmlDocPtr parse_rss(struct MemoryChunk * mem);

#endif
