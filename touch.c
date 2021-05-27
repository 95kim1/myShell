/* proj4: implementation of shell
 * name: sunghee kim
 * 
 * project of system programming course of sogang univ.
 * 
 * touch command
 * 
 * this source code is for 'touch' of linux shell.
 * but it has no 100% functions.
 * 
 * option:
 * -a: to change file access and modification time.
 * -m: it is used only modify time of a file.
 * -r: to dupdate time of one file with reference to the other file.
 * -t: to create a file by specifying the time.
 * -c: it doesn't create n empty file.
 */
#include "csapp.h"

enum
{
  _a_, // to change file access and modification time.
  _m_, // it is used only modify time of a file.
  _r_, // to dupdate time of one file with reference to the other file.
  _t_, // to create a file by specifying the time.
  _c_, // it doesn't create n empty file.
};

int main(int argc, char *argv[])
{

  return 0;
}