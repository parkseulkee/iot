#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <mysql/mysql.h>
//
// This program is intended to help you test your web server.
// You can use it to test that you are correctly having multiple 
// threads handling http requests.
//
// htmlReturn() is used if client program is a general web client
// program like Google Chrome. textReturn() is used for a client
// program in a embedded system.
//
// Standalone test:
// # export QUERY_STRING="name=temperature&time=3003.2&value=33.0"
// # ./dataGet.cgi

#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "qlrqodtmfrl2"
#define DB_NAME "project"

void htmlReturn(void)
{
	char content[MAXLINE] , temp[MAXLINE];
	char *buf;
	char *ptr;

	/* Make the response body */
	sprintf(content, "%s<html>\r\n<head>\r\n", content);
	sprintf(content, "%s<title>CGI test result</title>\r\n", content);
	sprintf(content, "%s</head>\r\n", content);
	sprintf(content, "%s<body>\r\n", content);
	sprintf(content, "%s<h2>Welcome to the CGI program</h2>\r\n", content);
	strcpy(temp,getenv("QUERY_STRING"));
	buf = temp;
	sprintf(content,"%s<p>Env : %s</p>\r\n", content, buf);
	ptr = strsep(&buf, "&");
	while (ptr != NULL){
		sprintf(content, "%s%s\r\n", content, ptr);
		ptr = strsep(&buf, "&");
	}
	sprintf(content, "%s</body>\r\n</html>\r\n\r\n", content);
  
	/* Generate the HTTP response */
	printf("Content-Length: %d\r\n", strlen(content));
	printf("Content-Type: text/html\r\n");
	printf("%s", content);
	fflush(stdout);
}

void textReturn(void)
{
	char content[MAXLINE] , temp[MAXLINE];
	char *buf;
	char *ptr;
	
	strcpy(temp,getenv("QUERY_STRING"));
	buf = temp;
	printf("env : %s\n",buf);
	sprintf(content,"%sEnv : %s\n", content, buf);
	ptr = strsep(&buf, "&");
	while (ptr != NULL){
		sprintf(content, "%s%s\n", content, ptr);
		ptr = strsep(&buf, "&");
	}
	
	/* Generate the HTTP response */
	printf("Content-Length: %d\n", strlen(content));
	printf("Content-Type: text/plain\r\n\r\n");
	printf("%s", content);
	fflush(stdout);
}

void parseBody(char *cmd, char *name, int *count){
	char *body , *ptr;
	
	body = getenv("QUERY_STRING");

	ptr = strsep(&body,"=");
	ptr = strsep(&body,"&");
	strcpy(cmd,ptr);
	
	
	if(body != NULL && strstr(body,"name")){
		ptr = strsep(&body,"=");
		ptr = strsep(&body,"&");
		strcpy(name,ptr);
	}
	
	if(body != NULL && strstr(body,"count")){
		ptr = strsep(&body,"=");
		ptr = strsep(&body,"");
		*count = atoi(ptr);
	}
}

void dataReturn(){
	char cmd[MAXLINE], s_name[MAXLINE] ;
	int sensorId = 0, sensorCnt = 0, count = 0;
	char query[MAXLINE], context[MAXLINE] = "";
	MYSQL *connection = NULL, conn;
	MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
    int field,i;
    
	mysql_init(&conn);

	parseBody(cmd,s_name,&count);
	
	
	connection = mysql_real_connect(&conn,DB_HOST,DB_USER,DB_PASS,DB_NAME,0,NULL,0);
	if(connection == NULL){
		fprintf(stderr, "mySQL connection error");
		return;
	}
	
	// query setting
	if(strcmp(cmd,"LIST") == 0){
		sprintf(query,"SELECT name FROM sensorList");
	}
	else if(strcmp(cmd,"INFO") == 0){
		sprintf(query,"SELECT name,count,sum,avg FROM sensorList WHERE name = '%s'",s_name);
	}
	else if(strcmp(cmd,"GET") == 0){
		sprintf(query,"SELECT id,count FROM sensorList WHERE name = '%s'",s_name);
		mysql_query(connection, query);
		sql_result = mysql_store_result(connection);
		sql_row = mysql_fetch_row(sql_result);
		sensorId = atoi(sql_row[0]);
		sensorCnt = atoi(sql_row[1]);
		mysql_free_result(sql_result);
		
		if(count == 0){
			sprintf(query,"SELECT time , data FROM sensorData%d WHERE id=%d", sensorId , sensorCnt);
		}
		else{
			sensorCnt = sensorCnt-count+1>0 ? sensorCnt-count+1:1;
			sprintf(query,"SELECT time , data FROM sensorData%d WHERE id>=%d",sensorId, sensorCnt);
		}
	}
	else{
		fprintf(stderr,"command error");
		return;
	}
	
	mysql_query(connection, query);
	sql_result = mysql_store_result(connection);
	field = mysql_num_fields(sql_result);
	while ( (sql_row = mysql_fetch_row(sql_result)) != NULL )
    {
		for(i=0;i<field;i++){
			sprintf(context,"%s%s ",context,sql_row[i]);
		}
		sprintf(context,"%s\n",context);
    }
    printf("%s",context);
    fflush(stdout);
    mysql_free_result(sql_result);
	mysql_close(connection);
}

int main(void){
	// textReturn();
	// htmlReturn();
	dataReturn();
	return (0);
}
