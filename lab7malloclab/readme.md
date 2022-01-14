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

### malloclab具体需要做什么？

目的：实现c语言中malloc、free内存管理系统。

内存管理**实际上就是设计一个高效地管理一个字节数组中内容的数据结构**（这个数组在实际的进程中其实就是进程的堆空间）。

只要明白我们是在设计一个数据结构，那么问题就回到了**选择和设计**数据结构的分析过程：

下面基本上就是我们的内存管理系统所需要面临的全部问题。

1. 如何存储（表示）空闲块和分配块？该使用那种基础数据结构呢？
2. 如何分配块？
3. 如何释放块？
4. 如何合并相邻的空闲块？又该在什么时候合并？

**下面为了叙述方便，这个字节数组都用堆来表示**。

通过书上的介绍我们有下面四种结构可以选择：

1. 隐式链表： 用一位来表示是否分配即可，分配块需要O(n)，释放为O(1)
2. 显式链表： 用链表来将所有的空闲块串起来，分配块需要 O(空闲块数量) ，释放为O(1)
3. 分离适配：维护一个空闲链表的数组
4. 分离存储：用一个个装等价类的链表来存储空闲块， 分配块平均可以达到O(long 空闲块数量) ，释放为O(1)
5. rb-tree： 我在网上（我忘了在哪找到的了）找到的一个用红黑树做的97分的高分做法

我的实现有前三种方法，因为第一种的实现CSAPP作者已经写好了，所以我就主要介绍后两种。

### 核心函数

下面是显示空闲链表的核心函数，可以看出这就是很简单的链表操作而已。

```c
// 在pos前面插入node
#define insert_before(pos, node)                                      \
    {                                                                 \
        GET_SUCC(GET_PREV(pos)) = (char *)node; /* pos.PREV -> node*/ \
        GET_PREV(node) = GET_PREV(pos);         /*pos.PREV<-node*/    \
        GET_PREV(pos) = (char *)node;           /*node<-pos*/         \
        GET_SUCC(node) = (char *)pos;           /*node->pos*/         \
    }
// 从空闲链表中删除pos
#define del(pos)                                                  \
    {                                                             \
        GET_SUCC(GET_PREV(pos)) = GET_SUCC(pos); /*PREV -> NEXT*/ \
        GET_PREV(GET_SUCC(pos)) = GET_PREV(pos); /*PREV<-NEXT*/   \
    }
// 将bp插入到空闲链表中
#define insertFreeList(bp)        \
    {                             \
        char *p = GET_SUCC(head); \
        insert_before((p), bp);   \
    }
#define freeNode(bp) del(bp);
```

实现完一个显示链表就有80多分了，在对其mm_realloc进行一个优化就和首次extend_heap的大小修改为1<<6就可以得到90分了。另外我的链表采用的best_fit,具体可以参考我的代码：[mm.c](https://gitee.com/lin-xi-269/csapplab/blob/master/lab7malloclab/src/mm.c)我这份代码中规中矩的，**个人感觉没有特别厉害的优化了，优点就是比较简洁，注释也比较全面。不必处理链表插入删除的特殊情况，缺点就是分不够高（内存利用率不够高），只有90分。**另外我还实现了分离存储的版本，分数只有88分了，但是速度提升了10倍:[my_naive.c](https://gitee.com/lin-xi-269/csapplab/blob/master/lab7malloclab/grade/handin/my_naive.c)

事实上我的一系列分析过程都写在注释里了，如果你要参考的话，可以访问我的[仓库](https://gitee.com/lin-xi-269/csapplab/tree/master/lab7malloclab/grade/handin)查看。

### 推荐

如果你想看看<font color='cornflowerblue'>如何做针对数据做trick让分数更高</font>（98）可以看看[李秋豪](https://www.cnblogs.com/liqiuhao/p/8252373.html)的博客

如果<font color='orange'>你想挑战自己</font>，并且全部都用指针实现，你可以尝试去看看[mm-tree.c](https://github.com/pgoodman/csc369/blob/master/malloclab/grade/handin/mm-tree.c)， 说实话他写的真的太厉害了，全部都是指针写的，实现了红黑树的高分（97）版本。(这个人貌似是cmu的学生，我的traces就是从他这里下载的)



### 总结

我的这份代码是我一个月前写的了，所以具体的那种详细过程我也记不大清了，但是代码和注释能让我回忆起来，这也让我再次明白坚持写易懂、简洁的代码是十分重要的！能有清晰注释就更好不过了。malloclab我记得当时我最棘手的问题，是没明白结点地址的真实含义，因为以前写链表都是struct，这回直接操作地址，我一时没想明白那个结点地址和next、prev的关系，后来才发现，next的存储位置就可以是结点地址啊！对于不同的结点来说，存储next的地方肯定是不同的嘛！这是我一直segmentation fault的主要原因，后来想通了也就一通百通了。这个lab让我明白了对地址访问是极为容易出错的，并且要明白地址的真实含义，老实说，这确实是一个很好的理解指针的实验！我以后会用指针重构一遍这个代码（希望我能在5月份之前回来...）。



### 一些启发

1. 用指针重构的难点在于哪里？该如何考虑呢？
2. 怎么样去实现红黑树的版本？
3. B树会有好的表现吗？

### 结果

输入

```bash
make &&  ./mdriver  -t ./traces/  -v 
```
之后
![](https://gitee.com/wzjia/picturetwo/raw/master/image-20220114154515702.png)

### extend

另外我还实现了作业题9.17和9.18的代码，也是比较清楚的。具体可到仓库查看

### 仓库

[仓库](https://gitee.com/lin-xi-269/csapplab/tree/master/lab7malloclab)



