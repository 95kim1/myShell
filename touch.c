/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * No.4 touch command
 * : creating files or update file state
 * 
 * this source code is for 'touch' of linux shell.
 * but it has no 100% functions.
 * 
 * usage: touch [-options] path [path,...]
 * 
 * option:
 * -a: to change file access and modification time.
 * -m: it is used only modify time of a file.
 * -t: to create a file by specifying the time.
 * -c: it doesn't create n empty file.
 */
#include "csapp.h"
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#define MAX_DEPTH 128 /* maximum path depth */
#define MAX_PATH 1024 /* maximum path length */

enum
{
  _a_, // to change file access and modification time.
  _m_, // it is used only modify time of a file.
  _t_, // to create a file by specifying the time.
  _c_, // it doesn't create n empty file.
};

struct cmd
{
  char paths[MAX_DEPTH][MAX_PATH]; /* storage for paths */
  int len;                         /* the number of paths */
  int option;                      /* options */
  char t[20];                      /* -t option's value */
};

/* separate paths and options from argv */
struct cmd parceArgv(char *x_argv[]);

/* parce argv */
void getOption(struct cmd *x_cmd, char *x_argv[]);
void extractOption(struct cmd *x_cmd, char *x_argv[], int *idx);

int parceTOption(struct cmd *x_cmd, char *x_argv[], int *x_idx, int *x_i);

int isValidTimeFormat(char *xarr_time);
int getYear(char *xarr_time, int x_year, int cnt);
int isValidMMDDhhmm(char *xarr_time, int x_year, int x_i);
int isValidMonth(char *xarr_time, int x_i, int *x_month);
int isValidDay(char *xarr_time, int x_i, int x_year, int x_month);
int isValidHour(char *xarr_time, int x_i);
int isValidMinute(char *xarr_time, int x_i);
int isValidSec(char *xarr_time, int x_i);

void getPaths(struct cmd *x_cmd, char *x_argv[]);

void createFile(const char *x_path);
int existFile(const char *x_path);
void extractDirPath(char *xarr_dirPath, const char *x_path);
int existDir(const char *path);

void operateCmd(const struct cmd *x_cmd);

void myTouch(const char *x_path, const struct cmd *x_cmd);
time_t getTime(const char *xarr_t);
void getTimeFromStr(const char *xarr_t, int *y, int *M, int *d, int *h, int *m, int *s);
int stoi(const char *x_str, int x_i, int x_cnt);

int main(int argc, char *argv[])
{
  /* no operands */
  if (argv[1] == NULL)
  {
    printf("touch: no operands\n");
    exit(0);
  }

  /* parcing operands */
  struct cmd cmd = parceArgv(argv);

  /* operate options and touch cmd */
  operateCmd(&cmd);

  return 0;
}

/* $begin operateCmd */
void operateCmd(const struct cmd *x_cmd)
{
  for (int i = 0; i < x_cmd->len; i++)
    myTouch(x_cmd->paths[i], x_cmd);
}
/* $end operateCmd */

/* $$begin myTouch */
void myTouch(const char *x_path, const struct cmd *x_cmd)
{
  int option = x_cmd->option;
  char arr_t[MAX_PATH];
  strcpy(arr_t, x_cmd->t);

  /* 파일 경로가 맞는지 체크 */
  if (x_path[strlen(x_path) - 1] == '/')
  {
    printf("touch: cannot create directory\n");
    exit(0);
  }

  /* 디렉토리 추출 */
  char arr_dirPath[MAX_PATH];
  extractDirPath(arr_dirPath, x_path);

  /* 디렉토리 존재 확인 */
  if (!existDir(arr_dirPath))
  {
    printf("touch: No such directory, %s\n", arr_dirPath);
    exit(0);
  }

  /* no c option */
  if (!(option & (1 << _c_)))
    /* create file */
    createFile(x_path);

  if (!existFile(x_path))
    return;

  /* file info */
  struct utimbuf timebuf;
  struct stat stbuf;

  if (stat(x_path, &stbuf) < 0)
    unix_error("touch error");

  timebuf.actime = stbuf.st_atime;
  timebuf.modtime = stbuf.st_mtime;

  if (option & (1 << _a_))
  {
    timebuf.actime = time(NULL);
    if (utime(x_path, &timebuf) < 0)
      unix_error("touch error");
  }
  if (option & (1 << _m_))
  {
    timebuf.modtime = time(NULL);
    if (utime(x_path, &timebuf) < 0)
      unix_error("touch error");
  }
  if (option & (1 << _t_))
  {
    timebuf.actime = timebuf.modtime = getTime(arr_t);
    if (utime(x_path, &timebuf) < 0)
      unix_error("touch error");
  }
}
/* $$end myTouch */

