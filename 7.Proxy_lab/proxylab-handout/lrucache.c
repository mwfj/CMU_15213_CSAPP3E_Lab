#include "lrucache.h"
#include <sys/time.h>

/**
 * @brief LRU APIs
 */
static LRUCache* lRUCacheCreate();
static char* lRUCacheGet(LRUCache* obj, cache_key_t *key);
static void lRUCachePut(LRUCache* obj, cache_key_t *key, char *value);
static void lRUCacheFree(LRUCache* obj);

/**
 * @brief inside operation functions
 */
static cache_t *InitCacheNode();
static inline void MoveToHead(LRUCache *obj, cache_t *cache_node);
static inline void DeleteFromTail(LRUCache *obj);
static inline void CreateAfterHead(LRUCache *obj, cache_t *node);

/**
 * @brief Utility functions for LRU operations
 */
static inline void init_key_struct(cache_key_t*);
static int get_cache_pos(LRUCache *lru_cache, cache_key_t *key);
static inline int get_free_cache_pos(LRUCache *lru_cache);

LRUCache* obj = NULL; /* LRU Cache storage */


// void print_key(cache_key_t *cp){
//     if(cp != NULL){
//         printf("key->host: %s\n", cp->host);
//         printf("key->port: %s\n", cp->port);
//         printf("key->path: %s\n", cp->path);
//     }
// }

// void print_cache_node(cache_t *cp){
//     printf("---------------------------------------------------\n");
//     printf("cache value: %s\n", cp->value);
//     printf("cache key: \n");
//     print_key(cp->key);
//     printf("cache timestamp: %ld\n", cp->timestamp);
//     printf("---------------------------------------------------\n");
// }

// void print_whole_cache(LRUCache* cachep){
//     cache_t* head = cachep->head;

//     while(head){
//         print_cache_node(head);
//         head = head->next;
//     }
// }

/*******************************************************************
 * 
 * @brief Outside wrapper begin
 *
 *******************************************************************/
void 
initLRUCache(){
    obj = lRUCacheCreate();
}

void 
deInitLRUCache(){
    lRUCacheFree(obj);
}

void 
insert_cache(char *host, char* port, char *path, char *value){
    printf("insert_cache, host: %s, port: %s, path: %s\n", host, port, path);
    cache_key_t *my_key = make_the_key(host, port, path);
    lRUCachePut(obj, my_key, value);
    // print_whole_cache(obj);
}

char *
read_cache(cache_key_t *key){
    // printf("read_cache\n");
    // print_key(key);
    char *res = lRUCacheGet(obj, key);
    // printf("read_cache finished\n");
    return res;
}

cache_key_t *
make_the_key(char *host, char *port, char *path){
    cache_key_t *my_key = malloc(sizeof(cache_key_t));
    init_key_struct(my_key);
    my_key->host = malloc(strlen(host) + 1);
    memcpy(my_key->host, host, strlen(host) + 1);
    // my_key->port = port;
    my_key->port = malloc(strlen(port) + 1);
    memcpy(my_key->port, port, strlen(port) + 1);
    my_key->path = malloc(strlen(path) + 1);
    memcpy(my_key->path, path, strlen(path) + 1);
    return my_key;
}

void 
copy_the_key(cache_key_t *dest, cache_key_t *source){

    if(source != NULL){
        dest->host = malloc(strlen(source->host) + 1);
        dest->path = malloc(strlen(source->path) + 1);
        memcpy(dest->host, source->host, strlen(source->host) + 1);
        memcpy(dest->path, source->path, strlen(source->path) + 1);
        dest->port = source->port;
    }
}

/*******************************************************************
 * 
 * @brief Outside wrapper end
 *
 *******************************************************************/

static inline void 
init_key_struct(cache_key_t* key){
    key->host = NULL;
    key->path = NULL;
    key->port = NULL;
}

static inline int 
get_free_cache_pos(LRUCache *lru_cache){
    int pos = 0;
    while( (pos < MAX_OBJECT_SIZE) && lru_cache->cachemap[pos])
        pos++;
    return pos;
}

static int 
get_cache_pos(LRUCache *lru_cache, cache_key_t *key){
    if(key == NULL)
        return -1;
    lock_reader();
    int pos = -1;
    int i = 0;
    /* Iterate all the cache node to find the matched node */
    for(; i < MAX_CACHE_CAPACITY; i++){
        if( 
            (lru_cache->cachemap[i] == NULL) || 
            (lru_cache->cachemap[i]->key == NULL) ||
            (lru_cache->cachemap[i]->timestamp == 0)
        ){
            // printf("inside: get_cache_pos: %d\n", i);
            continue;
        } 

        // printf("get_cache_pos: %d\n", i);
        if (
            !strcmp(key->host, lru_cache->cachemap[i]->key->host) &&
            !strcmp(key->path, lru_cache->cachemap[i]->key->path) &&
            !strcmp(key->port, lru_cache->cachemap[i]->key->port)
        )
        { 
            // printf("cache pos matched!!\n");
            pos = i;
            break; 
        }
    }
    unlock_reader();

    if(pos == -1 || pos >= MAX_CACHE_CAPACITY)
        return pos;
    /* if cache matched, then update timestamp */
    if((lru_cache->cachemap[i] != NULL) && lru_cache->cachemap[i]->timestamp){
        lock_writer();
        struct timeval tv;
        gettimeofday(&tv, NULL);
        lru_cache->cachemap[i]->timestamp = (tv.tv_sec * 1000LL + tv.tv_usec / 1000);
        unlock_writer();
    }
    return pos;
}

