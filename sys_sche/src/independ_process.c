/******************************************************************************
                                                                            
 File Name:      independ_process.c                                             
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com				
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20140709		Kevin.Zu		Create 

*******************************************************************************/
#include "independ_process.h"
#include "protocol.h"
#include "sub_socket_comm.h"

extern independ_process_t g_independ_process;
extern aioi_proto_t g_aioi_proto;
extern sub_socket_comm_t g_sub_socket;

static int self_test_process(void *para)
{
	int i,ret,j;
	struct esl_attribute_node *EslNode;
	struct hlist_node *attr_pos;
	independ_process_t *iprocess = &g_independ_process;
	aioi_proto_t *proto = &g_aioi_proto;
	char para_buf[100];
	unsigned int ParaLen = 0;
	unsigned char *send_buf;
	unsigned char send_frame[UART_SEND_BUF_LEN];
	unsigned int send_frame_len;
	unsigned char recv_frame[UART_RECV_BUF_LEN];
	unsigned int recv_len;
	short esl_id;
	sub_socket_comm_t *sub_sock;

	sub_sock = &g_sub_socket;

	memset(para_buf,0,100);
	memset(send_frame,0,UART_SEND_BUF_LEN);
	memset(recv_frame,0,UART_RECV_BUF_LEN);
	

	for(i = 0;i < iprocess->esl_attr->head_array_len;i ++){
		hlist_for_each(attr_pos,&(iprocess->esl_attr->head_array)->head + i){
			EslNode = list_entry(attr_pos,struct esl_attribute_node,node);//(ptr, type, member)
			esl_id = EslNode->id;
			
			ret = proto->esl_proto->cmd_build(EslNode->id,CMD_SELF_TEST,para_buf,ParaLen,send_frame,&send_frame_len);

			if(ret != OK){
				printf("%s: cmd build err!\n",__FUNCTION__);
				return ret;
			}

			for(j= 0;j < send_frame_len;j ++){
				printf("%02hhx ",send_frame[j]);
			}
			

			proto->commx_array[EslNode->com_no].EslNode = EslNode;
			proto->commx_array[EslNode->com_no].send_frame = send_frame;
			proto->commx_array[EslNode->com_no].send_len = send_frame_len;
			_DBG("&++ line:%d\n",__LINE__);
			pthread_mutex_lock(&proto->commx_array[EslNode->com_no].mutex);
			ret = proto->commx_array[EslNode->com_no].send_and_recv(proto->commx_array,EslNode->com_no,recv_frame,&recv_len);
			pthread_mutex_unlock(&proto->commx_array[EslNode->com_no].mutex);
			usleep(100);
			if(ret == ERR_DELAY){
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
			}else{
				printf("%s: esl:%d ret:%d!\n",__FUNCTION__,EslNode->id,ret);
			}
		}
		
	}

	send_buf = get_free_para_buf(sub_sock->send_frame_q);
	if(send_buf != NULL){
		memset(send_buf,0,PARA_BUF_SIZE);
		send_buf[1] = 'e';
		send_buf[5] = 0x39;
		send_buf[4] = 0x39;
		send_buf[3] = 0x39;
		send_buf[2] = 0x39;
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

	return OK;
}

static int iprocess_create(struct independ_process *iprocess,unsigned int id,void *para)
{
	struct process_fun_info *func_info;
	
	if(id >= iprocess->func_array_len){
		printf("%s:%d err independ_process id\n",__FUNCTION__,__LINE__);
		return ERR;
	}

	func_info = iprocess->func_info_array + id;
	
	
	func_info->thread = SDL_CreateThread(func_info->func,para);
	if(func_info->thread == NULL){
		printf("%s: __thread create err! iprocess->thread:%d\n",__FUNCTION__,func_info->thread);
		return ERR;
	}
}


struct process_fun_info func_info_array[] = {
	{self_test_process,NULL}
};

extern esl_attr_hlist_t g_esl_attr_hlist;
independ_process_t g_independ_process = {
	.func_array_len = sizeof(func_info_array)/sizeof(struct process_fun_info),
	.func_info_array = func_info_array,
	.esl_attr = &g_esl_attr_hlist,
	.create_process = iprocess_create
};

