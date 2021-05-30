/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * No.3 rmdir command
 * : delete empty directories
 * 
 * this source code is for 'rmdir' of linux shell.
 * but it has no 100% functions.
 * 
 * usage: rmdir [-options] path [path,...]
 * 
 * option:
 * -p: each directory argument is treated as a pathname 
 *    of which all compnents will be removed.
 * -v: display verbose information for every directory processd.
 */
#include "csapp.h"
#include <dirent.h>
#define MAXDEPTH 128     /* path depth */
#define MAXDIRNAME 256   /* directory name length */
#define MAXPATHLEN 8192  /* maximum path length */
#define PARENT_DIR "../" /* parent directory */
#define CURRENT_DIR "./" /* current directory */
/* options */
enum
{
  _p_,
  _v_,
};

/* get the option as a bits 
  and the index which indicates the beginning of path arguments */
int parceInput(int *x_option, char *x_argv[]);
int getIntOpt(char c);
/* remove ./a/b/c -> remove */
void removeDirs(char *x_path, int option, int fd);
int separatePaths(char *x_path, char x_paths[][MAXDIRNAME]);
void rmdir_p(int x_depth, int x_len, char x_paths[][MAXDIRNAME], int option, int fd);
int isEmptyDir(const char *x_path);

/* begin main */
int main(int argc, char *argv[])
{
  int option = 0;

  /* error: no operands */
  if (argv[1] == NULL)
  {
    printf("rmdir: no operands\n");
    exit(0);
  }

  /* get the option and index of path operand */
  int i_path = parceInput(&option, argv);

  if (argv[i_path] == NULL)
  {
    printf("rmdir: no target\n");
    exit(0);
  }

  while (argv[i_path] != NULL)
    /* remove directory of path */
    removeDirs(argv[i_path++], option, 1);

  return 0;
}
/* end main */

/* $begin parceInput */
/* get the option as a bits 
  and the index which indicates the beginning of path arguments */
int parceInput(int *x_option, char *x_argv[])
{
  if (x_argv[1] == NULL)
    return 1;
  /* extract options */
  int i = 1;
  for (i = 1; x_argv[i] && x_argv[i][0] == '-'; i++)
  {
    int j = 1;
    int len = strlen(x_argv[i]);
    while (j < len)
    {
      /* extract an option */
      int opt = getIntOpt(x_argv[i][j++]);

      /* wrong option */
      if (opt < 0)
      {
        unix_error("rmdir error");
        exit(0);
      }

      /* recording the option */
      *x_option |= (1 << opt);
    }
  }

  return i;
}
/* $end parceInput */

/* $begin getIntOpt */
int getIntOpt(char c)
{
  switch (c)
  {
  case 'p':
    return _p_;
  case 'v':
    return _v_;
  }
  return -1;
}
/* $end getIntOpt */

/* $begin removeDirs */
void removeDirs(char *x_path, int option, int fd)
{
  /* rmdir -p ... */
  if (option & (1 << _p_))
  {
    /* separate each path */
    char paths[MAXDEPTH][MAXDIRNAME];
    int len = separatePaths(x_path, paths);
    rmdir_p(0, len, paths, option, fd);
  }
  else /* rmdir [no -p] ... */
  {
    if (!isEmptyDir(x_path))
    {
      printf("rmdir: failed to remove directory %s not empty\n", x_path);
      exit(0);
    }

    if (rmdir(x_path) < 0)
      unix_error("rmdir error");

    if (option & (1 << _v_))
    {
      write(fd, "rmdir: removing directory, \'", 28);
      write(fd, x_path, strlen(x_path));
      write(fd, "\'\n", 2);
    }
  }
}

/* $$begin separatePaths */
/* separate each path with '/' */
int separatePaths(char *x_path, char x_paths[][MAXDIRNAME])
{
  int len = 0;
  int j = 0;
  int i = 0;

  len = strlen(x_path);
  if (x_path[len - 1] == '/')
    x_path[len - 1] = '\0';
  len = 0;

  for (j = 0, i = 0; x_path[i] != '\0'; i++)
  {
    x_paths[len][j++] = x_path[i];
    if (x_path[i] == '/')
    {
      x_paths[len][j] = '\0';
      len++;
      j = 0;
    }
  }

  // a/b/c -> {a, /, \0}, {b, /, \0}, {c, /}
  // so add '\0' to last string(character array)
  if (j != 0)
    x_paths[len][j] = '\0';

  return len + 1;
}
/* $$end separatePaths */

/* $$begin rmdirs */
/* rmdir with p option */
void rmdir_p(int x_depth, int x_len, char x_paths[][MAXDIRNAME], int option, int fd)
{
  /* end condition of recusion furnction */
  if (x_depth == x_len)
    return;

  /* change directory */
  if (chdir(x_paths[x_depth]) < 0)
    unix_error("rmdir error");

  /* rcursive call */
  rmdir_p(x_depth + 1, x_len, x_paths, option, fd);

  /* change directory to parent directory */
  if (chdir(PARENT_DIR) < 0)
    unix_error("rmdir error");

  /* check directory is empty */
  if (!isEmptyDir(x_paths[x_depth]))
  {
    printf("rmdir: failed to remove directory ");
    for (int j = 0; j <= x_depth; j++)
      printf("%s", x_paths[j]);
    printf(" not empty\n");
    exit(0);
  }

  /* remove directory */
  if (rmdir(x_paths[x_depth]) < 0)
    unix_error("rmdir error");

  /* print results with -v optoin true */
  if (option & (1 << _v_))
  {
    write(fd, "rmdir: removing directory, \'", 28);

    for (int i = 0; i < x_depth; i++)
      write(fd, x_paths[i], strlen(x_paths[i]));

    int len = strlen(x_paths[x_depth]);
    if (x_paths[x_depth][len - 1] == '/')
      len--;

    write(fd, x_paths[x_depth], len);
    write(fd, "\'\n", 2);
  }
}
/* $$end rmdirs */
/* $end removeDirs */

/* $begin isEmptyDir */
int isEmptyDir(const char *x_path)
{
  struct dirent *dirent;
  DIR *dh = opendir(x_path);

  if (!dh)
    unix_error("rmdir error: Directory error");

  for (int i = 0; i <= 2; i++)
  {
    dirent = readdir(dh);
    if (i == 2 && !dirent)
      return 1;
  }
  return 0;
}
/* $end isEmptyDir */