/* $begin shellmain */
#include "csapp.h"
#include <errno.h>
#include <string.h>
#define MAXARGS 128
#define MAXPATH 1024
#define READ 0
#define WRITE 1

char shellPath[MAXPATH];

/* Function prototypes */
void Eval(char *cmdline);
int parceBgUnit(char cmd[][MAXPATH], char *cmdline);

void Eval2(char *cmdline);
int parceByPipeline(char cmd[][MAXPATH], char *cmdline);

void eval(char *cmdline, int *flag, int *fd_p2c, int *fd_c2p);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void getCmdPath(char *prog, char *path);
/* built-in functions */
void cd(char *argv[]);
void pwd();
//void echo(char *argv[]);

int main()
{
    char cmdline[MAXLINE]; /* Command line */

    if (getcwd(shellPath, MAXPATH) == NULL)
        unix_error("error");

    while (1)
    {
        /* Read */
        printf("CSE4100-SP-P#4> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        Eval(cmdline);
    }
}
/* $end shellmain */

/* separate background commands */
void Eval(char *cmdline)
{
    char cmd[MAXARGS][MAXPATH];
    int cnt = parceBgUnit(cmd, cmdline);
    for (int i = 0; i < cnt; i++)
        Eval2(cmd[i]);
}

/* "ls | cat & ls" -> "ls | cat",  " ls"*/
int parceBgUnit(char cmd[][MAXPATH], char *cmdline)
{
    int tot_len = strlen(cmdline);
    int cnt = 0;
    int j = 0;
    for (int i = 0; i < tot_len; i++)
    {
        cmd[cnt][j++] = cmdline[i];

        if (cmdline[i] == '&')
        {
            cmd[cnt][j] = '\0';
            cnt++;
            j = 0;
        }
    }
    if (j > 0)
        cmd[cnt++][j] = '\0';
    return cnt;
}

/* separate commands with pipeline */
void Eval2(char *cmdline)
{
    char cmd[MAXARGS][MAXPATH];
    int cnt = parceByPipeline(cmd, cmdline);
    int flag[2] = {0, 0}; // flag[0]==1 => parent -> child  flag[1]==1 => child -> parent
    int fd_p2c[2];        // will be piped to transfer data from parent to child
    int fd_c2p[2];        // will be piped to transefer data from child to parent
    //printf("--Eval2: cnt: %d==\n", cnt);
    for (int i = 0; i < cnt; i++)
    {
        //printf("--Eval2: %s, eval==\n", cmd[i]);

        if (i == 0)
            flag[0] = 0;
        else if (cnt > 1) /* pipe: parent to child */
        {
            flag[0] = 1;
            pipe(fd_p2c);

            /* read from fd_c2p[READ] and write to fd_p2c[WRITE]*/
            char line[8912];
            line[0] = '\0';

            //printf("---before read: %s,,\n===", line);
            while (read(fd_c2p[READ], line, 8912) > 0)
            {
                write(fd_p2c[WRITE], line, strlen(line));
                //printf("--pipe read: %s,\n==", line);
            }

            close(fd_c2p[READ]);
        }

        if (i == cnt - 1)
            flag[1] = 0;
        else if (cnt > 1)
        { /* pipe: child to parent */
            flag[1] = 1;
            pipe(fd_c2p);
        }

        eval(cmd[i], flag, fd_p2c, fd_c2p);
    }
}

/* "ls | cat" -> "ls ", " cat" */
int parceByPipeline(char cmd[][MAXPATH], char *cmdline)
{
    int tot_len = strlen(cmdline);
    int cnt = 0;
    int j = 0;
    for (int i = 0; i < tot_len; i++)
    {
        cmd[cnt][j++] = cmdline[i];

        if (cmdline[i] == '|')
        {
            cmd[cnt][j - 1] = '\0';
            cnt++;
            j = 0;
        }
    }
    if (j > 0)
        cmd[cnt++][j] = '\0';
    return cnt;
}

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline, int *flag, int *fd_p2c, int *fd_c2p)
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);

    if (argv[0] == NULL)
        return; /* Ignore empty lines */

    if (!builtin_command(argv))
    { //quit -> exit(0), & -> ignore, other -> run
        if ((pid = Fork()) == 0)
        {
            char prog[MAXPATH];
            getCmdPath(prog, argv[0]);

            if (flag[0]) // pipe: parent to child
            {
                dup2(fd_p2c[READ], READ); // In child process, stdio indicates file that fd_p2c[READ] is indicating
                close(fd_p2c[READ]);      // stdio indicates file that fd_p2c[READ] indicating, so close this descriptor
                close(fd_p2c[WRITE]);     // do not use fd_p2c[WRITE] in child process
            }

            if (flag[1]) // pipe: child to parent
            {
                dup2(fd_c2p[WRITE], WRITE);
                close(fd_c2p[WRITE]);
                close(fd_c2p[READ]);
            }

            if (execve(prog, argv, environ) < 0)
            {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
            exit(0);
        }

        if (flag[0])
        {
            close(fd_p2c[READ]);
            close(fd_p2c[WRITE]);
        }

        if (flag[1])
            close(fd_c2p[WRITE]);

        /* Parent waits for foreground job to terminate */
        if (!bg)
        {
            int status;
            Waitpid(pid, &status, 0);
        }
        else //when there is backgrount process!
            printf("%d %s", pid, cmdline);
    }
    return;
}

