
#ifndef REQUEST_H
#define REQUEST_H
#include "csapp.h"
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define HOST_MAX_LEN 1024
#define POST_MAX_LEN 10
#define PATH_MAX_LINE 1024
#define METHOD_MAX_LEN 128
#define HEADERS_MAX_LEN 128
#define VERSION_MAX_LEN 128
#define GET 0
#define Header_MAX_LINE 128
#define HeaderV_MAX_LINE 1024
static const char *user_agent_hdr =
    "Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r";
struct Reqest_Line {
    char method[METHOD_MAX_LEN];  // 请求方法类型，目前我们只处理GET 0代表GET
    char host[HOST_MAX_LEN];   
    int port;
    char path[PATH_MAX_LINE];
    char version[VERSION_MAX_LEN];
};


struct Request_Header {
    char name[Header_MAX_LINE];
    char value[HeaderV_MAX_LINE];
};

struct Request {
    struct Reqest_Line req_l;
    struct Request_Header headers[HEADERS_MAX_LEN];  // 请求头
    int headers_len;   // 这个和请求头合成一个会更好，或者用链表实现
    char *message_body; // 可能会有正文部分
};

// 读到了正确的request就返回0，否则返回-1
int read_request(int connfd,struct Request * req);
int read_requesthdrs(rio_t* rp, struct Request_Header* rh);

void change_request(struct Request* req);
void init_request(struct Request * req);
int parse_uri(char* uri, struct Reqest_Line* req_l);
int transmit(struct Request* req);
void output(int fd, int clientfd,char *res);
#endif