/*
 * tsh - A tiny shell program with job control
 *
 */

#include <stdio.h>

#include "base.h"
#include "jobs.h"
#include "signalHandle.h"
#include "tools.h"
/* Global variables */
extern char **environ;          /* defined in libc */
static char prompt[] = "tsh> "; /* command line prompt (DO NOT CHANGE) */
static char sbuf[MAXLINE];      /* for composing sprintf messages */
pid_t shellId;
/* End global variables */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void init(int argc, char **argv, int *emit_prompt);
inline static int readCmdline(char *cmdline);
/*
 * main - The shell's main routine
 * 执行init、readCmdline、eval
 */
int main(int argc, char **argv)
{
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */
    init(argc, argv, &emit_prompt);

    while (1) {
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if (readCmdline(cmdline) == 1) continue;
        /* Evaluate the command line 核心 */
        eval(cmdline);
    }

    exit(0); /* control never reaches here */
}

// 处理命令行选项，以及初始化信号和job
void init(int argc, char **argv, int *emit_prompt)
{
    shellId = getpid();
    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);
    char c;
    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
            case 'h': /* print help message */
                usage();
                break;
            case 'v': /* emit additional diagnostic info */
                verbose = 1;
                break;
            case 'p':             /* don't print a prompt */
                *emit_prompt = 0; /* handy for automatic testing */
                break;
            default:
                usage();
        }
    }

    initSignal();
    /* Initialize the job list */
    initjobs(jobs);
}
// 按行读取string，并返回读出的char数量
inline static int readCmdline(char *cmdline)
{
    /* Read command line */
    if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)) app_error("fgets error");
    if (feof(stdin)) { /* End of file (ctrl-d) */
        exit(0);
    }
    size_t n = strlen(cmdline);
    cmdline[n - 1] = '\0';
    return n;
}

#define max(a, b) ((a) > (b) ? (a) : (b))
// 核心逻辑函数，主要是来处理管道 所涉及的一系列 子进程重定向的问题
// 以及 >> << > 这样的重定向符号的处理
// 然后再 加载运行 由管道分割的 每一个命令。 && 和 || 也可以参考这种方法实现
inline static void mySystem(char *cmdline)
{
    char *argv[MAXARGS];
    sigset_t origMask, blockMask;
    sigemptyset(&blockMask);
    /* Block SIGCHLD 为了保证addjob在deletejob之前, 且保证前台进程的终止能被检查到 */
    sigaddset(&blockMask, SIGCHLD);

    sigprocmask(SIG_BLOCK, &blockMask, &origMask);
    pid_t pid, pgid;
    signal(SIGTTOU, SIG_IGN);
    char *cmdlinei[MAXARGS];
    int bg = 0;
    setbuf(stdout, NULL);  // 取消缓冲区

    // 我们开始 取出每一个命令
    // cmdlinei 分割出来的管道中的每一个cmdline
    size_t nProcess = pipeCmdline(cmdline, cmdlinei);
    int pipePrevInput = 0, pipeCurrent[2];  // 建立管道

    // 这里是在建立n - 1个管道，让父进程fork出 n - 1个进程，然后依次让他们重定向
    for (int i = 1; i <= nProcess; ++i) {
        if (pipe(pipeCurrent) == -1) unix_error("pipe error \n");
        int bg_t = parseline(cmdlinei[i - 1], argv);

        if ((pid = Fork()) == 0) {
            if (i != nProcess) Dup2(pipeCurrent[1], 1);  // 最后的 输出 不重定向
            close(pipeCurrent[1]);
            setpgid(0, (i == 1) ? 0 : pgid);
            Dup2(pipePrevInput, 0);  // 重定向current 输入 为上一个进程的管道的输出
            close(pipePrevInput);  // 记得要关！！不可能close 会有问题，并且也不是一个好习惯
            redirect(argv);  // 实现重定向，事实上不必检查这个函数的结果，因为execve会检查
            if (argv[0][0] == '.' || argv[0][0] == '/')  //直接加载，这只是一个简单判断
                execve(argv[0], argv, environ);
            else
                execvp(argv[0], argv);  // from path to execv
            printf("%s: Command not found\n", argv[0]);
            _exit(127);
        }
        bg = max(bg, bg_t);  // 只要有一次bg我们就当这整一个进程组是后台进程了
        if (i == 1) pgid = pid;  // 现在是父进程， 如果是处理第一个子进程 ，那么 他就是组长
        setpgid(pid, pgid);      
        close(pipeCurrent[1]);
        // 建立pid 和pgid 之间的映射，为了之后的处理管道组中有多个进程，如果是c++的话，我会使用unordered_map 
        for (int i = 0; i < MAXARGS; ++i)
            if (!group[i].pid) {
                group[i].pid = pid, group[i].pgid = pgid;
                break;
            }

        pipePrevInput = pipeCurrent[0];   // 管道逐步向前推进
    }
    if (bg) {
        addjob(jobs, pgid, BG, cmdline, nProcess);
        sigprocmask(SIG_SETMASK, &origMask, NULL);
        printf("[%d] (%d) %s\n", getjobpid(jobs, pid)->jid, pid, cmdline);
    } else {
        addjob(jobs, pgid, FG, cmdline, nProcess);
        fg_num += nProcess;
        sigprocmask(SIG_SETMASK, &origMask, NULL);  
        waitfg(pgid);
    }
}

//
void eval(char *cmdline)
{
    char *argv[MAXARGS];
    if (argv[0] == NULL) return;
    parseline(cmdline, argv);  // 按照空格和''分割cmdline
    if (!builtin_cmd(argv)) {  // 目前仅内置命令不支持管道
        mySystem(cmdline);
    }
}

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
int builtin_cmd(char **argv)
{
    if (!argv[0]) return 0;
    int stateBG = !strcmp("bg", argv[0]);
    if (stateBG || !strcmp("fg", argv[0])) {
        switch (do_bg_fg(argv)) {
        }
        return 1;
    } else if (!strcmp("jobs", argv[0])) {
        listjobs(jobs);
        return 1;
    } else if (!strcmp("quit", argv[0])) {
        exit(0);
    }
    return 0; /* not a builtin command */
}
