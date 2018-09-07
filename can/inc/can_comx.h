/******************************************************************************

 File Name:      can_comx.h
 Copyright:
 Version:
 Description:
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com	

*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20140115		Kevin.Zu		Create 

*******************************************************************************/
#ifndef __CAN_COMX_H__
#define __CAN_COMX_H__

#include <pthread.h>
#include <sys/time.h>
#include "list.h"
#include "round_robin_queue.h"
#include "can_bus.h"
#include "esl_attr.h"

#define CAN_PORT_NUM 2
#define COM_FRAME_LEN 200


//#define TIME_OUT_UMS 200000
#define RE_TRANS_TIMES 2
#define TIME_OUT_UMS 200000
#define TIME_SEC_NUM 2
#define CAN_RECV_TIMEOUT (1000000 * TIME_SEC_NUM)

/************ recv hlist *************/
#define H_HEAD_ARRY_SIZE 20
#define H_ELEM_Q_SIZE 20
#define PACKET_ELEM_Q_SIZE 100

struct recv_hlist_head
{
	struct hlist_head head;
	
};

struct recv_h_node{
	unsigned short addr;
	unsigned char ack;
	struct timeval tv_now;
	struct list_head packet_head;
	struct hlist_node node;
}__attribute__((packed));

struct recv_packet_node{
	unsigned char index;
	unsigned char dlc;
	unsigned char data[8];
	struct list_head pack_list;
}__attribute__((packed));


struct can_frame_list{
	unsigned short addr;
	unsigned char ack;
	unsigned char *frame;
	unsigned int frame_len;
	struct list_head list;
};


typedef struct can_comx{
	can_bus_t can;
	char *send_frame;
	unsigned int send_len;
	
	unsigned char is_thread_alive;
	unsigned char is_independent_thread_alive;
	pthread_mutex_t mutex;

	SDL_Thread *thread;
	SDL_Thread *independent_thread;

	PARA_BUF_QUEUE_T *answer_q;

	_BUF_QUEUE_T *h_elem_q;
	_BUF_QUEUE_T *packet_elem_q;

/********** recv hlist ***********/
	struct recv_hlist_head *recv_head_array_ack;
	unsigned int recv_head_array_ack_len;

	struct recv_hlist_head *recv_head_array_req;
	unsigned int recv_head_array_req_len;

	struct list_head frame_list;

	
	int (*init)(struct can_comx *canx,unsigned int i);
	int (*deinit)(struct can_comx *canx,unsigned int i);
	int (*send_and_recv)(void *canx,unsigned int com_no,unsigned char *,unsigned int *);
	int (*add_up_data_item)(void *canx,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr);
	int (*del_up_data_item)(void *canx,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr);
	int (*send_broadcast)(void *canx,unsigned int com_no);
}can_comx_t;


#endif

