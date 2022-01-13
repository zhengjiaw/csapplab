#include "jobs.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sio.h"

struct Group group[MAXARGS];
int nextjid = 1;
size_t curjob = 0;

int verbose = 0;            /* if true, print additional output */
struct job_t jobs[MAXJOBS]; /* The job list */
size_t jobmru = 0;
volatile int fg_num = 0;  // 前台进程中的运行进程个数
extern pid_t shellId;
/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pgid)
{
    tcsetpgrp(0, pgid);  // Move fg to foreground
    // 等待前台进程组中所有进程的结束
    // 给每一个子进程维护一个状态，signalHandle处理状态
    // 循环检测所有的进程的状态位，只要还有进程的状态是fs我们就接着pause
    while (fg_num) pause();

    tcsetpgrp(0, shellId);  // 还原shell
    return;
}

static int restartjob(struct job_t *job, int state)
{
    if (job == NULL) return -1;
    if (state == FG) fg_num++;
    if (job->state == UNDEF) return -1;
    if (job->state == BG && state == BG) return 0;
    kill(-job->pgrp, SIGCONT);  // 唤醒这一整个组
    job->state = state;
    return 1;
}
// 读出pid或者是 job number 对应的jobs中的jid
static int bg_fg_read_pos(char **argv)
{
    int pos;
    if (argv[1] == NULL || argv[1][0] == '\0')  // 没有输入数字
        return -1;
    else if (isdigit(argv[1][0])) {
        pos = (size_t)pid2jid(getInt(argv[1], 0, "bg_fg_read_pos"));
        if (pos == 0) return -2;  // No such process
    } else if (argv[1][0] == '%') {
        pos = (size_t)getInt(argv[1] + 1, 0, "bg_fg_read_pos");
        if (pos >= nextjid || pos == 0) return -3;  // No such job
    } else
        return -4;  // 不是数字
    return pos;
}

// let process to background, successfully completed return 0 else -1
int do_bg_fg(char **argv)
{
    int pos = bg_fg_read_pos(argv);  // 得到jid
    if (pos == -1) {                 // 没有输入数字
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return -1;
    }
    if (pos == -2) {  // process超过范围
        printf("(%s): No such process\n", argv[1]);
        return -1;
    }
    if (pos == -3) {  // job超过范围
        printf("%s: No such job\n", argv[1]);
        return -1;
    }
    if (pos == -4) {  // 不是数字
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return -1;
    }
    int res, state = (!strcmp(argv[0], "bg") ? BG : FG);  // 确定前后台
    struct job_t *job = getjobjid(jobs, pos);             // 得到相对应的job
    res = restartjob(job, state);                         // 相当于唤醒这一个组
    if (state == BG) {   // 后台可以直接返回了
        printf("[%d] (%d) %s\n", job->jid, job->pgrp, job->cmdline);
        return 0;
    }
    if (res == -1) {
        printf("%s: No such job\n", argv[1]);
        return -1;
    } else if (res == 0) {
        printf("tsh: %s: job %d already in %s\n", argv[0], job->jid,
               state == BG ? "background" : "foreground");
        return -1;
    }
    jobmru = curjob;
    curjob = (size_t)pos;
    waitfg(job->pgrp);
    return 0;
}

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job)
{
    job->processNumber = 0;
    job->pgrp = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs)
{
    for (int i = 0; i < MAXJOBS; i++) clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs)
{
    int i, max = 0;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid > max) max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
/*addjob successfully completed return 1 else 0*/
int addjob(struct job_t *jobs, pid_t pgrp, int state, char *cmdline, int processNumber)
{
    if (pgrp < 1) return 0;

    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgrp == 0) {
            jobs[i].pgrp = pgrp;
            jobs[i].state = state;
            jobs[i].processNumber = processNumber;
            jobs[i].jid = nextjid++;
            if (nextjid >= MAXJOBS) nextjid = 1;  // circular queue
            strcpy(jobs[i].cmdline, cmdline);
            if (verbose) {
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pgrp, jobs[i].cmdline);
            }
            return 1;
        }
    }
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
/*delete successfully completed return 1 else 0*/
int deletejob(struct job_t *jobs, pid_t pgrp)
{
    int i;

    if (pgrp < 1) return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgrp == pgrp) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs) + 1;
            return 1;
        }
    }
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
inline struct job_t *getjobpid(struct job_t *jobs, pid_t pid)
{
    if (pid < 1) return NULL;
    for (int i = 0; i < MAXJOBS; ++i)
        if (jobs[i].pgrp == pid) return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
inline struct job_t *getjobjid(struct job_t *jobs, int jid)
{
    if (jid < 1) return NULL;
    for (int i = 0; i < MAXJOBS; ++i)
        if (jobs[i].jid == jid) return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
inline int pid2jid(pid_t pid)
{
    if (pid < 1) return 0;
    for (int i = 0; i < MAXJOBS; i++)
        if (jobs[i].pgrp == pid) return jobs[i].jid;
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs)
{
    for (size_t i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pgrp != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pgrp);
            switch (jobs[i].state) {
                case BG:
                case FG:
                    printf("Running ");
                    break;
                case STOP:
                    printf("Stopped ");
                    break;
                default:
                    printf("listjobs: Internal error: job[%zd].state=%d ", i, jobs[i].state);
            }
            printf("%s\n", jobs[i].cmdline);
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/