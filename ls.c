/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * ls command
 * 
 * this source code is for 'ls' of linux shell.
 * but it has no 100% functions.
 * 
 * -a: display every directory and file
 * -l: display as lists
 * -F: add / to directory
 * -ii: display inode
 * -s: display size
 * -h: display size as unit K  (ex 4096 -> 4K)
 * -S: display elements sorted by size
 * -t: display elements sorted by time
 */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "csapp.h"

#define MAXPATH 8192    // maximum length of path operands
#define MAXFILENAME 256 // maximum length of filename
#define MAXLIST 1024    //maximum number of entries which will be displayed
#define MAXUSERNAME 128 // maximum length of user name

/* options */
enum
{
  a,
  l,
  F,
  ii,
  s,
  h,
  S,
  t,
};

enum
{
  directory,
  file,
};

/* type of parced command storage */
struct cmd
{
  char path[MAXPATH];
  int option; //ex) ls -ali => option=11 (==1011_(2))
};

/* type of entry of list of directories and files */
struct list_entry
{
  char filename[MAXFILENAME]; //filename
  int type;                   //file or directory
  int inode;                  //inode
  int permission;             //permission
  int linkCnt;                //연결된 파일 개수
  int uid;                    //소유계정
  int gid;                    //소유그룹
  int size;                   //크기 (byte)
  time_t time;                //변경일자 (month date time)
  int year;                   //변경일자 (year);
};

/* max length of entry of list_entry */
struct max_length
{
  int inode;
  int linkCnt;
  int user;
  int group;
  int size;
};

int total;

/* prototypes of functions for ls */
int parceCmdOption(struct cmd *cmd, char *argv[]);
void initCmd(struct cmd *cmd);
void initMl(struct max_length *ml);
struct list_entry getEntryInfo(struct dirent *dirent, struct max_length *ml);
void printEntry(int fd, struct list_entry *entry, const int option, const struct max_length ml);
void printList(int fd, struct list_entry list[], const int size, const int option, const struct max_length ml);

void getPermissionStr(char *temp, int i, int permission);
void getBlank(char temp[], int len);

struct list_entry copy(struct list_entry *entry);
void swap(struct list_entry *a, struct list_entry *b);
int partition(int start, int end, struct list_entry list[], int (*comp)(struct list_entry *, struct list_entry *));
void sortList(int start, int end, struct list_entry list[], int (*comp)(struct list_entry *, struct list_entry *));
/* sorting by file size */
int compS(struct list_entry *a, struct list_entry *b);
/* sorting by time */
int compT(struct list_entry *a, struct list_entry *b);

/* $begin main */
/* main function: ls */
int main(int argc, char *argv[])
{
  struct cmd cmd;
  struct max_length ml;

  initCmd(&cmd);

  /* get parced command: options */
  int i = parceCmdOption(&cmd, argv);

  int fd = 1;
  /* pipe */
  //fd = ...

  /* first argument(path) */
  if (argv[i] != NULL)
    strcpy(cmd.path, argv[i]);

  /* infoFlag: 
   * 0 means one directory
   * 1 means equal to or more than two directories */
  int infoFlag = 0;
  if (argv[i + 1] != NULL)
    infoFlag++;

  while (cmd.path[0] != '\0')
  {
    initMl(&ml);

    struct dirent *dirent;
    DIR *dh = opendir(cmd.path);

    if (!dh)
      unix_error("Directory error");

    /* storage of info of current directory's all files and directories */
    struct list_entry list[MAXLIST]; // list of informations of files or directories
    int idx = 0;                     // index of entry of list

    /* get info of current directory's all files and directories */
    total = 0;
    while ((dirent = readdir(dh)) != NULL)
      list[idx++] = getEntryInfo(dirent, &ml);

    /* write info of list depending on the struct cmd */
    if (argv[i] && infoFlag)
    {
      write(fd, argv[i], strlen(argv[i]));
      write(fd, ":\n", 2);
    }
    printList(fd, list, idx, cmd.option, ml);

    /* no path arguments in user command */
    if (argv[i] == NULL)
      break;

    /* i: index of array indicating path */
    i++;
    if (argv[i])
    {
      strcpy(cmd.path, argv[i]);
      write(fd, "\n", 1);
    }
    else
      strcpy(cmd.path, "\0");
  }
  return 0;
}
/* $end main */

/* print list of current working directory's info of files and directories (depending on cmd option) (to fd) */
/* $begin printList */
void printList(int fd, struct list_entry list[], const int size, const int option, const struct max_length ml)
{
  printf("===\n");
  /* sort */
  if (option & (1 << S))
    sortList(0, size - 1, list, compS);
  else if (option & (1 << t))
    sortList(0, size - 1, list, compT);

  printf("-----\n");

  /* print informations as a list */
  if (option & (1 << l))
  {
    char temp[40];
    write(fd, "total: ", 7);
    sprintf(temp, "%d\n", total);
    write(fd, temp, strlen(temp));
  }

  /* write(print) */
  for (int i = 0; i < size; i++)
  {
    if (!(option & (1 << a)) && list[i].filename[0] == '.')
      continue;
    printEntry(fd, list + i, option, ml);
  }

  /* no l option, then write(print) LF */
  if (!(option & (1 << l)))
    write(fd, "\n", 1);
}
/* $end printList */

