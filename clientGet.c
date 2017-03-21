/*
 * clientGet.c: A very, very primitive HTTP client for console.
 * 
 * To run, prepare config-cg.txt and try: 
 *      ./clientGet
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * For testing your server, you will want to modify this client.  
 *
 * When we test your server, we will be using modifications to this client.
 *
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include "stems.h"

#define MAX_TOKENS 10

struct timeval startTime;

void initWatch(void)
{
  gettimeofday(&startTime, NULL);
}

float getWatch(void)
{
  struct timeval curTime;
  float tmp;

  gettimeofday(&curTime, NULL);
  tmp = (curTime.tv_sec - startTime.tv_sec) * 1000.0;
  return (tmp + (curTime.tv_usec - startTime.tv_usec) / 1000.0);
}

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  // printf("%s\n",buf);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];
  int n;
  
  Rio_readinitb(&rio, fd);
  
   /* Read and display the DATA Body */
  n= Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

void printData(char hostname[], int port, char msg[])
{
	int clientfd;
	float t1,t2;
	
	clientfd = Open_clientfd(hostname, port);
	clientSend(clientfd, msg);
	t1 = getWatch();
	clientPrint(clientfd);
	t2 = getWatch();
	Close(clientfd);
	
	// printf("t1 : %f , t2 : %f\n",t1,t2);
}

int cmdProcessing(char hostname[], int port, char webaddr[])
{
	char cmdLine[MAXLINE];
	char *cmdTokens[MAX_TOKENS];
	char *token;
	char msg[MAXLINE];
	int tokenNum = 0;
	
	fputs("# ",stdout);
	fgets(cmdLine,MAXLINE,stdin);
	
	token = strtok(cmdLine," \t\n\r");
	while(token){
		cmdTokens[tokenNum++] = token;
		token = strtok(NULL," \t\n\r");
	}
	cmdTokens[tokenNum] = NULL;
	
	if(tokenNum == 0) return 0;
	
	if(strcmp(cmdTokens[0],"exit") == 0){
		return 1;
	}
	
	if(strcmp(cmdTokens[0],"LIST") == 0){
		sprintf(msg,"%s?command=LIST", webaddr);
		printData(hostname, port, msg);
		return 0;
	}
	
	if(strcmp(cmdTokens[0],"INFO") == 0 && tokenNum == 2){
		sprintf(msg,"%s?command=INFO&name=%s",webaddr, cmdTokens[1]);
		printData(hostname, port, msg);
		return 0;
	}
	
	if(strcmp(cmdTokens[0],"GET") == 0 && tokenNum >= 2){
		sprintf(msg,"%s?command=GET&name=%s", webaddr, cmdTokens[1]);
		if(tokenNum == 3){
			sprintf(msg,"%s&count=%s",msg,cmdTokens[2]);
		}
		printData(hostname, port, msg);
		return 0;
	}
	
	return 0;
}

/* currently, there is no loop. I will add loop later */
void userTask(char hostname[], int port, char webaddr[])
{
	int isExit;
	
	initWatch();
	
	while(1){
		isExit = cmdProcessing(hostname,port,webaddr);
		if(isExit) break;
	}
	exit(0);
}

void getargs_cg(char hostname[], int *port, char webaddr[])
{
  FILE *fp;

  fp = fopen("config-cg.txt", "r");
  if (fp == NULL)
    unix_error("config-cg.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", webaddr);
  fclose(fp);
}

int main(void)
{
	char hostname[MAXLINE], webaddr[MAXLINE];
	int port;
	pid_t pid;
	int status;
	  
	pid = fork();
	if(pid > 0){
		getargs_cg(hostname, &port, webaddr);
		userTask(hostname, port, webaddr);
		wait(&status);
	}
	else if(pid == 0){
		Execve("./pushServer",NULL,NULL);
	}
	  
	return(0);
}
