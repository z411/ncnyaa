/* Copyright (c) 2016 z411, see LICENSE for details */

#include "core.h"
#include "net.h"

#include <stdlib.h>
#include <string.h>

struct MemoryChunk
*download_to_mem(const char * url)
{
    CURL *curl;
    CURLcode res;
    
    struct MemoryChunk * mem = malloc(sizeof(struct MemoryChunk));
    mem->memory = NULL;
    mem->size = 0;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mem_write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)mem);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        
        res = curl_easy_perform(curl);
        
        curl_easy_cleanup(curl);
        
        if(res != CURLE_OK)
            return NULL;
    }
    
    return mem;
}

int
download_to_file(const char * url, const char * out_filename)
{
    CURL *curl;
    CURLcode res;
    
    FILE *fp;
    
    curl = curl_easy_init();
    if (curl)
    {
        if ((fp = fopen(out_filename, "wb"))) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            
            res = curl_easy_perform(curl);
            
            fclose(fp);
        }
        
        curl_easy_cleanup(curl);
        
        if(res != CURLE_OK)
            return 0;
    } else {
        return 0;
    }
    
    return 1;
}

char
*url_encode(const char * str)
{
    return curl_escape(str, strlen(str));
}

extern size_t
mem_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    /*
        Callback function for libcurl, which saves data into a
        MemoryChunk struct.
    */
         
    size_t realsize = size * nmemb;
    struct MemoryChunk *mem = (struct MemoryChunk *)userp;
 
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
 
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
 
    return realsize;
}

xmlDocPtr
parse_rss(struct MemoryChunk * mem)
{   
    // Use the MemoryChunk and get it into a xmlDoc using libxml
    xmlDocPtr doc;
    
    if (mem->memory) {
        doc = xmlReadMemory(mem->memory, (int)mem->size, "noname", NULL, XML_PARSE_NOBLANKS);
    } else {
        return NULL;
    }
    
    return doc;
}

