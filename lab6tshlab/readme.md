### 写在前面



我的这个tsh实现用到了会话的概念，并实现了管道和重定向，是shlab的扩展版。 

**这tsh可以支持2重、3重、多重管道，也可以支持重定向，并且前台进程可以读入数据，这些都不是shlab的要求，是我自己增加的功能。所以我的tsh代码量会比较大，但是基本上是全注释的，核心代码基本上行行注释**。

如果你阅读了我的代码，并且发现了一些CSAPP未涉及的知识点，那么你可以参考《Linux/Uinx系统编程手册》，这也是CSAPP作者推荐的书，里面会给你答案的。

如果你去搜过GitHub你会发现很多myshell都很容易就死了，还有就是很多使用信号的建议他们都没有采用，而上面这两点我都实现地比较好，我的shell不容易死，并且支持很多类似bash的功能，信号部分输出也用了直接读硬盘的方法（read）会比较安全，另外也做了防止shell被子进程杀死的处理。

还有一些tsh因为CSAPP作者没有介绍前台进程组的概念，因此tshlab没有要求前台进程组可以实现读数据的功能（tshrf也一样），这样的tsh虽然也可以通过lab的测试，但从本质上来说是不那么好玩的。**因为例如一个 `/bin/bash -c read line` 这样的读入命令都执行不了，这个新进程会被直接暂停。**而实现这只需要一行代码:tcsetpgrp(0, pgid);具体可以看看我的代码。



### 建议

1. 最起码要看完第八章，并且做完了大部分家庭作业
2. 准备《Linux/Uinx系统编程手册》纸质书、电子书都行，这也是一本学习Linux很好的书
3. 信号处理的时候，会设计多进程的思想，能先看一点后面的多进程，有这思想是最好了。
4. 对于segmentation fault这种错误，建议直接gdb



### 环境配置

下面这条语句会下载**最新版**的lab资料和PDF中英（我当时做的版本）对照，**建议先建一个文件夹**lab6shlab

```bash
wget https://gitee.com/lin-xi-269/csapplab/raw/origin/lab6shlab/install.sh && bash install.sh
```



### 一些前置准备

1. 工作目录是shlab-handout，进到目录推荐先：make clean && make

2. 你将修改的文件是: tsh.c

3. 检测方法是： test.sh，这个脚本是我自己写的，调用的是CSAPP提供的脚本。

