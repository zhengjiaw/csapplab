





这个lab主要是第四章和第五章的东西，也就是要运用y86编程，并且要运用第五章的程序优化。

### 建议

1. 首先这个lab的PDF比较长，要耐下心来看，我在这里稍微总结一下，

   1. PartA就是用y86汇编编写三个简单的程序
   2. PartB就是用第四章学的HCL语言增加一个iaddq指令，具体可以看看从图4-18开始的部分
   3. PartC是用y86写一个ncopy程序，要让这个程序尽量快，通过修改指令集，或者改善ncopy编码。

   

2. PartA和PartB都是比较简单的，在看完书上相应章节就可以开始做，但是PartC如果你想要做好，那么你最起码应该看完第五章，并且第四题后面的条件分支的习题也应该做了。

3. 要静下心来看PDF

### 环境配置

下面这条语句会下载最新版的lab资料和PDF中英对照

```bash
wget https://gitee.com/lin-xi-269/csapplab/raw/origin/lab4archlab/install.sh && bash install.sh
```

如果你是Ubuntu20.04及以上，（事实上Ubuntu18.04，16也会遇到错误，只不过可以有不同的解决办法，但是我的脚本对于Ubuntu18.04，16应该也是可行的）那么你可能会在

```bash
cd archlab-handout/sim
tar xvf sim.tar 
cd sim
make clean && make 
```

之后面临很多错误，那么如果你想要保持可以使用GUI调试就直接运行下面的我写的脚本即可，当然你也可以去Makefile里面注释掉那些GUI的部分（但是我觉得那破坏了完整性，虽然GUI调试我没用上....）

```bash
wget https://gitee.com/lin-xi-269/csapplab/raw/master/lab4archlab/archlab-handout/installTclTk.sh && bash installTclTk.sh 
```

最后是这样就成功了