/* $$$begin getTime */
/* from stirng "[[YY]YY]MMDDhhmm[.xx]" to time_t */
time_t getTime(const char *xarr_t)
{
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  tm->tm_year += 1900;
  int year = tm->tm_year;
  tm->tm_mon += 1;

  getTimeFromStr(xarr_t, &tm->tm_year, &tm->tm_mon, &tm->tm_mday, &tm->tm_hour, &tm->tm_min, &tm->tm_sec);

  if (tm->tm_year < 100)
    tm->tm_year += (year / 100) * 100;

  tm->tm_year -= 1900;
  tm->tm_mon -= 1;

  return mktime(tm);
}
/* $$$end getTime */

/* $$$$begin getTimeFromStr */
void getTimeFromStr(const char *xarr_t, int *y, int *M, int *d, int *h, int *m, int *s)
{
  int len = strlen(xarr_t);
  switch (len)
  {
  case 11: //MMDDhhmm.ss
    *s = stoi(xarr_t, 9, 2);
  case 8: //MMDDhhmm
    *M = stoi(xarr_t, 0, 2);
    *d = stoi(xarr_t, 2, 2);
    *h = stoi(xarr_t, 4, 2);
    *m = stoi(xarr_t, 6, 2);
    return;
  case 13: //YYMMDDhhmm.ss
    *s = stoi(xarr_t, 11, 2);
  case 10: //YYMMDDhhmm
    *y = stoi(xarr_t, 0, 2);
    *M = stoi(xarr_t, 2, 2);
    *d = stoi(xarr_t, 4, 2);
    *h = stoi(xarr_t, 6, 2);
    *m = stoi(xarr_t, 8, 2);
    return;
  case 15: //YYYYMMDDhhmm.ss
    *s = stoi(xarr_t, 13, 2);
  case 12: //YYYYMMDDhhmm
    *y = stoi(xarr_t, 0, 4);
    *M = stoi(xarr_t, 4, 2);
    *d = stoi(xarr_t, 6, 2);
    *h = stoi(xarr_t, 8, 2);
    *m = stoi(xarr_t, 10, 2);
    return;
  }
}
/* $$$$end getTimeFromStr */

int stoi(const char *x_str, int x_i, int x_cnt)
{
  int ans = 0;
  int b = 1;
  for (int i = x_cnt - 1; i >= 0; i--)
  {
    ans += (x_str[x_i + i] - '0') * b;
    b *= 10;
  }
  return ans;
}

/* $$$begin createFile */
void createFile(const char *x_path)
{
  FILE *fp = fopen(x_path, "w");
  fclose(fp);
}
/* $$$end createFile */

/* $$$begin existFile */
int existFile(const char *x_path)
{
  if (access(x_path, F_OK) != -1)
    return 1;
  return 0;
}
/* $$$end existFile */

/* $$$begin extractDirPath */
void extractDirPath(char *xarr_dirPath, const char *x_path)
{
  strcpy(xarr_dirPath, x_path);
  for (int i = strlen(x_path) - 1; i >= 0; i--)
  {
    if (xarr_dirPath[i] == '/')
    {
      xarr_dirPath[i] = '\0';
      break;
    }
    xarr_dirPath[i] = '\0';
  }
  if (xarr_dirPath[0] == '\0')
    strcpy(xarr_dirPath, "./");
}
/* $$$end extractDirPath */

/* $$$begin existDir */
/* get existence of a directory */
int existDir(const char *path)
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
/* $$$end existDir */

