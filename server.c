#include "stems.h"
#include "request.h"


// 
// To run:
// 1. Edit config-ws.txt with following contents
//    <port number>
// 2. Run by typing executable file
//    ./server
// Most of the work is done within routines written in request.c
//


sem_t mutex,full,empty;
struct element *queue;
int front,rear,queSize;

void getargs_ws(int *port, int *p, int *n)
{
  FILE *fp;

  if ((fp = fopen("config-ws.txt", "r")) == NULL)
    unix_error("config-ws.txt file does not open.");

  fscanf(fp, "%d", port);
  fscanf(fp, "%d", p);
  fscanf(fp, "%d", n);
  fclose(fp);
}

void *consumer(void *param)
{
	struct element item;
	pid_t pid;
	pthread_t tid;
	
	while(1){
		// consumer
		sem_wait(&full);
		sem_wait(&mutex);
		front = (front+1) % queSize;
		item = queue[front];
		sem_post(&mutex);
		sem_post(&empty);
		
		if(item.connfd == -1) break;
		requestHandle(item.connfd, item.time);
		Close(item.connfd);
	}
	// thread exit
	pthread_exit(0);
}

int main(void)
{
	int listenfd, connfd, port, clientlen;
	int poolSize, i;
	struct sockaddr_in clientaddr;
	pid_t pid;
	int status;
	pthread_t *thread_pool;
	pthread_attr_t attr;
	
	initWatch();
	getargs_ws(&port,&poolSize,&queSize);
	
	listenfd = Open_listenfd(port);
	
	pid = fork();
	if(pid > 0){
		// semaphore init
		sem_init(&mutex,0,1);
		sem_init(&full,0,0);
		sem_init(&empty,0,queSize);
		// thread pool , queue allocation
		thread_pool = (pthread_t *)malloc(sizeof(pthread_t) * poolSize);
		queue = (struct element *)malloc(sizeof(struct element) * queSize);
		// threadSize ~ thread create
		for(i=0 ; i < poolSize ; i++) {
			pthread_attr_init(&attr);
			pthread_create(&thread_pool[i],&attr,consumer,NULL);
		}
		
		while (1) {
			clientlen = sizeof(clientaddr);
			connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
			// producer
			sem_wait(&empty);
			sem_wait(&mutex);
			rear = (rear+1) % queSize;
			queue[rear].connfd = connfd;
			queue[rear].time = getWatch();
			sem_post(&mutex);
			sem_post(&full);
		}
		// exit state send
		for(i=0 ; i <  poolSize ; i++){
			connfd = -1;
			
			sem_wait(&empty);
			sem_wait(&mutex);
			rear = (rear+1) % queSize;
			queue[rear].connfd = connfd;
			sem_post(&mutex);
			sem_post(&full);
		}
		// thread join
		for(i=0 ; i < poolSize; i++) {
			pthread_join(thread_pool[i],NULL);
		}
		
		wait(&status);
	}
	else if(pid == 0){
		Execve("./pushClient",NULL,NULL);
	}
	
	return(0);
}
