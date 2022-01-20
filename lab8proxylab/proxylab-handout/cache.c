#include "cache.h"

#include "wr.h"
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define CACHE_LEN 50
Cache_Set cache_s;
//这个cache是一个一维数组，每一组代表一个object
void *init_cache(Cache_Set *cache_s)
{
    cache_s->size = 0;
    cache_s->cache = malloc(CACHE_LEN * (sizeof(GroupNode)));  // 写一个链表会更好
    memset(cache_s->cache, 0, CACHE_LEN * (sizeof(GroupNode)));
}
// 用key查找cache，完善点的话可以考虑多加一个method的判断
static inline int find_cache(Cache_Set *cache_s, struct Key *key)
{
    // 读者
    lock_reader();
    size_t cnt = cache_s->size;
    Cache cache = cache_s->cache;

    int i = 0;
    for (; cache[i].t; ++i) {  // t被初始化为0，之后访存过就是time()（程序运行时间）
        // printf("%s-------%s\n", cache[i].key.host, cache[i].key.path);
        if (!strcmp(key->host, cache[i].key.host) && key->port == cache[i].key.port &&
            !strcmp(key->path, cache[i].key.path))
            break;
    }
    unlock_reader();

    // 这是一次写
    if (cache[i].t) {
        lock_writer();
        cache[i].t = clock();
        unlock_writer();
    }
    return i;
}
// 读，也就是客户端来的时候，看看能不能在缓存中找到相应的key
char *read_cache(Cache_Set *cache_s, struct Key *key)
{
    // 读者
    int pos = find_cache(cache_s, key);
    char *res = NULL;
    lock_reader();
    if (cache_s->cache[pos].t != 0) res = cache_s->cache[pos].value;
    unlock_reader();
    return res;
}

static inline void group_copy(GroupNode* group, struct Key *key, char *value, size_t value_size)
{
    // 这个也是写者
    memcpy(&group->key, key, sizeof(struct Key));
    group->t = clock();
    memcpy(group->value, value, value_size);
    group->size = value_size;
}

// 写入缓存
int write_cache(Cache_Set *cache_s, struct Key *key, char *value)
{
    size_t value_size = strlen(value) + 1;
    // 读者
    lock_reader();
    size_t cnt = cache_s->size;
    unlock_reader();
    int pos = find_cache(cache_s, key);
    if (value_size > MAX_OBJECT_SIZE) return -1;
    lock_writer();
    // 说明没有找到key
    //找不到： 第一种情况，不需要发生eviction
    if (cache_s->cache[pos].t == 0) {
        if (value_size + cnt <= MAX_CACHE_SIZE) {
            cache_s->cache[pos].value = (char *)malloc(sizeof(char) * value_size);
            group_copy(&cache_s->cache[pos], key, value, value_size);
            cache_s->size += value_size;
        }
        // 第二种情况需要发生eviction
        else {
            size_t min_t_pos = 0;
            Cache cache = cache_s->cache;
            int i = 0;
            for (; cache[i].t; ++i)
                if (cache[i].t < cache[min_t_pos].t) min_t_pos = i;

            group_copy(&cache_s->cache[min_t_pos], key, value, value_size);
        }
    } else {
        // 找到了, 这又是一次写,这个是用于cache按一定时间更新的操作，或者是用于不同的操作，
        // 在我们这个proxy里按理说，应该永远都不会执行到这，要完善这个部分需要写入全部才对
        cache_s->cache[pos].t = clock();
    }
    unlock_writer();
    return 0;
}