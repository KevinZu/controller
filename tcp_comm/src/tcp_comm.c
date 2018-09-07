/******************************************************************************
                                                                            
 File Name:      tcp_comm.c                                             
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com				
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20131203		Kevin.Zu		Create 

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "sys.h"
#include "tcp_comm.h"
#include "list.h"


int MY_Sock;


#ifndef SOCKET_TIMEOUT
/* when stream has sequence error: */
/* having SOCKET_TIMEOUT > 0 helps the system to recover */
#define SOCKET_TIMEOUT 0
#endif
#define EXOSIP_MAX_SOCKETS 10
#define NI_MAXHOST      1025
#define NI_NUMERICHOST  1
#define SEND_BUF_SIZE 2048



static void _tcp_comm_close_sockinfo(struct _tcp_sockets *sockinfo)
{
	close(sockinfo->socket);
//	if (sockinfo->buf!=NULL){
//		free(sockinfo->buf);
//		sockinfo->buf = NULL;
//	}
//	if (sockinfo->sendbuf!=NULL){
//		free(sockinfo->sendbuf);
//		sockinfo->sendbuf = NULL;
//	}

	memset(sockinfo, 0, sizeof(*sockinfo));
}

static int _tcp_comm_free(tcp_comm_t *tcp)
{
	//int pos;
	//struct list_head *pos;
	//sub_socket_comm_t *sub_sock;

	if(tcp->sub_sock_comm != NULL){
		IM_LOG("%s:%d      sub_sock_comm->destory_connection\n",__FUNCTION__,__LINE__);
		tcp->sub_sock_comm->destory_connection(tcp->sub_sock_comm);
		destory_sub_socket_comm(tcp->sub_sock_comm);
		tcp->sub_sock_comm = NULL;
	}

	return OK;
}



#define END_HEADERS_STR "\r\n\r\n"
#define CLEN_HEADER_STR "\r\ncontent-length:"



static int _tcp_comm_send_sockinfo (struct _tcp_sockets *sockinfo, const char *msg, int msglen)
{
	int i;
	while (1) {
		i = send(sockinfo->socket, (const void *) msg, msglen, 0);
		if (i < 0) {
			perror("send");
			continue;
		} else if (i == 0) {
			break; /* what's the meaning here? */
		} else if (i < msglen) {
			msglen -= i;
			msg += i;
			continue;
		}
		break;
	}
	return OK;
}


int _tcp_tl_is_connected(int sock)
{
	int res;
	struct timeval tv;
	fd_set wrset;
	int valopt;
	socklen_t sock_len;
	tv.tv_sec = SOCKET_TIMEOUT / 1000;
	tv.tv_usec = (SOCKET_TIMEOUT % 1000) * 1000;

	FD_ZERO(&wrset);
	FD_SET(sock, &wrset);

	res = select(sock + 1, NULL, &wrset, NULL, &tv);
	if (res > 0) {
		sock_len = sizeof(int);
		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *) (&valopt), &sock_len) == 0) {
			if (valopt) {
				_DBG("Cannot connect socket node / %s[%d]\n",
							strerror(errno), errno);
				return -1;
			} else {
				return 0;
			}
		} else {
				_DBG("Cannot connect socket node / error in getsockopt %s[%d]\n",
						strerror(errno), errno);
			return -1;
		}
	} else if (res < 0) {
		_DBG("Cannot connect socket node / error in select %s[%d]\n",
					strerror(errno), errno);
		return -1;
	} else {
		_DBG("Cannot connect socket node / select timeout (%d ms)\n",
					SOCKET_TIMEOUT);
		return 1;
	}
}



static int _tcp_comm_check_connected(struct _tcp_sockets *tcp_sock)
{
	int pos;
	int res;


	if (tcp_sock->socket > 0 && tcp_sock->ai_addrlen > 0) {
		res = connect(tcp_sock->socket, &tcp_sock->ai_addr, tcp_sock->ai_addrlen);
		if (res < 0) {
			int status = errno;

			if (status == EISCONN) {
				tcp_sock->ai_addrlen=0; /* already connected */
				return OK;
			}

			if (status != EINPROGRESS && status != EALREADY) {

				_tcp_comm_close_sockinfo(&tcp_sock);
				return OK;
			} else {
				res = _tcp_tl_is_connected(tcp_sock->socket);
				if (res > 0) {

					return OK;
				} else if (res == 0) {
					/* stop calling "connect()" */
					tcp_sock->ai_addrlen=0;
					return OK;
				} else {
					_tcp_comm_close_sockinfo(&tcp_sock);
					return OK;
				}
			}
		}else{
			/* stop calling "connect()" */
			tcp_sock->ai_addrlen=0;
		}
	}

	return 0;
}


static int _tcp_comm_connect_socket(char *host, int port,struct _tcp_sockets *tcp_sock)
{
	int res;
	int sock = -1;
	struct sockaddr_in addr;

	tcp_sock->socket = 0;

	sock = (int) socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		//_TRACE;
		return -1;
	}


	/* set NON-BLOCKING MODE */
	int val;

	val = fcntl(sock, F_GETFL);
	if (val < 0) {
		close(sock);
		sock = -1;
		//_TRACE;
		return -1;
	}
	val |= O_NONBLOCK;
	if (fcntl(sock, F_SETFL, val) < 0) {
		close(sock);
		sock = -1;
		//_TRACE;
		return -1;
	}


	// fill the server address
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, host, &addr.sin_addr.s_addr);
	addr.sin_port = htons(port);
	
	res = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (res < 0) {
		//_TRACE;
		close(sock);
		sock = -1;
	}

	if (sock > 0) {
		tcp_sock->socket = sock;

		tcp_sock->ai_addrlen = sizeof(addr);
		memset(&tcp_sock->ai_addr, 0, sizeof(struct sockaddr));

		memcpy(&tcp_sock->ai_addr, &addr, sizeof(addr));

		strncpy(tcp_sock->remote_ip, host,
					 sizeof(tcp_sock->remote_ip) - 1);

		tcp_sock->remote_port = port;
		return OK;
	}

	return -1;
}





