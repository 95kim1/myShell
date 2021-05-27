/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * mkdir command
 * 
 * this source code is for 'mkdir' of linux shell.
 * but it has no 100% functions.
 * 
 * -m: Set the permission of directory which will be created.
 * -p: Each directory argument is treated as a pathname
 *    of which all components will be created.
 * -v: Display verbose information for every directory processd. 
 */
#include "csapp.h"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#define MAXPATHLEN 8192 // maximum path length
#define MAXDIRNAME 256  // directory's maximum length of name
#define MAXDIR 32       //the number of dirs as cmd arguments
#define MAXDEPTH 32     //path depth

/* options */
enum
{
  _m_,
  _p_,
  _v_,
};

/* create directory */
void makeDirectory(char *path, int option, int mode);

/* check path exist */
int existDir(char *path);
/* separate path with / */
int parcePaths(char *path, char parcedPath[][MAXDIRNAME]);

/* extract path from argv and return the number of paths */
int getPath(char path[][MAXPATHLEN], char *argv[], int pathIdx);
/* extract option from argv and return the begining path index of argv */
int getOption(int *option, int *mode, char *argv[]);
/* get mode as integer, ex) -m=755 -> return 0755 */
int getMode(char *options);
/* get option as bits */
int charOptToIntOpt(char opt);

int main(int argc, char *argv[])
{
  char path[MAXDIR][MAXPATHLEN];
  int option, mode = 0755;

  /* get option and starting index of directory paths */
  int pathIdx = getOption(&option, &mode, argv);

  /* get directory paths and the number of paths that will be created */
  int pathCnt = getPath(path, argv, pathIdx);

  for (int i = 0; i < pathCnt; i++)
  {
    /* make directory */
    makeDirectory(path[i], option, mode);
  }

  return 0;
}

/* $begin makeDirectory */
/* create directory with option */
void makeDirectory(char *path, int option, int mode)
{
  char parcedPath[MAXDEPTH][MAXDIRNAME];
  char temp_path[MAXPATHLEN];
  char cwd[MAXPATHLEN];
  for (int i = 0; i < MAXDEPTH; i++)
    parcedPath[i][0] = '\0';

  /* separate path with '/' */
  int cnt = parcePaths(path, parcedPath);
  int exist;

  /* get path without last directory */
  strcpy(temp_path, path);
  int len = strlen(path);
  for (int i = len - 2; i >= 0; i--)
  {
    if (temp_path[i] == '/')
    {
      temp_path[i] = '\0';
      break;
    }
  }

  /* check that path(without last directory) exists */
  exist = existDir(temp_path);

  // error except for "No such file or directory"
  if (exist < 0)
    unix_error("mkdir error");

  // path(without lastdirectory) doesn't exist with no p option
  if (exist == 0 && !(option & (1 << _p_)))
  {
    printf("mkdir: failed to create directory \'%s\'\n", path);
    exit(0);
  }

  /* make directories */
  char curr[MAXPATHLEN];
  curr[0] = '\0';
  for (int i = 0; i < cnt; i++)
  {
    //record current path
    strcat(curr, parcedPath[i]);

    //create
    mkdir(curr, mode);

    // with -v option
    if (option & (1 << _v_))
      printf("mkdir: creating directory, \'%s\'\n", curr);
  }
}
/* $end makeDirectory */

/* get existence of a directory */
/* $$begin existDir */
int existDir(char *path)
{
  DIR *dir = opendir(path);
  if (dir)
  {
    closedir(dir);
    return 1; //exists
  }
  else if (ENOENT == errno)
    return 0; //doesn't exist
  else
    return -1; //any other errors
}
/* $$end existDir */

/* $$begin parcePaths */
/* ./a/b/c/ -> ./, a/, b/, c/ */
int parcePaths(char *path, char parcedPath[][MAXDIRNAME])
{
  int cnt = 0, len = strlen(path);
  for (int j = 0, i = 0; i < len; i++)
  {
    parcedPath[cnt][j++] = path[i];
    if (path[i] == '/')
    {
      cnt++;
      j = 0;
    }
  }

  /* .../.../dir : cnt indicates dir, so cnt++ */
  if (parcedPath[cnt][0] != '\0')
    cnt++;
  return cnt;
}
/* $$end parcePaths */

/* $begin getPath */
/* extract path from argv and return the number of paths */
int getPath(char path[][MAXPATHLEN], char *argv[], int pathIdx)
{
  int cnt = 0;
  for (int i = pathIdx; argv[i] != NULL; i++)
  {
    strcpy(path[cnt++], argv[i]);
  }
  return cnt;
}
/* $end getPath */

/* $begin getOption */
/* extract option from argv and return the begining path index of argv */
/* no path -> error and return -1 */
int getOption(int *option, int *mode, char *argv[])
{
  /* no path */
  if (argv[1] == NULL)
  {
    printf("mkdir: missing operand\n");
    exit(0);
  }

  if (argv[1][0] == '-')
  {
    /* no path */
    if (argv[2] == NULL)
    {
      printf("mkdir: missing path operand\n");
      exit(0);
    }

    int len = strlen(argv[1]);
    for (int i = 1; i < len; i++)
    {
      int opt = charOptToIntOpt(argv[1][i]);

      /* invalid option */
      if (opt < 0)
      {
        printf("mkdir: invalid options\n");
        exit(0);
      }

      if (opt == _m_)
      {
        *mode = getMode(argv[1]);
        if (*mode < 0)
        {
          printf("mkdir: invalid mode form: valid ex)  -m=XXX, 0 <= X <= 7\n");
          exit(0);
        }

        *option += (1 << opt);
        return 2;
      }

      *option += (1 << opt);
    }

    return 2;
  }

  return 1;
}
/* $end getOption */

/* $$begin getMode */
/* get mode */
/* ex) -m=755 -> return 0755 */
int getMode(char *options)
{
  int i = 0;
  while (options[i] != '\0' && options[i] != '=')
  {
    if (options[i] == ' ')
      return -1;
    i++;
  }

  if (options[i] == '\0' || options[i] == ' ')
    return -1;

  int mode = 0;
  int b = 64;
  i++;
  while (b > 0 && options[i] != '\0')
  {
    if (!('0' <= options[i] && options[i] <= '7'))
      return -1;
    mode += b * (options[i] - '0');
    b /= 8;
    i++;
  }

  if (b > 0)
    return -1;

  return mode;
}
/* $$end getMode */

/* $$begin charOptToIntOpt */
/* get option as bits */
/* ex) -pv -> return  101_(2)*/
int charOptToIntOpt(char opt)
{
  switch (opt)
  {
  case 'p':
    return _p_;
  case 'm':
    return _m_;
  case 'v':
    return _v_;
  }
  return -1;
}
/* $$end charOptToIntOpt */