/* get directory which has shell command programs */
void getCmdPath(char *prog, char *path)
{
    int len = strlen(path);
    int i;
    for (i = len - 1; i >= 0 && path[i] != '/'; i--)
        ;

    strcpy(prog, shellPath);
    strcat(prog, "/");

    if (i == -1)
        strcat(prog, path);
    else
        strcat(prog, path + i + 1);
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char *argv[])
{
    if (!strcmp(argv[0], "exit")) /* exit command */
        exit(0);
    if (!strcmp(argv[0], "pwd")) /* pwd command */
    {
        pwd();
        return 1;
    }
    if (!strcmp(argv[0], "cd")) /* cd command */
    {
        cd(argv);
        return 1;
    }
    // if (!strcmp(argv[0], "echo")) /* echo command without pipe */
    // {
    //     echo(argv);
    //     return 1;
    // }
    if (!strcmp(argv[0], "&")) /* Ignore singleton & */
        return 1;
    return 0; /* Not a builtin command */
}
/* $end eval */

/* $begin built in functions */
/* cd builtin command */
void cd(char *argv[])
{
    /* no options */
    if (argv[1] == NULL)
    {
        printf("help: cd (path)\n");
        return;
    }

    char path[MAXPATH];
    strcpy(path, argv[1]);

    if (!strcmp(path, "~")) /* HOME directory */
    {
        char *home;
        if ((home = getenv("HOME")) == NULL)
        {
            printf("env error: No $HOME\n");
            return;
        }
        strcpy(path, home);
    }

    if (chdir(path) < 0)
    {
        fprintf(stderr, "cd error: %s\n", strerror(errno));
        return;
    }

    pwd();
}

void pwd()
{
    char path[MAXPATH];
    if (getcwd(path, sizeof path) == NULL)
    {
        printf("error: can not get current working directory\n");
        exit(0);
    }
    printf("%s\n", path);
}
/* $end built in functions */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv)
{
    char *delim; /* Points to first space delimiter */
    int argc;    /* Number of args */
    int bg;      /* Background job? */

    buf[strlen(buf) - 1] = ' ';   /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' ')))
    {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0) /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}

// void echo(char *argv[])
// {
//     for (int i = 1; argv[i] != NULL; i++)
//     {
//         printf("%s", argv[i]);
//         if (argv[i + 1] != NULL)
//             printf(" ");
//     }
//     printf("\n");
// }
/* $end parseline */
