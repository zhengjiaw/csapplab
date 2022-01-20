#include "thread_run.h"

static ithread threads[THREAD_LIMIT];
static int nthreads;

sbuf_t sbuf;

void init(void)
{
    nthreads = INIT_THREAD_N;
    sbuf_init(&sbuf, SBUFSIZE);

    // create initail server threads
    create_threads(0, nthreads);
}
void doit(int fd);

void *serve_thread(void)
{
    Pthread_detach(pthread_self());
    while (1) {
        usleep(1000*330);
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        Close(connfd);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // 可以取消
    }
}

void create_threads(int start, int end)
{
    for (int i = start; i < end; i++) {
        // create thread
        Pthread_create(&(threads[i].tid), NULL, serve_thread, NULL);
    }
}
int prev = -1;
void *adjust_threads(void *vargp)
{
    sbuf_t *sp = &sbuf;
    while (1) {
        prev = nthreads;
        // if sbuf is full, double threads
        if (sbuf_full(sp)) {
            if (nthreads == THREAD_LIMIT) {
                fprintf(stderr, "too many threads, can't double\n");
                continue;
            }
            // double n
            int dn = 2 * nthreads;
            create_threads(nthreads, dn);
            nthreads = dn;
        }
        // half threads
        else if (sbuf_empty(sp)) {
            if (nthreads == 1) continue;

            // half n
            int hn = nthreads / 2;
            /*
             * all server thread are divide to 2 parts
             *
             * keep [0, hn] running
             * kill [hn, nthreads] threads
             */
            int i;
            for (i = hn; i < nthreads; i++) {
                Pthread_cancel(threads[i].tid);
            }
            nthreads = hn;
        }
        if (prev != nthreads) printf("nthreads : %d\n", nthreads);
    }
}
