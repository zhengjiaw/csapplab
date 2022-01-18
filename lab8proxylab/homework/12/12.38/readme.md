```bash
wget http://csapp.cs.cmu.edu/3e/ics3/code/conc/echoservert_pre.c
wget http://csapp.cs.cmu.edu/3e/ics3/code/conc/echo_cnt.c
wget http://csapp.cs.cmu.edu/3e/ics3/code/conc/sbuf.c
wget http://csapp.cs.cmu.edu/3e/ics3/code/conc/sbuf.h
```



整体思路是：

1. 维护一个文件描述符循环队列sbuf
2. 维护一个线程数组
3. 是一个一对多的生产者消费者模型。线程就是消费者，每次消费一个文件描述符，生产者只有一个:主线程的Accept
4. 最后还需要一个调整线程，每次被调度都只检测一次然后就sleep(1)就行，多核处理器上也可以不用sleep



在0.3s一次GET请求且给线程sleep(1)的情况下，我们可以看到线程数量的显著的放缩过程

此图来源于12.38/outputTiny0.3_1,当然我的请求都是强制被杀死了的，所有相当于客户端没有close。

![](https://gitee.com/wzjia/picturetwo/raw/master/image-20220118200103251.png)