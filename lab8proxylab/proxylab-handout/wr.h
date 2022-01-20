#ifndef WR_H
#define WR_H
#include<semaphore.h>
extern sem_t mutex, w;
extern unsigned int read_cnt;
extern unsigned volatile int readCnt;
extern unsigned volatile char prev_is_write;  // 多加一个这个函数的判断
void lock_reader(void);
void unlock_reader(void);
void lock_writer(void);
void unlock_writer(void);
void* init_wr();

#endif