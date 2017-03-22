#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = stems.o clientPost.o
TARGET = clientPost

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread 

.SUFFIXES: .c .o 

all: clientPost

clientPost: clientPost.o stems.o
	$(CC) $(CFLAGS) -o clientPost clientPost.o stems.o $(LIBS) -lm -lwiringPi

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clientPost.o: stems.h

clean:
	-rm -f $(OBJS) clientPost
