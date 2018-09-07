/******************************************************************************
                                                                            
 File Name:      event.h                                            
 Copyright:      									
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com		
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20130605		Kevin.Zu		Create 
 20131205		Kevin.Zu		Ported to linux

*******************************************************************************/
#ifndef __EVENT_H__
#define __EVENT_H__
//#include <pthread.h>
#include "list.h"

typedef int (*EVENT_HANDLE)(void *data);


typedef struct event_elem{
	void *params;
	EVENT_HANDLE event_handle;
	struct list_head list_e;
}EVENT_ELEM_T;


void event_process(struct list_head *event_head,void *ret);
//int event_add_list(EVENT_ELEM_T *event_elem,struct list_head *event_head);
//int event_del_list(EVENT_ELEM_T *event_elem,struct list_head *event_head);

#endif

