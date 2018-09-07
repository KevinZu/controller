/******************************************************************************

 File Name:      can_comx.c
 Copyright:
 Version:
 Description:
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com	

*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20140115		Kevin.Zu		Create 

*******************************************************************************/
#include <stdio.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/socket.h>
#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/raw.h>
#include <sys/time.h>
#include "sys.h"
#include "esl_attr.h"
#include "can_comx.h"
#include "can_bus.h"
#include "protocol.h"

#include <sys/time.h>
#include "can_dev_monitor.h"

#ifdef   __cplusplus
    extern   "C" 
    {
#endif


#define __RECV_H


can_comx_t can_comx_array[CAN_PORT_NUM];


extern aioi_proto_t g_aioi_proto;
extern esl_protocol_t g_esl_proto;
extern can_dev_monitor_t g_can_monitor;


/********************** can recv hlist handler ********************/

static int recv_hlist_add(struct recv_hlist_head *recv_head_array,struct recv_h_node*recv_node)
{
	INIT_HLIST_NODE(&recv_node->node);
	hlist_add_head(&recv_node->node,&(recv_head_array + DATA_MOD(recv_node->addr,H_HEAD_ARRY_SIZE))->head);

	return OK;
}



static struct recv_h_node *recv_hlist_get(struct recv_hlist_head *recv_head,unsigned int addr)
{
	struct recv_h_node*recv_node;
	struct hlist_node *pos;
	struct hlist_head *h;
	h = &recv_head->head;

	hlist_for_each(pos,h){
		const struct hlist_node *__mptr = pos;
		recv_node = (struct recv_h_node *)((char *)__mptr - offsetof(struct recv_h_node,node) );
		
		if(recv_node->addr == addr){
			return recv_node;
		}
	}

	return NULL;
}



/*************************** can  recv packet handler*****************************/




/********************* thread ***************/

static int can_independent_thread_func(void *para)
{
	int i;
	struct can_comx *can;
	struct hlist_node *pos;
	struct list_head *li;
	struct  timeval	tv;
	struct hlist_head *h;
	struct recv_h_node *recv_node;
	unsigned int time_us;
	struct recv_packet_node *packet;
	
	can = (struct can_comx *)para;

	while(1){
		// TODO: Check the timeout of packet receiving
		for(i = 0;i < can->recv_head_array_ack_len;i ++){
			h = &(can->recv_head_array_ack + i)->head;
			hlist_for_each(pos,h){
				gettimeofday(&tv,NULL);
				recv_node = hlist_entry(pos,struct recv_h_node,node);
				time_us = 1000000 * (tv.tv_sec - recv_node->tv_now.tv_sec) + tv.tv_usec - recv_node->tv_now.tv_usec;
				if(time_us > CAN_RECV_TIMEOUT){
					printf("__________ time out! addr:%d    ack:%d \n",recv_node->addr,recv_node->ack);
					list_for_each(li,&recv_node->packet_head){
						packet = list_entry(li,struct recv_packet_node,pack_list);
						__list_del(packet->pack_list.prev,packet->pack_list.next);

						q_buf_insert(can->packet_elem_q,(unsigned char *)packet);
					}
					__hlist_del(pos);
					q_buf_insert(can->h_elem_q,(unsigned char*)recv_node);
				}
			}
		}

		for(i = 0;i < can->recv_head_array_req_len;i ++){
			h = &(can->recv_head_array_req + i)->head;
			hlist_for_each(pos,h){
				gettimeofday(&tv,NULL);
				recv_node = hlist_entry(pos,struct recv_h_node,node);
				time_us = 1000000 * (tv.tv_sec - recv_node->tv_now.tv_sec) + tv.tv_usec - recv_node->tv_now.tv_usec;
				if(time_us > CAN_RECV_TIMEOUT){
					printf("__________ time out! addr:%d    ack:%d \n",recv_node->addr,recv_node->ack);
					list_for_each(li,&recv_node->packet_head){
						packet = list_entry(li,struct recv_packet_node,pack_list);
						__list_del(packet->pack_list.prev,packet->pack_list.next);

						q_buf_insert(can->packet_elem_q,(unsigned char *)packet);
					}
					__hlist_del(pos);
					q_buf_insert(can->h_elem_q,(unsigned char*)recv_node);
				}
			}
		}
		
		//
		if(can->is_thread_alive == 0)
			return OK;
		usleep(10);
	}
}

/*
 int test_update(struct can_dev_monitor *monitor,unsigned short esl_id)
{
	int i;
	struct hlist_node *pos;
	struct monitor_h_node *Monitor_node;


	return OK;
}
 extern can_dev_monitor_t g_can_monitor;
 */

void test(can_dev_monitor_t *monitor,unsigned short id)
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
	//printf("%s:this id is not registed!\n");

	return OK;
}

