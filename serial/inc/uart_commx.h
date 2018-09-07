/******************************************************************************
                                                                            
 File Name:      uart_commx.h                                       
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com			
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20131211		Kevin.Zu		Create 

*******************************************************************************/
#ifndef __UART_COMMX_H__
#define __UART_COMMX_H__

//#include <pthread.h>
#include "event.h"
#include "list.h"
#include "serial.h"
#include "SDL.h"
#include "SDL_thread_c.h"
#include "SDL_sysmutex_c.h"
#if 0
#include "round_robin_queue.h"
#endif


#define UART_MAX_NUM 8
#define UART_SEND_FARME_SIZE 512
#define UART_RECV_FARME_SIZE 512




//struct cyclic_query_queue{
//	EVENT_ELEM_T event;
//	SDL_mutex mutex;
//};



typedef struct uart_commx{
	DpUart_t uartx;
	char *send_frame;
	unsigned int send_len;
	//unsigned char *recv_frame;
	//unsigned int recv_len;
	unsigned char is_thread_alive;
	SDL_Thread *thread;
	struct list_head cyclic_query_list_head;
	pthread_mutex_t cyclic_lock;
	//SDL_mutex *mutex;
	pthread_mutex_t mutex;
#if 0
	PARA_BUF_QUEUE_T *uart_request_q;
#endif

	int (*init)(struct uart_commx *uart,unsigned int i);
	int (*deinit)(struct uart_commx *uart,unsigned int i);
	int (*send_and_recv)(void *uart,unsigned char *,unsigned int,unsigned int *);
	int (*add_cyclic_query)(struct uart_commx *uart,EVENT_ELEM_T *event,unsigned char com_no);
	int (*del_cyclic_query)(struct uart_commx *uart,EVENT_ELEM_T *event,unsigned char com_no);
	int (*add_up_data_item)(void *uart,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr);
	int (*del_up_data_item)(void *uart,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr);
	int (*send_broadcast)(void *uart,unsigned int com_no);
}uart_commx_t;


int uart_commx_array_init(unsigned int serial_num);
int uart_commx_array_deinit(unsigned int serial_num);
int uart_commx_array_restart(unsigned int serial_num);

//struct cyclic_query_queue *queue_array;


#endif

