#ifndef POOL_H
#define POOL_H
#include "../csapp.h"
#include "sys/select.h"
typedef struct {
    int maxfd;                    // read_set中最大的文件描述符
    fd_set read_set;              // 全部要读的描述符
    fd_set ready_set;             // 准备好的描述符的  集合
    int nready;                   // 准备好的描述符的 个数
    int maxi;                     // 客户端数组中最大的下标
    int clientfd[FD_SETSIZE];     // 要监视的客户端
    rio_t clientrio[FD_SETSIZE];  // 文件描述符的缓冲区
} pool;
void init_pool(int listenfd, pool* p);
void add_client(int connfd, pool* p);
void check_clients(pool* p);
// 初始化文件描述符连接池
void init_pool(int listenfd, pool* p)
{
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
    p->maxi = -1;
    memset(p->clientfd, -1, FD_SETSIZE);  // 初始时仅有这一个描述符
}
#define max(a, b) ((a) > (b) ? (a) : (b))
// 添加新的客户端
void add_client(int connfd, pool* p)
{
    // 因为我们的添加都是从listendf活跃之后来的，所以要减去listenfd，以便check_clients进行
    p->nready--;
    int i = 0;
    // 查找空位
    for (; i < FD_SETSIZE; ++i)
        if (p->clientfd[i] < 0) {
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->clientrio[i], connfd);

            FD_SET(connfd, &p->read_set);
            // 更新max
            p->maxfd = max(p->maxfd, connfd);
            p->maxi = max(p->maxi, i);
            break;
        }
    if (i == FD_SETSIZE)  // 找不到空位了
        app_error("add_client error: Too many clients");
}

// 和每一个活跃的客户端进行交互
void doit(int fd);
void check_clients(pool* p)
{
    char buf[MAXLINE];
    rio_t rio;
    for (int i = 0; i <= p->maxi && p->nready > 0; ++i) {
        int connfd = p->clientfd[i];
        rio = p->clientrio[i];
        if (connfd > 0 && FD_ISSET(connfd, &p->ready_set)) {
            p->nready--;
            doit(connfd);
            Close(connfd);  // line:netp:tiny:close
            FD_CLR(connfd, &p->read_set);
            p->clientfd[i] = -1;
        }
    }
}

#endif