static int can_thread_func(void *para)
{
	int ret,i;
	struct can_comx *can;
	struct can_frame frame;
	unsigned char *buf;
	unsigned short addr;
	unsigned char dir;
	unsigned char attr;
	unsigned char index;
	unsigned char ack;
	struct can_frame_list *pos = NULL,*can_frame = NULL;
	struct list_head  *pos_h;
	unsigned char flag = 0;
	struct aioi_proto *proto;
	proto = &g_aioi_proto;
	struct return_data *ret_d;
	unsigned char *node_buf;

	can_data_t ack_update;
	unsigned char ack_data[10];


	struct recv_packet_node *packet_node;
	struct recv_hlist_head *h_head;
	struct recv_h_node *h_node;

	can_dev_monitor_t *monitor = &g_can_monitor;
	
	can = (struct can_comx *)para;

	memset(ack_data,0,10);
	
	while(1){
		ret = can->can.recv(&can->can,&frame);
		
		if(ret < 0){
			printf("############### can frame recv err!\n");
			can->is_thread_alive == 0;
			return ERR;
		}

		addr = _id_get_addr(frame.can_id);
		attr = _id_get_attr(frame.can_id);
		ack = _id_get_ack(frame.can_id);
		index = _id_get_index(frame.can_id);
		dir = _id_get_dir(frame.can_id);

		if(ret == 0){  //can_dlc == 0
			//  Heart Beat Packet Process
			//printf("id:%d  ",addr);
#ifdef __MONITOR__
			monitor->update(monitor,addr);
			//test(monitor,addr);
			IM_LOG("-m ");
#endif
			usleep(100);
			continue;
		}


		if(dir != 0x01){
			IM_LOG("-d ");
			continue;
		}
		
		flag = 0;

		IM_LOG("-c ");

		if(ack == 1){
			h_head = can->recv_head_array_ack + DATA_MOD(addr,H_HEAD_ARRY_SIZE);
			h_node = recv_hlist_get(h_head,addr);
			if(h_node != NULL){
				flag = 1;
			}
		}else if(ack == 0){
			h_head = can->recv_head_array_req + DATA_MOD(addr,H_HEAD_ARRY_SIZE);
			h_node = recv_hlist_get(h_head,addr);
			if(h_node != NULL){
				flag = 1;
			}
		}else{
			printf("!!! wrong ack!\n");
		}

		
		if(flag == 0 && attr == 0){
			if(ack == 1){
				if(frame.data[0] != 0x02){
					IM_LOG("%s:%d:      addr:%d \n",__FUNCTION__,__LINE__,addr);
					buf = get_free_para_buf(can->answer_q);
					if(buf != NULL){
						memset(buf,0,PARA_BUF_SIZE);
						*(unsigned short *)buf = addr;
						buf[2] += frame.can_dlc;
						memcpy(buf + 3,frame.data,frame.can_dlc);

						ret = para_process_buf_insert(can->answer_q);
						if(ret != OK){
							printf("###############  Can ack q insert err! line:%d\n",__LINE__);
							can->is_thread_alive = 0;
							return ret;
						}
						IM_LOG("%s:%d:      addr:%d \n",__FUNCTION__,__LINE__,addr);
					}
				}
			}else{
				buf = get_free_para_buf(proto->para_buf_q);
				if(buf != NULL){
					memset(buf,0,PARA_BUF_SIZE);
					ret_d = (struct return_data *)buf;
					ret_d->type = frame.data[0];//DATA_TYPE_DISP;
					ret_d->len = frame.can_dlc;
					ret_d->id = addr;
					
					memcpy(buf + sizeof(struct return_data),frame.data + 1,ret_d->len - 1);
					IM_LOG("line:%d:      id:%d \n",__LINE__,ret_d->id);
					
					ret = para_process_buf_insert(proto->para_buf_q);
					if(ret != OK){
						printf("############### Can update q insert err! line:%d\n",__LINE__);
						can->is_thread_alive = 0;
						return ret;
					}
					
					ack_update.id = _create_can_id(0,addr,0,0,0,1);
					ack_data[0] = 0x80 + 0x32;
					ack_data[1] = 'o';
					ack_update.data = ack_data;
					ack_update.can_dlc = 2;

					ret = can->can.send(&can->can,&ack_update);
					if(ret != OK){
						printf("############### can send err!\n");
						can->is_thread_alive = 0;
						return ERR;
					}
				}
			}

		}else if(flag == 0 && attr == 1){
_get_recv_h_node1:
			h_node = (struct recv_h_node *)get_q_buf(can->h_elem_q);
			if(h_node == NULL){
				usleep(100);
				goto _get_recv_h_node1;
			}

			h_node->ack = ack;
			h_node->addr = addr;
			gettimeofday(&h_node->tv_now,NULL);
			INIT_LIST_HEAD(&h_node->packet_head);
_get_recv_packet_node1:
			packet_node = (struct recv_packet_node *)get_q_buf(can->packet_elem_q);
			if(packet_node == NULL){
				usleep(100);
				goto _get_recv_packet_node1;
			}

			packet_node->dlc = frame.can_dlc;
			packet_node->index = index;
			memcpy(packet_node->data,frame.data,packet_node->dlc);

			list_add_tail(&packet_node->pack_list,&h_node->packet_head);
			
			if(ack == 1)
				recv_hlist_add(can->recv_head_array_ack,h_node);
			else
				recv_hlist_add(can->recv_head_array_req,h_node);

		}else if(flag == 1 && attr == 0){
_get_recv_packet_node2:
			packet_node = (struct recv_packet_node *)get_q_buf(can->packet_elem_q);
			if(packet_node == NULL){
				usleep(100);
				goto _get_recv_packet_node2;
			}

			packet_node->dlc = frame.can_dlc;
			packet_node->index = index;
			memcpy(packet_node->data,frame.data,packet_node->dlc);

			list_add_tail(&packet_node->pack_list,&h_node->packet_head);

			if(ack == 1){
				IM_LOG("%s:%d:      addr:%d \n",__FUNCTION__,__LINE__,addr);
				buf = get_free_para_buf(can->answer_q);
				if(buf != NULL){
					*(unsigned short *)buf = h_node->addr;
					buf[2] = 0;
					list_for_each(pos_h,&h_node->packet_head){
						packet_node = list_entry(pos_h,struct recv_packet_node,pack_list);
						memcpy(buf + 3 + buf[2],packet_node->data,packet_node->dlc);
						buf[2] += packet_node->dlc;

						__list_del(packet_node->pack_list.prev,packet_node->pack_list.next);

						q_buf_insert(can->packet_elem_q,(unsigned char *)packet_node);
					}
					hlist_del(&h_node->node);
					q_buf_insert(can->h_elem_q,(unsigned char*)h_node);
					
					para_process_buf_insert(can->answer_q);
				}
			}else{
					buf = get_free_para_buf(proto->para_buf_q);
					if(buf != NULL){
						ret_d = (struct return_data *)buf;
						ret_d->len = 0;
						list_for_each(pos_h,&h_node->packet_head){
							packet_node = list_entry(pos_h,struct recv_packet_node,pack_list);
							memcpy(buf + sizeof(struct return_data) + ret_d->len,packet_node->data,packet_node->dlc);
							ret_d->len += packet_node->dlc;

							__list_del(packet_node->pack_list.prev,packet_node->pack_list.next);
							
							q_buf_insert(can->packet_elem_q,(unsigned char *)packet_node);
						}

						ret_d->type = (char)(*(buf + sizeof(struct return_data)));
						ret_d->id = addr;
						hlist_del(&h_node->node);
						q_buf_insert(can->h_elem_q,(unsigned char*)h_node);

						para_process_buf_insert(proto->para_buf_q);
					}
			}

		}else if(flag == 1 && attr == 1){

_get_recv_packet_node3:
			packet_node = (struct recv_packet_node *)get_q_buf(can->packet_elem_q);
			if(packet_node == NULL){
				usleep(100);
				goto _get_recv_packet_node3;
			}

			packet_node->dlc = frame.can_dlc;
			packet_node->index = index;
			memcpy(packet_node->data,frame.data,packet_node->dlc);

			list_add_tail(&packet_node->pack_list,&h_node->packet_head);

			gettimeofday(&h_node->tv_now,NULL);
		}
	
		if(can->is_thread_alive == 0){
			IM_LOG("************** %s is exit!\n",__FUNCTION__);
			return OK;
		}
		usleep(10);
	}
	return OK;
}


