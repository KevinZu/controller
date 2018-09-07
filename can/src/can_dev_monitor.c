/******************************************************************************

 File Name:      can_dev_monitor.c
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
#include "sys.h"
#include "can_dev_monitor.h"
#include "sub_socket_comm.h"
#include "protocol.h"

struct monitor_hlist_head monitor_head_array_req[MONITOR_HLIST_SIZE];

static int monitor_init(struct can_dev_monitor *monitor)
{
	int i,ret;
	unsigned int esl_num;
	struct monitor_hlist_head *pos;
	struct monitor_h_node *Monitor_node;
	
	struct esl_attribute_node *EslNode;
	struct hlist_node *attr_pos;
	
	if(monitor == NULL)
		return ERR_PARA;

	monitor->is_thread_alive = 0;

	pos = monitor->head_array;
	for(i = 0;i < MONITOR_HLIST_SIZE;i ++){
		INIT_HLIST_HEAD(&pos->head);
		pos ++;
	}

	
	for(i = 0;i < monitor->esl_attr->head_array_len;i ++){
		hlist_for_each(attr_pos,&(monitor->esl_attr->head_array)->head + i){
			EslNode = list_entry(attr_pos,struct esl_attribute_node,node);//(ptr, type, member)
			Monitor_node = (struct monitor_h_node *)malloc(sizeof(struct monitor_h_node));

			
			if(Monitor_node == NULL){
				printf("%s:Monitor_node malloc err!\n",__FUNCTION__);
				return ERR;
			}
			
			INIT_HLIST_NODE(&Monitor_node->node);
			Monitor_node->id = EslNode->id;
			gettimeofday(&Monitor_node->tv,NULL);
			hlist_add_head(&Monitor_node->node,&(monitor->head_array + DATA_MOD(Monitor_node->id,MONITOR_HLIST_SIZE))->head);
		}
	}


	ret = pthread_mutex_init(&monitor->mutex,NULL);
	if(ret != OK){
		printf("monitor->mutex init err! line:%d\n",__LINE__);
		return ERR;
	}
	
	return OK;
}

static int monitor_deinit(struct can_dev_monitor *monitor)
{
	int i;
	struct hlist_node *pos;
	struct monitor_h_node *Monitor_node;

	for(i = 0;i < /*monitor->esl_attr->head_array_len*/MONITOR_HLIST_SIZE;i ++){
		hlist_for_each(pos,&(monitor->head_array)->head + i){
			Monitor_node = list_entry(pos,struct monitor_h_node,node);
			hlist_del(pos);
			free(Monitor_node);
		}
	}
	
	return OK;
}

static int monitor_update(struct can_dev_monitor *monitor,unsigned short id)
{
//printf("head_array_len:%d   id=%d   ",m->esl_attr->head_array_len,id);
	int i;
	struct hlist_node *pos;
	struct monitor_h_node *Monitor_node;

	//IM_LOG(" id=%d\n ",id);
#if 0
	for(i = 0;i < monitor->esl_attr->head_array_len;i ++){

		//IM_LOG("i=%d ",i);
		hlist_for_each(pos,&(monitor->head_array)->head + i){
			Monitor_node = list_entry(pos,struct monitor_h_node,node);
			IM_LOG("%d ",Monitor_node->id);
			if(Monitor_node->id == id){
				pthread_mutex_lock(&monitor->mutex);
				gettimeofday(&Monitor_node->tv,NULL);
				pthread_mutex_unlock(&monitor->mutex);
				usleep(100);
				return OK;
			}
		}

	}
#else
	hlist_for_each(pos,&(monitor->head_array)->head +  DATA_MOD(id,MONITOR_HLIST_SIZE)){
		Monitor_node = list_entry(pos,struct monitor_h_node,node);
		//IM_LOG("%d ",Monitor_node->id);
		if(Monitor_node->id == id){
			pthread_mutex_lock(&monitor->mutex);
			gettimeofday(&Monitor_node->tv,NULL);
			pthread_mutex_unlock(&monitor->mutex);
			usleep(100);
			return OK;
		}
	}
#endif
	//printf(":this id is not registed!\n");

	return OK;
}

extern aioi_proto_t g_aioi_proto;
extern sub_socket_comm_t g_sub_socket;

