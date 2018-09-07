/******************************************************************************

 File Name:      can_bus.h
 Copyright:
 Version:
 Description:
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com	

*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20140219		Kevin.Zu		Create 

*******************************************************************************/
#ifndef __CAN_BUS__
#define __CAN_BUS__

#include <sys/types.h>
#include "SDL.h"
#include "SDL_thread_c.h"
#include "SDL_sysmutex_c.h"
#include "round_robin_queue.h"

#define _DEV_NAME_LEN 15


typedef struct can_data{
	//can_id_t id;
	unsigned int id;
	unsigned char can_dlc;
	unsigned char *data;
}can_data_t;


#define _create_can_id(type,addr,dir,attr,index,ack) \
	(((unsigned int)type << 27 & 0x18000000) | \
	((unsigned int)addr << 11 & 0x07fff800) | \
	((unsigned int)dir << 10 & 0x00000400) | \
	((unsigned int)attr << 9 & 0x00000200) |\
	((unsigned int)index << 1 & 0x000001fe) |\
	((unsigned int)ack & 0x00000001)\
	| CAN_EFF_FLAG)


#define _id_get_type(id)	(((unsigned int)id & 0x18000000) >> 27)
#define _id_get_addr(id)	(((unsigned int)id & 0x07fff800) >> 11)
#define _id_get_dir(id)		(((unsigned int)id & 0x00000400) >> 10)
#define _id_get_attr(id)		(((unsigned int)id & 0x00000200) >> 9)
#define _id_get_index(id)	(((unsigned int)id & 0x000001fe) >> 1)
#define _id_get_ack(id)		((unsigned int)id & 0x00000001) 




typedef struct can_bus{
	char *dev_name;
	int socket;
	unsigned int ID;
	fd_set rset;

	pthread_mutex_t mutex_recv;
	pthread_mutex_t mutex_send;

	int (*init)(struct can_bus *can,unsigned int);
	int (*deinit)(struct can_bus *can);


	int (*recv)(struct can_bus *can,struct can_frame *frame);
	int (*send)(struct can_bus *can,can_data_t  *data);
}can_bus_t;

int can_bus_init(struct can_bus *can,unsigned int no);


#endif

