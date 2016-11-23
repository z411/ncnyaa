/* Copyright (c) 2016 z411, see LICENSE for details */

#include "core.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXLINE 100

struct Config config = { 1, NULL, NULL, 0 };

void
run_command(const char * cmd, struct TorrentItem * item)
{
    setenv("TITLE", item->title, 1);
    setenv("URL", item->link, 1);
    system(config.cmd);
}

void
apply_config_opt(char * key, char * val)
{
    if (!strcmp(key, "download")) {
        config.download = (!strcmp(val, "yes")) ? 1 : 0;
    } else if (!strcmp(key, "category")) {
        config.category = strtol(val, NULL, 10);
    } else if (!strcmp(key, "cmd")) {
        if (config.cmd == NULL) {
            config.cmd = malloc((strlen(val)+1)*sizeof(*config.cmd));
            if (config.cmd != NULL) strcpy(config.cmd, val);
        } else
            printf("config: Duplicate option, ignoring: %s\n", key);
    } else if (!strcmp(key, "path")) {
        if (config.cmd == NULL) {
            config.path = malloc((strlen(val)+1)*sizeof(*config.path));
            if (config.path != NULL) strcpy(config.path, val);
        } else
            printf("config: Duplicate option, ignoring: %s\n", key);
    } else {
        printf("config: Invalid option, ignoring: %s\n", key);
    }
}

void
parse_config(char * filename)
{
    FILE *fp;
    char line[MAXLINE];

    if ((fp = fopen(filename, "r")) != NULL) {
        char * key;
        char * val;

        while (fgets(line, MAXLINE, fp)) {
            /* Ignore comment */
            if(line[0] == '#') continue;

            /* Remove endline */
            size_t ln = strlen(line) - 1;
            if(line[ln] == '\n')
                line[ln] = 0;

            /* Ignore empty lines */
            if(!strlen(line)) continue;

            /* Parse config line */
            key = strtok(line, "=");
            val = strtok(NULL, "=");
            if (val != NULL) {
                strip(key);
                strip(val);
                apply_config_opt(key, val);
            } else {
                printf("config: Invalid line, ignoring: %s\n", line);
            }
        }
        
        fclose(fp);
    } else {
        perror("Error reading configuration file");
        exit(-1);
    }
}

void
cleanup()
{
    free(config.cmd);
    free(config.path);
}

void
strip(char * str)
{
    char *p, *q;
    for (p = q = str; *p == ' '; p++);
    while (*p) *q++ = *p++;
    for (; *(q-1) == ' '; q--);
    *q = '\0';
}
    
