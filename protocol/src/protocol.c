/******************************************************************************
                                                                            
 File Name:      protocol.c                                             
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
#include "sys.h"
#include "protocol.h"
#include "can_comx.h"
#include "sub_socket_comm.h"
#include "sys_manager.h"
#include "independ_process.h"

extern aioi_proto_t g_aioi_proto;
extern sys_manager_t g_sys_manager;
extern independ_process_t g_independ_process;


unsigned char get_check_sum(unsigned char *f,unsigned int len)
{
	unsigned char check_sum;
#if 1
	unsigned char i = 0;

	for (i = 0;i < len; i ++){
		check_sum += *(f + i);
	}

	check_sum = 0xff - check_sum + 0x01;

#else
	check_sum = 0;
#endif
	return check_sum;
}


static unsigned int get_offset(const char *buf,char in)
{
	unsigned int i;

	for(i = 0;i < RECV_FRAME_BUF_LEN;i ++){
		if(*(buf + i) == in)
			break;
	}
	
	if(i >= RECV_FRAME_BUF_LEN)
		i = 0;
	
	return i;
}

static int cmd_Z_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	int ret;
	unsigned int i;
	aioi_proto_t *proto;
	unsigned char cmd_analysis_para_buf[UART_SEND_BUF_LEN ];
	unsigned char send_frame[UART_SEND_BUF_LEN];
	unsigned int send_frame_len;
	unsigned char recv_frame[UART_RECV_BUF_LEN];
	unsigned int recv_len;
	struct hlist_node *pos;
	struct hlist_head *hhead;
	struct esl_attribute_node *EslNode;
	int cmd_analysis_ret;
	unsigned short esl_addr;
	
	if(ack_buf == NULL)
		return ERR_PARA;

	memset(cmd_analysis_para_buf,0,UART_SEND_BUF_LEN);
	memset(send_frame,0,UART_SEND_BUF_LEN);
	memset(recv_frame,0,UART_RECV_BUF_LEN);

	proto = &g_aioi_proto;
	ack_buf[0] = 'O';
	ack_buf[1] = get_check_sum(ack_buf,1);
	ack_buf[2] = 0x0d;

	*ack_len = 3;

	_DBG("Z command!\n");

	_DBG("#### %s:   para:%s para_len:%d\n",__FUNCTION__,para,para_len);
	if(para_len > 4 ){
		switch(para[1]){
			case '1':
				esl_addr += (para[2] - 0x30) * 1000;
				esl_addr += (para[3] - 0x30) * 100;
				esl_addr += (para[4] - 0x30) * 10;
				esl_addr += (para[5] - 0x30); 

				//ret = system_reset_can(esl_addr,CMD_SYS_RESET,NULL,0,send_frame,&send_frame_len);
				
				ret = proto->esl_proto->cmd_build(esl_addr,CMD_SYS_RESET,NULL,0,send_frame,&send_frame_len);
	   			if(ret != OK){
	   				printf("%s: cmd build err!\n",__FUNCTION__);
	   				return ret;
	   			}

				
				EslNode = proto->esl_attr->get_node(proto->esl_attr,esl_addr);
				if(EslNode == NULL)
					return ERR;

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
	   			ret = proto->esl_proto->cmd_analysis(recv_frame,recv_len,&cmd_analysis_ret,cmd_analysis_para_buf);
	   			if(ret != OK){
	   				printf("%s: cmd analysis err!\n",__FUNCTION__);
	   			}
	   			// TODO: Cmd analysis return handle;
				break;
			default:
				break;
		}
	}else{
	   	for(i = 0;i < ESL_ATTR_HEAD_SIZE;i ++){
	   		hhead = &proto->esl_attr->head_array[i].head;
	   		hlist_for_each(pos,hhead){
	   			EslNode = container_of(pos,struct esl_attribute_node,node);
	   			ret = proto->esl_proto->cmd_build(EslNode->id,CMD_SYS_RESET,NULL,0,send_frame,&send_frame_len);
	   			if(ret != OK){
	   				printf("%s: cmd build err!\n",__FUNCTION__);
	   				return ret;
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
	   			ret = proto->esl_proto->cmd_analysis(recv_frame,recv_len,&cmd_analysis_ret,cmd_analysis_para_buf);
	   			if(ret != OK){
	   				printf("%s: cmd analysis err!\n",__FUNCTION__);
	   			}
	   			// TODO: Cmd analysis return handle;
	   		}
	   	}
	}
	return OK;
}


static int cmd_P_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	int ret;
	unsigned int j,i,offset;
	aioi_proto_t *proto;
	unsigned short id;
	unsigned char cmd_analysis_para_buf[UART_SEND_BUF_LEN ];
	char para_buf[100];
	unsigned int ParaLen = 0,count = 0;
	unsigned char send_frame[UART_SEND_BUF_LEN];
	unsigned int send_frame_len;
	unsigned char recv_frame[UART_RECV_BUF_LEN];
	unsigned int recv_len;
	struct event_params *params,*paras;
	struct esl_attribute_node *EslNode;
	int cmd_analysis_ret;
	char disp_type;
	EVENT_ELEM_T *event;
	
	proto = &g_aioi_proto;
	
	if(ack_buf == NULL)
		return ERR_PARA;
	memset(para_buf,0,100);
	memset(send_frame,0,UART_SEND_BUF_LEN);
	memset(recv_frame,0,UART_RECV_BUF_LEN);
	memset(cmd_analysis_para_buf,0,UART_SEND_BUF_LEN);

	ack_buf[0] = 'O';
	ack_buf[1] = get_check_sum(ack_buf,1);
	ack_buf[2] = 0x0d;

	*ack_len = 3;

	_DBG("P command!\n");
	offset = get_offset(para,0x0d);
	_DBG("offset:%d\n",i);
	
	switch(para[1]){
		case '1':
			for(i = 4;i < offset;i += 9){
				id = (para[i] - 0x30)*1000 + (para[i + 1] - 0x30)*100 + (para[i + 2] - 0x30)*10 + (para[i + 3] - 0x30);
				count = 0;
				for(ParaLen = 0;;ParaLen += 1){
					if(para[i + count + 4] == ' '){
						break;
					}
					para_buf[ParaLen] = para[i + count + 4];
					count += 1;
					if(count == 4 ){
						if(para[i + count + 4] == '-'){
							i += 5;//i += 9;
							count = 0;
						}else{
							ParaLen += 1;
							para_buf[ParaLen] = para[i + count + 4];
							ParaLen += 1;
							break;
						}
					}
				}
				
				EslNode = proto->esl_attr->get_node(proto->esl_attr,id);
				if(EslNode == NULL)
					return ERR;

				disp_type = para_buf[0] - 0x30;
				ret = proto->esl_proto->cmd_build(id,CMD_UPDATE_DISPLAY,para_buf,ParaLen,send_frame,&send_frame_len);

				if(ret != OK){
					printf("%s: cmd build err!\n",__FUNCTION__);
					return ret;
				}
				
				_DBG("&++ line:%d\n",__LINE__);
				proto->commx_array[EslNode->com_no].EslNode = EslNode;
				proto->commx_array[EslNode->com_no].send_frame = send_frame;
				proto->commx_array[EslNode->com_no].send_len = send_frame_len;
				_DBG("&++ line:%d\n",__LINE__);
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
				_DBG("&++ line:%d\n",__LINE__);
				ret = proto->esl_proto->cmd_analysis(recv_frame,recv_len,&cmd_analysis_ret,cmd_analysis_para_buf);
				if(ret != OK){
					printf("%s: cmd analysis err!\n",__FUNCTION__);
				}
				if(cmd_analysis_ret == OK){
					if(disp_type == 3){
						proto->commx_array[EslNode->com_no].add_up_data_item(&proto->commx_array[EslNode->com_no],EslNode,E_GET_KEY_STATUS,id);
					}
				}
			}
			break;
			
		case 'n':  //nova
			_DBG("____ cmd Pn!\n");
#if 0
			for(i = 4;i < offset;i += NOVA_ESL_DISP_LEN){
				count = 0;
#else
			for(i = 4;i < offset - 2;){
#endif

				id = (para[i] - 0x30)*1000 + (para[i + 1] - 0x30)*100 + (para[i + 2] - 0x30)*10 + (para[i + 3] - 0x30);
#if 0
				for(ParaLen = 0;;ParaLen += 1){
					if(para[i + count + 4] == ' '){
						break;
					}
					para_buf[ParaLen] = para[i + count + 4];
					count += 1;
					if(count == NOVA_DISP_LEN - 1){
						if(para[i + count + 4] == '-'){
							i += NOVA_DISP_LEN;//5;//i += 9;
							count = 0;
						}else{
							ParaLen += 1;
							para_buf[ParaLen] = para[i + count + NOVA_DISP_LEN - 1];
							ParaLen += 1;
							break;
						}
					}
				}

				
				
				EslNode = proto->esl_attr->get_node(proto->esl_attr,id);
				if(EslNode == NULL)
					return ERR;

				_DBG("********** DISP data:\n");
				for(j = 0;j < ParaLen;j ++){
					_DBG("%02hhx ",para_buf[j]);
				}
				_DBG("\n***************\n");

				disp_type = para_buf[0] - 0x30;
				ret = proto->esl_proto->cmd_build(id,CMD_UPDATE_DISPLAY,para_buf,ParaLen,send_frame,&send_frame_len);

				if(ret != OK){
					printf("%s: cmd build err!\n",__FUNCTION__);
					return ret;
				}

				_DBG("####### send data:\n");
				for(j= 0;j < send_frame_len;j ++){
					printf("%02hhx ",send_frame[j]);
				}
				_DBG("\n***************\n");
				
				//printf("&++ line:%d\n",__LINE__);

				proto->commx_array[EslNode->com_no].EslNode = EslNode;
				proto->commx_array[EslNode->com_no].send_frame = send_frame;
				proto->commx_array[EslNode->com_no].send_len = send_frame_len;
				_DBG("&++ line:%d\n",__LINE__);
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
				_DBG("&++ line:%d\n",__LINE__);
				ret = proto->esl_proto->cmd_analysis(recv_frame,recv_len,&cmd_analysis_ret,cmd_analysis_para_buf);
				if(ret != OK){
					printf("%s: cmd analysis err!\n",__FUNCTION__);
				}
				if(cmd_analysis_ret == OK){
					if(disp_type == 3){
						proto->commx_array[EslNode->com_no].add_up_data_item(&proto->commx_array[EslNode->com_no],EslNode,E_GET_KEY_STATUS,id);
					}
				}
			}
#else 
				i += 4;

				for(ParaLen = 0;;ParaLen += 1){
					if(para[i] == EDATA_END_FLAG){
						i += 1;
						break;
					}
					para_buf[ParaLen] = para[i];
					i += 1;
				}


				
				EslNode = proto->esl_attr->get_node(proto->esl_attr,id);
				if(EslNode == NULL)
					return ERR;

				_DBG("********** DISP data:\n");
				for(j = 0;j < ParaLen;j ++){
					_DBG("%02hhx ",para_buf[j]);
				}
				_DBG("\n***************\n");

				disp_type = para_buf[0] - 0x30;
				ret = proto->esl_proto->cmd_build(id,CMD_UPDATE_DISPLAY,para_buf,ParaLen,send_frame,&send_frame_len);

				if(ret != OK){
					printf("%s: cmd build err!\n",__FUNCTION__);
					return ret;
				}

				_DBG("####### send data:\n");
				for(j= 0;j < send_frame_len;j ++){
					printf("%02hhx ",send_frame[j]);
				}
				_DBG("\n***************\n");
				
				//printf("&++ line:%d\n",__LINE__);

				proto->commx_array[EslNode->com_no].EslNode = EslNode;
				proto->commx_array[EslNode->com_no].send_frame = send_frame;
				proto->commx_array[EslNode->com_no].send_len = send_frame_len;
				_DBG("&++ line:%d\n",__LINE__);
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
				_DBG("&++ line:%d\n",__LINE__);
				ret = proto->esl_proto->cmd_analysis(recv_frame,recv_len,&cmd_analysis_ret,cmd_analysis_para_buf);
				if(ret != OK){
					printf("%s: cmd analysis err!\n",__FUNCTION__);
				}
				if(cmd_analysis_ret == OK){
					if(disp_type == 3){
						proto->commx_array[EslNode->com_no].add_up_data_item(&proto->commx_array[EslNode->com_no],EslNode,E_GET_KEY_STATUS,id);
					}
				}
			}

#endif
			break;
		default:
			printf("no this sub para!\n");
	}

	return OK;
}


static int cmd_G_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	unsigned short scan_esl_addr = 0;
	int ret;
	unsigned int i;
	EVENT_ELEM_T *epos;
	aioi_proto_t *proto;
	struct hlist_node *pos;
	struct hlist_head *hhead;
	struct event_params *params,*paras;
	struct esl_attribute_node *EslNode;
	char is_open_scan;
	EVENT_ELEM_T *event;

	
	if(ack_buf == NULL)
		return ERR_PARA;

	proto = &g_aioi_proto;
	
	scan_esl_addr += (para[1] - 0x30) * 1000;
	scan_esl_addr += (para[2] - 0x30) * 100;
	scan_esl_addr += (para[3] - 0x30) * 10;
	scan_esl_addr += (para[4] - 0x30);

	EslNode = proto->esl_attr->get_node(proto->esl_attr,scan_esl_addr);
	if(EslNode != NULL){
		if(EslNode->type != SCAN_ESL_TYPE){
			printf("esl type err!no scan esl!\n");
			ack_buf[0] = 'N';
			ack_buf[1] = get_check_sum(ack_buf,1);
			ack_buf[2] = 0x0d;

			*ack_len = 3;
		}else{
			is_open_scan = para[5];
			if(is_open_scan == 'O' || is_open_scan == 'o'){
				ret = proto->commx_array[EslNode->com_no].add_up_data_item(&proto->commx_array[EslNode->com_no],EslNode,E_GET_SCAN_DATA,scan_esl_addr);
				if(ret != OK){
					ack_buf[0] = 'N';
					ack_buf[1] = get_check_sum(ack_buf,1);
					ack_buf[2] = 0x0d;
					*ack_len = 3;

					return OK;
				}else{
					ack_buf[0] = 'O';
					ack_buf[1] = get_check_sum(ack_buf,1);
					ack_buf[2] = 0x0d;

					*ack_len = 3;
				}
			}else if(is_open_scan == 'C' || is_open_scan == 'c'){

				proto->commx_array[EslNode->com_no].del_up_data_item(&proto->commx_array[EslNode->com_no],EslNode,E_GET_SCAN_DATA,scan_esl_addr);
			}else{
				printf("no open/close scan cmd!err cmd flag!\n");
				ack_buf[0] = 'N';
				ack_buf[1] = get_check_sum(ack_buf,1);
				ack_buf[2] = 0x0d;

				*ack_len = 3;
				return OK;
			}
			ack_buf[0] = 'O';
			ack_buf[1] = get_check_sum(ack_buf,1);
			ack_buf[2] = 0x0d;

			*ack_len = 3;
		}
	}else{
		printf("scan esl id err!\n");
		ack_buf[0] = 'N';
		ack_buf[1] = get_check_sum(ack_buf,1);
		ack_buf[2] = 0x0d;

		*ack_len = 3;
	}
	return OK;
}

//-------------------------
//          indicator lamp
//-------------------------
static int cmd_T_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	unsigned short scan_esl_addr = 0;
	int ret;
	unsigned int i;
	aioi_proto_t *proto;
	//struct hlist_node *pos;
	//struct hlist_head *hhead;
	struct esl_attribute_node *EslNode;
	unsigned char sw_indicator_lamp;
	char para_buf[30];
	unsigned int ParaLen = 0,count = 0;
	unsigned char send_frame[UART_SEND_BUF_LEN];
	unsigned int send_frame_len;
	unsigned char recv_frame[UART_RECV_BUF_LEN];
	unsigned int recv_len;
	//unsigned short id;

	
	if(ack_buf == NULL)
		return ERR_PARA;

	proto = &g_aioi_proto;
	
	scan_esl_addr += (para[1] - 0x30) * 1000;
	scan_esl_addr += (para[2] - 0x30) * 100;
	scan_esl_addr += (para[3] - 0x30) * 10;
	scan_esl_addr += (para[4] - 0x30);

	EslNode = proto->esl_attr->get_node(proto->esl_attr,scan_esl_addr);
	if(EslNode != NULL){
		if(EslNode->type != LIGHT_ESL_TYPE){
			printf("esl type err!no scan esl!\n");
			ack_buf[0] = 'N';
			ack_buf[1] = get_check_sum(ack_buf,1);
			ack_buf[2] = 0x0d;

			*ack_len = 3;
		}else{
			sw_indicator_lamp = para[5];
			_DBG("sw_indicator_lamp:%d\n",para[5]);

			sw_indicator_lamp = (sw_indicator_lamp >> 1) & 0x03;
			*para_buf = sw_indicator_lamp;
			ParaLen = 1;

			_DBG("++++++ %s:%d \n",__FUNCTION__,__LINE__);
			ret = proto->esl_proto->cmd_build(scan_esl_addr,CMD_LED_CONTROL,para_buf,ParaLen,send_frame,&send_frame_len);
			_DBG("++++++ %s:%d \n",__FUNCTION__,__LINE__);
			if(ret != OK){
				printf("cmd build err!\n");
				ack_buf[0] = 'N';
				ack_buf[1] = get_check_sum(ack_buf,1);
				ack_buf[2] = 0x0d;

				*ack_len = 3;
			}else{
				proto->commx_array[EslNode->com_no].EslNode = EslNode;
				proto->commx_array[EslNode->com_no].send_frame = send_frame;
				proto->commx_array[EslNode->com_no].send_len = send_frame_len;
				pthread_mutex_lock(&proto->commx_array[EslNode->com_no].mutex);
				proto->commx_array[EslNode->com_no].send_and_recv(proto->commx_array,EslNode->com_no,recv_frame,&recv_len);
				pthread_mutex_unlock(&proto->commx_array[EslNode->com_no].mutex);
				usleep(100);
				
				ack_buf[0] = 'O';
				ack_buf[1] = get_check_sum(ack_buf,1);
				ack_buf[2] = 0x0d;

				*ack_len = 3;
			}
		}
	}else{
		printf("dengkong esl id err!\n");
		ack_buf[0] = 'N';
		ack_buf[1] = get_check_sum(ack_buf,1);
		ack_buf[2] = 0x0d;

		*ack_len = 3;
	}
	return OK;
}

static int cmd_A_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	unsigned short esl_addr = 0,new_addr = 0;
	int i,ret;
	aioi_proto_t *proto;
	char para_buf[30];
	unsigned int ParaLen = 0,count = 0;
	unsigned char send_frame[UART_SEND_BUF_LEN];
	unsigned int send_frame_len;
	unsigned char recv_frame[UART_RECV_BUF_LEN];
	unsigned int recv_len;
	unsigned int com_num;
	struct esl_attribute_node *EslNode;

	
	if(ack_buf == NULL || para == NULL)
		return ERR_PARA;

	proto = &g_aioi_proto;

	//memset(dev_name,0,10);

	ret = cfg_get_net_type();
	if(ret == NET_TYPE_CAN){
		cfg_get_dev_num_can(&com_num);
	}else if(ret == NET_TYPE_485){
		cfg_get_serial_num(&com_num);
	}else{
		printf("Net type ERR!\n");
		return ret;
	}
	
	
	switch(para[1]){
		case 'c':
			esl_addr += (para[2] - 0x30) * 1000;
			esl_addr += (para[3] - 0x30) * 100;
			esl_addr += (para[4] - 0x30) * 10;
			esl_addr += (para[5] - 0x30);


			new_addr += (para[6] - 0x30) * 1000;
			new_addr += (para[7] - 0x30) * 100;
			new_addr += (para[8] - 0x30) * 10;
			new_addr += (para[9] - 0x30);

			*(unsigned short *)para_buf = new_addr;
			ParaLen = sizeof(unsigned short);


			EslNode = proto->esl_attr->get_node(proto->esl_attr,esl_addr);
			if(EslNode == NULL)
				return ERR;


			ret = proto->esl_proto->cmd_build(esl_addr,CMD_MODIFY_ADDR,para_buf,ParaLen,send_frame,&send_frame_len);
			if(ret != OK){
				printf("cmd build err!\n");
				ack_buf[0] = 'N';
				ack_buf[1] = get_check_sum(ack_buf,1);
				ack_buf[2] = 0x0d;

				*ack_len = 3;
			}else{
				for(i = 0;i < com_num;i ++){
					proto->commx_array[EslNode->com_no].EslNode = EslNode;
					proto->commx_array[i].send_frame = send_frame;
					proto->commx_array[i].send_len = send_frame_len;
					pthread_mutex_lock(&proto->commx_array[i].mutex);
					proto->commx_array[i].send_and_recv(proto->commx_array,i,recv_frame,&recv_len);
					pthread_mutex_unlock(&proto->commx_array[i].mutex);
					usleep(100);
				}
				
				ack_buf[0] = 'O';
				ack_buf[1] = get_check_sum(ack_buf,1);
				ack_buf[2] = 0x0d;

				*ack_len = 3;
			}
			break;
		default:
			break;
	}
	

	return OK;

}



static int cmd_H_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	int i,ret;
	struct timeval tv;

	if(ack_buf == NULL || para == NULL)
		return ERR_PARA;

	// Heart beat process

	ack_buf[0] = 'O';
	ack_buf[1] = get_check_sum(ack_buf,1);
	ack_buf[2] = 0x0d;
	

	return OK;

}

static int cmd_I_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	int ret;
	unsigned int j,i,offset;
	aioi_proto_t *proto;
	unsigned short id,set_id;
	unsigned char cmd_analysis_para_buf[UART_SEND_BUF_LEN ];
	char para_buf[100];
	unsigned int ParaLen = 0,count = 0;
	unsigned char send_frame[UART_SEND_BUF_LEN];
	unsigned int send_frame_len;
	unsigned char recv_frame[UART_RECV_BUF_LEN];
	//unsigned int recv_len;
	struct event_params *params,*paras;
	//struct esl_attribute_node *EslNode;
	int cmd_analysis_ret;
	char disp_type;
	EVENT_ELEM_T *event;
	
	proto = &g_aioi_proto;
	
	if(ack_buf == NULL)
		return ERR_PARA;
	memset(para_buf,0,100);
	memset(send_frame,0,UART_SEND_BUF_LEN);
	memset(recv_frame,0,UART_RECV_BUF_LEN);
	memset(cmd_analysis_para_buf,0,UART_SEND_BUF_LEN);

	ack_buf[0] = 'O';
	ack_buf[1] = get_check_sum(ack_buf,1);
	ack_buf[2] = 0x0d;

	*ack_len = 3;

	_DBG("I command!\n");
	offset = get_offset(para,0x0d);
	_DBG("offset:%d\n",offset);

	IM_LOG("len: %d cmd Is para:\n",para_len);
	for(i = 0;i < para_len;i ++){
		IM_LOG(" %c ",para[i]);
	}
	IM_LOG("\n");
	
	switch(para[1]){
		case 's':
			set_id = (para[2] - 0x30)*1000 + (para[3] - 0x30)*100 + (para[4] - 0x30)*10 + para[5] - 0x30;
			printf("---- set_id:  %d\n",set_id);
			
			*(unsigned short *)para_buf = set_id;
			ParaLen = 2;

			ret = proto->esl_proto->cmd_build(0xffff,CMD_ID_ALLOC,para_buf,ParaLen,send_frame,&send_frame_len);
			if(ret != OK){
					printf("%s: cmd build err!\n",__FUNCTION__);
					return ret;
			}

			for(i = 0;i < g_sys_manager.com_num;i ++){
				proto->commx_array[i].send_frame = send_frame;
				proto->commx_array[i].send_len = send_frame_len;
				IM_LOG("-------- %d ---------\n",i);
				pthread_mutex_lock(&proto->commx_array[i].mutex);
				proto->commx_array[i].send_broadcast(proto->commx_array,i);
				pthread_mutex_unlock(&proto->commx_array[i].mutex);
				usleep(100);
			}
			break;
		default:
			printf("no this sub para!\n");
			break;
	}

	return OK;
}


static int cmd_S_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	int i,ret;
	struct timeval tv;

	if(ack_buf == NULL || para == NULL)
		return ERR_PARA;

	//

	ack_buf[0] = 'O';
	ack_buf[1] = get_check_sum(ack_buf,1);
	ack_buf[2] = 0x0d;
	
	g_independ_process.create_process(&g_independ_process,INDEP_PROCESS_SELF_TEST,NULL);


	return OK;

}

static int cmd_F_process(const char *para,unsigned int para_len,char *ack_buf,unsigned int *ack_len)
{
	int ret;
	unsigned int j,i,offset,count;

//	aioi_proto_t *proto;
//	char para_buf[100];
//	unsigned int ParaLen = 0,count = 0;
//	unsigned char send_frame[UART_SEND_BUF_LEN];
//	unsigned int send_frame_len;
//	unsigned char recv_frame[UART_RECV_BUF_LEN];
//	unsigned int recv_len;

	unsigned int port_num;
	unsigned char port_no;
	unsigned char esl_num;

	unsigned short esl_id;
	unsigned char esl_type;
	unsigned char esl_gs;
//	unsigned short esl_id;
//	unsigned char esl_type;
//	unsigned char esl_gs;
	//struct port_attr *port;
	
	
//	struct event_params *params,*paras;
//	struct esl_attribute_node *EslNode;
//	int cmd_analysis_ret;
//	char disp_type;
//	EVENT_ELEM_T *event;
	
//	proto = &g_aioi_proto;
	
	if(ack_buf == NULL)
		return ERR_PARA;
//	memset(para_buf,0,100);
//	memset(send_frame,0,UART_SEND_BUF_LEN);
//	memset(recv_frame,0,UART_RECV_BUF_LEN);



	ack_buf[0] = 'O';
	ack_buf[1] = get_check_sum(ack_buf,1);
	ack_buf[2] = 0x0d;

	*ack_len = 3;

	_DBG("F command!\n");
	offset = get_offset(para,0x0d);
	_DBG("offset:%d\n",i);
	
	switch(para[1]){
		case 'u':  //nova
			_DBG("____ cmd Pn!\n");
			
			port_num = para[2] - 0x30;  //port_num;

			
			cfg_set_dev_num_can(port_num);

			count = 3;

			for(i = 0;i < port_num;i ++){
				
				port_no = para[count] - 0x30;
				count += 1;

				// write esl_num
				esl_num = (para[count] - 0x30) * 100 + (para[count + 1] - 0x30) * 10 + para[count + 2] - 0x30;
				cfg_set_esl_num(port_no,esl_num);

				count += 3;
				for(j = 0; j < esl_num;j ++){
					esl_id = (para[count] - 0x30) * 1000 + (para[count + 1] - 0x30) * 100 + (para[count + 2] - 0x30)*10 + para[count + 3] - 0x30;
					count += 4;

					switch(para[count]){
						case 'S':
							esl_type = SCAN_ESL_TYPE;
							break;
						case 'D':
							esl_type = LIGHT_ESL_TYPE;
							break;
						case 'J':
							esl_type = PICK_ESL_TYPE;
							break;
						default:
							printf("%s : %d     err esl type!\n",__FUNCTION__,__LINE__);
							esl_type = 255;
							break;
					}
					count += 1;

					esl_gs = para[count] - 0x30;
					count += 1;

					cfg_set_spec_com_attr(port_no,j,esl_id,esl_type, esl_gs);
				}

				if(para[count] != CONF_PORT_PARAM_END){
					printf("fun:%s line: %d   wrong frame!\n",__FUNCTION__,__LINE__);
					return ERR;
				}

				count += 1;   // '0x1f'
			}


			system("reboot");

			break;
		default:
			printf("no this sub para!\n");
			break;
	}

	return OK;
}


func_node_t aioi_func_node_array[] = 
{
	{'Z',cmd_Z_process,{NULL,NULL}},
	{'P',cmd_P_process,{NULL,NULL}},
	{'G',cmd_G_process,{NULL,NULL}},
	{'T',cmd_T_process,{NULL,NULL}},
	{'A',cmd_A_process,{NULL,NULL}},
	{'H',cmd_H_process,{NULL,NULL}},
	{'I',cmd_I_process,{NULL,NULL}},
	{'S',cmd_S_process,{NULL,NULL}},
	{'F',cmd_F_process,{NULL,NULL}},
//	{'z',cmd_Z_process,{NULL,NULL}},
	{0,NULL,{NULL,NULL}}
};




extern uart_commx_t uart_commx_array[];
extern can_comx_t can_comx_array[];

int aioi_proto_init(struct aioi_proto *proto,unsigned int net_type,unsigned int com_num)
{
	int ret;
	unsigned int i;
	unsigned char *para_buf;
	struct event_params *params;
	struct hlist_node *pos;
	struct hlist_head *hhead;
	struct esl_attribute_node *EslNode;
	
	if(proto == NULL || proto->fun_h == NULL || proto->esl_attr == NULL)
		return ERR_PARA;

	ret = proto->fun_h->init(proto->fun_h);
	if(ret != OK)
		return ERR;

	ret = init_func_hlist(aioi_func_node_array,proto->fun_h);
	if(ret != OK)
		return ERR;

	
	switch(net_type){
		case NET_TYPE_485:
			proto->commx_array = (dev_com_t *)malloc(sizeof(dev_com_t) * com_num);
			for(i = 0;i < com_num;i ++){
				proto->commx_array[i].spec_com = uart_commx_array;
				proto->commx_array[i].mutex = uart_commx_array[i].mutex;
				proto->commx_array[i].add_up_data_item = uart_commx_array[i].add_up_data_item;
				proto->commx_array[i].del_up_data_item = uart_commx_array[i].del_up_data_item;
				proto->commx_array[i].send_and_recv = uart_commx_array[i].send_and_recv;
				proto->commx_array[i].send_broadcast = uart_commx_array[i].send_broadcast;
			}
			break;
		case NET_TYPE_CAN:
			proto->commx_array = (dev_com_t *)malloc(sizeof(dev_com_t) * com_num);
			for(i = 0;i < com_num;i ++){
				proto->commx_array[i].spec_com = can_comx_array;
				proto->commx_array[i].mutex = can_comx_array[i].mutex;
				proto->commx_array[i].add_up_data_item = can_comx_array[i].add_up_data_item;
				proto->commx_array[i].del_up_data_item = can_comx_array[i].del_up_data_item;
				proto->commx_array[i].send_and_recv = can_comx_array[i].send_and_recv;
				proto->commx_array[i].send_broadcast = can_comx_array[i].send_broadcast;
			}
			break;
		default:
			return ERR;
	}
	
	proto->para_buf_q->front = 0;
	proto->para_buf_q->rear = 0;

	for(i = 0;i < FRAME_BUF_Q_SIZE;i ++){
		para_buf = (unsigned char *)malloc(PARA_BUF_SIZE);
		if(para_buf == NULL){
			printf("para buf malloc err!\n");
			return ERR;
		}
		_DBG("+*+* para_buf addr:0x%x\n",para_buf);
		memset(para_buf,0,PARA_BUF_SIZE);
		proto->para_buf_q->cmd_para[i] = para_buf;
	}
	
	return OK;
}

int aioi_proto_deinit(struct aioi_proto *proto)
{
	int ret;
	unsigned int i;
	unsigned char *para_buf;
	
	if(proto == NULL || proto->fun_h == NULL)
		return ERR_PARA;

	
	for(i = 0;i < FRAME_BUF_Q_SIZE;i ++){
		para_buf = proto->para_buf_q->cmd_para[i];
		if(para_buf != NULL){
			printf("%%line:%d  addr: %x\n",__LINE__,para_buf);
			free(para_buf);
		}

	}

	ret = proto->fun_h->deinit(proto->fun_h);
	if(ret != OK)
		return ERR;
	
	return OK;
}

int aioi_proto_process(struct aioi_proto *proto,const char *recv_frame,unsigned int recv_len,char *send_buf,unsigned int *send_len)
{
	int ret;
	struct func_node *fun_nod;
	unsigned char CMD;
	AIOI_PROCESS_FUN_T func;
	
	CMD = recv_frame[0];

	fun_nod = proto->fun_h->get_func_node(proto->fun_h,CMD);
	if(fun_nod != NULL){
		func = (AIOI_PROCESS_FUN_T)fun_nod->param;
		ret = func(recv_frame,recv_len,send_buf,send_len);
		if(ret != OK)
			return ret;
	}

	return OK;
}

int aioi_get_up_data(struct aioi_proto *proto,char *ret_buf,unsigned int *len)
{
	unsigned char *para;
	struct return_data *ret_d;
	unsigned short esl_id,set_id;
	unsigned char para_len;

	
	para = get_process_para_buf(proto->para_buf_q);
	if(para == NULL)
		return ERR;

	_DBG("$------------- %s:%d:There is a message for update!\n",__FUNCTION__,__LINE__);
	ret_d = (struct return_data *)para;
	_DBG("para: 0x%x \n",para);
	//IM_LOG("$------------- ret_d->type:%d   ret_d->id: %d   ret_d->len:%d \n",ret_d->type,ret_d->id,ret_d->len);
	_DBG("$------------- %s:%d:There is a message for update!\n",__FUNCTION__,__LINE__);
	switch(ret_d->type){
		case DATA_TYPE_SCAN:
			_DBG("$------------- %s:%d:DATA_TYPE_SCAN!\n",__FUNCTION__,__LINE__);
			esl_id = ret_d->id;
			para_len = ret_d->len;
			ret_buf[0] = 't';
			ret_buf[4] = esl_id % 10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[3] = esl_id % 10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[2] = esl_id % 10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[1] = esl_id % 10 + 0x30;

			if(*(para + sizeof(struct return_data) + ret_d->len - 2) == 0x0d){
				memcpy(ret_buf + 5,para + sizeof(struct return_data),ret_d->len - 1);
				ret_buf[3 + ret_d->len] = get_check_sum(ret_buf, 3 + ret_d->len);
				ret_buf[4 + ret_d->len] = 0x0d;
				*len = 5 + ret_d->len;
			}else{
				memcpy(ret_buf + 5,para + sizeof(struct return_data),ret_d->len);
				ret_buf[4 + ret_d->len] = get_check_sum(ret_buf,4 + ret_d->len);
				ret_buf[5 + ret_d->len] = 0x0d;
				*len = 6 + ret_d->len;
			}
			//ret_buf[6 + ret_d->len - 1] = get_check_sum(ret_buf, 6 + ret_d->len - 1);
			//ret_buf[6 + ret_d->len] = 0x0d;
			//*len = 7 + ret_d->len;
			break;
		case DATA_TYPE_DISP:
			_DBG("$------------- %s:%d:DATA_TYPE_DISP!\n",__FUNCTION__,__LINE__);
			esl_id = ret_d->id;
			ret_buf[0] = 'd';
			ret_buf[4] = esl_id%10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[3] = esl_id%10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[2] = esl_id%10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[1] = esl_id%10 + 0x30;


			memcpy(ret_buf + 5,para + sizeof(struct return_data),ret_d->len);
			ret_buf[4 + ret_d->len] = get_check_sum(ret_buf, 4 + ret_d->len);
			ret_buf[5 + ret_d->len] = 0x0d;
			*len = 6 + ret_d->len;
			break;
		case DATA_TYPE_SET_ID:
			set_id = ret_d->id;//(unsigned short *)para;
			ret_buf[0] = '@';
			ret_buf[4] = set_id%10 + 0x30;
			set_id = set_id/10;
			ret_buf[3] = set_id%10 + 0x30;
			set_id = set_id/10;
			ret_buf[2] = set_id%10 + 0x30;
			set_id = set_id/10;
			ret_buf[1] = set_id%10 + 0x30;
			ret_buf[5] = get_check_sum(ret_buf,5);
			ret_buf[6] = 0x0d;
			*len = 7;
			break;
		case DATA_TYPE_HEART_BEAT:
			
			break;
		case DATA_TYPE_ERR_INFO:
//			IM_LOG("$------------- %s:%d:DATA_TYPE_ERR_INFO!\n",__FUNCTION__,__LINE__);
			esl_id = ret_d->id;
			ret_buf[0] = 'e';
			ret_buf[4] = esl_id%10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[3] = esl_id%10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[2] = esl_id%10 + 0x30;
			esl_id = esl_id/10;
			ret_buf[1] = esl_id%10 + 0x30;
			ret_buf[5] = para + sizeof(struct return_data);
			ret_buf[6] = get_check_sum(ret_buf,5);
			ret_buf[7] = 0x0d;
			*len = 8;
			break;
		default:
			break;
	}
		
	para_process_buf_delete(proto->para_buf_q);

	_DBG("$------------- %s:%d:There is a message for update!\n",__FUNCTION__,__LINE__);
	return OK;
}

#ifdef __DEV_COM__

#else
extern uart_commx_t uart_commx_array[];
#endif
extern func_hlist_t g_process_func;
extern esl_attr_hlist_t g_esl_attr_hlist;
extern esl_protocol_t g_esl_proto;
PARA_BUF_QUEUE_T para_buf_q;



aioi_proto_t g_aioi_proto = 
{
	.esl_attr = &g_esl_attr_hlist,
	.fun_h = &g_process_func,
#ifdef __DEV_COM__
	.commx_array = NULL,
#else
	.commx_array = uart_commx_array,
#endif
	.esl_proto = &g_esl_proto,
	.para_buf_q = &para_buf_q,
	
	.init = aioi_proto_init,
	.deinit = aioi_proto_deinit,
	.process = aioi_proto_process,
	.get_up_data = aioi_get_up_data
};


