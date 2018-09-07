/******************************************************************************
                                                                            
 File Name:      protocol.h                                          
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com				
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20131203		Kevin.Zu		Create 

*******************************************************************************/
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include <unistd.h>
#include "func_hlist.h"
#include "esl_attr.h"
#include "uart_commx.h"
#include "esl_proto.h"
#include "round_robin_queue.h"

#define CONF_PORT_PARAM_END 0x1f

#define EDATA_END_FLAG (0x1D)

#define RET_BUF_SIZE 20
enum{
	DATA_TYPE_DISP = 0x31,
	DATA_TYPE_SCAN,
	DATA_TYPE_SET_ID,
	DATA_TYPE_HEART_BEAT,
	DATA_TYPE_ERR_INFO
};
//#define DATA_TYPE_SCAN 2
//#define DATA_TYPE_DISP 1
#define DATA_TYPE_KEY 3


#define DATA_ID_LEN			4
#define NOVA_DISP_LEN		10
#define NOVA_ESL_DISP_LEN	(DATA_ID_LEN +  NOVA_DISP_LEN)//(4(id) + 10(data))


//------------- config file ------------

/*
struct port_attr{
	unsigned char port_no;
	unsigned char esl_num[3];
}__attribute__((packed));

struct esl_attr{
	unsigned char esl_id[4];
	unsigned char esl_type;
	unsigned char gs_no;
}__attribute__((packed));
*/


//--------------------------------

//#define SCAN_DATA_LEN 8



struct event_params{
//	char cmd;
	unsigned int com_no;
	unsigned char shelf_no;
	short id;
	const char *para_in;
	unsigned int para_len;
	char *send_data;
	unsigned int send_len;
	EVENT_ELEM_T *event;
};

struct return_data{
	char type;
	unsigned short id;
	unsigned char len;
}__attribute__((packed));


typedef struct answer_frame{
	unsigned char result;
	unsigned char check_sum;
	unsigned char CR;
}__attribute__((packed))answer_frame_t;


typedef struct dev_com{
	char *send_frame;
	unsigned int send_len;
	pthread_mutex_t mutex;
	pthread_mutex_t mutex_s;
	void *EslNode;
	void *spec_com;
	

	int (*send_and_recv)(void *comx,unsigned int,unsigned char *,unsigned int *);
	int (*add_up_data_item)(void *comx,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr);
	int (*del_up_data_item)(void *comx,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr);
	int (*send_broadcast)(void *canx,unsigned int com_no);
}dev_com_t;

typedef int (*AIOI_PROCESS_FUN_T)(const char *,unsigned int,char *,unsigned int *);


typedef struct aioi_proto{
	esl_attr_hlist_t *esl_attr;
	func_hlist_t *fun_h;
//#ifdef __DEV_COM__
	dev_com_t *commx_array;
//#else
//	uart_commx_t *commx_array;
//#endif
	esl_protocol_t *esl_proto;
	PARA_BUF_QUEUE_T *para_buf_q;
	
	
	int (*init)(struct aioi_proto *,unsigned int,unsigned int);
	int (*deinit)(struct aioi_proto *);
	int (*process)(struct aioi_proto *,const char *recv,unsigned int recv_len,char *send,unsigned int *send_len);
	int (*get_up_data)(struct aioi_proto *,char *ret_buf,unsigned int *);
}aioi_proto_t;


#endif

