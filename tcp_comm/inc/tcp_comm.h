/******************************************************************************
                                                                            
 File Name:      tcp_comm.h                                       
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
#ifndef __TCP_COMM_H__
#define __TCP_COMM_H__

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "SDL.h"
#include "SDL_thread_c.h"
#include "list.h"
#include "sub_socket_comm.h"


#define TCP_COMM_PORT 6780
#define MESSAGE_MAX_LENGTH 4096



//extern struct tl_protocol tl_udp;
//



typedef struct tcp_comm{
	int main_socket;
	int listen_port;
	int main_socket_status;
	fd_set readfds, writefds, exceptfds; 

	unsigned char is_thread_alive;
	SDL_Thread *thread;
//	struct list_head sub_socket_list_head;
	sub_socket_comm_t *sub_sock_comm;
	

	int (*init)(struct tcp_comm *);
	int (*deinit)(struct tcp_comm *);
	int (*thread_func)(void *);
//	int (*read_message)(struct tcp_comm *);
}tcp_comm_t;




#endif

