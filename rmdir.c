/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * rmdir command
 * 
 * this source code is for 'rmdir' of linux shell.
 * but it has no 100% functions.
 * 
 * option:
 * -p: each directory argument is treated as a pathname 
 *    of which all compnents will be removed.
 * -v: display verbose information for every directory processd.
 */
#include "csapp.h"
#include <dirent.h>
#define MAXDEPTH 32      /* path depth */
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
/* remove ./a/b/c -> remove */
void removeDirs(char *x_path, int option);
int separatePaths(char *x_path, char x_paths[][MAXDIRNAME]);
void rmdir_p(int x_depth, int x_len, char x_paths[][MAXDIRNAME], int option);
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

  while (argv[i_path] != NULL)
    /* remove directory of path */
    removeDirs(argv[i_path++], option);

  return 0;
}
/* end main */

/* $begin parceInput */
/* get the option as a bits 
  and the index which indicates the beginning of path arguments */
int parceInput(int *x_option, char *x_argv[])
{
  *x_option = 0;
  if (x_argv[1][0] == '-')
  {
    for (int i = 1; x_argv[1][i] != '\0'; i++)
    {
      if (x_argv[1][i] == 'p')
        *x_option |= (1 << _p_);
      else if (x_argv[1][i] == 'm')
        *x_option |= (1 << _v_);
      else /* wrong option error */
      {
        printf("rmdir: wrong options, %c\n", x_argv[1][i]);
        exit(0);
      }
    }
    if (x_argv[2] == NULL) /* no path as an operand */
    {
      printf("rmdir: no path operand\n");
      exit(0);
    }
    return 2;
  }
  return 1;
}
/* $end parceInput */

/* $begin removeDirs */
void removeDirs(char *x_path, int option)
{
  /* rmdir -p ... */
  if (option & (1 << _p_))
  {
    /* separate each path */
    char paths[MAXDEPTH][MAXDIRNAME];
    int len = separatePaths(x_path, paths);
    rmdir_p(0, len, paths, option);
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
  }
}

/* $$begin separatePaths */
/* separate each path with '/' */
int separatePaths(char *x_path, char x_paths[][MAXDIRNAME])
{
  int len = 0;
  int j = 0;
  int i = 0;
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
void rmdir_p(int x_depth, int x_len, char x_paths[][MAXDIRNAME], int option)
{
  /* end condition of recusion furnction */
  if (x_depth == x_len)
    return;

  /* change directory */
  if (chdir(x_paths[x_depth]) < 0)
    unix_error("rmdir error");

  /* rcursive call */
  rmdir_p(x_depth + 1, x_len, x_paths, option);

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
    printf("rmdir: removing directory, ");
    for (int i = 0; i <= x_depth; i++)
      printf("%s", x_paths[i]);
    printf("\n");
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