/* $begin parceArgv */
/* separate paths and options from argv */
struct cmd parceArgv(char *x_argv[])
{
  struct cmd cmd;

  /* init */
  cmd.option = 0;
  cmd.len = 0;

  /* option */
  getOption(&cmd, x_argv);

  /* path */
  getPaths(&cmd, x_argv);

  if (cmd.len == 0)
  {
    printf("touch: No path operand\n");
    exit(0);
  }

  return cmd;
}
/* $end parceArgv */

/* $$begin getPaths */
void getPaths(struct cmd *x_cmd, char *x_argv[])
{
  x_cmd->len = 0;
  for (int i = 1; x_argv[i] != NULL; i++)
  {
    /* not path */
    if (x_argv[i][0] == '-')
    {
      for (int j = strlen(x_argv[i]) - 1; j > 0; j--)
        if (x_argv[i][j] == 't')
          i++;
      continue;
    }
    /* path */
    strcpy(x_cmd->paths[(x_cmd->len)++], x_argv[i]);
  }
}
/* $$end getPaths */

/* $$begin getOption */
/* It is used at parceArgv() */
void getOption(struct cmd *x_cmd, char *x_argv[])
{
  for (int i = 1; x_argv[i] != NULL; i++)
  {
    /* not option */
    if (x_argv[i][0] != '-')
      continue;
    /* option */
    extractOption(x_cmd, x_argv, &i);
  }
}
/* $$end getOption */

/* $$$begin extractOption */
void extractOption(struct cmd *x_cmd, char *x_argv[], int *idx)
{
  char *x_str = x_argv[*idx];
  int option = 0;
  int i = 1;
  while (x_str[i] != '\0')
  {
    if (x_str[i] == 'c')
      option |= (1 << _c_);
    else if (x_str[i] == 'a')
      option |= (1 << _a_);
    else if (x_str[i] == 'm')
      option |= (1 << _m_);
    else if (x_str[i] == 't')
    {
      x_cmd->option |= (1 << parceTOption(x_cmd, x_argv, idx, &i));
      return;
    }
    i++;
  }
  x_cmd->option |= option;
}
/* $$$end extractOption */

/* $$$$begin parceTOption */
int parceTOption(struct cmd *x_cmd, char *x_argv[], int *x_idx, int *x_i)
{
  char arr_time[1024];
  int i = 0;
  int idx = 0;

  /* 1. -t[timestamp] */
  if (x_argv[*x_idx][*x_i + 1] != '\0')
  {
    strcpy(arr_time, x_argv[*x_idx] + *x_i + 1);
    i = strlen(arr_time);
  }
  /* 2. -t [timestamp] */
  else
  {
    idx++;
    strcpy(arr_time, x_argv[*x_idx + 1]);
  }

  if (!isValidTimeFormat(arr_time))
  {
    printf("touch: invalid time format %s\n", arr_time);
    exit(0);
  }

  x_i += i;
  x_idx += idx;

  strcpy(x_cmd->t, arr_time);

  return _t_;
}
/* $$$$end parceTOption */

/* $$$$$begin isValidTimeFormat */
int isValidTimeFormat(char *xarr_time)
{
  int len = strlen(xarr_time);

  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  int year = t->tm_year + 1900;

  switch (len)
  {
  case 8: //MMDDhhmm
    if (isValidMMDDhhmm(xarr_time, year, 0))
      return 1;
    return 0;
  case 10: //YYMMDDhhmm
    if (!(year = getYear(xarr_time, year, 2)))
      return 0;
    if (isValidMMDDhhmm(xarr_time, year, 2))
      return 1;
    return 0;
  case 11: //MMDDhhmm.ss
    if (!isValidMMDDhhmm(xarr_time, year, 0))
      return 0;
    if (isValidMonth(xarr_time, strlen(xarr_time) - 2, NULL))
      return 1;
    return 0;
  case 12: //YYYYMMDDhhmm
    if (!(year = getYear(xarr_time, year, 4)))
      return 0;
    if (isValidMMDDhhmm(xarr_time, year, 4))
      return 1;
    return 0;
  case 13: //YYMMDDhhmm.ss
    if (!(year = getYear(xarr_time, year, 2)))
      return 0;
    if (!isValidMMDDhhmm(xarr_time, year, 2))
      return 0;
    if (isValidSec(xarr_time, len - 2))
      return 1;
    return 0;
  case 15: //YYYYMMDDhhmm.ss
    if (!(year = getYear(xarr_time, year, 4)))
      return 0;
    if (!isValidMMDDhhmm(xarr_time, year, 4))
      return 0;
    if (isValidSec(xarr_time, len - 2)) //is the second valid?
      return 1;
    return 0;
  }
  return 0;
}

