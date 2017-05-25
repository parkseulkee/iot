# 온습도 알리미 IoT
라즈베리 파이를 이용해 일정기간마다 집안의 온습도 측정하여 일정 온습도가 넘으면 사용자에게 경고 알람을 보내는 프로젝트.
서버와 클라이언트 통신은 HTTP 프로토콜을 이용한다.
서버는 클라이언트의 동적 요청을 처리하기 위해 CGI 프로그램을 실행시킨다.

## 프로젝트 개발 기간 및 환경
* **기간 :** 2016-09 ~ 2016-11
* **OS :** Ubuntu 14.04 LTS, Rasbian
* **Language :** C

## 개발 내용
* 라즈베리 파이 클라이언트 : 일정기간마다 집안의 온습도를 측정하여 리눅스 웹 서버에게 데이터를 보냄
* 우분투 서버 : 받은 데이터를 데이터베이스에 삽입, 사용자의 GET 요청이 있으면 그에 맞는 데이터를 전송해줌
* 클라이언트 : 사용자는 최근 집안의 온습도를 콘솔 명령어를 통해 서버에게 데이터를 얻음
* 푸시 알람 서버 : 라즈베리 파이에게 받은 데이터가 일정 온습도를 넘으면 사용자에게 경고 알람을 전송

## Source
* 라즈베리 파이 클라이언트 : [clientPost.c](https://github.com/parkseulkee/iot/blob/master/raspberry%20client/clientPost.c)
* 사용자 클라이언트 : [dataGet.c](https://github.com/parkseulkee/iot/blob/master/dataGet.c)
* 우분투 웹 서버 : [server.c](https://github.com/parkseulkee/iot/blob/master/server.c)
