
/******************************************************************************
                                                                            
 File Name:      func_hlist.h                                             
 Copyright:    										
 Version:                                                                
 Description:  
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com			
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20130702		Kevin.Zu		Create 

*******************************************************************************/

#ifndef __FUNC_HLIST_H__
#define __FUNC_HLIST_H__

#include "list.h"



//#define FUNC_HEAD_ARRAY_LEN 100



struct func_head
{
	struct hlist_head head;
};


typedef struct func_node
{
	unsigned short id;
/*
	DATA_PROCESS_FUN func_process;
	CMD_ACK_FUNC func_ack;
*/
	void *param;//param;
	struct hlist_node node;
}func_node_t;

typedef struct func_hlist{
	struct func_head *head_array;
	unsigned int head_array_len;

	int (*init)(struct func_hlist *);
	int (*deinit)(struct func_hlist *);
	int (*add_func_node)(struct func_hlist *,struct func_node *);
	int (*del_func_node)(struct func_hlist *,unsigned int);
	struct func_node *(*get_func_node)(struct func_hlist *,unsigned int);
	
}func_hlist_t;

int init_func_hlist(func_node_t *func_node_array,func_hlist_t *func_hlist);
//struct func_head *create_process_hlist(unsigned int head_array_len);


#endif
