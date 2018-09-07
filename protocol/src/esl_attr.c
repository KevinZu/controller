/******************************************************************************
                                                                            
 File Name:      esl_attr.c                                            
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
#include <stddef.h>
#include "esl_attr.h"
#include "sys.h"
#include "config.h"




int init_esl_attr_hlist(struct esl_attr_hlist *esl_attr_hlist,unsigned char com_num)
{
	int ret,i,j;
	unsigned int esl_num;
	struct esl_attribute_node *node;
	
	if(esl_attr_hlist == NULL)
		return ERR;
	//ret = cfg_get_serial_num(&com_num);
	//if(ret < 0){
	//	_DBG("Cfg file err!\n");
	//	return ERR;
	//}
	_DBG("ComNum:%d\n",com_num);

	for(i = 0;i < com_num;i ++){
		ret = cfg_get_esl_num(i,&esl_num);
		if(ret < 0){
			printf("cfg_get_esl_num err!\n");
			return ERR;
		}
		_DBG("esl_num:%d\n",esl_num);
		for(j = 0;j < esl_num;j ++){
			node = (struct esl_attribute_node *)malloc(sizeof(struct esl_attribute_node));
			if(node == NULL){
				printf("esl_attribute_node malloc err!\n");
				return ERR;
			}
			memset(node,0,sizeof(struct esl_attribute_node));
			
			ret = cfg_get_spec_com_attr(i,j,&node->id,&node->type,&node->shelf_no);
			if(ret < 0){
				printf("cfg_get_spec_com_attr err!\n");
				return ERR;
			}
			
			//_DBG("----- id:%d  type:%d shelf no: %d\n",node->id,node->type,node->shelf_no);
			node->com_no = i;
			
			esl_attr_hlist->add_node(esl_attr_hlist,node);
		}
	}
	
	return OK;
}



static int esl_attr_hlist_init(struct esl_attr_hlist *hlist,unsigned int com_num)
{
	int i;
	
	struct esl_attr_head *pos;
	
	pos = hlist->head_array;
	for(i = 0;i < hlist->head_array_len;i ++){
		INIT_HLIST_HEAD(&pos->head);
		pos ++;
	}
	//return OK;
	return init_esl_attr_hlist(hlist,com_num);
}


static int esl_attr_hlist_deinit(struct esl_attr_hlist *hlist)
{

	return OK;
}


static int esl_attr_hlist_add(struct esl_attr_hlist *hlist,struct esl_attribute_node *esl_attr_node)
{
	INIT_HLIST_NODE(&esl_attr_node->node);
	hlist_add_head(&esl_attr_node->node,&(hlist->head_array + DATA_MOD(esl_attr_node->id,hlist->head_array_len))->head);

	return OK;
}



static int esl_attr_hlist_del(struct esl_attr_hlist *hlist,unsigned int id)
{
	
	struct esl_attribute_node *EslNode;
	struct hlist_node *pos;
	
	
	hlist_for_each(pos,&(hlist->head_array+ DATA_MOD(id,hlist->head_array_len))->head){

		const struct hlist_node *__mptr = pos;
		EslNode = (struct esl_attribute_node *)((char *)__mptr - offsetof(struct esl_attribute_node,node) );
		
		if(EslNode->id == id){
			hlist_del(pos);
			break;
		}
	}

	return OK;
}

static struct esl_attribute_node *esl_attr_hlist_get(struct esl_attr_hlist *hlist,const unsigned int id)
{

	struct esl_attribute_node *EslNode;
	struct hlist_node *pos;
	

	hlist_for_each(pos,&(hlist->head_array+ DATA_MOD(id,hlist->head_array_len))->head){
		EslNode = list_entry(pos,struct esl_attribute_node,node);//(ptr, type, member)
		
		if(EslNode->id == id){
			return EslNode;
		}
	}

	return NULL;
}






struct esl_attr_head esl_attr_hlist_head[ESL_ATTR_HEAD_SIZE];

esl_attr_hlist_t g_esl_attr_hlist = {
	.head_array = esl_attr_hlist_head,
	.head_array_len = ESL_ATTR_HEAD_SIZE,
	.init = esl_attr_hlist_init,
	.deinit = esl_attr_hlist_deinit,
	.add_node = esl_attr_hlist_add,
	.del_node = esl_attr_hlist_del,
	.get_node = esl_attr_hlist_get
};



