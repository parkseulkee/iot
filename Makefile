#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o stems.o clientGet.o clientPost.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread 

.SUFFIXES: .c .o 

all: server clientPost clientGet dataGet.cgi dataPost.cgi pushServer pushClient push.cgi

server: server.o request.o stems.o
	$(CC) $(CFLAGS) -o server server.o request.o stems.o $(LIBS)

clientGet: clientGet.o stems.o
	$(CC) $(CFLAGS) -o clientGet clientGet.o stems.o

clientPost: clientPost.o stems.o
	$(CC) $(CFLAGS) -o clientPost clientPost.o stems.o $(LIBS) -lm

dataGet.cgi: dataGet.c stems.o
	$(CC) $(CFLAGS) -o dataGet.cgi dataGet.c stems.o -lmysqlclient

dataPost.cgi: dataPost.c stems.o
	$(CC) $(CFLAGS) -o dataPost.cgi dataPost.c stems.o -lmysqlclient 
pushServer: pushServer.o request.o stems.o
	$(CC) $(CFLAGS) -o pushServer pushServer.o request.o stems.o $(LIBS)
pushClient: pushClient.o stems.o
	$(CC) $(CFLAGS) -o pushClient pushClient.o stems.o $(LIBS)
push.cgi: push.c stems.o
	$(CC) $(CFLAGS) -o push.cgi push.c stems.o -lmysqlclient
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

server.o: stems.h request.h
clientGet.o: stems.h
clientPost.o: stems.h
pushServer.o : stems.h
pushClient.o : stems.h

clean:
	-rm -f $(OBJS) server clientPost clientGet dataGet.cgi dataPost.cgi pushServer pushClient push.cgi
