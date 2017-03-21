/*
 * clientPost.c: A very, very primitive HTTP client for sensor
 * 
 * To run, prepare config-cp.txt and try: 
 *      ./clientPost
 *
 * Sends one HTTP request to the specified HTTP server.
 * Get the HTTP response.
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include "stems.h"

#define PUSHFIFO "./pushfifo"
#define SENSORCOUNT 3

struct sensor sensorList[SENSORCOUNT] = { {"dummy",0} , {"temperature",0} , {"humidity",0} };

void clientSend(int fd, char *filename, char *body)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "POST %s HTTP/1.1\n", filename);
  sprintf(buf, "%sHost: %s\n", buf, hostname);
  sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
  sprintf(buf, "%sContent-Length: %d\n\r\n", buf, strlen(body));
  sprintf(buf, "%s%s\n", buf, body);
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

  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}


/* currently, there is no loop. I will add loop later */
void userTask(char *hostname, int port, char *filename)
{
	int clientfd;
	char msg[MAXLINE] , request_body[MAXLINE] , name[MAXLINE];
	float data;
	int length , i;
	FILE *fp;
	
	while(1){
		mknod(PUSHFIFO, S_IFIFO|0666, 0);
		fp = fopen(PUSHFIFO,"r");
		fscanf(fp,"%d",&length);
		fgets(msg,length,fp);
		fclose(fp);
		unlink(PUSHFIFO);
		
		strcpy(request_body,msg);
		strtok(request_body, "=");
		strcpy(name, strtok(NULL, "&"));
		strtok(NULL, "=");
		strtok(NULL, "=");
		data = atof(strtok(NULL, ""));
		
		for(i=0 ; i < SENSORCOUNT ; i++){
			if(strcmp(name,sensorList[i].name) == 0 && data > sensorList[i].threshold){
				clientfd = Open_clientfd(hostname, port);
				clientSend(clientfd, filename, msg);
				clientPrint(clientfd);
				Close(clientfd);
			}
		}
	}
}

void getargs_cp(char *hostname, int *port, char *filename)
{
  FILE *fp;
  int i;
  
  fp = fopen("config-pc.txt", "r");
  if (fp == NULL)
    unix_error("config-cp.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  for(i = 0 ; i < SENSORCOUNT ; i++)
	fscanf(fp, "%f", &sensorList[i].threshold);
  fclose(fp);
}

int main(void)
{
  char hostname[MAXLINE], filename[MAXLINE];
  int port;

  getargs_cp(hostname, &port, filename);

  userTask(hostname, port, filename);
  
  return(0);
}
