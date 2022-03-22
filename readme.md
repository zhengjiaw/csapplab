CSAPP目前来看一共分为8个lab官网为[CS:APP3e, Bryant and O'Hallaron (cmu.edu)](http://csapp.cs.cmu.edu/3e/students.html)

这个项目是我学习CSAPP的时候做的全部lab，平均分为98分，整体代码基本上是关键部分都有详细注释的，并且都会readme.md的说明及思路。我这些实现的代码都比较简洁，并且在shell和proxy扩展了lab，使得那两份代码能支持更多一些的实用性功能。

如果你是Ubuntu系列的话，我已经写好了脚本，运行下面的命令，即可安装CSAPP的所有lab，以及相关的环境，和中文版的文档翻译。

```bash
https://gitee.com/lin-xi-269/csapplab/raw/origin/installAll.sh
bash installAll.sh
```

也可以在我的知乎专栏[CSAPPLab - 知乎 (zhihu.com)](https://www.zhihu.com/column/c_1458204480777273344) 上看到8个lab的更加清晰的介绍。**每一个lab我都会给出我的优化思路，已经优化的过程和细节描述，并给出环境配置的一些坑，是很有帮助的。**

lab完成情况，得分都已经换算成了百分制。

1. lab1datalab 100/100
2. lab2bomb   100/100
3. lab3attack  100/100
4. lab4archlab 97/100
5. lab5cachelab  100/100
6. lab6tshlab 100/100
7. lab7malloclab 90/100
8. lab8proxylab 100/100

最后的得分情况为：98分



下面从我的个人经历来说说做完这些lab大概可以获得的能力：

1. 对计算机补码，二进制操作会有更加深刻的认识，对浮点数存储方式能有清晰的了解，并且明白union的实现。
2. 能够具有翻译汇编（反汇编的能力）和掌握使用gdb调试的基本功。
3. 能够了解到CPU的运行方式，对CPU识别指令的过程，以及栈溢出攻击有所了解
4. 能够加深对流水线，以及底层优化指令运行方法的理解，提高优化代码的能力。
5. 学习到LRU的实现，了解cache的工作原理，写出让cache命中率更高的代码
6. 能学到信号的使用，信号量的处理，管道的使用
7. 学到内存管理的知识，加深对对指针和链表的理解。
8. 学到多进程（多线程）编程的思想，学习到客户端、服务端开发的雏形















