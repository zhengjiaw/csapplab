#ifndef BASE_H
#define BASE_H
#include <fcntl.h>
#include <errno.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include<signal.h>
/* Misc manifest constants */
#define MAXLINE 1024 /* 111 max line size */
#define MAXARGS 128  /* max args on a command line */

/* Our own error-handling functions */
void unix_error(char *msg);
void posix_error(int code, char *msg);
void dns_error(char *msg);
void app_error(char *msg);
void usage(void);

/* Process control wrappers */
pid_t Fork(void);
void Execve(const char *filename, char *const argv[], char *const envp[]);
pid_t Wait(int *status);
pid_t Waitpid(pid_t pid, int *iptr, int options);
void Kill(pid_t pid, int signum);
unsigned int Sleep(unsigned int secs);
void Pause(void);
unsigned int Alarm(unsigned int seconds);
void Setpgid(pid_t pid, pid_t pgid);
pid_t Getpgrp();

int Dup2(int fd1, int fd2);
int Open(const char *pathname, int flags, mode_t mode);

#endif