#include <setjmp.h>
#include <unistd.h>

#include "../csapp.h"

char *tfgets(char *res, int n, FILE *restrict_stream)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    int nfds = STDIN_FILENO + 1;
    Select(nfds, &readfds, NULL, NULL, &timeout);
    if (FD_ISSET(STDIN_FILENO, &readfds)) return fgets(res, n, restrict_stream);
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
