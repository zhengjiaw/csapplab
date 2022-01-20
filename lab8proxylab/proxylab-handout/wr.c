#include "wr.h"

#include "csapp.h"
sem_t mutex, w;
unsigned int read_cnt;
unsigned volatile int readCnt;
unsigned volatile char prev_is_write;  // 多加一个这个函数的判断

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