static cache_t *
InitCacheNode(){
    cache_t *cache_node = malloc(sizeof(cache_t));
    cache_node->value = NULL;
    cache_node->prev = NULL;
    cache_node->next = NULL;
    cache_node->timestamp = 0;
    cache_node->key = NULL;
    return cache_node;
}

static inline void 
MoveToHead(LRUCache *obj, cache_t *cache_node){
    cache_node->prev->next = cache_node->next;
    cache_node->next->prev = cache_node->prev;
    cache_node->next = obj->head->next;
    cache_node->prev = obj->head;
    obj->head->next->prev = cache_node;
    obj->head->next = cache_node;
}

static inline void 
DeleteFromTail(LRUCache *obj){
    cache_t *node = obj->tail->prev;

    int pos = get_cache_pos(obj, node->key);

    node->timestamp = -1;

    node->prev->next = obj->tail;
    obj->tail->prev = node->prev;
    obj->cachemap[pos] = NULL;
    free(node->key->host);
    free(node->key->port);
    free(node->key->path);
    free(node->key);
    free(node->value);
    free(node);
}

static inline void 
CreateAfterHead(LRUCache *obj, cache_t *node){
    node->next = obj->head->next;
    node->prev = obj->head;
    /* node->next->prev = node */
    obj->head->next->prev = node;
    obj->head->next = node;
}

static LRUCache* 
lRUCacheCreate() {
    LRUCache *newCacheNode = malloc(sizeof(LRUCache));

    newCacheNode->head = InitCacheNode();
    newCacheNode->tail = InitCacheNode();

    /* Initially, head and tail connect each other */
    newCacheNode->head->next = newCacheNode->tail;
    newCacheNode->tail->prev = newCacheNode->head;

    newCacheNode->total_capacity = MAX_CACHE_CAPACITY;
    newCacheNode->current_capacity = 0;

    /* Init cache map */
    for(int i=0; i < MAX_CACHE_CAPACITY; i++)
        newCacheNode->cachemap[i] = NULL;

    return newCacheNode;
}

static char* 
lRUCacheGet(LRUCache* obj, cache_key_t *key) {
    
    int pos = get_cache_pos(obj, key);
    /* The key has not stored in any node */
    if(pos == -1 || pos >= MAX_CACHE_CAPACITY ){
        printf("invalid pos: %d\n", pos);
        return NULL;
    }
    /* Otherwise, get the match cache node */
    lock_reader();
    cache_t *node = obj->cachemap[pos];
    unlock_reader();

    /* Insert the node into the begin of the cache */
    lock_writer();
    MoveToHead(obj, node);
    unlock_writer();

    return node->value;
}

static void 
lRUCachePut(LRUCache* obj, cache_key_t *key, char* value) {

    if(value == NULL){
        printf("element put failed, value must not be NULL\n");
        return ;
    }

    /**
     * If the value is too large, our proxy will quit to cache it. 
     */
    size_t value_size = strlen(value) + 1;
    if( value_size > MAX_OBJECT_SIZE){
        printf("value size is too big, not save to cache.\n");
        return ;
    }

    cache_t *node = NULL;
    /* 
     * The node has already existed in cache, update the node value
     * move the node into the front of the cache
     */
    int pos = get_cache_pos(obj, key);
    if(pos != -1 && pos < MAX_CACHE_CAPACITY){
        lock_reader();
        node = obj->cachemap[pos];
        unlock_reader();

        lock_writer();
        if(!node->value){
            node->value = malloc(value_size);
            memcpy(node->value, value, value_size);
        }
        MoveToHead(obj, node);
        unlock_writer();
        return;
    }
    lock_writer();
    /* the key has not store any node right now, create a new one */
    node = InitCacheNode();
    node->key = key;
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    node->timestamp = (tv.tv_sec * 1000LL + tv.tv_usec / 1000);
    /* free the previous node */
    if(node->value != NULL)
        free(node->value);

    node->value =  malloc(value_size);
    memcpy(node->value, value, value_size);
    if(obj->current_capacity >= obj->total_capacity)
        DeleteFromTail(obj);
    else
        (obj->current_capacity) ++;

    CreateAfterHead(obj, node);
    pos = get_free_cache_pos(obj);
    obj->cachemap[pos] = node;
    unlock_writer();
}

static void 
lRUCacheFree(LRUCache* obj) {
    cache_t *head = obj->head;
    cache_t *tail = obj->tail;
    cache_t *cur = head->next;

    while(cur != tail){
        cache_t *tmp = cur;
        cur = cur->next;
        free(tmp->key->host);
        free(tmp->key->port);
        free(tmp->key->path);
        free(tmp->key);
        free(tmp->value);
        free(tmp);
    }

    if(obj->head)
        free(obj->head);
    if(obj->tail)
        free(obj->tail);
    free(obj);
}