#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "../csapp.h"
sem_t mutex, w;
unsigned int read_cnt;
unsigned volatile int readCnt;
unsigned volatile char prev_is_write;  // 多加一个这个函数的判断
volatile int book;
void init()
{
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    prev_is_write = 0;
    readCnt = 0;
    book = 0;
}

void reader(void);
void writer();

int main(int argc, char **argv)
{
    init();
    read_cnt = 2;
    unsigned int write_cnt = 10; // 即便是开到 100 我们的读者还是正常的读的
    setbuf(stdout, NULL);
    if (argc >= 2) read_cnt = atoi(argv[1]);
    if (argc >= 3) write_cnt = atoi(argv[2]);
    pthread_t pid;
    for (int i = 0; i < read_cnt; ++i) Pthread_create(&pid, NULL, reader, NULL);
    for (int i = 1; i <= write_cnt; ++i) Pthread_create(&pid, NULL, writer, NULL);
    Pthread_exit(0);
}

void reader(void)
{
    while (1) {
        P(&mutex);
        ++readCnt;
        if (readCnt == 1) P(&w);  // 第一个读者,需要测试
        V(&mutex);

        P(&mutex);
        printf("%d ___ %lu\n", book, Pthread_self());
        if (--readCnt == 0) V(&w);  // 最后一个读者，需要增加
        prev_is_write = 0;
        V(&mutex);
    }
}
void writer(void)
{
    while (1) {
        while (prev_is_write && readCnt)
            ;
        P(&w);
        book = time(0);
        prev_is_write = 1;
        V(&w);
    }
}