4. 只有PID或者/usr/bin/ps 的结果不一样就说明正确

   

   我建议不要全在tsh.c中写，这样tsh.c这个文件会很大，不好处理，所以我创建了一个文件tsh，将tsh.c移动到tsh中，以后的工作路径就是shlab-handout/tsh了,**如果你也和我做的一样，那么你可以原封不动的使用我提供的test.sh脚本来检测正确性，否则你需要做一下更改**

   大概像这样，test.sh会生成3个文件，tshmy.txt是我们自己的shell对trace产生的结果，shref.txt是CSAPP作者提供的shell产生的结果。

   tshdiff 是diff上面两个文件产生的，我们主要看diff就行，只有PID或者那个/usr/bin/ps 的结果不一样就说明正确。另外**test.sh也十分使用调试，具体可以看下图，第10行改成{14..14} 就可以调试第14个样例，是很方便的**

   ![image-20220113212559523](https://gitee.com/wzjia/picturetwo/raw/master/image-20220113212559523.png)![image-20220113192711034](https://gitee.com/wzjia/picturetwo/raw/master/image-20220113192711034.png)

### shlab具体需要做什么？

1. 实现一个shell不带终端的shell

也就是模仿shell的行为，比如创建进程、回收进程、给进程发送信号。为了描述清楚，下面先介绍一些shell中的概念。

### 概念

**会话**：由多个进程组、（可能拥有一个）控制终端构成。如果一个进程拥有一个控制终端，那么打开/dev/tty就能获得该终端的文件描述符。其中由一个首进程 pid == sid == pgid。 （shlab对此没有要求）

**进程组**：由多个进程构成，其中有一个首进程pid == pgid

**前台进程组**：某特定时刻唯一一个可以从终端读入的进程组 （shlab 可以不用实现读入的要求）

**作业job**：一个命令：一个进程组

而shell一般就是一个会话的首进程，并且和一个控制终端相关联。

**最后我的实现也没有创建一个新会话，而是在原会话使用bash的终端，模拟了bash，另外我省略了一些会话相关的信号的处理，因为我并不是想真正做一个shell，而是想尽可能多的模仿出bash的核心功能，从中学习信号、会话、管道、父子进程、进程组等的概念与使用。**



### 设计思路：

所以我对任务的实现是分出了多个进程组的，一个命令就是一个进程组就是一个job，维护一个前台进程组就行了。

1. shell先将命令按照空格区分（所以我是处理不了那些，没有空格分割的'|' 或者 > ,bash 可以很好地处理这些  ）

2. 父进程tsh就是shell，按照'|' 管道分出**每一个命令，每一个命令是一个进程，父进程依次fork，并执行重定向等处理**

3. 分出的首进程就是，组pid， 然后一个命令中的所有进程都在一个进程组里

4. 父进程要捕捉，子进程的死亡、暂停、因为暂停和死亡的处理不同，所以引入job.processNumber和 fg_num ，依次表示一个job中还存在的进程，**和前台进程组中还在运行的进程数量**。

5. 另外，因为我这里是用信号来捕捉，（事实上我觉得这是最好的方法了）， **所以我只能捕捉到pid**，但是要pgid才能和一个job关联，所以父进程会进行一下group，（一个简单的类似hash的表），映射pid到pgid，方便singhandle处理。

6. 最后就是一些信号处理的细节，一定要保证addjob在deletejob之前！！所以要addjob之后才解开SIGCHLD的阻塞，还有fg_num也是如此。

   

### 下面是各个文件的作用：

1. tsh.c：是整个程序的前端部分，**加上整个程序的逻辑处理**，和管道处理，已经命令的创建和执行，还包括了addjob
2. jobs.c： 实现了jobs控制所需的代码
3. signalHandle.c： 信号处理的代码
4. sio.c ：cs_app 作者提供的信号安全的printf，实际上就是封装了的read
5. tools.c： 我自己实现的一系列字符串处理函数
6. get_num.c： 这是Michael Kerrisk（unix/linux系统编程手册原作者）提供的string_to——numberd的代码，他做了很好的error handle ,值得学习于应用。



### 核心函数

1. 首先sigchld_handler函数是最重要的，**它负责我们创建的子进程的一系列操作 和job进行联系** 它是决定着子进程是否在job中，并且在子进程状态变化时给出回应。**shell的其他两个信号处理函数，只需要负责发信号给前台进程即可**

   ```c
   // 处理儿子死亡，暂停，被信号杀死、继续的情况
   void sigchld_handler(int sig)
   {
       int olderrno = errno, status;
       sigset_t mask_all, prev_all;
   
       Sigfillset(&mask_all);
       pid_t pid, pgid;
       while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) >
              0) {  // 循环等待子进程退出，确保不产生僵尸进程
           int i = 0;
           for (; i < MAXARGS; ++i)
               if (pid == group[i].pid) {
                   pgid = group[i].pgid;
                   break;
               }
           struct job_t *job = getjobpid(jobs, pgid);
           // exited
           if (WIFEXITED(status) || WIFSIGNALED(status)) {
               group[i].pid = 0;                // delete pid
               if (job->state == FG) fg_num--;  // 前台进程数量 --
               if (--job->processNumber > 0) continue;  // 如果这个进程组 还有进程，就稍微等待一下
               if (WIFSIGNALED(status))
                   put_help(pid2jid(pgid), pgid, "terminated", 2);  // 死于信号，发个信息给终端让用户看
   
               Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);  // signal atomic 删除这个进程组
               deletejob(jobs, pgid);
               Sigprocmask(SIG_SETMASK, &prev_all, NULL);
               // stop
           } else if (WIFSTOPPED(status)) {
               put_help(pid2jid(pgid), pgid, "stopped", 20);
               if (job->state == FG) fg_num--;
               job->state = STOP;
               // continue
           } else if (WIFCONTINUED(status)) {
               // 事实上在这里也可以这么处理:  if (job->state == FG) fg_num++;
               // 但是考虑到前台进程一定是由shell指定的，所以不会是被其他进程设定
               // 我们在shell处理好fg即可，如果这么处理的话，应该在fg的时候要考虑，要保证信号先到然后再等待，比较难以实现
           }
       }
       if (pid == -1 && errno != ECHILD) Sio_error("waitpid error");
       errno = olderrno;
   }
   ```

   2. job_t的设计,job_t因为我的shell要支持管道，那么就要处理多个前台进程在一个组的情况（前台进程组），所以得用组来表示，另外一个比较棘手的问题是，**（这是我的lab的附加部分的问题，shlab不需要考虑）**

      如果一个前台进程组的组长先死了，那么当他的进程组的别的进程死了，在被回收的时候，就无法找到这个原组长了。

      所以我想了两种方法

      1. 每次死的时候都把pid当成当成组长去试探、寻找，成功了就更换组员id，但是我没有存组员，所以那样得改变整体结构很麻烦，并且效率也不高。
      2. 用hash表，在创建的时候先保存每一个组员到组长的映射，这样就可以保证每一个组员死的时候能找到相应的pgrp也就能找到job了。当然我并没有真正实现hash表（数据小，没必要），只是两个简单的数组套一下即可。

      ```c
      struct job_t {                                 /* The job struct */
        /* status or processes when more than one */ /* job PID */
        pid_t pgrp;                                  /* process group of this job */
        int processNumber;     // 进程组中 还活着的进程数量
        int state;             /* UNDEF, BG, FG, or ST */
        char cmdline[MAXLINE]; /* command line */
        int jid;               /* job ID [1, 2, ...] */
      };
      ```

      具体实现可以参考我的代码，上面的注释是比较清晰的。

### 功能展示。

1. 支持所有bash本来就有的命令的执行，比如ls、grep、echo、‘rm’等命令。
2. 支持**多个进程组成**的管道，如下图

![image-20211127134404462](https://gitee.com/wzjia/picturetwo/raw/master/img/202111271425181.png)

3. 支持‘&’，fg [%jid | pid]，bg， jobs一些作业控制命令，另外我并没有实现bg ，fg 这样的直接执行上一个job的命令，但也差不多了，有兴趣的伙伴可以试试。下面两张图是一些操作过程。


![image-20211127134544634](https://gitee.com/wzjia/picturetwo/raw/master/img/202111271425183.png)

4. 实现了一些基本的重定向 ‘>’ , '<', '>>'，这个实际上没什么技术含量，哈哈，重定向一下就好啦。

   ![image-20211127135624116](https://gitee.com/wzjia/picturetwo/raw/master/img/202111271425184.png)

5. 这个tsh没有实现&&，|| 这样的命令，我觉得实现这些命令和管道是类似的，只不过不必重定向了

6. 这个shell也做了不少的error handle，不过肯定还是有没考虑到的，或者是不想写的，如果你有兴趣，可以试着完善一些



### 实验结果

在tsh下 ./test.sh之后等待个一小会儿，就能看到上面环境配置接收的三个文件了，下面是我的tshdiff的一部分，我肉眼检查过了。只有PID或者/usr/bin/ps 的结果不同。

![image-20220113214947024](https://gitee.com/wzjia/picturetwo/raw/master/image-20220113214947024.png)

![image-20220113214927130](https://gitee.com/wzjia/picturetwo/raw/master/image-20220113214927130.png)

### 总结

最后我觉得这个shell让我学到了很多信号、会话、进程组的概念，这些概念我以前也在书上看到过，但说实话根本没觉得多难，因为概念真的很简单，但是这个信号其实也涉及到多进程这样概念了，信号的处理和信号量的处理是有一些类似之处的，**这样我们在考虑的时候，就必须考虑信号原子性操作，稍有不慎就是极难调试的错误！！**还有就是前台进程组的概念：在一个会话中，某一时刻**只能有一个进程组能够从终端读入数据，其中只有一个进程能从终端读入数据**（这里可以看出，是可以多个后台进程组一起输出的）。





### 全部代码&仓库

https://gitee.com/lin-xi-269/csapplab/tree/master/lab6tshlab