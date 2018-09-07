/******************************************************************************
                                                                            
 File Name:      sys_manager.c                                             
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com				
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           		NAME           	DESCRIPTION                               
 20131205		Kevin.Zu		Create 

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
//#include <pthread.h>

#include "sys.h"
#include "sys_manager.h"
#include "log_s.h"


extern log_st *log_array[];


//============== log ===============
int log_id1;
#define LOG_LEVEL1 __NOPRINT_SAVE
#define LOG_NUM1 1
#define LOG_PATH1 "./log1"



static int sys_init(struct sys_manager *sys)
{
	int ret;
	int i;
	
	if(sys == NULL)
		return ERR_PARA;

	

	ret = cfg_get_net_type();
	if(ret < 0){
		printf("Can't get net type!\n");
		return ERR;
	}
	
	switch(ret){
		case NET_TYPE_485:
			ret = cfg_get_serial_num(&sys->com_num);
			if(ret < 0){
				printf("%s:Can't get serial num!\n",__FUNCTION__);
				return ERR;
			}
			sys->net_type = NET_TYPE_485;
			break;
		case NET_TYPE_CAN:
			ret = cfg_get_dev_num_can(&sys->com_num);
			if(ret < 0){
				printf("%s:Can't get can num!\n",__FUNCTION__);
				return ERR;
			}
			sys->net_type = NET_TYPE_CAN;
			break;
		default:
			printf("##### Erro net type!\n");
			return ERR;
	}

	if(sys->net_type == NET_TYPE_485){
		ret = uart_commx_array_init(sys->com_num);
		if(ret != OK){
			printf("uart_commx_array_init err!\n");
			return ret;
		}
	}else if(sys->net_type == NET_TYPE_CAN){
		ret = can_comx_array_init(sys->com_num);
		if(ret != OK){
			printf("can_comx_array_init err!\n");
			return ret;
		}
	}else{
		printf("sys->net_type err!\n");
		return ERR;
	}

	
	ret = sys->tcp_com->init(sys->tcp_com);
	if(ret != OK){
		printf("tcp_com init err!\n");
		return ret;
	}

	ret = sys->esl_attr->init(sys->esl_attr,sys->com_num);
	if(ret != OK){
		printf("esl_attr init err!\n");
		return ret;
	}
	
	ret = sys->aioi_proto->init(sys->aioi_proto,sys->net_type,sys->com_num);
	if(ret != OK){
		printf("aioi_proto init err!\n");
		return ret;
	}
	
	ret = sys->esl_proto->init(sys->esl_proto,sys->net_type);
	if(ret != OK){
		printf("esl_proto init err!\n");
		return ret;
	}

#ifdef __MONITOR__
	ret = sys->can_monitor->init(sys->can_monitor);
	if(ret != OK){
		printf("can dev monitor init err!\n");
		return ret;
	}
#endif

#if 0
	for(i = 0;i < MAX_LOG_FILE_NUM;i ++){
		log_array[i] = NULL;
	}
	log_id1 = LOG_INIT(LOG_PATH1,LOG_FILE_SIZE,LOG_LEVEL1,LOG_NUM1);
#endif

	return OK;
}

static int sys_deinit(struct sys_manager *sys)
{
	int ret;
	
	if(sys == NULL)
		return ERR_PARA;

	if(sys->net_type == NET_TYPE_485){
		ret = uart_commx_array_deinit(sys->com_num);
		if(ret != OK)
			return ret;
	}else if(sys->net_type == NET_TYPE_CAN){
		ret = can_comx_array_deinit(sys->com_num);
		if(ret != OK)
			return ret;
	}else{
		printf("sys->net_type err!\n");
		return ERR;
	}

#ifdef __MONITOR__
	ret = sys->can_monitor->deinit(sys->can_monitor);
	if(ret != OK){
		return ret;
	}
#endif

	ret = sys->tcp_com->deinit(sys->tcp_com);
	if(ret != OK)
		return ret;

	ret = sys->esl_attr->deinit(sys->esl_attr);
	if(ret != OK)
		return ret;
	
	return OK;
}

static int sys_monitor(struct sys_manager *sys)
{
	int ret,i;
	sub_socket_comm_t *pos;

	pos = sys->tcp_com->sub_sock_comm;

	if(sys == NULL)
		return ERR_PARA;

	if(sys->tcp_com->is_thread_alive == 0){
		ret = sys->tcp_com->deinit(sys->tcp_com);
		if(ret == OK)
			sys->tcp_com->init(sys->tcp_com);
	}

#if 0
	if(pos != NULL){
		//if(pos->is_recv_thread_alive == 0 || pos->is_send_thread_alive == 0){
		//	destory_sub_socket_comm(pos);
		//}

		ret = _tcp_tl_is_connected(pos->socket);
		if(ret > 0){
		}else if(ret == 0){
			
		}else{
			destory_sub_socket_comm(pos);
			pos = NULL;
		}
	}
#endif
#if 0
	list_for_each_entry(pos, &sys->tcp_com->sub_socket_list_head, sub_socket_list){
		if(pos->is_recv_thread_alive == 0 || pos->is_send_thread_alive == 0){
			list_del(&pos->sub_socket_list);
			destory_sub_socket_comm(pos);
			break;
		}
		ret = _tcp_tl_is_connected(pos->tcp_sock);
		if(ret > 0){
		}else if(ret == 0){
			
		}else{
			list_del(&pos->sub_socket_list);
			destory_sub_socket_comm(pos);
			break;
		}
	}
#endif

	if(sys->net_type == NET_TYPE_485){
		uart_commx_array_restart(sys->com_num);
	}else if(sys->net_type == NET_TYPE_485){
		
	}else{
		
	}

	return OK;
}

extern tcp_comm_t g_tcp_comm;
#ifdef __DEV_COM__

#else
extern uart_commx_t uart_commx_array[];
#endif
extern esl_attr_hlist_t g_esl_attr_hlist;
extern aioi_proto_t g_aioi_proto;
esl_protocol_t g_esl_proto;
extern can_dev_monitor_t g_can_monitor;

sys_manager_t g_sys_manager = {
	.tcp_com = &g_tcp_comm,
	//.uart_num = 0,
#ifdef __DEV_COM__
//	.comx_array = NULL;
#else
//	.comx_array = uart_commx_array,
#endif
	.esl_attr = &g_esl_attr_hlist,
	.aioi_proto = &g_aioi_proto,
	.esl_proto = &g_esl_proto,
	.can_monitor = &g_can_monitor,

	.init = sys_init,
	.deinit = sys_deinit,
	.monitor = sys_monitor
};


