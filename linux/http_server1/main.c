/*
 * main.c
 *
 *  Created on: 2012-8-2
 *      Author: root
 */

//注意问题:
//        	 	1 处理僵尸问题
//				2 处理超时问题(要是20秒不做事就退出:a.select(异步IO),b.alarm,c IO多路复用)
//				3 保存连接数目(统计在线连接数): a 定义全局变量 (不能实现)  b 共享內存技术

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_server.h"

int main(int argc, char* argv[]) {
	start_server(80);
	return 0;
}
