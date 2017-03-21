#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	char request_body[MAXLINE];
	int content_length;
	char* name;
	float data, time;
	
	content_length = atoi(getenv("CONTENT_LENGTH"));
	read(STDIN_FILENO,request_body,content_length+1);
	
	strtok(request_body, "=");
	name = strtok(NULL, "&");
		
	strtok(NULL, "=");
	time = atof(strtok(NULL, "&"));
		
	strtok(NULL, "=");
	data = atof(strtok(NULL, "&"));
	
	fprintf(stderr, "warning: [ %s sensor ] : %f 시각에 %f 값 발생\n", name, time, data);
	printf("HTTP/1.0 200 OK\n");
	fflush(stdout);
	
	return (0);
}
