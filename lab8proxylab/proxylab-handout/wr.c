#include "wr.h"

#include "csapp.h"
sem_t mutex, w;
unsigned int read_cnt;
unsigned volatile int readCnt;
unsigned volatile char prev_is_write;  // 多加一个这个函数的判断
// 这是一个经过改良的读者-写者模型
// 主要是防止 有多个写者等待，会一直调用写者
// 添加了prev_is_write,只要上次访问了写者，就会限制写者访问了。（不过这浪费了一些cpu时间）
// 可以有效地避免上述情况，能确保读者能访问
void *init_wr()
{
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    prev_is_write = 0;
    readCnt = 0;
}

inline void lock_reader(void)
{
    P(&mutex);
    ++readCnt;
    if (readCnt == 1) P(&w);  // 第一个读者,需要测试
    V(&mutex);
    P(&mutex);
}
inline void unlock_reader(void)
{
    if (--readCnt == 0) V(&w);  // 最后一个读者，需要增加
    prev_is_write = 0;
    V(&mutex);
}
inline void lock_writer(void)
{
    while (prev_is_write && readCnt)
        ;
    P(&w);
}
inline void unlock_writer(void)
{
    prev_is_write = 1;
    V(&w);
}