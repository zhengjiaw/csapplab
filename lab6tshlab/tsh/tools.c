#include "tools.h"

#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#include "base.h"
//这是一个按照空格和''分割buf（c_str）的函数，每一个分割出来的string的首地址会存入到argv[]中，最后一个*argv为NULL
// 注意：返回的首地址存放再static char中，所以第二次调用会覆盖前一次的结果！
//return : 1表示有BG输入，0表示没有
int parseline(const char *buf, char **argv)
{
    static char res[MAXLINE][MAXPARSE];
    char knew = 1;
    size_t argc = 0;
    size_t marks = INT32_MAX;
    for (size_t i = 0, k = 0; buf[i]; ++i, knew = 1) {
        while (buf[i] && buf[i] == ' ' && marks == INT32_MAX) i++;
        while (buf[i] && buf[i] != '\'') {
            if (buf[i] == ' ' && marks == INT32_MAX) break;
            res[argc][k++] = buf[i++];
        }
        if (buf[i] == '\'') {
            if (marks == INT32_MAX) {
                marks = i + 1;
                if (i == 0 || buf[i - 1] != ' ')
                    argc--;
                else
                    knew = 0;
            } else
                marks = INT32_MAX;
        }
        if (knew == 1) res[argc][k] = '\0', k = 0;
        argc++;
        if (!buf[i]) break;
        if (argc >= 100) return -1;
    }
    for (int i = 0; i < argc; ++i) argv[i] = res[i];
    argv[argc] = NULL;
    if (argc == 0) return 0;
    int bg;
    /* should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0) {
        argv[--argc] = NULL;
    }

    return bg;
}
// 这是一个按照delimiter字符的分割cmdline(c_str)函数
static inline size_t split(const char *cmdline, char **argv, char delimiter)
{
    static char buf[MAXLINE]; /* holds local copy of command line */
    strcpy(buf, cmdline);
    buf[strlen(buf)] = 0;
    size_t n = strlen(buf), argc = 0;
    for (size_t i = 0; i < n; ++i) {
        size_t j = i;
        while (j < n && cmdline[j] != delimiter) ++j;
        buf[j] = '\0';
        argv[argc++] = buf + i;
        i = j;
    }
    argv[argc] = NULL;
    return argc;
}
// 这个函数按照'|'分割出一个个指令，存放以NULL结尾的argv中，注意：c_str存放再static char中，第二次调用会覆盖 
// return 命令的个数
size_t pipeCmdline(const char *cmdline, char **argv) { return split(cmdline, argv, '|'); }

//从命令后的空格开始寻找'<' or '>' 重定向成功返回1 出错返回 0
// 重定向函数
size_t redirect(char **argv)
{
    size_t argc = 1;
    while (argv[argc]) {
        if (!strcmp(argv[argc], "<")) {
            if (!argv[++argc]) {
                puts("tsh: syntax error near unexpected token `newline'\n");
                return 0;
            }
            int fd = open(argv[argc], O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "tsh: %s: No such file or directory", argv[argc]);
                return 0;
            }
            argv[argc - 1] = argv[argc] = NULL;
            Dup2(fd, STDIN_FILENO);
        } else if (!strcmp(argv[argc], ">")) {
            if (!argv[++argc]) {
                puts("tsh: syntax error near unexpected token `newline'\n");
                return 0;
            }
            int fd = Open(argv[argc], O_RDWR | O_CREAT, S_IRUSR | S_IWGRP | S_IWUSR | S_IRGRP);
            Dup2(fd, STDOUT_FILENO);
            argv[argc - 1] = argv[argc] = NULL;
        } else if (!strcmp(argv[argc], ">>")) {
            if (!argv[++argc]) {
                puts("tsh: syntax error near unexpected token `newline'\n");
                return 0;
            }
            int fd = open(argv[argc], O_RDWR | O_APPEND, S_IRUSR | S_IWGRP | S_IWUSR | S_IRGRP);
            if (fd == -1) {
                printf("tsh: %s: No such file or directory", argv[argc]);
                return 0;
            }
            Dup2(fd, STDOUT_FILENO);
            argv[argc - 1] = argv[argc] = NULL;
        }
        argc++;
    }
    return 1;
}
