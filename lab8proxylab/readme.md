### 建议

1. 至少看完第九章，并且最好做了9.17,9.18(extend有我的写代码)的家庭作业再来写这个lab能让你轻松一些
2. 如果对自己链表不是很有信心，建议先看一下c语言链表的实现
3. 如果对地址（指针）的使用不是得心应手，那么你来的正好！
4. gdb调试segmentation fault很有用，但不是一直都用！如果你一直segmentation fault，建议先停下来，好好想想结构是否设计错了，对地址的理解是否出现了偏差，链表的操作是否存在问题？想清楚之后再出发！！

### 环境配置

下面这条语句会下载**最新版**的lab资料和PDF中英（我当时做的版本）对照以及我找到的traces，官方的self-study下载缺少测试数据，所以我从GitHub找到了traces，我对比过了，和别的博主找到的traces是一样的。

```bash
wget https://gitee.com/lin-xi-269/csapplab/raw/origin/lab7malloclab/install.sh && bash install.sh
```

另外如果你不是python2用户的话，请在将/lab8proxylab/proxylab-handout/nop-server.py 第一行改为\#!/usr/bin/python3  这会影响第二个、第三个测试的进行。

### 一些前置准备

1. 工作目录是malloclab/malloclab-handout，进到目录推荐先：make clean && make

2. 你将要修改的文件是: mm.c

3. 检测方法是：

   ```bash
   make &&  ./mdriver  -t ./traces/  -v 
   ```

4. 检测结果：执行上面那条命令就会输出相应的分数。满分为100分

5. 调试方法

```bash
make && gdb --args ./mdriver  -t ./traces/  -v
```

6. 小技巧，gcc -MM *.c 可以生成这个文件夹里所以.c文件的依赖关系，直接用Makefile的自动化配置有一个缺点：.o文件和.h文件没有依赖，我更新了.H文件make却显示全部是最新，所以我比较喜欢直接用gcc -MM。