int getYear(char *xarr_time, int x_year, int x_cnt)
{
  int year = 0;
  int temp;
  int b = 1;
  for (int i = x_cnt - 1; i >= 0; i--)
  {
    temp = (xarr_time[i] - '0');
    if (0 > temp || temp > 9)
      return 0;
    year += b * temp;
    b *= 10;
  }

  if (x_cnt == 2)
    year += (x_year / 100) * 100;

  return year;
}

int isValidMMDDhhmm(char *xarr_time, int x_year, int x_i)
{
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (x_year < 100) /* YYMMDDhhmm -> year < 100 */
    x_year += (tm->tm_year + 1900) / 100 * 100;

  int month;
  if (!isValidMonth(xarr_time, x_i, &month))
    return 0;
  if (!isValidDay(xarr_time, x_i + 2, x_year, month))
    return 0;
  if (!isValidHour(xarr_time, x_i + 4))
    return 0;
  if (!isValidMinute(xarr_time, x_i + 6))
    return 0;
  return 1;
}

int isValidMonth(char *xarr_time, int x_i, int *x_month)
{
  int month = 0;
  int temp = 0;

  temp = xarr_time[x_i] - '0';
  if (0 > temp || temp > 1)
    return 0;
  month += temp * 10;

  temp = xarr_time[x_i + 1] - '0';
  if (0 > temp || temp > 9)
    return 0;
  month += temp;

  if (month < 0 || month > 12)
    return 0;

  if (x_month)
    *x_month = month;

  return 1;
}

int isValidDay(char *xarr_time, int x_i, int x_year, int x_month)
{
  int day = 0;
  int temp = 0;

  temp = xarr_time[x_i] - '0';
  if (0 > temp || temp > 3)
    return 0;
  day += temp * 10;

  temp = xarr_time[x_i + 1] - '0';
  if (0 > temp || temp > 10)
    return 0;
  day += temp;

  int end_day = 0;

  if (x_month < 8)
  {
    if (x_month % 2) // 1 3 5 7
      end_day = 31;
    else if (x_month != 2) // 4 6
      end_day = 30;
    else if (x_year % 4 && !(x_year % 100) || (x_year % 400)) //윤년
      end_day = 29;
    else
      end_day = 28;
  }
  else
  {
    if (x_month % 2) // 9 11
      end_day = 30;
    else // 8 10 12
      end_day = 31;
  }

  if (1 <= day && day <= end_day)
    return 1;
  return 0;
}

int isValidHour(char *xarr_time, int x_i)
{
  int hour = 0;
  int temp = 0;

  temp = xarr_time[x_i] - '0';
  if (0 > temp || temp > 2)
    return 0;
  hour += temp * 10;

  temp = xarr_time[x_i + 1] - '0';
  if (0 > temp || temp > 10)
    return 0;
  hour += temp;

  if (0 <= hour && hour <= 23)
    return 1;
  return 0;
}

int isValidMinute(char *xarr_time, int x_i)
{
  int m = 0;
  int temp = 0;

  temp = xarr_time[x_i] - '0';
  if (0 > temp || temp > 5)
    return 0;
  m += temp * 10;

  temp = xarr_time[x_i + 1] - '0';
  if (0 > temp || temp > 9)
    return 0;
  m += temp;

  if (0 <= m && m <= 59)
    return 1;
  return 0;
}

int isValidSec(char *xarr_time, int x_i)
{
  return isValidMinute(xarr_time, x_i);
}

/* $$$$$end isValidTimeFormat */