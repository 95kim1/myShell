CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = myShell ls mkdir rmdir touch cat echo

all: $(PROGS)

myShell: myShell.c csapp.c

ls: ls.c csapp.c

mkdir: mkdir.c csapp.c

rmdir: rmdir.c csapp.c

touch: touch.c csapp.c

cat: cat.c csapp.c

echo: echo.c csapp.c

clean:
	rm -rf *~ $(PROGS)

