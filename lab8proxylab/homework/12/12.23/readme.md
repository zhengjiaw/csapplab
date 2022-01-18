```bash
wget http://csapp.cs.cmu.edu/3e/ics3/code/conc/echoservers.c -O echoserversOrigin.c
cp echoserversOrigin.c echoservers.c  


客户端提前关闭连接 会造成 服务器发回来的ACK ，找不到主机了，read会收到一个错误。
之前的处理是：导致服务器崩溃。所以为了让服务器继续运行。我们发出一个提示即可而不是退出程序。
```diff

--- echoserversOrigin.c	2022-01-17 07:06:28.902344235 +0000
+++ echoservers.c	2022-01-17 08:48:47.421058369 +0000
@@ -2,6 +2,8 @@
  * echoservers.c - A concurrent echo server based on select
  */
 /* $begin echoserversmain */
+#include <stdio.h>
+
 #include "csapp.h"
 
 typedef struct {
@@ -110,17 +112,17 @@
         /* If the descriptor is ready, echo a text line from it */
         if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
             p->nready--;
-            if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
+            if ((n = rio_readlineb(&rio, buf, MAXLINE)) > 0) {
                 byte_cnt += n;  // line:conc:echoservers:beginecho
                 printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
                 Rio_writen(connfd, buf, n);  // line:conc:echoservers:endecho
             }
-
             /* EOF detected, remove descriptor from pool */
             else {
                 Close(connfd);                 // line:conc:echoservers:closeconnfd
                 FD_CLR(connfd, &p->read_set);  // line:conc:echoservers:beginremove
                 p->clientfd[i] = -1;           // line:conc:echoservers:endremove
+                if (n < 0) fprintf(stderr, "error in fd %d:Connection reset by peer\n", connfd);
             }
         }
     }
```