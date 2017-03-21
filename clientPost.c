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
#include <math.h>
#include "stems.h"

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
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);
  
  
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* gaussian Random */
float gaussianRandom(float average, float stdev) {
  float v1, v2, s, temp;

  do {
    v1 =  2 * ((float) rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 까지의 값
    v2 =  2 * ((float) rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 까지의 값
    s = v1 * v1 + v2 * v2;
  } while (s >= 1 || s == 0);

  s = sqrt( (-2 * log(s)) / s );

  temp = v1 * s;
  temp = (stdev * temp) + average;

  return temp;
}

/* currently, there is no loop. I will add loop later */
void userTask(char *myname, char *hostname, int port, char *filename, float period, float average, float std_dev)
{
	int clientfd;
	char msg[MAXLINE];
	float time,value;
	float t1,t2;
	
	initWatch();
	
	while(1){
		time = getWatch();
		value = gaussianRandom(average, std_dev);
		sprintf(msg, "name=%s&time=%f&value=%f", myname, time, value);
		
		clientfd = Open_clientfd(hostname, port);
		clientSend(clientfd, filename, msg);
		t1 = getWatch();
		clientPrint(clientfd);
		t2 = getWatch();
		Close(clientfd);
		
		printf("t1 : %f , t2 : %f\n",t1,t2);
		sleep(period);
	}
}

void getargs_cp(char *myname, char *hostname, int *port, char *filename, float *period, float *average, float *std_dev)
{
  FILE *fp;

  fp = fopen("config-cp.txt", "r");
  if (fp == NULL)
    unix_error("config-cp.txt file does not open.");

  fscanf(fp, "%s", myname);
  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%f", period);
  fscanf(fp, "%f", average);
  fscanf(fp, "%f", std_dev);
  fclose(fp);
}

int main(void)
{
	char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE];
	int port;
	float period,average,std_dev;

	srand(time(NULL));
	
	getargs_cp(myname, hostname, &port, filename, &period, &average, &std_dev);

	userTask(myname, hostname, port, filename, period, average, std_dev);
  
	return(0);
}
