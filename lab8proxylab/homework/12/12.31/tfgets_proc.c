#include <setjmp.h>

#include "../csapp.h"
sigjmp_buf buf;

void sigchld_handler(int sig) { siglongjmp(buf, 1); }

char *tfgets(char *res, int n, FILE *restrict_stream)
{
    if (Fork() == 0) {
        Sleep(5);
        exit(0);
    }
    Signal(SIGCHLD, sigchld_handler);
    switch (sigsetjmp(buf, 1)) {
        case 0:
            return fgets(res, n, restrict_stream);
        case 1:
            return NULL;
    }
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
