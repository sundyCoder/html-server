/*
 * http_server.c
 *
 *  Created on: 2012-8-2
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "http_server.h"

#define WEB_ROOT "/WEB"

//hello\r\n 13 10
int read_line(int fd, char *buf, int maxsize) { //how to deal with masize>sizeof(buf) ?
	char ch;
	int i;
	for (i = 0; i < maxsize; ++i) {
		int n = recv(fd, &ch, 1, 0); //equivaent to read(fd,&ch,1)
		if (1 == n) {
			buf[i] = ch;
			//printf("ch = %d\n",ch);   //testing...........
			if (ch == '\n')
				break;
		} else {
			return -1;
		}
	}
	return i + 1;
}

void do_get(int fd, char *filename) {

	if (strcmp(filename, "/") == 0)
		filename = "/index.html"; //处理默认首页的问题
	char path[128] = { '\0' };
	//strcat(WEB_ROOT,filename);
	sprintf(path, "%s%s", WEB_ROOT, filename);
	FILE *fp = fopen(path, "r");
	if (NULL == fp) {
		printf("The file %s is not found!\n", path);
		char errmsg[32] = "<h1>404 file not found!</h1>";
		send(fd, errmsg, strlen(errmsg), 0);
		return;
	}

	while (1) {
		char buf[1024] = { '\0' };
		int read_size = fread(buf, 1, sizeof(buf), fp);
		if (read_size <= 0)
			break;
		send(fd, buf, read_size, 0);
	}
	fclose(fp);
	return;
}

void get_client_data(int client_fd) {
	//web server core code
	while (1) {
		char buf[128] = { '\0' };
		int size = read_line(client_fd, buf, sizeof(buf)); //0 stands no flags
		printf("size:%d,read_line = %s", size, buf);
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

//重构
void data_process(int client_fd) {
	struct sockaddr_in c_addr;
	socklen_t c_len = sizeof(struct sockaddr);
	getpeername(client_fd, (struct sockaddr *) &c_addr, &c_len);
	printf("a client connected:ip = %s,port = %d\n",
			inet_ntoa(c_addr.sin_addr), c_addr.sin_port);

	get_client_data(client_fd);

	printf("a client disconnected:ip = %s,port = %d\n", inet_ntoa(
			c_addr.sin_addr), c_addr.sin_port);
	//close(client_fd);
	exit(0);
}

void myHandler(int p) {
	wait(NULL);
	return;
}

int init_sock(int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0); //
	if (-1 == sockfd) {
		perror("socket");
		exit(-1);
	}

	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

	struct sockaddr_in ser_addr;
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(port);
	memset(ser_addr.sin_zero, '\0', sizeof(ser_addr.sin_zero));
	int ret = bind(sockfd, (struct sockaddr *) &ser_addr,
			sizeof(struct sockaddr));
	if (-1 == ret) {
		perror("bind");
		exit(-1);
	}

	ret = listen(sockfd, 10);
	if (-1 == ret) {
		perror("listen");
		exit(-1);
	}
	printf("The http server is ready!\n");
	return sockfd;
}

//GET /index.html HTTP/1.1\r\n

void start_server(int port) {
	int sockfd = init_sock(port);
	while (1) {
		int client_fd = accept(sockfd, NULL, NULL);
		if (-1 == client_fd) {
			perror("accept");
			continue; //break  //exit(1);
		}

		signal(SIGCHLD, myHandler);

		int shmid = shmget(1234, 512, IPC_CREAT | 0644);
		struct shmid_ds *buf = malloc(sizeof(struct shmid_ds));
		int pid = fork();
		if (-1 == pid) {
			perror("fork");
			break;
		} else if (pid == 0) {

			shmat(shmid, NULL, 0); //映射共享內存
			shmctl(shmid, IPC_STAT, buf);
			printf("the number is:%d\n", (int) buf->shm_nattch);
			data_process(client_fd);
		}
		close(client_fd);
	}
	close(sockfd);
	return;
}


/*设置超时,尚未成功
void timeout(int clientfd) {
	fd_set rfds;
	struct timeval tv;
	int retval;

	 Watch stdin (fd 0) to see when it has input.
	FD_ZERO(&rfds);

	 FD_SET(0, &rfds);


	 Wait up to five seconds.
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	retval = select(clientfd, &rfds, NULL, NULL, &tv);
	 Don’t rely on the value of tv now!

	if (retval == -1)
		perror("select()");
	else if (retval)
		printf("Data is available now.\n");
	 FD_ISSET(0, &rfds) will be true.
	else
		printf("No data within five seconds.\n");

}
*/

