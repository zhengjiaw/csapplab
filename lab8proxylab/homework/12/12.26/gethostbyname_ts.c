/*
 * 12.26.c
 */
#include <stdio.h>

#include "../csapp.h"

/*
 * struct hostent *gethostbyname(const char *name)
 *
 * struct hostent {
 *   char *h_name;
 *   char **h_aliases;
 *   int  h_addrtype;
 *   int  h_length;
 *   char **h_addr_list;
 * }
 */
static sem_t mutex;

static void init_mutex(void) { Sem_init(&mutex, 0, 1); }

static void copy_nn(char **src, char ***dst)  // 深层次 copy二维数组
{
    int i;
    for (i = 0; src[i] != NULL; i++) {
    }

    *dst = (char **)Malloc(sizeof(char *) * (i + 1));
    char **dist = *dst;
    for (i = 0; src[i] != NULL; i++) {
        dist[i] = (char *)Malloc(strlen(src[i]));
        strcpy(dist[i], src[i]);
    }
    dist[i] = NULL;
}

struct hostent *gethostbyname_ts(const char *name, struct hostent *host)
{
    struct hostent *sharehost;

    P(&mutex);
    sharehost = gethostbyname(name);
    // copy int
    host->h_addrtype = sharehost->h_addrtype;
    host->h_length = sharehost->h_length;
    // copy char *
    host->h_name = (char *)Malloc(strlen(sharehost->h_name));
    strcpy(host->h_name, sharehost->h_name);
    // copy char **
    copy_nn(sharehost->h_aliases, &host->h_aliases);
    // copy char **
    copy_nn(sharehost->h_addr_list, &host->h_addr_list);
    V(&mutex);

    return host;
}

int main(int argc, char *argv[])
{
    init_mutex();

    struct hostent host;
    gethostbyname_ts("www.aliyun.com", &host);
    // result in &host
    printf("%s\n", host.h_name);
    puts("alias:---------------");
    for (int i = 0; host.h_aliases[i]; ++i) printf("%s\n", host.h_aliases[i]);
    puts("addr:---------------");
    for (int i = 0; host.h_addr_list[i]; ++i) printf("%s\n", host.h_addr_list[i]);
    return 0;
}