static int can_send_broadcast(void *comx_array,unsigned int com_no)
{
	int ret,i,fd;
	unsigned int count;
	unsigned char attr,index;
	can_data_t  can_data;
	struct can_comx *can;
	dev_com_t *dev_com_array;
	struct can_comx *can_array;

	if(comx_array == NULL){
		return ERR_PARA;
	}


	dev_com_array = (dev_com_t *)comx_array;
	can_array = (struct can_comx *)dev_com_array[com_no].spec_com;
	
	can = &can_array[com_no];
	can->send_frame = dev_com_array[com_no].send_frame;
	can->send_len = dev_com_array[com_no].send_len;
	
	fd = can->can.socket;
	if(fd < 0){
		printf("can is not opened!\n");
		return ERR;
	}

	count = 0;
	for(i=0;i<can->send_len;i++)
		IM_LOG("%02x ",can->send_frame[i]);
	IM_LOG("\n");

//	IM_LOG("%s:%d:     \n",__FUNCTION__,__LINE__);

	index = 0;
	
	for(i = can->send_len;i > 0;){
		if(i > 8){
			can_data.can_dlc = 8;
			attr = 1;
			i -= 8;
		}else{
			can_data.can_dlc = i;
			attr = 0;
			i = 0;
			//count += i;
		}
		
		index += 1;
		can_data.id = _create_can_id(1,0xffff,0,attr,index - 1,0);       //brocadcast frame
		can_data.data = can->send_frame + count;
		count += can_data.can_dlc;

		ret = can->can.send(&can->can,&can_data);
		if(ret != OK){
			printf("can send err!\n");
			return ERR;
		}

	}
	
	

	return OK;
}



