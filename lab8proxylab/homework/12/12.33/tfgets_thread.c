#include <pthread.h>
#include <setjmp.h>

#include "../csapp.h"
sigjmp_buf buf;
struct Args {
    char *res;
    int n;
    FILE *restrict_stream
};
volatile unsigned char flag = 0;
void sigchld_handler(int sig) { siglongjmp(buf, 1); }
void thread(struct Args *args)  // 读线程
{
    fgets(args->res, args->n, args->restrict_stream);
    flag = 1;
    Pthread_exit(0);
}

void thread_sleep()  // 计时，充当信号量了
{
    Sleep(5);
    flag = 2;
    pthread_exit(0);
}
char *tfgets(char *res, int n, FILE *restrict_stream)
{
    pthread_t tid, tid_sleep;
    struct Args args;
    args.res = res;
    args.n = n;
    args.restrict_stream = restrict_stream;
    Pthread_create(&tid, NULL, thread, &args);
    Pthread_create(&tid_sleep, NULL, thread_sleep, NULL);
    while (!flag) {
    }
    if (flag == 1) {
        Pthread_cancel(tid_sleep);
        return res;
    }
    Pthread_cancel(tid);
    return NULL;
}

int main(int argc, char *argv[])
{
    char buf[MAXLINE];

    if (tfgets(buf, MAXLINE, stdin) == NULL)
        printf("BOOM!\n");
    else
        printf("%s", buf);

    return 0;
}
