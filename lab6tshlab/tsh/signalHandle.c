#include "signalHandle.h"

#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "base.h"
#include "jobs.h"
#include "sio.h"
extern pid_t shellId;

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void initSignal()
{
    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT, sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler); /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler); /* Terminated or stopped child */
    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);
    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);
}

static inline void stopHandler(pid_t pid)  // ignore
{
}
// 用于安全的字符串输出，他安全的原因是因为没有缓冲区，直接写到OS里面的
inline static void put_help(int jid, int pgid, char *c, int singal_number)
{
    Sio_puts("Job");
    Sio_puts(" [");
    Sio_putl(jid);
    Sio_puts("]");
    Sio_puts(" (");
    Sio_putl(pgid);
    Sio_puts(") ");
    Sio_puts(c);
    Sio_puts(" by signal ");
    Sio_putl(singal_number);
    Sio_puts("\n");
}

/*
 * Signal - wrapper for the sigaction function
 */
// 处理儿子死亡，暂停，被信号杀死、继续的情况
void sigchld_handler(int sig)
{
    int olderrno = errno, status;
    sigset_t mask_all, prev_all;

    Sigfillset(&mask_all);
    pid_t pid, pgid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) >
           0) {  // 循环等待子进程退出，确保不产生僵尸进程
        int i = 0;
        for (; i < MAXARGS; ++i)
            if (pid == group[i].pid) {
                pgid = group[i].pgid;
                break;
            }
        struct job_t *job = getjobpid(jobs, pgid);
        // exited
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            group[i].pid = 0;                // delete pid
            if (job->state == FG) fg_num--;  // 前台进程数量 --
            if (--job->processNumber > 0) continue;  // 如果这个进程组 还有进程，就稍微等待一下
            if (WIFSIGNALED(status))
                put_help(pid2jid(pgid), pgid, "terminated", 2);  // 死于信号，发个信息给终端让用户看

            Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);  // signal atomic 删除这个进程组
            deletejob(jobs, pgid);
            Sigprocmask(SIG_SETMASK, &prev_all, NULL);
            // stop
        } else if (WIFSTOPPED(status)) {
            put_help(pid2jid(pgid), pgid, "stopped", 20);
            if (job->state == FG) fg_num--;
            job->state = STOP;
            // continue
        } else if (WIFCONTINUED(status)) {
            // 事实上在这里也可以这么处理:  if (job->state == FG) fg_num++;
            // 但是考虑到前台进程一定是由shell指定的，所以不会是被其他进程设定
            // 我们在shell处理好fg即可，如果这么处理的话，应该在fg的时候要考虑，要保证信号先到然后再等待，比较难以实现
        }
    }
    if (pid == -1 && errno != ECHILD) Sio_error("waitpid error");
    errno = olderrno;
}
// 辅助函数 用于查找前台进程组对应的job
inline static struct job_t *findFgGroup()
{
    pid_t pid, pgid;
    int i = 0;
    for (; i < MAXARGS; ++i)
        if (jobs[i].state == FG) {
            pgid = group[i].pgid;
            break;
        }
    return getjobpid(jobs, pgid);
}
void sigtstp_handler(int sig)
{
    struct job_t *job = findFgGroup();
    kill(-job->pgrp, SIGSTOP);  // 暂停这一个组
}  // signal
void sigint_handler(int sig)
{
    struct job_t *job = findFgGroup();
    kill(-job->pgrp, SIGINT);  // 杀掉这一个组

}  // signal

void sigquit_handler(int sig)
{
    Sio_puts("Terminating after receipt of SIGQUIT signal\n");
    exit(1);  // to do
}

/************************************
 * Wrappers for Unix signal functions
 ***********************************/

/* $begin sigaction */
handler_t *Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
                                  /* Restart syscalls if possible  */
    action.sa_flags = SA_RESTART;

    if (sigaction(signum, &action, &old_action) < 0) unix_error("Signal error");
    return (old_action.sa_handler);
}
/* $end sigaction */

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if (sigprocmask(how, set, oldset) < 0) unix_error("Sigprocmask error");
    return;
}

void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0) unix_error("Sigemptyset error");
    return;
}

void Sigfillset(sigset_t *set)
{
    if (sigfillset(set) < 0) unix_error("Sigfillset error");
    return;
}

void Sigaddset(sigset_t *set, int signum)
{
    if (sigaddset(set, signum) < 0) unix_error("Sigaddset error");
    return;
}

void Sigdelset(sigset_t *set, int signum)
{
    if (sigdelset(set, signum) < 0) unix_error("Sigdelset error");
    return;
}

int Sigismember(const sigset_t *set, int signum)
{
    int rc;
    if ((rc = sigismember(set, signum)) < 0) unix_error("Sigismember error");
    return rc;
}

int Sigsuspend(const sigset_t *set)
{
    int rc = sigsuspend(set); /* always returns -1 */
    if (errno != EINTR) unix_error("Sigsuspend error");
    return rc;
}