static int can_send_and_recv(void *comx_array,unsigned int com_no,unsigned char *recv_frame,unsigned int *recv_len )
{
	int ret,select_ret;
	int i;
	int fd;
	fd_set rfds;
	unsigned char re_trans_times;
	unsigned int time_us;
	//char *str_ret;
	//struct timeval tv;

	struct  timeval    tv0;
        struct  timezone   tz0;

	struct  timeval    tv_n;
        struct  timezone   tz_n;
		
	struct can_comx *can;
	can_data_t  can_data;
	//int sret;
	unsigned int readlen = 0,count;
	char * ptr;
	*recv_len = 0;
	//unsigned int timeout;
	//unsigned char recv_buf[300];
	unsigned char recv_data_len;
	dev_com_t *dev_com_array;
	struct can_comx *can_array;
	struct esl_attribute_node *EslNode;
	unsigned char *ack_frame;

	unsigned short addr,esl_addr;
	unsigned char attr;
	unsigned char index;

	if(recv_frame == NULL || comx_array == NULL){
		return ERR_PARA;
	}
	
	re_trans_times = 0;

	dev_com_array = (dev_com_t *)comx_array;
	can_array = (struct can_comx *)dev_com_array[com_no].spec_com;
	EslNode = dev_com_array[com_no].EslNode;
	
	//timeout = 1050; //10S
	can = &can_array[com_no];
	can->send_frame = dev_com_array[com_no].send_frame;
	can->send_len = dev_com_array[com_no].send_len;
	fd = can->can.socket;
	_DBG("+++++---- can fd:%d   com_no:%d\n",fd,com_no);
//	FD_ZERO(&rfds);
//	FD_SET(fd,&rfds);
	//tv.tv_sec  = timeout / 1000;
	//tv.tv_usec = (timeout%1000)*1000;
	
	
	if(fd < 0){
		printf("can is not opened!\n");
		return ERR;
	}

	//memset(recv_buf,0,300);
can_send_frame:
	count = 0;
	_DBG("\n\n\n");
	for(i=0;i<can->send_len;i++)
		_DBG("%02x ",can->send_frame[i]);
	_DBG("\n");

	addr = EslNode->id;
	IM_LOG("%s:%d:      addr:%d \n",__FUNCTION__,__LINE__,addr);

	index = 0;
	for(i = can->send_len;i > 0;){
		if(i > 8){
			can_data.can_dlc = 8;
			attr = 1;
			i -= 8;
		}else{
			can_data.can_dlc = i;
			attr = 0;
			i = 0;
			//count += i;
		}
		
		index += 1;
		can_data.id = _create_can_id(0,addr,0,attr,index - 1,0);
		can_data.data = can->send_frame + count;
		count += can_data.can_dlc;

		ret = can->can.send(&can->can,&can_data);
		if(ret != OK){
			printf("can send err!\n");
			return ERR;
		}

	}
	

	gettimeofday(&tv0,&tz0);

	while(1){
		ack_frame = get_process_para_buf(can->answer_q);
		if(ack_frame == NULL){
			gettimeofday(&tv_n,&tz_n);
			time_us = 1000000 * (tv_n.tv_sec - tv0.tv_sec) + tv_n.tv_usec - tv0.tv_usec;
			if(time_us > TIME_OUT_UMS){
				printf("Can ack frame recv time out!!line:%d\n",__LINE__);
				//return OK;
				re_trans_times += 1;
				if(re_trans_times > 2){
					printf("___#___#_____esl:%d is not ack!\n\n",addr);
					//return OK;
					return ERR_DELAY;
				}
				goto can_send_frame;
			}else{
				continue;
			}
		}


check_addr:	
		esl_addr = *(unsigned short *)ack_frame;
		if(esl_addr != addr ){
			para_process_buf_delete(can->answer_q);
			printf("return err! addr:%d   esl_addr:%d\n",addr,esl_addr);
			ack_frame = get_process_para_buf(can->answer_q);
			if(ack_frame == NULL){
				printf("%s:%d    frame buf empty!\n",__FUNCTION__,__LINE__);
				//return OK;
				goto can_send_frame;
			}else{
				goto check_addr;//return OK;
			}
		}

		

		_DBG("&--------- %s:%d   This msg is a ack frame for me!\n",__FUNCTION__,__LINE__);
		*recv_len = ack_frame[2];
		memcpy(recv_frame,ack_frame + 3,*recv_len);
		
		para_process_buf_delete(can->answer_q);

		return OK;
	}

	return OK;
}

