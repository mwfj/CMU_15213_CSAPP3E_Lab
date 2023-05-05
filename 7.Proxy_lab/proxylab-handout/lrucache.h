#ifndef __LRUCACHE_H__
#define __LRUCACHE_H__

#include "csapp.h"
#include "proxy.h"
#include "reader-writer.h"
#include "common.h"

typedef struct{
    char *host;
    char *port;
    char *path;
}cache_key_t;

typedef struct CacheNode{
    char *value;
    cache_key_t *key;
    struct CacheNode *prev;
    struct CacheNode *next;
    uint64_t timestamp;
} cache_t; /* double link list */

typedef struct {
    int total_capacity;
    int current_capacity;
    cache_t *head;
    cache_t *tail;
    cache_t *cachemap[MAX_CACHE_CAPACITY]; /* store the key from the corresponding node */
} LRUCache;

/**
 * @brief Outside wrapper functions
 */
void initLRUCache();
void deInitLRUCache();
void insert_cache(char *host, char *port, char *path, char *value);
char *read_cache(cache_key_t *key);
cache_key_t * make_the_key(char *host, char *port, char *path);
void copy_the_key(cache_key_t *dest, cache_key_t *source);

#endif /* __LRUCACHE_H__ */