/*
 * 5.17.c
 这份代码是参考了他的：
 https://dreamanddead.github.io/CSAPP-3e-Solutions/chapter5/5.17/
 我将测试集加大了，这样应该可以保证是正确的
 另外我将effective_memset里面的逻辑写得更加紧凑了，并且用了循环展开，
 这样大概可以提升3%的速度（因为我没去学怎么测CPE，所以就只这么模糊地测试比较了一下）
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *basic_memset(void *s, int c, size_t n)
{
    size_t cnt = 0;
    unsigned char *schar = s;
    while (cnt < n) {
        *schar++ = (unsigned char)c;
        cnt++;
    }
    return s;
}

/*
 * K = sizeof(unsigned long)
 * cs store K chars for memset
 */
void *effective_memset(void *s, unsigned long cs, size_t n)
{
    /* align to K */
    size_t K = sizeof(unsigned long);
    unsigned char *schar = s;
    ++n;
    while (--n && (size_t)schar % K != 0) {  //一直char赋值直到找到第一个对齐的点
        *schar++ = (unsigned char)cs;
    }

    /* set K chars one time */

    unsigned long *slong = (unsigned long *)schar;
    size_t tail = n % K;
    unsigned long *loopEnd = (unsigned long *)(n + (unsigned long)slong - tail);
#define ex 8  // 展开八次，并不会带来明显的提速，主要还是靠8字节级别的赋值
    loopEnd -= ex;
    while (slong < loopEnd) {
        *slong++ = cs;
        *(slong + 1) = cs;
        *(slong + 2) = cs;
        *(slong + 3) = cs;
        *(slong + 4) = cs;
        *(slong + 5) = cs;
        *(slong + 6) = cs;
        *(slong + 7) = cs;
        slong += ex;
    }
    loopEnd += ex;

    while (slong < loopEnd) *slong++ = cs;

    /* pad the tail part */
    schar = (unsigned char *)slong;
    for (size_t i = 0; i < tail; i++) {
        *schar++ = (unsigned char)cs;
    }
    return s;
}
int main(int argc, char *argv[])
{
    size_t n = 30000;
    const size_t space = sizeof(char) * (n);

    void *basic_space = malloc(space);
    void *effective_space = malloc(space);

    int basic_fill = 0xFF;
    unsigned long effective_fill = ~0; // 111111111...111

    for (int i = 0; i < n; ++i) {  // 从0测到30000，这个测试集已经足够大了！
        void *basic_addr = basic_memset(basic_space, basic_fill, i);
        void *effective_addr = effective_memset(effective_space, effective_fill, i);
        assert(memcmp(basic_space, effective_space, i) == 0);
        assert(effective_addr == effective_space);
    }
    puts("Successful");
    free(basic_space);
    free(effective_space);
    return 0;
}
