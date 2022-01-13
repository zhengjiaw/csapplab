#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cachelab.h"

// 因为那些帮助信息实在是繁琐，所以我不想写emm,当然这不影响这份代码通过测试
void usage(void) { exit(1); }
int verbose = 0, s, E, b, S, B;  // opt的变量， 定义了cache的大小

int T = 0;  //这是全局的一个时刻表，第一个访存指令的时刻为1，之后每次数据访存都累加1
typedef __uint64_t uint64_t;
// 定义行
typedef struct lineNode {  // 不需要block， 用不上这个lab
    int t;                 // 时刻，即是valid 也是 LRU的判断标志
    uint64_t tag;          // 标记
} * groupNode;             // 组的定义很明显，就是行的数组，也就是struct lineNode*

enum Category { HIT, MISS, EVICTION };
unsigned int result[3];  // 结果，写成数组是为了 简洁
const char *categoryString[3] = {"hit ", "miss ", "eviction "};
groupNode *cache;  // 组的数组 也就是 cache
void init();       // 初始化cache
// 释放内存，其实这个函数写不写都无所谓，进程一死空间自然没，不过手动释放是一个好的习惯:)
void destory();
FILE *opt(int argc, char **argv);                           // 处理那些命令行选项
void findCache(uint64_t tag, int group_pos, char *result);  // 核心函数，很短，按照定义处理即可

int main(int argc, char **argv)
{
    FILE *tracefile = opt(argc, argv);
    // 现在我们得到了S，E，B,现在可以构造cache了
    init();
    // 接下来处理每一条指令
    char oper[2];
    uint64_t address;
    int size;  //访问的地址和字节数
               // 用%s 可以忽略空格，不需要考虑那些空格了
    while (fscanf(tracefile, "%s %lx,%d\n", oper, &address, &size) == 3) {
        if (oper[0] == 'I') continue;                  // 忽略I
        int group_pos = (address >> b) & ~(~0u << s);  // 从 第 b位开始取，取s位
        uint64_t address_tag = address >> (b + s);     // b + s之后的位都是
        char resultV[20];                              // 为了 -v 设置的string显示
        memset(resultV, 0, sizeof resultV);
        ++T;
        findCache(address_tag, group_pos, resultV);
        if (oper[0] == 'M') findCache(address_tag, group_pos, resultV);  // M需要两次

        if (verbose) fprintf(stdout, "%s %lx,%d %s\n", oper, address, size, resultV);
    }
    printSummary(result[0], result[1], result[2]);
    // destory();

    return 0;
}

// 初始化整个cache
void init()
{
    cache = (groupNode *)malloc(sizeof(groupNode) * S);
    for (int i = 0; i < S; ++i) {
        cache[i] = (struct lineNode *)malloc(sizeof(struct lineNode) * E);  // 每一组有E行
        for (int j = 0; j < E; ++j) cache[i][j].t = 0;  // 初始化时，全部都未访问
    }
}

// category是缓存的种类，resultV是main传下来的，为了verbose的输出
void setResult(groupNode group, enum Category category, int tag, int pos, char *resultV)
{
    ++result[category];
    group[pos].tag = tag;
    group[pos].t = T;
    if (verbose) strcat(resultV, categoryString[category]);
}

// 这是这整一个代码的核心部分了 具体思路为：
// 遍历这个组的所有行，然后看一下是否命中，最后再进行相应的操作即可
void findCache(uint64_t tag, int group_pos, char *resultV)
{
    groupNode group = cache[group_pos];
    int min_t_pos = 0, empty_line = -1;
    for (int i = 0; i < E; ++i) {
        struct lineNode line = group[i];
        if (!line.t)
            empty_line = i;  // 有空行
        else {
            if (line.tag == tag) {  // 命中，设置hit
                setResult(group, HIT, tag, i, resultV);
                return;
            }
            if (group[min_t_pos].t > line.t)
                min_t_pos = i;  // 取最小的时刻值，也就是最近最少访问的了
        }
    }
    setResult(group, MISS, tag, empty_line, resultV);
    if (empty_line == -1)  //要读或者写但是没有一个 空行 说明得发生eviction
        setResult(group, EVICTION, tag, min_t_pos, resultV);
}

void destory()
{
    // printf("%d\n", S);
    for (int i = 0; i < S; ++i) {
        free(cache[i]);  // 实在是很奇怪，我不知道这为什么会报错--？
        // 如果你知道的话，希望你能告诉我^_^(linxi177229@gmail.com)
    }
    free(cache);
}

FILE *opt(int argc, char **argv)
{
    FILE *tracefile;
    /* Parse the command line 这里c用int是为了保证兼容性，因为有的系统char是unsigned的*/
    for (int c; (c = getopt(argc, argv, "hvsEbt")) != EOF;) {
        switch (c) {
            case 'h': /* print help message */
                usage();
                break;
            case 'v': /* emit additional diagnostic info */
                verbose = 1;
                break;
            case 't': /* 文件 */
                tracefile = fopen(argv[optind], "r");
                if (tracefile == NULL) usage();
                break;
            case 's':  // 组数的位
                s = atoi(argv[optind]);
                if (s <= 0) usage();
                S = 1 << s;
                break;
            case 'E':  // 每一组的行数
                E = atoi(argv[optind]);
                if (E <= 0) usage();
                break;
            case 'b':
                b = atoi(argv[optind]);
                if (b <= 0) usage();
                B = 1 << b;
                break;
        }
    }
    // printf("-%d 4 -%d 1 -%d 4 -t \n", s, E, b);
    return tracefile;
}