#include <stdio.h>

#include "csapp.h"
#include "request.h"
#include "thread_run.h"
/* Recommended max cache and object sizes */
#include "cache.h"
#include "wr.h"
void doit(int fd);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/* You won't lose style points for including this long line in your code */

int main(int argc, char **argv)
{
    if (Signal(SIGPIPE, SIG_IGN) == SIG_ERR) unix_error("mask signal pipe error");
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    pthread_t tid;
    init();
    init_wr();
    init_cache(&cache_s);
    Pthread_create(&tid, NULL, adjust_threads, NULL);

    listenfd = Open_listenfd(argv[1]);
    setbuf(stdout, NULL);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
        sbuf_insert(&sbuf, connfd);                                /* Insert connfd in buffer */
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
    }
}

void doit(int fd)
{
    struct stat sbuf;
    struct Request req;
    printf("doit\n");
    if (read_request(fd, &req) == -1) return;
    /* Parse URI from GET request */
    change_request(&req);
    struct Key key;
    strcpy(key.host, req.req_l.host);
    key.port = req.req_l.port;
    strcpy(key.path, req.req_l.path);

    char *value = read_cache(&cache_s, &key);

    if (value != NULL) {
        Rio_writen(fd, value, strlen(value));
        return;
    }

    // 转发请求
    char res[MAX_CACHE_SIZE];
    int connfd = transmit(&req);
    if (connfd == -1) {
        fprintf(stderr, "error :proxy fd could not be found");
        return;
    }
    output(connfd, fd, res);
    write_cache(&cache_s, &key, res);
}
