/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * No.6 echo command
 * : print input message (stdout)
 * 
 * usage: echo msg
 * 
 * this source code is for 'echo' of linux shell.
 * but it has no 100% functions.
 * 
 */

#include "csapp.h"
#include <string.h>

void echo(char *argv[], int fd);

int main(int argc, char *argv[])
{
  int fd = 1;
  echo(argv, fd);
  return 0;
}

void echo(char *argv[], int fd)
{
  for (int i = 1; argv[i] != NULL; i++)
  {
    write(fd, argv[i], strlen(argv[i]));
    if (argv[i + 1] != NULL)
      write(fd, " ", 1);
  }
  write(fd, "\n", 1);
}