/******************************************************************************
                                                                            
 File Name:      func_hlist.c                                             
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
#include <stddef.h>
#include "sys.h"
#include "func_hlist.h"

static int process_hlist_init(func_hlist_t *hlist)
{
	int i;
	
	struct func_head *pos;
	
	pos = hlist->head_array;
	for(i = 0;i < hlist->head_array_len;i ++){
		INIT_HLIST_HEAD(&pos->head);
		pos ++;
	}

	return OK;
}


static int process_hlist_deinit(func_hlist_t *hlist)
{

	return OK;
}


static int process_hlist_add(func_hlist_t *hlist,struct func_node *func_node)
{
	INIT_HLIST_NODE(&func_node->node);
	hlist_add_head(&func_node->node,&(hlist->head_array + DATA_MOD(func_node->id,hlist->head_array_len))->head);

	return OK;
}



static int process_hlist_del(func_hlist_t *fhlist,unsigned int id)
{
	//int ret,i;
	
	struct func_node *FuncNode;
	struct hlist_node *pos;
	
	
	hlist_for_each(pos,&(fhlist->head_array+ DATA_MOD(id,fhlist->head_array_len))->head){

		const struct hlist_node *__mptr = pos;
		FuncNode = (struct func_node *)((char *)__mptr - offsetof(struct func_node,node) );
		
		if(FuncNode->id == id){
			hlist_del(pos);
			break;
		}
	}

	return OK;
}

static struct func_node *process_hlist_get(func_hlist_t *fhlist,unsigned int id)
{

	struct func_node *FuncNode;
	struct hlist_node *pos;
	struct hlist_head *h;
	h = &(fhlist->head_array+ DATA_MOD(id,fhlist->head_array_len))->head;
	

	hlist_for_each(pos,/*&(fhlist->head_array+ DATA_MOD(id,fhlist->head_array_len))->head*/h){
		
		const struct hlist_node *__mptr = pos;
		FuncNode = (struct func_node *)((char *)__mptr - offsetof(struct func_node,node) );
		
		if(FuncNode->id == id){
			return FuncNode;
		}
	}

	return NULL;
}


/************************* gloabal *********************/
int init_func_hlist(func_node_t *func_node_array,func_hlist_t *func_hlist)
{
	func_node_t *pos;
	
	if(func_node_array == NULL || func_hlist == NULL)
		return ERR;
	
	for(pos = func_node_array;pos->param != NULL;pos ++){
		func_hlist->add_func_node(func_hlist,pos);
	}

	return OK;
}

/************************** picking protocol function hlist ********************************/
#define G_PICK_ARRAY_SIZE 50

struct func_head esl_hlist_head[G_PICK_ARRAY_SIZE];

func_hlist_t g_esl_func = {
	esl_hlist_head,
	G_PICK_ARRAY_SIZE,
	process_hlist_init,
	process_hlist_deinit,
	process_hlist_add,
	process_hlist_del,
	process_hlist_get
};

/*****************************specific process & state handle hlist ****************************/
#define G_PROCESS_STATE_SIZE 20

struct func_head process_hlist_head[G_PROCESS_STATE_SIZE];

func_hlist_t g_process_func = {
	process_hlist_head,
	G_PROCESS_STATE_SIZE,
	process_hlist_init,
	process_hlist_deinit,
	process_hlist_add,
	process_hlist_del,
	process_hlist_get
};

/******************************** other function hlist **************************************/



