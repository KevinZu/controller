
/******************************************************************************
                                                                            
 File Name:      sub_socket_comm.h                                       
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com			
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           		NAME           	DESCRIPTION                               
 20131210		Kevin.Zu		Create 

*******************************************************************************/
#ifndef __SUB_SOCKET_MANAGER_H__
#define __SUB_SOCKET_MANAGER_H__

#include <sys/socket.h>
#include <netinet/in.h>

#include "sys.h"
#include "list.h"
//#include "thread.h"
#include "SDL.h"
#include "SDL_thread_c.h"
#include "cmd_handler.h"
#include "round_robin_queue.h"
#include "can_dev_monitor.h"


#define RECV_FRAME_BUF_LEN 1024//2048
#define SEND_FRAME_BUF_LEN 1024
#define REMOTE_IP_LEN 65

#define RECV_BUF_LEN 102
#define SIZE_OF_MSG_LEN 2


/* persistent connection */
struct _tcp_sockets {
	int socket;
	struct sockaddr ai_addr;
	size_t ai_addrlen;
	char remote_ip[REMOTE_IP_LEN];
	int remote_port;
};



typedef struct sub_socket_comm{
/********** socket ************/
	struct _tcp_sockets tcp_sock;
//	int socket;

/************* buf *************/
	PARA_BUF_QUEUE_T *send_frame_q;
	TCP_BUF_QUEUE_T *tcp_recv_buf_q;
	pthread_mutex_t recv_q_mutex;
//	unsigned char copy_count;

	char *buf;      /* recv buffer */
	size_t bufsize; /* allocated size of buf */
	size_t buflen;  /* current length of buf */
	char *sendbuf;  /* send buffer */
	size_t sendbufsize;
	size_t sendbuflen;

/********** threads ************/
	unsigned char is_recv_thread_alive;
	unsigned char is_send_thread_alive;
	SDL_Thread *recv_thread;
	SDL_Thread *send_thread;
	cmd_handler_t *cmd_handler;

/********** can dev monitor ************/
	can_dev_monitor_t *can_monitor;

/*********** connection manager **************/
	int (*create_connection)(struct sub_sock_para *sub_para,struct sub_socket_comm *sub_sock);
	int (*destory_connection)(struct sub_socket_comm *sub_sock);
}sub_socket_comm_t;

struct sub_sock_para{
	int socket;
	int remote_port;
	char *remote_ip;
};

sub_socket_comm_t *creat_sub_socket_comm(void *data);
int destory_sub_socket_comm(sub_socket_comm_t *sub_sock);

#endif

