#include <sys/stat.h>
#include <stdio.h>

int main(void)
{
  int err = mkdir("./testtt/", 755);
  printf("%d\n", err);
  return 0;
}