//extern aioi_proto_t g_aioi_proto;

static int can_add_up_data_item(void *comx,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr)
{
	int ret;
	struct can_comx *can;
	struct event_params *params,*paras;

	can_comx_t *commx_array;
	commx_array = can_comx_array;
	char para[10];
	unsigned char send_frame_len;
	unsigned char send_frame[100];
	unsigned char recv_frame[50];
	unsigned char recv_len;
	unsigned char cmd_analysis_para_buf[50];
	aioi_proto_t *proto;
	int cmd_analysis_ret;

	if(comx == NULL || EslNode == NULL){
		return ERR_PARA;
	}

	can = (struct can_comx *)comx;

	memset(para,0,10);
	memset(send_frame,0,100);
	memset(recv_frame,0,50);

	proto = &g_aioi_proto;
	para[0] = 0;
	// TODO: Send upload command, Attached data type;

	switch(event_type){
		case E_GET_KEY_STATUS:
			// spec. cmd
			break;
		case E_GET_SCAN_DATA:
			// spec. cmd
			ret = proto->esl_proto->cmd_build(EslNode->id,CMD_SCAN_SWITCH,para,1,send_frame,&send_frame_len);
			break;
		default:
			return ERR;
	}


	proto->commx_array[EslNode->com_no].EslNode = EslNode;
	proto->commx_array[EslNode->com_no].send_frame = send_frame;
	proto->commx_array[EslNode->com_no].send_len = send_frame_len;
	
	
	pthread_mutex_lock(&proto->commx_array[EslNode->com_no].mutex);
	proto->commx_array[EslNode->com_no].send_and_recv(proto->commx_array,EslNode->com_no,recv_frame,&recv_len);
	pthread_mutex_unlock(&proto->commx_array[EslNode->com_no].mutex);
	
	
	usleep(100);
	if(recv_len > 0){
			//memcpy(send_buf,proto->commx_array[i].recv_frame,proto->commx_array[i].recv_len);
			//*send_len = proto->commx_array[i].recv_len;
	}else{
		printf("%s: esl:%d not ack!\n",__FUNCTION__,EslNode->id);
	}
#ifdef __CMD_RET_ANALYSIS
	ret = proto->esl_proto->cmd_analysis(recv_frame,recv_len,&cmd_analysis_ret,cmd_analysis_para_buf);
	if(ret != OK){
		printf("%s: cmd analysis err!\n",__FUNCTION__);
	}
	// TODO: Cmd analysis return handle;
#endif

	return OK;
}




