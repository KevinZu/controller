/******************************************************************************

 File Name:      can_dev_monitor.h
 Copyright:
 Version:
 Description:
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com	

*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20140702		Kevin.Zu		Create 

*******************************************************************************/

#ifndef __CAN_DEV_MONITOR_H__
#define __CAN_DEV_MONITOR_H__

#include <pthread.h>
#include <sys/time.h>
#include "list.h"
#include "esl_attr.h"
#include "SDL.h"


#define MONITOR_HLIST_SIZE 30

#define DELAY_SEC_NUM 5
#define CAN_NO_ACK_TIMEOUT (1000000 * DELAY_SEC_NUM)

struct monitor_hlist_head{
	struct hlist_head head;
};

struct monitor_h_node{
	short id;
	struct timeval tv;
	struct hlist_node node;
}__attribute__((packed));


typedef struct can_dev_monitor
{
	unsigned int esl_num;
	struct monitor_hlist_head *head_array;
	
	SDL_Thread *thread;
	unsigned char is_thread_alive;
	pthread_mutex_t mutex;

	esl_attr_hlist_t *esl_attr;
	
	int (*init)(struct can_dev_monitor *monitor);
	int (*deinit)(struct can_dev_monitor *monitor);

	int (*update)(struct can_dev_monitor *monitor,unsigned short esl_id);

	int (*create_thread)(struct can_dev_monitor *monitor);  //call by sub_tcp_com
	int (*del_thread)(struct can_dev_monitor *monitor); //call by sub_tcp_com
	int (*thread_func)(void *para);
}can_dev_monitor_t;


#endif

