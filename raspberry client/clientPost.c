/*
 * clientPost.c: A very, very primitive HTTP client for sensor
 * 
 * To run, prepare config-cp.txt and try: 
 *      ./clientPost
 *
 * Sends one HTTP request to the specified HTTP server.
 * Get the HTTP response.
 */

#include <wiringPi.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include <math.h>
#include "stems.h"

#define MAXTIMINGS 83
#define DHTPIN 7

struct timeval startTime;
int dht11_dat[5] = {0, 0, 0, 0, 0};

void initWatch(void)
{
	gettimeofday(&startTime,NULL);
}

float getWatch(void)
{
	struct timeval curTime;
	float tmp;
	
	gettimeofday(&curTime,NULL);
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

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

void read_dht11_dat(float *humidity, float *temperature)
{
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;
  uint8_t flag = HIGH;
  uint8_t state = 0;
  float f;
  char buf[MAXLINE];
  
  for(i=0;i<5;i++){
	dht11_dat[i] = 0;
  }
  
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, LOW);
  delay(18);
  digitalWrite(DHTPIN, HIGH);
  delayMicroseconds(30);
  pinMode(DHTPIN, INPUT);
  for (i = 0; i < MAXTIMINGS; i++) {
    counter = 0;
    while (digitalRead(DHTPIN) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 200) break;
    }
    laststate = digitalRead(DHTPIN);
    if (counter == 200) break; // if while breaked by timer, break for
    if ((i >= 4) && (i % 2 == 0)) {
      dht11_dat[j / 8] <<= 1;
      if (counter > 20) dht11_dat[j / 8] |= 1;
      j++;
    }
  }
  if ((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] +
				      dht11_dat[3]) & 0xff))) {
	sprintf(buf,"%d.%d",dht11_dat[0],dht11_dat[1]);
	*humidity = (float)atof(buf);
	sprintf(buf,"%d.%d",dht11_dat[2],dht11_dat[3]);
	*temperature = (float)atof(buf);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char *hostname, int port, char *filename, float period)
{
	int clientfd;
	char msg[MAXLINE];
	float time;
	float humidity_value,temperature_value;
	float t1,t2;
	if (wiringPiSetup() == -1) exit(1);
	
	while(1){
		humidity_value = temperature_value = -1;
		time = getWatch();
		read_dht11_dat(&humidity_value,&temperature_value);
		
		if(humidity_value == -1 && temperature_value == -1) continue;
		else{
			// humidity value send
			sprintf(msg, "name=humidity&time=%f&value=%f",time, humidity_value);
			clientfd = Open_clientfd(hostname, port);
			clientSend(clientfd, filename, msg);
			t1 = getWatch();
			clientPrint(clientfd);
			t2 = getWatch();
			Close(clientfd);
			//printf("t1 : %f, t2 : %f\n",t1,t2);
			
			// temperature value send
			sprintf(msg, "name=temperature&time=%f&value=%f",time, temperature_value);
			clientfd = Open_clientfd(hostname, port);
			clientSend(clientfd, filename, msg);
			t1 = getWatch();
			clientPrint(clientfd);
			t2 = getWatch();
			Close(clientfd);
			//printf("t1 : %f, t2 : %f\n",t1,t2);
		}
		sleep(period);
	}
}

void getargs_cp(char *hostname, int *port, char *filename, float *period)
{
  FILE *fp;

  fp = fopen("config-pi.txt", "r");
  if (fp == NULL)
    unix_error("config-pi.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%f", period);
  fclose(fp);
}

int main(void)
{
	char hostname[MAXLINE], filename[MAXLINE];
	int port;
	float period;

	srand(time(NULL));
	initWatch();
	
	getargs_cp(hostname, &port, filename, &period);

	userTask(hostname, port, filename, period);
  
	return(0);
}
