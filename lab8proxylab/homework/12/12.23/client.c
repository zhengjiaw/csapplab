/*
 * 12.23.client.c - An echo client
 */
#include "../csapp.h"
//  this code from https://dreamanddead.github.io/CSAPP-3e-Solutions/chapter12/12.23/
int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port;
    char *buf = "something to send\n";
    rio_t rio;

    host = "127.0.0.1";
    port = "10086";

    clientfd = Open_clientfd(host, port);

    Rio_readinitb(&rio, clientfd);
    Rio_writen(clientfd, buf, strlen(buf));
    /*\\Close(clientfd);*/  // 这会造成 服务器发回来的ACK ，找不到主机了，read会受到一个错误。
    exit(0);
}