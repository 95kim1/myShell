/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * No.4 cat command
 * 
 * this source code is for 'cat' of linux shell.
 * but it has no 100% functions.
 * 
 * option:
 * -n: add line numbers
 */

#include "csapp.h"
#include <string.h>
#define MAX_DEPTH 128 /* maximum path depth */
#define MAX_PATH 1024 /* maximum path length */

enum
{
  _n_,
};

struct cmd
{
  char path[MAX_DEPTH][MAX_PATH];
  int len;
  int option;
  int fd; //output file descriptor
};

void initCmd(struct cmd *cmd);

void parceCmd(struct cmd *cmd, char *argv[]);
void getOption(struct cmd *cmd, char *argv[]);
void getPath(struct cmd *cmd, char *argv[]);

void cat_(struct cmd cmd);

int main(int argc, char *argv[])
{
  struct cmd cmd;

  initCmd(&cmd);

  parceCmd(&cmd, argv);

  cat_(cmd);

  return 0;
}

/* $begin initCmd */
void initCmd(struct cmd *cmd)
{
  cmd->len = 0;
  cmd->option = 0;
  cmd->fd = 1;
}
/* $end initCmd */

/* $begin parceCmd */
void parceCmd(struct cmd *cmd, char *argv[])
{
  getOption(cmd, argv);
  getPath(cmd, argv);
}
/* $end parceCmd */

/* $$begin getOption */
void getOption(struct cmd *cmd, char *argv[])
{
  for (int i = 0; argv[i] != NULL; i++)
  {
    if (argv[i][0] != '-')
      continue;
    int j = 1;
    while (argv[i][j] != '\0')
    {
      if (argv[i][j] == 'n')
        cmd->option |= (1 << _n_);
      j++;
    }
  }
}
/* $$end getOption */

/* $$begin getPath */
void getPath(struct cmd *cmd, char *argv[])
{
  for (int i = 1; argv[i] != NULL; i++)
  {
    if (argv[i][0] == '-')
      continue;
    strcpy(cmd->path[(cmd->len)++], argv[i]);
  }
}
/* $$end getPath */

/* $begin cat_ */
void cat_(struct cmd cmd)
{
  for (int i = 0; i < cmd.len; i++)
  {
    char *path = cmd.path[i];
    FILE *fp = fopen(path, "r");

    if (!fp)
      unix_error("cat error");

    int num = 1;
    char numStr[9];
    char line[10240];
    while (fgets(line, 10240, fp))
    {
      if (cmd.option & (1 << _n_))
      {
        sprintf(numStr, "%6d  ", num);
        write(cmd.fd, numStr, strlen(numStr));
        num++;
      }
      write(cmd.fd, line, strlen(line));
    }

    fclose(fp);
  }
}
/* $end cat_ */