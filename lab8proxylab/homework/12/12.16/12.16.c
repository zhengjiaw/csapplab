#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void thread() { printf("%lu\n", pthread_self()); }
int main(int argc, char **argv)
{
    if (argc == 1) exit(1);
    size_t n = (size_t)atoi(argv[1]);
    pthread_t tid;
    for (size_t i = 0; i < n; ++i) {
        pthread_create(&tid, NULL, thread, NULL);
    }
    pthread_exit(0);
}