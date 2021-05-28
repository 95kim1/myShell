/* $begin shellmain */
#include "csapp.h"
#include <errno.h>
#include <string.h>
#define MAXARGS 128
#define MAXPATH 1024

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
/* built-in functions */
void cd(char *argv[]);
void pwd();
//void echo(char *argv[]);

int main()
{
    char cmdline[MAXLINE]; /* Command line */

    while (1)
    {
        /* Read */
        printf("CSE4100-SP-P#4> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    }
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline)
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
            if (execve(argv[0], argv, environ) < 0)
            { //ex) /bin/ls ls -al &
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
            exit(0);
        }

        /* Parent waits for foreground job to terminate */
        if (!bg)
        {
            int status;
            waitpid(pid, &status, 0);
        }
        else //when there is backgrount process!
            printf("%d %s", pid, cmdline);
    }
    return;
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
        unix_error("cd error");

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
