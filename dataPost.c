#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <linux/stat.h>
#include <mysql/mysql.h>

#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "qlrqodtmfrl2"
#define DB_NAME "project"

#define MAXNAME 20
#define MAXQUERY 200

#define PUSHFIFO "pushfifo"

//
// This program is intended to help you test your web server.
// 

void query_err(MYSQL* connection)
{
	fprintf(stderr, "Mysql query error : %s", mysql_error(connection));
}


int main(int argc, char *argv[])
{
	char *method;
	char request_body[MAXLINE];
	int content_length;
	
	char *name;
	float data, time;
	
	FILE *fp;
	int index, last_id;
	char query[MAXQUERY];
	MYSQL *connection=NULL , conn;
	MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
	
	method = getenv("REQUEST_METHOD");
	content_length = atoi(getenv("CONTENT_LENGTH"));
	read(STDIN_FILENO,request_body,content_length+1);
	
	// pipe length , body send
	fp = fopen(PUSHFIFO,"w");
	fprintf(fp,"%d",content_length);
	fputs(request_body, fp);
	fclose(fp);
	
	printf("%s\r\n",request_body);
	fflush(stdout);
	
	
	//name 분리
	strtok(request_body, "=");
	name = strtok(NULL, "&");
	
	//time 분리
	strtok(NULL, "=");
	time = atof(strtok(NULL, "&"));
	
	//data 분리
	strtok(NULL, "=");
	data = atof(strtok(NULL, "&"));
	
	//mysql 초기화;
	mysql_init(&conn);
	
	//SQL 연결
	if((connection = mysql_real_connect(&conn,DB_HOST,DB_USER,DB_PASS,DB_NAME,0,(char*)NULL,0)) == NULL)
	{
		query_err(connection);
		return 1;
	}
		
	sprintf(query,"SELECT id FROM sensorList WHERE name='%s';",name);
	if(mysql_query(connection, query))
	{
		query_err(connection);
		return 1;
	}
	sql_result = mysql_store_result(connection);
		
	if(mysql_num_rows(sql_result) != 0){
		sql_row = mysql_fetch_row(sql_result);
		index = atoi(sql_row[0]);
		//sensorList에 센서들을 위한 테이블이 미리 정의되어 있어야한다.
		sprintf(query, "UPDATE sensorList set sensorList.count = sensorList.count + 1 WHERE sensorList.name='%s';", name);
		if(mysql_query(connection, query))
		{
			query_err(connection);
			return 1;
		}
			
		sprintf(query, "UPDATE sensorList set sensorList.sum = sensorList.sum + %f WHERE sensorList.name='%s';", data, name);
		if(mysql_query(connection, query))
		{
			query_err(connection);
			return 1;
		}
			
		sprintf(query, "UPDATE sensorList set sensorList.avg = sensorList.sum / sensorList.count WHERE sensorList.name='%s';", name);
		if(mysql_query(connection, query))
		{
			query_err(connection);
			return 1;
		}
			
		sprintf(query, "INSERT INTO sensorData%d VALUES ('%f', '%f', NULL);", index, time, data);
		if(mysql_query(connection, query))
		{
			query_err(connection);
			return 1;
		}
		
		mysql_free_result(sql_result);
	}
	else{
		mysql_free_result(sql_result);
		
		sprintf(query, "INSERT INTO sensorList VALUES ('%s', NULL, '%d', '%f', '%f');", name, 1, data, data);
		if(mysql_query(connection, query))
		{
			query_err(connection);
			return 1;
		}
		
		sprintf(query,"SELECT id FROM sensorList WHERE name='%s';",name);
		if(mysql_query(connection, query))
		{
			query_err(connection);
			return 1;
		}
		sql_result = mysql_store_result(connection);
		sql_row = mysql_fetch_row(sql_result);
		last_id = atoi(sql_row[0]);
		
		sprintf(query, "CREATE TABLE sensorData%d (time FLOAT, data FLOAT, id INT not null auto_increment primary key);", last_id);
		if(mysql_query(connection, query))
		{
			query_err(connection);
			return 1;
		}
			
		sprintf(query, "INSERT INTO sensorData%d VALUES ('%f', '%f', NULL);", last_id, time, data);
		if(mysql_query(connection, query))
		{
			query_err(connection);
			return 1;
		}
		mysql_free_result(sql_result);
	}
		
		
	mysql_close(connection);
	
	return (0);
}