static int can_del_up_data_item(void *comx,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr)
{
	int ret;
	struct can_comx *can;
	struct event_params *params,*paras;
	can_comx_t *commx_array;
	commx_array = can_comx_array;
	char para[10];
	unsigned char send_frame_len;
	unsigned char send_frame[100];
	unsigned char recv_frame[50];
	unsigned char recv_len;
	unsigned char cmd_analysis_para_buf[50];
	aioi_proto_t *proto;
	int cmd_analysis_ret;
	

	if(comx == NULL || EslNode == NULL){
		return ERR_PARA;
	}

	can = (struct can_comx *)comx;

	memset(para,0,10);
	memset(send_frame,0,100);
	memset(recv_frame,0,50);


	proto = &g_aioi_proto;
	
	para[0] = 1;

	switch(event_type){
		case E_GET_KEY_STATUS:
			// spec. cmd
			break;
		case E_GET_SCAN_DATA:
			// spec. cmd
			ret =  proto->esl_proto->cmd_build(EslNode->id,CMD_SCAN_SWITCH,para,1,send_frame,&send_frame_len);
			break;
		default:
			return ERR;
	}


	proto->commx_array[EslNode->com_no].EslNode = EslNode;
	proto->commx_array[EslNode->com_no].send_frame = send_frame;
	proto->commx_array[EslNode->com_no].send_len = send_frame_len;
	
	
	pthread_mutex_lock(&proto->commx_array[EslNode->com_no].mutex);
	proto->commx_array[EslNode->com_no].send_and_recv(proto->commx_array,EslNode->com_no,recv_frame,&recv_len);
	pthread_mutex_unlock(&proto->commx_array[EslNode->com_no].mutex);
	
	
	usleep(100);
	if(recv_len > 0){
			//memcpy(send_buf,proto->commx_array[i].recv_frame,proto->commx_array[i].recv_len);
			//*send_len = proto->commx_array[i].recv_len;
	}else{
		printf("%s: esl:%d not ack!\n",__FUNCTION__,EslNode->id);
	}
#ifdef __CMD_RET_ANALYSIS
	ret = proto->esl_proto->cmd_analysis(recv_frame,recv_len,&cmd_analysis_ret,cmd_analysis_para_buf);
	if(ret != OK){
		printf("%s: cmd analysis err!\n",__FUNCTION__);
	}
	// TODO: Cmd analysis return handle;
#endif


	return OK;
}



PARA_BUF_QUEUE_T can_answer_q;
struct recv_hlist_head recv_head_array_ack[H_HEAD_ARRY_SIZE];
struct recv_hlist_head recv_head_array_req[H_HEAD_ARRY_SIZE];