因为这个lab我分了不少文件，下文提及的文件，都会出现在这个仓库里：[lab8proxylab](https://gitee.com/lin-xi-269/csapplab/tree/master/lab8proxylab) 并且这个仓库还提供了我写的11、12的章的家庭作业的参考。

### Proxylab具体需要做什么？

目的：**实现一个简易版本的代理应用程序proxy** 

从csapp提供的检查脚本可以看出对这个proxy的要求是比较低的。下面三个是基本要求（40分）：

1. 能读取客户端的请求，并对此做出更改（不做更改也能通过测试）
2. 能够将客户端的请求转发给 目的地服务器
3. 将目的地服务器返回的内容传给客户端

还有一些锦上添花的要求：占30分。

1. proxy能并发处理请求。（多线程、多进程、IO复用 doit
2. proxy能缓存请求的应答报文。

### 实现思路

首先是基本要求：基本要求很容易看出来我们只要让proxy兼并服务器和客户端的功能即可，**即作为服务器接收连接，再作为客户端将请求发送出去即可**。

并发：我采用的是线程池，初始化10线程，并且线程的数量能根据请求的数量做动态调整，比如缓冲区满了就减半，缓冲区空线程数就乘以2，但是线程数量的最小值是INIT_THREAD_N。（事实上这个实现完全照搬了家庭作业12.38）

缓存：就是一个简单的hash表，当然我并且实现hash表，我的查找是O(n)，并且我是基于LRU实现了eviction。不过这部分比较有一点难度的是，**要处理好多个线程读和写的问题，特别是因为LRU的关系，读者和写者很难用一个函数描述了，所以我实现了lock和unlock系列函数来帮助处理。**



### 具体实现/改动

1. **csapp**:改动了Rio_readn、Rio_readnb、Rio_writen、Rio_readlineb

因为我们要更好的考虑健壮性，不能让程序随意死，特别是这种由于客户端提前关闭的情况。具体改动为：

read函数对于ECONNRESET，write对于EPIPE进行特别地提升而不是结束。

```c
if (errno != ECONNRESET)     // read 
if (errno == EPIPE)         // write 
```

2. **proxy**：主要是doit改变了一下处理流程即也是proxy的处理流程。已经忽略了PIPE信号。

   doit整体思路：

   1. 读请求

   2. 改变请求的一些请求头

   3. 去cache中查找是否有这个请求

   4. 有则返回

   5. 否则：转发请求

   6. 写回给客户，并且将结果写到value中

3. **requset**： 定义了request类型，以及相应的请求分解以及合并的处理,下面是整个requset模块的设计

```c
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
    struct Reqest_Line req_l;   // 请求行
    struct Request_Header headers[HEADERS_MAX_LEN];  // 请求头
    int headers_len;   // 这个和请求头合成一个会更好，或者用链表实现
    char *message_body; // 可能会有正文部分，这个lab可以不考虑
};
```

4. sbuf：一对多生产者消费者模型的典型实现，文件描述符（客户端连接）是item，每一个新的客户端连接都会增加一个item，而每一个serve_thread线程都会消耗一个item。

thread_run：线程池处理模型：首先先创建INIT_THREAD_N个线程，然后再维护一个adjust_threads用于调整线程的数量即可。这里比较难的是：**如何在需求线程时保证完整性**，因为我们知道sem_wai()是一个取消点，并且我们的线程只要第一次sem_wait成功之后，那么就不应该被需求了，所以我在sbuf_remove，即**第一次P成功后设置了不可以被取消**，并在close之后恢复可取消，这样就能保证执行的完整性了。

![image-20220120181137895](https://gitee.com/wzjia/picturetwo/raw/master/image-20220120181137895.png)

![image-20220120181220734](https://gitee.com/wzjia/picture/raw/master/image-20220120181220734.png)

cache：和lab5的cache实现没有太大的区别，只是没有映射了，而是直接匹配key。cache需要size的原因是因为csapp限制了cache的大小为MAX_CACHE_SIZE， 所以我们在存储的时候要对此做出要求。（当然在这个脚本中，不限制也是可以通过的）t就是LRU的标志，初始化为0，然后访存后就被赋为clock()，程序执行时间。

主要难点是：处理多线程下的cache更新关系，我使用的是读者-写者锁，感觉还是比较好用的。具体参考cache.c

```c
struct Key{
    char host[HOST_MAX_LEN];
    int port;
    char path[PATH_MAX_LINE];
};
// 每一行
struct LineNode{
    int t;// 时间戳
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
```

wr：一个稍微做了些改动的读者-写者锁，改动是：让读者不会被写者阻挡死，保证了读者的优先级。具体可以参考wr.c



### 结果



![image-20220120182328407](https://gitee.com/wzjia/picture/raw/master/image-20220120182328407.png)



### 一些测试

我对baidu.com做了一些测试，我发现我的proxy不能代理访问它这是我的第一次尝试

```bash
curl -v --proxy http://localhost:1026 http://www.baidu.com/

结果是被重定向到https://www.baidu.com/了
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 302 Found
< Content-Length: 154
< Content-Type: text/html
< Date: Thu, 20 Jan 2022 10:26:22 GMT
< Location: https://www.baidu.com/
```

然后我再次尝试

```bash
curl -v --proxy http://localhost:1026 https://www.baidu.com/
得到：
* Closing connection 0
```

后来我发现，他curl发送给proxy的是

```bash
CONNECT www.baidu.com:443 HTTP/1.1
```

这句话的意思应该是建了ssl隧道？我对ssl的细节记的不是很清楚了，但是我的proxy是完全不支持CONNECT的，所以这是这个proxy存在的一个问题，我现在也不太能解决他，因为我不是很清楚CONNET的机制，以后学了我会回来更新的！



### 总结

这个lab加上12章的家庭作业，我明白了：如果一个问题能够用单线程比较好的解决就最好不过了，如果要并发的话，一定得做好相应的模块处理，和一系列并发细节。最好就是能用常见的模型就尝试一下，如果不能再考虑创新（这得需要证明不会出现死锁才能用的放心）。另外网络编程的细节也好多，比如printf这种函数是不应该出现的，这很容易丢数据。不过这一章真的有趣，proxy和tiny都是很有意思的东西，我以后要尝试继续优化一下它们。另外，那些好用的工具，一般来说先用熟其中一个、最多两个是最好了，想一下子全部都用好是不太高效的。

Good luck and have fun!