static int _tcp_comm_open(struct tcp_comm *tcp)
{
	int res;
	int sock = -1;
	struct sockaddr_in local;

	if (tcp->listen_port < 0)
		tcp->listen_port = TCP_COMM_PORT;//5060;

	sock = (int) socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		printf("socket err!\n");
		return -1;
	}

	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(TCP_COMM_PORT);//htons(tl_tcp.proto_port);
	bzero(&(local.sin_zero), 8);
	
	if (bind(sock, (struct sockaddr *) &local, sizeof(struct sockaddr)) < 0) {
		close(sock);
		printf("bind err!\n");
		return -1;
	}

	res = listen(sock, 1/*SOMAXCONN*/);
	if (res < 0) {
		close(sock);
		sock = -1;
		
		printf("listen err!res:%d\n",res);
		return -1;
	}

	//tcp_socket = sock;

	return sock;
}



int tcp_comm_thread(void *para);

static int tcp_comm_init(struct tcp_comm *tcp)
{
	int ret;
	sub_socket_comm_t *sub_sock;
	
	if(tcp == NULL)
		return ERR_PARA;
	
	tcp->main_socket = 0;

	sub_sock = creat_sub_socket_comm(NULL);
	if(sub_sock == NULL){
		printf("Sub socket create err!\n");
		return ERR;
	}
	
	tcp->sub_sock_comm = sub_sock;
	
	ret = _tcp_comm_open(tcp);
	if(ret < 0){
		printf("Main sock open err!%s:%d\n",__FILE__,__LINE__);
		return ERR;
	}

	tcp->main_socket = ret;
	tcp->main_socket_status = SOCK_ENABLE;
	

	tcp->thread = SDL_CreateThread(tcp->thread_func, tcp);
	if(tcp->thread == NULL){
		printf("tcp creat thread err!\n");
		return ERR;
	}

	//INIT_LIST_HEAD(&tcp->sub_socket_list_head);
	//tcp->sub_sock_comm = NULL;
	
	return OK;
}

static int tcp_comm_deinit(struct tcp_comm *tcp)
{
	int ret;
	if(tcp == NULL)
		return ERR_PARA;

	tcp->is_thread_alive = 0;
	if(tcp->thread != NULL){
		SDL_WaitThread(tcp->thread,0);
		tcp->thread = NULL;
	}

	if(tcp->main_socket > 0){
		close(tcp->main_socket); 
		FD_CLR(tcp->main_socket, &tcp->readfds);
		tcp->main_socket = -1;
	}

	_tcp_comm_free(tcp);
	
	return OK;
}



int tcp_comm_thread(void *para)
{
#ifdef __DEBUG__
	struct  timeval    tv0;
        struct  timezone   tz0;
	struct  timeval    tv1;
        struct  timezone   tz1;
	unsigned int time_us;
	gettimeofday(&tv0,&tz0);
#endif
	int ret;
	tcp_comm_t *tcp;
	sub_socket_comm_t *sub_sock = NULL;//,*pos;
	int sock = -1;
	struct sub_sock_para sub_para;
	char src6host[NI_MAXHOST];
	int recvport = 0;
	
	struct sockaddr_in remote;		
	socklen_t slen = sizeof(remote);


	tcp = (tcp_comm_t *)para;

	if(tcp->sub_sock_comm == NULL){
		while(1){printf("e");sleep(1);}
		return ERR;
	}
	
	while(1){
#ifdef __DEBUG__
		gettimeofday(&tv1,&tz1);
		time_us = 1000000 * (tv1.tv_sec - tv0.tv_sec) + tv1.tv_usec - tv0.tv_usec;
		if(time_us > 1000000){
			gettimeofday(&tv0,&tz0);
			printf("----%s----\n",__FUNCTION__);
		}
		
#endif
		usleep(10);

		sock = accept(tcp->main_socket, (struct sockaddr *) &remote, &slen);
		if (sock < 0) {
			printf("tcp_tl_read_message -- FD_ISSET -- accept err\n");
			perror("accept");
			
			return ERR;
		}

		sub_para.socket = sock;
		
		memset(src6host, 0, sizeof(src6host));
		recvport = ntohs(((struct sockaddr_in *) &remote)->sin_port);

		ret = getnameinfo((struct sockaddr *) &remote, slen,src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if(ret != 0){
			snprintf(src6host, sizeof(src6host), "127.0.0.1");
			continue;
		}else{
			sub_para.remote_ip = src6host;
			sub_para.remote_port = recvport;
		}

		IM_LOG("%s:%d      sub_sock_comm->destory_connection\n",__FUNCTION__,__LINE__);
		tcp->sub_sock_comm->destory_connection(tcp->sub_sock_comm);

		
		tcp->sub_sock_comm->create_connection(&sub_para,tcp->sub_sock_comm);
		

	}

	return OK;
}



tcp_comm_t g_tcp_comm = {
	.main_socket = -1,
	.listen_port = TCP_COMM_PORT,
	.main_socket_status = SOCK_DISABLE,
	.is_thread_alive = 1,
	.thread = NULL,

	.init = tcp_comm_init,
	.deinit = tcp_comm_deinit,
	.thread_func = tcp_comm_thread,
};