/* print a list entry */
/* $begin printEntry */
void printEntry(int fd, struct list_entry *entry, const int option, const struct max_length ml)
{
  char temp[33];
  char temp2[33];

  /* F option */
  if (option & (1 << F) && entry->type == directory)
    strcat(entry->filename, "/");

  /* print inode */
  if (option & (1 << ii))
  {
    sprintf(temp, "%d ", entry->inode);

    if (option & (1 << l))
    {
      getBlank(temp2, ml.inode - strlen(temp) + 1);
      write(fd, temp2, strlen(temp2)); //blanks
    }

    write(fd, temp, strlen(temp));
  }

  /* print file size */
  if (option & (1 << s))
  {
    sprintf(temp, "%d", entry->size);

    if (option & (1 << l))
      getBlank(temp2, ml.size - strlen(temp));

    if (option & (1 << h))
      strcat(temp, "K");
    strcat(temp, " ");

    if (option & (1 << l))
      write(fd, temp2, strlen(temp2)); //blanks
    write(fd, temp, strlen(temp));
  }

  /* l option */
  if (option & (1 << l))
  {
    /* permission */
    if (entry->type == directory)
      temp[0] = 'd';
    else
      temp[0] = '-';

    getPermissionStr(temp, 1, (entry->permission) >> 6);
    getPermissionStr(temp, 4, (entry->permission & 0x038) >> 3);
    getPermissionStr(temp, 7, (entry->permission & 0x007));

    strcat(temp, " ");
    write(fd, temp, strlen(temp));

    /* link count */
    sprintf(temp, "%d ", entry->linkCnt);

    getBlank(temp2, ml.linkCnt - strlen(temp) + 1);

    write(fd, temp2, strlen(temp2)); //blanks
    write(fd, temp, strlen(temp));

    /* user */
    strcpy(temp, getpwuid(entry->uid)->pw_name);
    strcat(temp, " ");

    getBlank(temp2, ml.user - strlen(temp) + 1);

    write(fd, temp2, strlen(temp2)); //blanks
    write(fd, temp, strlen(temp));

    /* group */
    strcpy(temp, getgrgid(entry->gid)->gr_name);
    strcat(temp, "  ");

    getBlank(temp2, ml.group - strlen(temp) + 1);

    write(fd, temp2, strlen(temp2)); //blanks
    write(fd, temp, strlen(temp));

    /* file size */
    sprintf(temp, "%d", entry->size);

    getBlank(temp2, ml.size - strlen(temp));

    if (option & (1 << h))
      strcat(temp, "K");
    strcat(temp, " ");

    write(fd, temp2, strlen(temp2)); //blanks
    write(fd, temp, strlen(temp));

    /* current time */
    time_t curTime = time(NULL);
    struct tm *pLocal = localtime(&curTime);

    /* time */
    time_t tempTime = entry->time;
    if (entry->year == pLocal->tm_year) /* month day hour:minute */
      sprintf(temp, "%2ld %2ld %02ld:%02ld ", tempTime / 1000000, (tempTime / 10000) % 100, (tempTime / 100) % 100, tempTime % 100);
    else /* month day year */
      sprintf(temp, "%2ld %2ld %5d ", tempTime / 1000000, (tempTime / 10000) % 100, entry->year);
    write(fd, temp, strlen(temp));

    strcpy(temp2, "\n");
  }
  else
    strcpy(temp2, "    ");
  /* filename */
  write(fd, entry->filename, strlen(entry->filename));
  write(fd, temp2, strlen(temp2)); //-l: "\n", no option: "  "
}
/* $end printEntry */

/* $$begin getBlank */
void getBlank(char temp[], int len)
{
  for (int i = 0; i < len; i++)
    temp[i] = ' ';
  temp[len] = '\0';
}
/* $$end getBlank */

/* $$begin getPermissionStr */
void getPermissionStr(char *temp, int i, int permission)
{
  if (permission & 4)
    temp[i] = 'r';
  else
    temp[i] = '-';

  if (permission & 2)
    temp[i + 1] = 'w';
  else
    temp[i + 1] = '-';

  if (permission & 1)
    temp[i + 2] = 'x';
  else
    temp[i + 2] = '-';

  temp[i + 3] = '\0';
}
/* $$end getPermissionStr */

/* get informations of current directory's all files and directories */
/* $begin getEntryInfo */
struct list_entry getEntryInfo(struct dirent *dirent, struct max_length *ml)
{
  struct list_entry entry;
  char temp[256];

  entry.inode = dirent->d_ino; //inode

