#include <iostream>
using namespace std;
#include <Winsock2.h>
#include "web.h"
#define WEB_ROOT "D:/WEB"

#pragma comment(lib,"ws2_32.lib")

void WebServer::init(){
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return;
	}
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) {
			WSACleanup( );
			return; 
	}
}

int WebServer::read_line(int fd, char *buf, int maxsize){
	char ch;
	int i;
	for (i = 0; i < maxsize; ++i) {
		int n = recv(fd, &ch, 1, 0); //equivaent to read(fd,&ch,1)
		if (1 == n) {
			buf[i] = ch;
			//printf("ch = %d\n",ch);   //testing...........
			if (ch == '\n'){
				++i;
				break;
			}
		} else if(0 == n){
			return -1;
		}else{
			return -1;
		}
	}
	return i;
}

void WebServer::do_get(int fd, char *filename){

	if (strcmp(filename, "/") == 0)
		filename = "/index.html"; //处理默认首页的问题
	char path[128] = { '\0' };
	//strcat(WEB_ROOT,filename);
	sprintf(path, "%s%s", WEB_ROOT, filename);
	cout<<"test for file open!"<<path<<endl;
	FILE *fp = fopen(path, "r");
	if (NULL == fp) {
		printf("The file %s is not found!\n", path);
		char errmsg[32] = "<h1>404 file not found!</h1>";
		send(fd, errmsg, strlen(errmsg), 0);
		return;
	}

	while (1) {
		char buf[2048] = { '\0' };
		int read_size = fread(buf, 1, sizeof(buf), fp);
		if (read_size <= 0)
			break;
		send(fd, buf, read_size, 0);
	}
	fclose(fp);
	return;
}

void WebServer::get_client_data(int client_fd){
	while (1) {
			char buf[128] = { '\0' };
			int size = read_line(client_fd, buf, sizeof(buf)); //0 stands no flags
			cout<<"size:"<<size<<"read_line:"<<buf<<endl;
			if (size <= 2)
				break;

			//get client's message
			char *p = strchr(buf, ' ');
			if (p) {
				*p++ = '\0';
				if (strcmp("GET", buf) == 0) {
					char *q = strchr(p, ' ');
					if (q) {
						*q = '\0';
						do_get(client_fd, p); //deal with GET
					}
				}
			}
	}
}

int WebServer::init_sock(){
	init();
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockfd){
		cout<<"socket error!"<<endl;
		return -1;
	}

	//char opt = '\0';
	//setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

	struct sockaddr_in ser_addr;
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = INADDR_ANY;
	ser_addr.sin_port = htons(port);
	memset(ser_addr.sin_zero,'\0',sizeof(ser_addr.sin_zero));

	int ret = bind(sockfd,(struct sockaddr *)&ser_addr,sizeof(struct sockaddr_in));
	if(-1 == ret){
		cout<<"bind error!"<<endl;
		return -1;
	}
	ret = listen(sockfd,20);
	if(-1 == ret){
		cout<<"listen error!"<<endl;
		return -1;
	}
	cout<<"The Web server is ready!"<<endl;
	return 0;
}

DWORD WINAPI MyWebServerProc(LPVOID lpParameter){
	struct Param p;
	p.fd = ((struct Param *)lpParameter)->fd;
	p.web = ((struct Param *)lpParameter)->web;
	int clientfd = p.fd;
	char buf[128] = "Welcome to JChen's Web Server!\n";
	send(clientfd,buf,strlen(buf),0);
	while(true){
		memset(buf,'\0',sizeof(buf));
	p.web->get_client_data(clientfd);

	struct sockaddr_in c_addr;
	int len = sizeof(struct sockaddr_in);
	getpeername(p.fd,(struct sockaddr *)&c_addr,&len);
	printf("a client disconnected:ip = %s,port = %d\n", inet_ntoa(c_addr.sin_addr), c_addr.sin_port);

	closesocket(clientfd); //very important!!!
	return 0;
}
}

void WebServer::start_server(){
	init_sock();
	char welcome[128] = {'\0'};
	while(true){
		struct sockaddr_in c_addr;
		int c_len = sizeof(struct sockaddr_in);
		int clientfd = accept(sockfd,(struct sockaddr *)&c_addr,&c_len);
		if (-1 == clientfd)
		{
			cout<<"accept error!"<<endl;
			continue;
		}
		sprintf(welcome,"IP:%s,port:%u is connected!\n",inet_ntoa(c_addr.sin_addr),c_addr.sin_port);
		cout<<welcome<<endl;
		send(sockfd,welcome,strlen(welcome),0);
		struct Param p;
		p.fd = clientfd;
		p.web = this;
		CreateThread(NULL,0,MyWebServerProc,&p,0,NULL); //多线程处理
	}
}

int main(){
	WebServer web(80);
	web.start_server();
	return 0;
}