![image-20220108125913611](https://gitee.com/wzjia/picturetwo/raw/master/image-20220108125913611.png)

### PartA

参考CSAPP书上图4-2, 4-3, 4-7, 4-8再加上一些专用函数的修改即可。 如果你执行make出错请看上面环境配置

**工作目录是misc**

```bash
cd archlab-handout/sim
make clean && make 
cd misc
```



因为这个部分比较简单，我就不放代码了，如果你想参考可以去我的[仓库](https://gitee.com/lin-xi-269/csapplab/lab4archlab/archlab-handout/sim)看。我创建了一个文件夹sim/PartA，下面是我的测试方式。

```bash
 ./yas  ../PartA/sum.ys && ./yis  ../PartA/sum.yo
```

```bash
 ./yas  ../PartA/rsum.ys && ./yis  ../PartA/rsum.yo 
```

```
 ./yas  ../PartA/copy.ys && ./yis  ../PartA/copy.yo 
```

​	下面是我的测试结果，通过了%rax就会是cba

![image-20220108124815307](https://gitee.com/wzjia/picture/raw/master/image-20220108124815307.png)





### PartB

工作目录是sim/seq, 要修改的文件是里面的seq-full.hcl 

```bash
cd sim/seq
```

这部分主要是要增加iaddq指令，为了PartC做准备

下面是是iaddq的说明，顺着书上就行，和那些练习题的解答方式是一样的。

![image-20220104211357669](https://gitee.com/wzjia/picture/raw/master/image-20220104211357669.png)

| 阶段   | iaddq V,rB                                                   |      |
| ------ | ------------------------------------------------------------ | ---- |
| 取指   | icode:ifun ← M1[PC],  rA;rB ←  M1[PC+1],  valC← M8[PC+2], valP← PC+10 |      |
| 译码   | valA← R[rB]                                                  |      |
| 执行   | valE← valA+valC                                              |      |
| 访存   |                                                              |      |
| 写回   | R[rB]← valE                                                  |      |
| 更新PC | PC← valP                                                     |      |

这部分仍然不放代码了，需要的话，你可以去[仓库](https://gitee.com/lin-xi-269/csapplab/lab4archlab/archlab-handout/sim)看

下面是测试方法

```Bash
make VERSION=full                 #根据seq-full.hcl文件构建新的仿真器
./ssim -t ../y86-code/asumi.yo    #在小的Y86-64程序中测试你的方法
cd ../y86-code; make testssim     #使用基准程序来测试你的方法
cd ../ptest; make SIM=../seq/ssim #测试除了iaddq以外的所有指令
cd ../ptest; make SIM=../seq/ssim TFLAGS=-i  #测试我们实现的iaddq指令
```

具体操作和运行结果如下图，这样全部都是Succeed就是成功了！

![image-20220108130928411](https://gitee.com/wzjia/picturetwo/raw/master/image-20220108130928411.png)



### PartC

工作目录是：sim/pipe

要修改的文件是pipe-full.hcl和ncopy.ys

PartC是这个实验里最难的部分，同时这个实验也是我认为所有lab中最难的（比malloc lab难度还难一些），这个part想拿到90%及以上是比较难的，如果后面那10%我觉得特别特别难，总之我目前是没办法优化上去了，我最后的结果是54.9/60.0（91.5%）。PartC我优化了整整一天，从早上一直到晚上，从0分到54.9分，**如果你有时间我也推荐你这么做，因为这非常有意思，并且会对循环展开和体系结构有一个全新的认识**！！如果你没有的话，可以尝试拿到20-35/60.0，这是不需要这么久的，大概可以只需要不到半天或者1、2个小时就行。

如果你改了pipe-full.hcl，可以用下面这条指令测试。

```
make && (cd ../y86-code; make testpsim) && (cd ../ptest; make SIM=../pipe/psim TFLAGS=-i)
```

下面是测试正确性和速度的脚本[run.sh](https://gitee.com/lin-xi-269/csapplab/raw/master/lab4archlab/archlab-handout/sim/pipe/run.sh)和运行结果

![image-20220108132036959](https://gitee.com/wzjia/picture/raw/master/image-20220108132036959.png)

![image-20220108132048454](https://gitee.com/wzjia/picturetwo/raw/master/image-20220108132048454.png)







#### 优化思路

我们要让每元素的周期数(CPE)尽量小,例如 例题中的368+9.0n ，这个9.0就是CPE

这是一个线性函数，在代码里和n有关的显然是我们的Loop，所以我们要想办法让循环次数变小，**这样我们可以减少**

**下图中这几个指令的执行次数，所以我们需要循环展开**（我这图是6*1）

![image-20220108134447950](https://gitee.com/wzjia/picturetwo/raw/master/image-20220108134447950.png)

在我第一次6*1展开后我发现小的数据CPE太高了，所以我就想加大循环展开次数，这样小的就直接在循环展开中走完。

顺着这个思路这个时候我们可以把循环展开到30次，我决策这样就可以优化到很高，害，可人家限制了1000字节，我写了好久的30*1的循环在22\*1 的时候就直接long program 了 ，我最多试了20\*1,但是这个效率并不高，因为后面用的因为不能整除的多出来的字节的只能用循环来做，那样效率太低了，随后放弃了这个方案。

后来我尝试了4,7,8,9,10,奈何这几个的效率都没有6高，6是一个很好的方案，因为后面的处理正好速度够快，也还能在1000字节以内，所以我最后选定了6*1（你也可以试试5\* 1）

**之后就是细节了：** 

1. 采用比较好的循环方法（指令少）

2. 尽量使用iaddq

3. 让尽量多的数据都用循环展开，也就是减少addq %rsi,%rdi的次数

4. cmov 没有用，不过我觉得如果能做出来caddq就很有用，奈何我连cmov的hcl都没找到....

5. 修改分支预测为课后习题4.56的那样，也就是预测往上走的。

6. 我使用了trick，我发现把xorq %rax,%rax去掉也能对，嘿嘿这能加一点分，当然这个方案是只适合这的，不具有普遍性。

7. 多次尝试更改指令顺序

   

因为为了速度我对代码做了很多优化，并且让它的顺序多次变化，这个代码的可读性有一定下降，但是思路就还是上面的思路，还是比较好读的。下面是最终版本代码

[pipe-full.hcl](https://gitee.com/lin-xi-269/csapplab/raw/master/lab4archlab/archlab-handout/sim/pipe/pipe-full.hcl)

[ncopy.ys](https://gitee.com/lin-xi-269/csapplab/raw/master/lab4archlab/archlab-handout/sim/pipe/ncopy.ys)

另外[仓库](https://gitee.com/lin-xi-269/csapplab/lab4archlab/archlab-handout/sim)有多个版本的代码

![image-20220108142928044](https://gitee.com/wzjia/picturetwo/raw/master/image-20220108142928044.png)

#### 一些问题

(cd ../ptest; make SIM=../pipe/psim TFLAGS=-i)令我很奇怪，因为我拿CSAPP的源hcl去测这些jxx指令也会失败，我改了预测之后也仍然有失败，我不知道这是什么问题，但是从我的结果run.sh来看,那些指令是正确执行了的。

，![image-20220108133328592](https://gitee.com/wzjia/picturetwo/raw/master/image-20220108133439516.png)



conflicting types of 'pc_ele' 我不知道这问题是怎么出来的，反正就是被我遇上了，可我之后用新的lab又没遇上....

总之如果你遇到了的话也很简单。

![image-20220106210001839](https://gitee.com/wzjia/picturetwo/raw/master/image-20220106210001839.png)

1. 我们发现sim.h没有include stages,加上即可

![image-20220106210023415](https://gitee.com/wzjia/picturetwo/raw/master/image-20220106210023415.png)

2. 给stages.h加上#ifndef,和endif

![image-20220106210037897](https://gitee.com/wzjia/picturetwo/raw/master/image-20220106210037897.png)

![image-20220106210046615](https://gitee.com/wzjia/picturetwo/raw/master/image-20220106210046615.png)



如果你遇到了关于matherr的错误，把他们全部注释即可，这是因为glibc版本不同的原因

matherr ，参考如下：

https://stackoverflow.com/questions/52903488/fail-to-build-y86-64-simulator-from-sources



### 总结

体系结构的知识很有意思，做了这个lab之后，我才发现我以前学的太笨啦，比如这些所谓的取指，译码阶段都是处理器自己定的具体的操作，我以前还以为是特定的，所以导致我在理解y86的译码阶段时很难受，后来才明白区分出这些阶段来是为了划分流水，区分的越细就可以划分出越深地流水。

另外第五章让我认识到了预加载、减少函数调用、内存引用，循环展开（目前编译器会做），重新结合（编译器也很可能做）等优化技巧。另外还有程序剖析这样的分析技巧，一步步发现自己程序的瓶颈。

哈哈最后我还是想继续优化PartC的，奈何要准备找实习...所以我还会回来的~

一些优化启发

1. 目前是5阶流水，是否可以给他加到8阶甚至更高呢？
2. caddq是否有办法实现呢？

### GUI使用方法



如果你是VSCode,那么你只要装了ssh一套插件，然后在Windows上运行一个x服务器(比如VcXsrv X Server)即可。当然你

的config至少应该加上这些yes，另外/etc/ssh/sshd_config也得设置X11Forwarding yes

![image-20220108140557040](https://gitee.com/wzjia/picturetwo/raw/master/image-20220108140557040.png)

当然你可以会遇到一些DISPLAY问题，根据我不断地测试，我发现这种问题都基本源于DISPLAY这个环境变量的设置不正确，你可以先尝试

下面这些指令

```bash
export DISPLAY=$(whoami):11.0
export DISPLAY=$(whoami):14.0
export DISPLAY=$(whoami):10.0
export DISPLAY=localhost:10.0
export DISPLAY=127.0.0.1:10.0
```

如果很不幸都不正确，那么就用xshell连接过去，然后看看他设置的echo $DISPLAY； 看看他怎么设置的，然后加到~/.profile末尾即可。xshell很稳，这招没失败过~

vs code的Remote X11 (SSH)插件也可以，但是就是有可能会失败，不过reload几遍，它也能成功。