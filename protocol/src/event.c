/******************************************************************************
                                                                            
 File Name:      event.c                                            
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
#include <stddef.h>
#include "sys.h"
#include "event.h"



void event_process(struct list_head *event_head,void *ret)
{
	EVENT_ELEM_T *pevent;
	struct list_head *pos,*n;

	
	if(list_empty(event_head))
		return;

	list_for_each_safe(pos,n,event_head){
		if(pos == NULL){
			printf("next:%d   prev:%d \n",event_head->next,event_head->prev);
			return;
		}

		pevent = list_entry(pos, EVENT_ELEM_T, list_e);
		*(int *)ret = pevent->event_handle(pevent->params);
	}
}



/********************************* event *****************************/

//LIST_HEAD(timer_event_list);
//LIST_HEAD(key_event_list);
//LIST_HEAD(proto_event_list);