static int can_init(struct can_comx *comx,unsigned int no)
{
	struct recv_hlist_head *pos;
	unsigned char *para_buf;
	int i,ret;

	if(comx == NULL)
		return ERR_PARA;

	comx->can.init = can_bus_init;
	comx->send_broadcast = can_send_broadcast;
	comx->send_and_recv = can_send_and_recv;
	comx->add_up_data_item = can_add_up_data_item;
	comx->del_up_data_item = can_del_up_data_item;

/************ buf queue ***************/
	comx->h_elem_q = _buf_q_init(H_ELEM_Q_SIZE,sizeof(struct recv_h_node));
	if(comx->h_elem_q == NULL){
		printf("%s:%d   h_elem_q create err!\n",__FUNCTION__,__LINE__);
		return ERR;
	}

	comx->packet_elem_q = _buf_q_init(PACKET_ELEM_Q_SIZE,sizeof(struct recv_packet_node));
	if(comx->packet_elem_q == NULL){
		printf("%s:%d   packet_elem_q create err!\n",__FUNCTION__,__LINE__);
		return ERR;
	}



/************ recv hlist *************/
//recv head array ack
	comx->recv_head_array_ack = recv_head_array_ack;
	comx->recv_head_array_ack_len = H_HEAD_ARRY_SIZE;

	pos = comx->recv_head_array_ack;
	for(i = 0;i < comx->recv_head_array_ack_len;i ++){
		INIT_HLIST_HEAD(&pos->head);
		pos ++;
	}

//recv head array req
	comx->recv_head_array_req = recv_head_array_req;
	comx->recv_head_array_req_len = H_HEAD_ARRY_SIZE;

	pos = comx->recv_head_array_req;
	for(i = 0;i < comx->recv_head_array_req_len;i ++){
		INIT_HLIST_HEAD(&pos->head);
		pos ++;
	}
	
	
	ret = pthread_mutex_init(&comx->mutex,NULL);
	if(ret != OK){
		printf("comx->mutex init err! line:%d\n",__LINE__);
		return ERR;
	}

	comx->can.init(&comx->can,no);
	INIT_LIST_HEAD(&comx->frame_list);

	comx->answer_q = &can_answer_q;
	comx->answer_q->front = 0;
	comx->answer_q->rear = 0;
	
	for(i = 0;i < FRAME_BUF_Q_SIZE;i ++){
		para_buf = (unsigned char *)malloc(PARA_BUF_SIZE);
		if(para_buf == NULL){
			printf("para buf malloc err!\n");
			return ERR;
		}
		memset(para_buf,0,PARA_BUF_SIZE);
		comx->answer_q->cmd_para[i] = para_buf;
	}

	
	comx->is_thread_alive = 1;
	_DBG("++++++ %s:%d \n",__FUNCTION__,__LINE__);
	comx->thread = SDL_CreateThread(can_thread_func, comx);
	if(comx->thread == NULL){
		printf("__________________thread create err! comx->thread:%d\n",comx->thread);
		return ERR;
	}

#ifdef __RECV_H
	comx->is_independent_thread_alive = 1;
	comx->independent_thread = SDL_CreateThread(can_independent_thread_func, comx);
	if(comx->independent_thread == NULL){
		printf("__________________ independent_thread create err! comx->thread:%d\n",comx->independent_thread);
		return ERR;
	}
#endif
}

static int can_deinit(struct can_comx *comx,unsigned int no)
{
	int i;

	if(comx == NULL)
		return ERR_PARA;

	comx->is_thread_alive = 0;
	SDL_WaitThread(comx->thread,0);

#ifdef __RECV_H
	comx->is_independent_thread_alive = 0;
	SDL_WaitThread(comx->independent_thread,0);
#endif
	
	for(i = 0;i < FRAME_BUF_Q_SIZE;i ++){
		
		if(comx->answer_q->cmd_para[i] != NULL){
			free(comx->answer_q->cmd_para[i]);
		}
	}
	

	_buf_q_deinit(comx->packet_elem_q);
	_buf_q_deinit(comx->h_elem_q);

	comx->can.deinit(&comx->can);

	return OK;
}




int can_comx_array_init(unsigned int port_num)
{
	int i;
	int ret;
	//unsigned int port_num;
	can_comx_t *can_array;
	
	can_array = can_comx_array;
	memset(can_array,0,sizeof(can_comx_t) * CAN_PORT_NUM);

	//ret = cfg_get_dev_num_can(&port_num);
	//if(ret < 0){
	//	return ret;
	//}

	for(i = 0;i < port_num;i ++){
		(can_array + i)->init = can_init;
		(can_array + i)->deinit = can_deinit;
		(can_array + i)->init(can_array + i,i);
	}

	return OK;
}


int can_comx_array_deinit(unsigned int port_num)
{
	int i,ret;


	for(i = 0;i < port_num;i ++){
		can_deinit(can_comx_array + i,i);
	}

	return OK;
}


#ifdef   __cplusplus
	}
#endif



