/******************************************************************************
                                                                            
 File Name:      esl_attr.h                                             
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com					
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20131224		Kevin.Zu		Create 


*******************************************************************************/
#ifndef __ESL_ATTR_H__
#define __ESL_ATTR_H__

#include "list.h"


struct esl_attribute_node{
	short id;
	unsigned char type;
	unsigned char com_no;
	unsigned char shelf_no;
	struct hlist_node node;
};


struct esl_attr_head
{
	struct hlist_head head;
};

typedef struct esl_attr_hlist{
	struct esl_attr_head *head_array;
	unsigned int head_array_len;

	int (*init)(struct esl_attr_hlist *,unsigned int);
	int (*deinit)(struct esl_attr_hlist *);
	int (*add_node)(struct esl_attr_hlist *,struct esl_attribute_node *);
	int (*del_node)(struct esl_attr_hlist *,unsigned int);
	struct esl_attribute_node *(*get_node)(struct esl_attr_hlist *,const unsigned int);
}esl_attr_hlist_t;

#define ESL_ATTR_HEAD_SIZE 20

//int init_esl_attr_hlist(struct esl_attr_hlist *esl_attr_hlist);


#endif

