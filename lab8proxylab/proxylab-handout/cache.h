#ifndef CACHE_H
#define CACHE_H

#include"request.h"
struct Key{
    char host[HOST_MAX_LEN];
    int port;
    char path[PATH_MAX_LINE];
};
// 每一行
struct LineNode{
    int t;  // 时间戳
    struct Key key;
    char *value;
    size_t size;
};
typedef struct LineNode GroupNode;  // 每组一行
typedef GroupNode * Cache;
typedef struct Cache_S{
    Cache  cache;   
    size_t size;

}Cache_Set;
extern Cache_Set cache_s;
void *init_cache(Cache_Set * cache);
char *read_cache(Cache_Set *cache_s, struct Key *key);
int write_cache(Cache_Set *cache_s, struct Key *key, char *value);
#endif