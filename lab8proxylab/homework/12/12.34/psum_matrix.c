#include "../csapp.h"

#define N (1 << 12)
#define M (1 << 12)
int matrixA[N][M];
int matrixB[N][M];
int matrixC[N][M];
int nelems_per_thread;

void* mul_core(void* cow)
{
    int start = *(int*)cow;
    int end = start + nelems_per_thread;
    printf("%d      %d\n", start, end);
    for (int i = start; i < end; ++i)  // 采用比较高效的循环模式
        for (int k = start; k < end; ++k) {   
            int r = matrixA[i][k];
            for (int j = start; j < end; ++j) {  
                matrixC[i][j] += matrixB[k][j] * r;
            }
        }
    return matrixC;
}
#define MAXTHREADS 32

int main(int argc, char** argv)
{
    int nthreads = 1;
    if (argc == 2) nthreads = atoi(argv[1]);
    nelems_per_thread = N / nthreads;
    int myid[MAXTHREADS];
    pthread_t tid[MAXTHREADS];
    for (int i = 0; i < nthreads; ++i) {
        myid[i] = i * nelems_per_thread;
        Pthread_create(&tid[i], NULL, mul_core, &myid[i]);
    }

    pthread_exit(0);
}