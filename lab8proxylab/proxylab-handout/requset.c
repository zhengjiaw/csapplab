#include <ctype.h>
#include <strings.h>
#include <unistd.h>

#include "csapp.h"
#include "request.h"
int read_request(int connfd, struct Request* req)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    rio_t rio;
    Rio_readinitb(&rio, connfd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  // line:netp:doit:readrequest
        return -1;
    printf("%s", buf);
    struct Reqest_Line* req_l = &req->req_l;
    sscanf(buf, "%s %s %s", req_l->method, uri, req_l->version);  // line:netp:doit:parserequest
    if (parse_uri(uri, req_l) < 0) return -1;

    req->headers_len = read_requesthdrs(&rio, req->headers);  // line:netp:doit:readrequesthdrs
    // 如果要处理正文部分的话，应该在这里继续尝试读
    return 0;
}

int read_requesthdrs(rio_t* rp, struct Request_Header* rh)
{
    char buf[MAXLINE];
    int len = 0;
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) {  // line:netp:readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        sscanf(buf, "%s:%s", rh[len].name, rh[len].value);
        len++;
        printf("%s", buf);
    }
    return len;
}

int parse_uri(char* uri, struct Reqest_Line* req_l)
{
    char host_port[MAXLINE];

    if (sscanf(uri, "HTTP://%[^/]%s", host_port, req_l->path) < 2)
        if (sscanf(uri, "HTTPS://%[^/]%s", host_port, req_l->path) < 2)
            if (sscanf(uri, "http://%[^/]%s", host_port, req_l->path) < 2)
                if (sscanf(uri, "https://%[^/]%s", host_port, req_l->path) < 2) return -1;

    if (sscanf(host_port, "%[^:]:%d", req_l->host, &req_l->port) < 1) return -1;
    return 0;
}
void change_request(struct Request* req)
{
    struct Reqest_Line* req_l = &req->req_l;
    if (req_l->port == -1) req_l->port = 80;
    char version[] = "HTTP/1.0";
    memcpy(req_l->version, version, sizeof version);
    struct Request_Header* req_h = req->headers;
    int i = 0;
    for (; i < req->headers_len; ++i)
        if (!strcasecmp(req_h[i].name, "Host")) break;
    req->headers_len = 0;
    // if (i == req->headers_len) {
    strcpy(req_h[req->headers_len].name, "Host");
    sprintf(req_h[req->headers_len++].value, "%s:%d\r", req_l->host, req_l->port);
    // }
    strcpy(req_h[req->headers_len].name, "User-Agent");
    strcpy(req_h[req->headers_len++].value, user_agent_hdr);

    strcpy(req_h[req->headers_len].name, "Connection");
    strcpy(req_h[req->headers_len++].value, "close\r");

    strcpy(req_h[req->headers_len].name, "Proxy-Connection");
    strcpy(req_h[req->headers_len++].value, "close\r");
}
int transmit(struct Request* req)
{
    char buf[MAXLINE], port_str[12];
    struct Reqest_Line req_l = req->req_l;
    struct Request_Header* req_h = req->headers;

    sprintf(port_str, "%d", req_l.port);
    int fd = open_clientfd(req_l.host, port_str);
    if (fd == -1) return -1;

    sprintf(buf, "%s %s %s\n", req_l.method, req_l.path, req_l.version);
    Rio_writen(fd, buf, strlen(buf));

    for (int i = 0; i < req->headers_len; ++i) {
        sprintf(buf, "%s: %s\n", req_h[i].name, req_h[i].value);
        Rio_writen(fd, buf, strlen(buf));
    }
    Rio_writen(fd, "\r\n", 2 * sizeof(char));
    // 如果要考虑正文部分的话，应该继续尝试writen
    return fd;
}
void output(int fd, int clientfd, char* res)
{
    char buf[MAXLINE];
    int n;
    rio_t rio;
    rio_readinitb(&rio, fd);
    while ((n = Rio_readnb(&rio, buf, sizeof buf)) > 0) {
        strncat(res, buf, n * sizeof(char));
        Rio_writen(clientfd, buf, n * sizeof(char));
    }
}
void init_request(struct Request* req)
{
    memset(req, 0, sizeof *req);
    req->req_l.port = -1;
}