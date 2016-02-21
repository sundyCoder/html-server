/*
 * http_server.h
 *
 *  Created on: 2012-8-2
 *      Author: root
 */

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

int read_line(int fd, char *buf, int maxsize);
void do_get(int fd, char *filename);
void get_client_data(int client_fd);
void data_process(int client_fd);
void myHandler(int p);
int init_sock(int port);
void start_server(int port);

void timeout();

#endif /* HTTP_SERVER_H_ */
