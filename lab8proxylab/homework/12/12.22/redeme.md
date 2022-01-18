wget http://csapp.cs.cmu.edu/3e/ics3/code/conc/select.c -O selectOrigin.c

diff -u selectOrigin.c  select.c  >> redeme.md 

主要就是改成了 else if
测试方法： 在命令行一行一行的狂输入很多行
然后再 telnet localhost 10086 
输入数据，会发现一直在等待，直到命令行的数据处理完，这边才会输出

```diff
--- selectOrigin.c	2022-01-17 06:57:05.346518141 +0000
+++ select.c	2022-01-17 06:55:32.777663634 +0000
@@ -1,5 +1,5 @@
 /* $begin select */
-#include "csapp.h"
+#include "../csapp.h"
 void echo(int connfd);
 void command(void);
 
@@ -25,12 +25,13 @@
         Select(listenfd + 1, &ready_set, NULL, NULL, NULL);  // line:conc:select:select
         if (FD_ISSET(STDIN_FILENO, &ready_set))              // line:conc:select:stdinready
             command();                                       /* Read command line from stdin */
-        if (FD_ISSET(listenfd, &ready_set)) {                // line:conc:select:listenfdready
+        else if (FD_ISSET(listenfd, &ready_set)) {                // line:conc:select:listenfdready
             clientlen = sizeof(struct sockaddr_storage);
             connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
-            echo(connfd); /* Echo client input until EOF */
+            echo(connfd); /* Echo client input until '\n' */
             Close(connfd);
         }
+        sleep(1);
     }
 }
 
@@ -41,3 +42,13 @@
     printf("%s", buf);                        /* Process the input command */
 }
 /* $end select */
+
+void echo(int connfd)
+{
+    ssize_t n;
+    char buf[MAXLINE];
+    rio_t rio;
+    Rio_readinitb(&rio, connfd);
+    n = Rio_readlineb(&rio, buf, sizeof buf);
+    Rio_writen(connfd, buf, n);
+}
\ No newline at end of file
```