  /* renew max length of inode */
  sprintf(temp, "%d", entry.inode);
  int len = strlen(temp);
  if (ml->inode < len)
    ml->inode = len;

  strcpy(entry.filename, dirent->d_name); //filename

  struct stat fs;
  int r = stat(entry.filename, &fs);

  total += fs.st_blocks;
  //printf("%s,%d-", entry.filename, fs.st_blocks);

  /* file or directory */
  if (S_ISDIR(fs.st_mode))
    entry.type = directory;
  else
    entry.type = file;

  /* permission */
  entry.permission = fs.st_mode & 0x1FF;

  /* link count of subdirectory */
  entry.linkCnt = fs.st_nlink;

  sprintf(temp, "%d", entry.linkCnt);
  len = strlen(temp);
  if (ml->linkCnt < len)
    ml->linkCnt = len;

  /* get user_id and group_id */
  entry.uid = fs.st_uid;
  entry.gid = fs.st_gid;
  //user
  len = strlen(getpwuid(entry.uid)->pw_name);
  if (ml->user < len)
    ml->user = len;
  //group
  len = strlen(getgrgid(entry.gid)->gr_name);
  if (ml->group < len)
    ml->group = len;

  /* get file size */
  entry.size = fs.st_size;

  sprintf(temp, "%d", entry.size);
  len = strlen(temp);
  if (ml->size < len)
    ml->size = len;

  /* get update modified time */
  struct tm *t = localtime(&fs.st_mtime);
  // MMDDHHmm
  entry.time = (t->tm_mon + 1) * 1000000 + t->tm_mday * 10000 + t->tm_hour * 100 + t->tm_min;
  // YYYY
  entry.year = t->tm_year;

  return entry;
}
/* $end getEntryInfo */

/* $begin parceCmdOption */
// return index of first path argument
int parceCmdOption(struct cmd *cmd, char *argv[])
{
  if (argv[1] == NULL)
    return 1;
  /* extract options */
  int i = 1;
  for (i = 1; argv[i] && argv[i][0] == '-'; i++)
  {
    int j = 1;
    int len = strlen(argv[i]);
    while (j < len)
    {
      int opt;
      switch (argv[i][j++])
      {
      case 'a':
        opt = (1 << a);
        break;
      case 'l':
        opt = (1 << l);
        break;
      case 'F':
        opt = (1 << F);
        break;
      case 'i':
        opt = (1 << ii);
        break;
      case 's':
        opt = (1 << s);
        break;
      case 'h':
        opt = (1 << h);
        break;
      case 'S':
      {
        cmd->option &= ~((1 << t));
        opt = (1 << S);
        break;
      }
      case 't':
      {
        cmd->option &= ~((1 << S));
        opt = (1 << t);
        break;
      }
      } //switch end
      /* recording the option */
      cmd->option |= opt;
    } //while end
  }

  return i;
}
/* $end parceCmdOption */

/* $begin initCmd */
void initCmd(struct cmd *cmd)
{
  char path[MAXPATH];
  if (getcwd(path, sizeof path) == NULL)
    unix_error("current working directory access error");
  strcpy(cmd->path, path); /* current working directory */
  cmd->option = 0;         /* no option */
}
/* $end initCmd */

/* $begin initMl */
void initMl(struct max_length *ml)
{
  ml->inode = 0;
  ml->linkCnt = 0;
  ml->user = 0;
  ml->group = 0;
  ml->size = 0;
}
/* $end initMl */

/* sort list */
/* $begin sortList */
struct list_entry copy(struct list_entry *entry)
{
  return *entry;
}

void swap(struct list_entry *a, struct list_entry *b)
{
  struct list_entry temp = copy(a);
  *a = copy(b);
  *b = copy(&temp);
}

int partition(int start, int end, struct list_entry list[], int (*comp)(struct list_entry *, struct list_entry *))
{
  int pivot = (start + end) / 2;

  struct list_entry pivotElem = list[pivot];
  swap(list + pivot, list + end);

  int storageIdx = start;
  int compIdx = start;

  while (compIdx < end)
  {
    if (comp(&pivotElem, list + compIdx) < 0)
    {
      swap(list + compIdx, list + storageIdx);
      storageIdx++;
    }

    compIdx++;
  }

  swap(list + end, list + storageIdx);

  return storageIdx;
}

void sortList(int start, int end, struct list_entry list[], int (*comp)(struct list_entry *, struct list_entry *))
{
  if (start >= end)
    return;

  int pivot = partition(start, end, list, comp);

  sortList(start, pivot - 1, list, comp);
  sortList(pivot + 1, end, list, comp);
}

/* compare functions for sort */
/* sorting by file size */
int compS(struct list_entry *a, struct list_entry *b)
{
  return a->size - b->size;
}
/* sorting by time */
int compT(struct list_entry *a, struct list_entry *b)
{
  return (int)(a->time) - (int)(b->time);
}
/* $$end sortList */
