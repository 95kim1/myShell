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
  _r_,
};

int getOption(char *argv[]);
int getPath(char path[][MAX_PATH], char *argv[]);

void swap(char *a, char *b);
int partition(int start, int end, char strs[][MAX_STR_LEN], int (*comp)(char *, char *));
void sort(int start, int end, char strs[][MAX_STR_LEN], int (*comp)(char *, char *));
int comp(char *a, char *b);
int comp_r(char *a, char *b);

int main(int argc, char *argv[])
{
  char path[MAX_DEPTH][MAX_PATH];

  /* parce */
  int option = getOption(argv);
  int cntPath = getPath(path, argv);

  char strings[MAX_STR_CNT][MAX_STR_LEN];
  int cntStr = 0;

  //읽기
  if (cntPath > 0)
  {
    for (int i = 0; i < cntPath; i++)
    {
      if (access(path[i], F_OK) < 0)
      {
        printf("sort: directory\n");
        exit(0);
      }

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

  //옵션에 따라 정렬
  if (option & (1 << _r_))
    sort(0, cntStr - 1, strings, comp_r);
  else
    sort(0, cntStr - 1, strings, comp);

  //쓰기
  for (int i = 0; i < cntStr; i++)
    write(1, strings[i], strlen(strings[i]));

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
      if (c == 'r')
        option |= (1 << _r_);
    }
  }
  return option;
}

int getPath(char path[][MAX_PATH], char *argv[])
{
  int cnt = 0;
  for (int i = 1; argv[i] != NULL; i++)
  {
    if (argv[i][0] == '-')
      continue;
    strcpy(path[cnt++], argv[i]);
  }
  return cnt;
}

void swap(char *a, char *b)
{
  char temp[MAX_STR_LEN];
  strcpy(temp, a);
  strcpy(a, b);
  strcpy(b, temp);
}

int partition(int start, int end, char strs[][MAX_STR_LEN], int (*comp)(char *, char *))
{
  int pivot = (start + end) / 2;

  char pivotElem[MAX_STR_LEN];
  strcpy(pivotElem, strs[pivot]);
  swap(strs[pivot], strs[end]);

  int storageIdx = start;
  int compIdx = start;

  while (compIdx < end)
  {
    if (comp(pivotElem, strs[compIdx]) > 0)
    {
      swap(strs[compIdx], strs[storageIdx]);
      storageIdx++;
    }

    compIdx++;
  }

  swap(strs[end], strs[storageIdx]);

  return storageIdx;
}

void sort(int start, int end, char strs[][MAX_STR_LEN], int (*comp)(char *, char *))
{
  if (start >= end)
    return;

  int pivot = partition(start, end, strs, comp);

  sort(start, pivot - 1, strs, comp);
  sort(pivot + 1, end, strs, comp);
}

int comp(char *a, char *b)
{
  int lenA = strlen(a);
  int lenB = strlen(b);
  int sA = 0, sB = 0;
  while (sA < lenA && sB < lenB)
  {
    if (a[sA] < b[sB])
      return -1;
    else if (a[sA] > b[sB])
      return 1;

    sA++;
    sB++;
  }
  if (sA == sB)
    return 0;
  if (sA < sB)
    return -1;
  if (sA > sB)
    return 1;
}

int comp_r(char *a, char *b)
{
  int lenA = strlen(a);
  int lenB = strlen(b);
  int sA = 0, sB = 0;
  while (sA < lenA && sB < lenB)
  {
    if (a[sA] < b[sB])
      return 1;
    else if (a[sA] > b[sB])
      return -1;

    sA++;
    sB++;
  }
  if (sA == sB)
    return 0;
  if (sA < sB)
    return 1;
  if (sA > sB)
    return -1;
}