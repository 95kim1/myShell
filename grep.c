/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * No.8 grep command
 * : catch strings (maches or no matches)
 * 
 * usage: grep [-v] [path,...]
 * 
 * this source code is for 'grep' of linux shell.
 * but it has no 100% functions.
 * 
 * option:
 * -v: grep no matchings
 */
#include "csapp.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_DEPTH 128 /* maximum path depth */
#define MAX_PATH 1024 /* maximum path length */
#define MAX_STR_LEN 1024
#define MAX_STR_CNT 2048

enum
{
  _v_,
};

int getOption(char *argv[]);
int getPath(char path[][MAX_PATH], char *pattern, char *argv[]);

void check(char strs[][MAX_STR_LEN], int cntStr, char *pattern, int comp);
int existPattern(char *str, char *pattern);
int Strcmp(char *str, int len, char *pattern);

int main(int argc, char *argv[])
{
  char path[MAX_DEPTH][MAX_PATH];
  char pattern[MAX_PATH];

  /* parce */
  int option = getOption(argv);
  int cntPath = getPath(path, pattern, argv);

  char strings[MAX_STR_CNT][MAX_STR_LEN];
  int cntStr = 0;

  //읽기
  if (cntPath > 0)
  {
    for (int i = 0; i < cntPath; i++)
    {
      FILE *fp = fopen(path[i], "r");
      while (fgets(strings[cntStr++], MAX_STR_LEN, fp))
        ;
      fclose(fp);
    }
  }
  else
  {
    while (fgets(strings[cntStr++], MAX_STR_LEN, stdin))
      ;
  }

  //옵션에 따라 선택
  if (option & (1 << _v_))
    check(strings, cntStr, pattern, 0);
  else
    check(strings, cntStr, pattern, 1);

  //쓰기
  for (int i = 0; i < cntStr; i++)
  {
    if (strings[i][0] != '\0')
      write(1, strings[i], strlen(strings[i]));
  }

  write(1, "\0", 1);
  return 0;
}

int getOption(char *argv[])
{
  int option = 0;
  for (int i = 1; argv[i] != NULL; i++)
  {
    if (argv[i][0] != '-')
      continue;
    int j = 1;
    int len = strlen(argv[i]);
    for (j = 1; j < len; j++)
    {
      char c = argv[i][j];
      if (c == 'v')
        option |= (1 << _v_);
    }
  }
  return option;
}

int getPath(char path[][MAX_PATH], char *pattern, char *argv[])
{
  int cnt = 0;
  int flag = 1;
  for (int i = 1; argv[i] != NULL; i++)
  {
    if (argv[i][0] == '-')
      continue;
    if (flag)
    {
      flag = 0;
      if (argv[i][0] == '\"')
        strcpy(pattern, argv[i] + 1);
      else
        strcpy(pattern, argv[i]);
      int len = strlen(pattern);
      if (pattern[len - 1] == '\"')
        pattern[len - 1] = '\0';
    }
    else
      strcpy(path[cnt++], argv[i]);
  }
  return cnt;
}

/* tern strings with pattern (or without pattern) into "\0" */
void check(char strs[][MAX_STR_LEN], int cntStr, char *pattern, int comp)
{
  for (int i = 0; i < cntStr; i++)
  {
    int cmp = existPattern(strs[i], pattern);
    if (cmp != comp)
    {
      strs[i][0] = '\0';
    }
  }
}

/* determine wether str has a pattern string */
/* return 1 if true, 0 if false */
int existPattern(char *str, char *pattern)
{
  int lenStr = strlen(str);
  int lenPattern = strlen(pattern);
  for (int i = 0; i < lenStr; i++)
  {
    if (lenStr - i < lenPattern)
      return 0;
    if (!Strcmp(str + i, lenPattern, pattern))
      return 1;
  }
  return 0;
}

/* comparison between  "str+0 ~ str+len-1" and pattern */
int Strcmp(char *str, int len, char *pattern)
{
  char copyStr[MAX_STR_LEN];
  strcpy(copyStr, str);
  copyStr[len] = '\0';
  return strcmp(copyStr, pattern);
}