static int monitor_thread_func(void *para)
{
	int i,ret;
	struct timeval tv;
	unsigned int time_us;
	struct hlist_node *pos;
	struct monitor_h_node *Monitor_node;
	struct can_dev_monitor *monitor = (struct can_dev_monitor *)para;
	aioi_proto_t *proto;
	unsigned char *buf;
	//struct return_data *ret_d;
	short esl_id;

	unsigned char *send_buf;
	sub_socket_comm_t *sub_sock;

	proto = &g_aioi_proto;
	sub_sock = &g_sub_socket;
	
	while(1){
		//IM_LOG("E ");
		for(i = 0;i < /*monitor->esl_attr->head_array_len*/MONITOR_HLIST_SIZE;i ++){
			//IM_LOG("m ");
			hlist_for_each(pos,&(monitor->head_array)->head + i){
				Monitor_node = list_entry(pos,struct monitor_h_node,node);
				gettimeofday(&tv,NULL);
				time_us = 1000000 * (tv.tv_sec - Monitor_node->tv.tv_sec) + tv.tv_usec - Monitor_node->tv.tv_usec;
				//IM_LOG(" %d-%d ",Monitor_node->id,time_us);
				if(time_us > CAN_NO_ACK_TIMEOUT){
					esl_id = Monitor_node->id;
					//printf(" %d +- ",esl_id);
#if 0
					buf = get_free_para_buf(proto->para_buf_q);
					if(buf != NULL){
						memset(buf,0,PARA_BUF_SIZE);
						ret_d = (struct return_data *)buf;
						ret_d->type = DATA_TYPE_ERR_INFO;//DATA_TYPE_DISP;
						ret_d->len = 1;
						ret_d->id = Monitor_node->id;

						buf[sizeof(struct return_data)] = '1';
						
	//					IM_LOG("line:%d:      id:%d \n",__LINE__,ret_d->id);
						
						ret = para_process_buf_insert(proto->para_buf_q);
						if(ret != OK){
							printf("############### Can update q insert err! line:%d\n",__LINE__);
							return ret;
						}
					}
#else
					send_buf = get_free_para_buf(sub_sock->send_frame_q);
					if(send_buf != NULL){
						memset(send_buf,0,PARA_BUF_SIZE);
						send_buf[1] = 'e';
						send_buf[5] = esl_id%10 + 0x30;
						esl_id = esl_id/10;
						send_buf[4] = esl_id%10 + 0x30;
						esl_id = esl_id/10;
						send_buf[3] = esl_id%10 + 0x30;
						esl_id = esl_id/10;
						send_buf[2] = esl_id%10 + 0x30;
						send_buf[6] = 0x30;                        // err type
						send_buf[7] = get_check_sum(send_buf + 1,5);
						send_buf[8] = 0x0d;


						send_buf[0] = 8;

						ret = para_process_buf_insert(sub_sock->send_frame_q);
						if(ret != OK){
							printf("para_process_buf_insert err !  fun: %s \n",__FUNCTION__);
							return ret;
						}
					}else{
						printf("sub_sock->send_frame_q get err!\n");
					}
#endif
				}

				usleep(5000);
			}

			usleep(100000);
			//IM_LOG("m ");
		}
		
		if(monitor->is_thread_alive == 0)
			return OK;
	}
	
	return OK;
}

static int monitor_create_thread(struct can_dev_monitor *monitor)
{

#ifdef __MONITOR__
	monitor->is_thread_alive = 1;
	monitor->thread = SDL_CreateThread(monitor_thread_func,monitor);
	if(monitor->thread == NULL){
		printf("%s: __thread create err! monitor->thread:%d\n",__FUNCTION__,monitor->thread);
		return ERR;
	}
#endif
	return OK;
}

static int monitor_del_thread(struct can_dev_monitor *monitor)
{
#ifdef __MONITOR__
	monitor->is_thread_alive = 0;
	SDL_WaitThread(monitor->thread,0);

	monitor->thread = NULL;
#endif

	return OK;
}


extern esl_attr_hlist_t g_esl_attr_hlist;

can_dev_monitor_t g_can_monitor = {
	.head_array = monitor_head_array_req,

	.esl_attr = &g_esl_attr_hlist,
	
	.init = monitor_init,
	.deinit = monitor_deinit,
	.update = monitor_update,

	.create_thread = monitor_create_thread,
	.del_thread = monitor_del_thread,
	.thread_func = monitor_thread_func
};


