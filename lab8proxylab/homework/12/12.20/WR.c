#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "../csapp.h"
sem_t mutex, w, readCnt; //readCnt信号量来限制读者的数量
unsigned int read_cnt = 3;
unsigned volatile char prev_is_write;  // 多加一个这个函数的判断
volatile int book;
void init()
{
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    Sem_init(&readCnt, 0, read_cnt);
    prev_is_write = 0;
    book = 0;
}

void reader(void);
void writer();

int main(int argc, char **argv)
{
    init();
    unsigned int write_cnt = 10;  // 即便是开到 100 我们的读者还是正常的读的
    setbuf(stdout, NULL);
    if (argc >= 2) read_cnt = atoi(argv[1]);
    if (argc >= 3) write_cnt = atoi(argv[2]);
    pthread_t pid;
    for (int i = 0; i < read_cnt + 10; ++i) Pthread_create(&pid, NULL, reader, NULL);
    for (int i = 1; i <= write_cnt; ++i) Pthread_create(&pid, NULL, writer, NULL);
    Pthread_exit(0);
}

void reader(void)
{
    while (1) {
        P(&readCnt);
        P(&mutex);
        printf("%d ___ %lu\n", book, Pthread_self());
        V(&mutex);
        sleep(1);  // 这样我们每一秒都 只能看到readCnt个新的输出，说明读者最多只有readCnt个
        V(&readCnt);
    }
}
void writer(void)
{
    while (1) {
        P(&mutex);
        book = time(0);
        V(&mutex);
    }
}
