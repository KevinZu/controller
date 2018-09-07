/******************************************************************************
                                                                            
 File Name:      uart_commx.h                                       
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20131211		Kevin.Zu		Create 

*******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "sys.h"
#include "uart_commx.h"
#include "config.h"
#include "protocol.h"


uart_commx_t uart_commx_array[UART_MAX_NUM];

//char a[]={0x31,0x32,0x33,0xfc};

#if 0
static int catch_frame_end(const char **in)
{
	char *ptr;
	ptr = *in;
	while(ptr - *in < UART_RECV_FARME_SIZE){
		if(*ptr == 0xfc && *(ptr + 1) == 0xfc){
			return OK;
		}
		ptr += 1;
	}
	return ERR;
}
#endif




extern aioi_proto_t g_aioi_proto;

static int _event_get_KeyResult(void *para)
{
	int ret;
	struct aioi_proto *proto;
	proto = &g_aioi_proto;
	struct event_params *params;
	char send_frame[UART_SEND_BUF_LEN];
	unsigned int send_frame_len;
	unsigned char recv_frame[UART_RECV_BUF_LEN];
	unsigned int recv_len;
	unsigned char *para_buf;
	struct return_data *ret_d;
	ack_head_t *ack_h;
	EVENT_ELEM_T *event;
	char cmd_para[10];
	unsigned short esl_addr;
	uart_commx_t *commx_array;
	commx_array = uart_commx_array;

	if(para == NULL){
		printf("para err!\n");
		return ERR_PARA;
	}

	params = (struct event_params *)para;
	printf("  ********** id: %d \n",params->id);
	event = params->event;
	if(event == NULL){
		printf("event  err!fun:%s\n",__FUNCTION__);
		return ERR_PARA;
	}
	memset(send_frame,0,UART_SEND_BUF_LEN);
	memset(recv_frame,0,UART_RECV_BUF_LEN);

	cmd_para[0] = 3;
	ret = proto->esl_proto->cmd_build(params->id,CMD_LCD_GET_ITEM,cmd_para,1,send_frame,&send_frame_len);
	if(ret != OK){
		printf("%s: cmd build err!\n",__FUNCTION__);
		return ret;
	}
	
	proto->commx_array[params->com_no].send_frame = send_frame;
	proto->commx_array[params->com_no].send_len = send_frame_len;
	pthread_mutex_lock(&proto->commx_array[params->com_no].mutex);
	proto->commx_array[params->com_no].send_and_recv(proto->commx_array,params->com_no,recv_frame,&recv_len);
	pthread_mutex_unlock(&proto->commx_array[params->com_no].mutex);
	usleep(100);
	if(recv_len > 0){
		//*send_len = proto->commx_array[i].recv_len;
		ack_h = (ack_head_t *)(recv_frame + 2);
		esl_addr = Tranverse16(ack_h->esl_addr);
		if(esl_addr != params->id){
			//free(params);
			printf("key--- esl_addr:%d  id:%d\n",esl_addr,params->id);
			return OK;
		}

		if(recv_frame[6]  & 0x80){
			printf("key is not press down!\n");
			return OK;
		}else{
			para_buf = get_free_para_buf(proto->para_buf_q);
			if(para_buf == NULL)
				return ERR;

			memset(para_buf,0,PARA_BUF_SIZE);
			
			ret_d = (struct return_data *)para_buf;
			ret_d->type = DATA_TYPE_DISP;
			//ack_h = (ack_head_t *)(proto->commx_array[params->com_no].recv_frame + 2);
			ret_d->len = ack_h->cmd_len;
			ret_d->id = params->id;
			printf("============== cmd_len : %d   *****id: %d \n",ack_h->cmd_len,params->id);
			
			memcpy(para_buf + sizeof(struct return_data),recv_frame + sizeof(ack_head_t) + 2,ack_h->cmd_len);
			
			para_process_buf_insert(proto->para_buf_q);
			
			printf("%%line:%d  addr: %x\n",__LINE__,params);
//================================================================
//		proto->commx_array[EslNode->com_no].del_up_data_item()
//************************************************************
			commx_array[params->com_no].del_cyclic_query(&commx_array[params->com_no],event,params->com_no);
//			printf("%%line:%d  addr: %x\n",__LINE__,event);
//			free(event);
//			params->event = NULL;

			printf("%%line:%d  addr: %x\n",__LINE__,params);
			free(params);
			params = NULL;
//================================================================
//			free(event);
		}
	}else{
		printf("%s: esl:%d not ack!\n",__FUNCTION__,params->id);
	}
	
	
	return OK;
}






static int _event_get_scan(void *para)
{
	int ret;
	struct aioi_proto *proto;
	proto = &g_aioi_proto;
	struct event_params *params;
	char send_frame[UART_SEND_BUF_LEN];
	unsigned int send_frame_len;
	unsigned char recv_frame[UART_RECV_BUF_LEN];
	unsigned int recv_len;
	unsigned char *para_buf;
	struct return_data *ret_d;
	ack_head_t *ack_h;
	unsigned short esl_addr;

	params = (struct event_params *)para;
	memset(send_frame,0,UART_SEND_BUF_LEN);
	memset(recv_frame,0,UART_RECV_BUF_LEN);
	
	ret = proto->esl_proto->cmd_build(params->id,CMD_CODE_SCAN,NULL,0,send_frame,&send_frame_len);
	if(ret != OK){
		printf("%s: cmd build err!\n",__FUNCTION__);
		return ret;
	}
	
	proto->commx_array[params->com_no].send_frame = send_frame;
	proto->commx_array[params->com_no].send_len = send_frame_len;

	pthread_mutex_lock(&proto->commx_array[params->com_no].mutex);
	proto->commx_array[params->com_no].send_and_recv(proto->commx_array,params->com_no,recv_frame,&recv_len);
	//pthread_mutex_unlock(&proto->commx_array[params->com_no].mutex);
	pthread_mutex_unlock(&proto->commx_array[params->com_no].mutex);
	usleep(100);
	
	if(recv_len > 0){
		//memcpy(send_buf,proto->commx_array[i].recv_frame,proto->commx_array[i].recv_len);
		//*send_len = proto->commx_array[i].recv_len;
		ack_h = (ack_head_t *)(recv_frame + 2);
		esl_addr = Tranverse16(ack_h->esl_addr);
		if(esl_addr != params->id){
			//free(params);
			printf("scan--- esl_addr:%d  id:%d\n",esl_addr,params->id);
			return OK;
		}

		if(recv_frame[6] > 0){
			para_buf = get_free_para_buf(proto->para_buf_q);
			if(para_buf == NULL)
				return ERR;

			memset(para_buf,0,PARA_BUF_SIZE);
			
			ret_d = (struct return_data *)para_buf;
			ret_d->type = DATA_TYPE_SCAN;
			
			ret_d->len = ack_h->cmd_len;
			ret_d->id = params->id;
			printf("++++++++++++++ ack_h->cmd_len : %d\n",ack_h->cmd_len);
			
			memcpy(para_buf + sizeof(struct return_data),recv_frame + sizeof(ack_head_t) + 2,ack_h->cmd_len);
			
			para_process_buf_insert(proto->para_buf_q);
		}
	}else{
		printf("%s: esl:%d not ack!\n",__FUNCTION__,params->id);
	}
	
	
	return OK;
}


static int check_frame(char *s,unsigned int len)
{	
	unsigned int i = 0;	
	while(i < len - 1){			
		if((unsigned char)*(s + i) == 0xfc && (unsigned char)*(s + i + 1) == 0xfc){					
			return i;
		}		
		i ++;
	}
	return ERR;
}

static int uart_send_and_recv(void *comx_array,unsigned int com_no,unsigned char *recv_frame,unsigned int *recv_len )
{
	int ret,select_ret;
	int i;
	int fd;
	fd_set rfds;
	char *str_ret;
	struct timeval tv;
	struct uart_commx *uart;
	//int sret;
	unsigned int readlen = 0;
	char * ptr;
//	ptr = uart->recv_frame;
	*recv_len = 0;
	unsigned int timeout;
	unsigned char recv_buf[300];
	unsigned char recv_data_len;
	dev_com_t *dev_com_array;
	struct uart_commx *uart_array;

	if(recv_frame == NULL || comx_array == NULL){
		return ERR_PARA;
	}

	dev_com_array = (dev_com_t *)comx_array;
	uart_array = (struct uart_commx *)dev_com_array[com_no].spec_com;
	
	timeout = 1050; //10S
	uart = &uart_array[com_no];
	uart->send_frame = dev_com_array[com_no].send_frame;
	uart->send_len = dev_com_array[com_no].send_len;
	fd = uart->uartx.fd_p;
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);       
	tv.tv_sec  = timeout / 1000;
	tv.tv_usec = (timeout%1000)*1000;
	
	
	if(uart->uartx.fd_p < 0){
		printf("uart is not opened!\n");
		return ERR;
	}

	memset(recv_buf,0,300);

	//ptr = recv_frame;
	ptr = recv_buf;

	for(i = 0;i < uart->send_len;i++)
		printf("%02hhx ",*(uart->send_frame + i));
	printf("\n");
	ret = uart->uartx.UartSend(uart->uartx.fd_p,uart->send_frame,uart->send_len);
	//printf("uart send ret: %d\n",ret);
	if(ret != uart->send_len){
		printf("uart send err!\n");
//		pthread_mutex_lock(&uart->mutex);
		return ret;
	}

	while(1){
		select_ret = select(fd+1,&rfds,NULL,NULL,&tv);
		if(select_ret == -1){
			printf("select err!line:%d\n",__LINE__);
//			pthread_mutex_unlock(&uart->mutex);
			break;
		}else if(select_ret > 0){
			//printf("select ok!ret:%d\n",select_ret);
			if(FD_ISSET(fd, &rfds)){
		//		
uart_read:
				ret = read(fd,ptr,150);
				
				//printf("sec: %d,usec: %d\n",tv.tv_sec,tv.tv_usec);
				if(ret < 0){
					printf("read err:%s\n",__FUNCTION__);
//					pthread_mutex_unlock(&uart->mutex);
					break;
				}else if(ret == 0){
//					pthread_mutex_unlock(&uart->mutex);
					break;
				}
				
				readlen += ret;
				printf("*** ret:%d\n",ret);
				ptr += ret;
				ret = check_frame(recv_buf,readlen);
				if(ret > 0){
					*recv_len = ret + 2;//readlen;
					memcpy(recv_frame,recv_buf,*recv_len);

					printf("***frame_len:%d \n",*recv_len);
				
					if(*recv_len > 8){
						for(i = 0;i < *recv_len;i++)
							printf("%02hhx ",*(recv_frame + i));
						printf("\n");
					}

//					pthread_mutex_unlock(&uart->mutex);
					break;
				}else{
					printf("***read_len:%d \n",readlen);
						for(i = 0;i < readlen;i++)
							printf("-%02hhx ",*(recv_buf + i));
						printf("\n");
					continue;
				}
				
			}else{
//				pthread_mutex_unlock(&uart->mutex);
				printf("not this dev!\n");
				break;
			}
		}else{
//			pthread_mutex_unlock(&uart->mutex);
			printf("timeout!\n");
			break;
		}
	}
	
}

#if 0
static int select_uartx(struct uart_commx *uart)
{
	
}
#endif


static int uart_add_up_data_item(void *uartx,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr)
{
	struct uart_commx *uart;
	struct event_params *params,*paras;
	//struct aioi_proto *proto;
	EVENT_ELEM_T *event,*epos;
	EVENT_HANDLE event_handle;
	uart_commx_t *commx_array;
	commx_array = uart_commx_array;

	if(uartx == NULL || EslNode == NULL){
		return ERR_PARA;
	}
	
	uart = (struct uart_commx *)uartx;
	//params = (struct event_params *)param;

	//proto = &g_aioi_proto;

	switch(event_type){
		case E_GET_KEY_STATUS:
			event_handle = _event_get_KeyResult;
			break;
		case E_GET_SCAN_DATA:
			event_handle = _event_get_scan;
			break;
		default:
			return ERR;
	}

	list_for_each_entry(epos,&commx_array[EslNode->com_no].cyclic_query_list_head,list_e){
		paras = (struct event_params *)epos->params;
		if(paras->id == scan_esl_addr && epos->event_handle == event_handle){
			//This process is already exist.
			return ERR;
		}
	}
	
	params = malloc(sizeof(struct event_params));
	params->id = EslNode->id;
	params->shelf_no = EslNode->shelf_no;
	params->com_no = EslNode->com_no;
	event = (EVENT_ELEM_T *)malloc(sizeof(EVENT_ELEM_T));
	if(event == NULL){
		printf("event malloc err!func:%s\n",__FUNCTION__);
		return ERR_NOMEM;
	}

	params->event = event;
	event->params = params;

	event->event_handle = event_handle;

	commx_array[EslNode->com_no].add_cyclic_query(&commx_array[EslNode->com_no],event,EslNode->com_no);


	return OK;
}



static int uart_del_up_data_item(void *uartx,struct esl_attribute_node *EslNode,unsigned char event_type,unsigned short scan_esl_addr)
{
	struct uart_commx *uart;
	struct event_params *params,*paras;
//	struct aioi_proto *proto;
	EVENT_ELEM_T *epos;
	EVENT_HANDLE event_handle;
	uart_commx_t *commx_array;
	commx_array = uart_commx_array;

	if(uartx == NULL || EslNode == NULL){
		return ERR_PARA;
	}
	
	uart = (struct uart_commx *)uartx;
	//params = (struct event_params *)param;
//	proto = &g_aioi_proto;
	
	switch(event_type){
		case E_GET_KEY_STATUS:
			event_handle = _event_get_KeyResult;
			break;
		case E_GET_SCAN_DATA:
			event_handle = _event_get_scan;
			break;
		default:
			return ERR;
	}

	list_for_each_entry(epos,&commx_array[EslNode->com_no].cyclic_query_list_head,list_e){
		paras = (struct event_params *)epos->params;
		if(paras->id == scan_esl_addr && epos->event_handle == event_handle){
			CYC_LOCK(&commx_array[EslNode->com_no].cyclic_lock);
			list_del(&epos->list_e);
			CYC_UNLOCK(&commx_array[EslNode->com_no].cyclic_lock);
			if(epos->params != NULL){
				printf("%%line:%d  addr: %x\n",__LINE__,epos->params);
				free(epos->params);
				epos->params = NULL;
			}
			printf("%%line:%d  addr: %x\n",__LINE__,epos);
			free(epos);
			epos = NULL;
			break;
		}
	}
	
	return OK;
}

static int uart_send_broadcast(void *uartx,unsigned char comno)
{

	return OK;
}



static int add_cyclic_query_item(struct uart_commx *uart,EVENT_ELEM_T *event,unsigned char com_no)
{
	int ret;
	
	if(uart == NULL || event == NULL){
		return ERR_PARA;
	}
	
	CYC_LOCK(&uart[com_no].cyclic_lock);
	list_add(&event->list_e,&uart->cyclic_query_list_head);
	CYC_UNLOCK(&uart[com_no].cyclic_lock);

	return OK;
}


static int del_cyclic_query_item(struct uart_commx *uart,EVENT_ELEM_T *event,unsigned char com_no)
{
	EVENT_ELEM_T *pevent;
	struct list_head *pos,*n;
	
	if(uart == NULL || event == NULL)
		return ERR_PARA;
	
	list_for_each_safe(pos,n,&uart->cyclic_query_list_head){
		pevent = list_entry(pos,EVENT_ELEM_T,list_e);

		if(pevent == event){
			CYC_LOCK(&uart[com_no].cyclic_lock);
			list_del(pos);
			CYC_UNLOCK(&uart[com_no].cyclic_lock);
			return OK;
		}
	}
	//event_del_list(event,&uart->cyclic_query_list_head);
	if(event != NULL)
		free(event);
	
	return OK;
}



static int uart_thread_func(void *para)
{
	int ret;
	struct uart_commx *uart;
	
	uart = (struct uart_commx *)para;
	
	while(1){
		event_process(&uart->cyclic_query_list_head, &ret);
		if(uart->is_thread_alive == 0)
			return OK;
	}
	return OK;
}

#define DEV_PATH_STR_LEN 20
static int uart_commx_init(struct uart_commx *uart,unsigned int i)
{
	int ret;
	unsigned char dev_name[10];
	char name_offset;

	if(uart == NULL)
		return ERR_PARA;

	memset(dev_name,0,10);
	//memset(dev_path,0,20);

	//sprintf(dev_path,"/dev/");
#if 0
	ret = cfg_get_dev_name(dev_name);
	if(ret != OK){
		printf("get dev name err!\n");
		return ret;
	}

	ret = strncmp(dev_name,"ttyO",4);
	if(ret = OK){
		name_offset = 4;
	}else{
		name_offset = 0;
	}
#endif
	//strcat(dev_path,dev_name);

	/*uartx initialize*/
	uart->uartx.uart_dev_path = (char *)malloc(DEV_PATH_STR_LEN);
	if(uart->uartx.uart_dev_path == NULL)
		return ERR;
	
	memset(uart->uartx.uart_dev_path,0,DEV_PATH_STR_LEN);
#if 0
	sprintf(uart->uartx.uart_dev_path,"/dev/");
	strcat(uart->uartx.uart_dev_path,dev_name);
	sprintf(dev_name,"%d",i);
	strcat(uart->uartx.uart_dev_path,dev_name);
#endif
	sprintf(uart->uartx.uart_dev_path,"/dev/ttyO%d",i + 4);

//	sprintf(uart->uartx.uart_dev_path,"/dev/ttyM%d",i);
	//sprintf(uart->uartx.uart_dev_path,"/dev/ttyO%d",i + name_offset);
	uart->uartx.uart_flags = O_RDWR;
	uart->uartx.speed = 19200;
	uart->uartx.nDatabits = 8;
	uart->uartx.nStopbits = 1;
	uart->uartx.nParity = 'n';
	uart->uartx.c_cc_vtime = 0;
	uart->uartx.c_cc_vmin = 1;
	uart->uartx.UartOpen = uart_open;
	uart->uartx.UartClose= uart_close;
	uart->uartx.UartRecv = uart_recv;
	uart->uartx.UartSend = uart_send;
	uart->uartx.fd_p = -1;
	ret = uart->uartx.UartOpen(&uart->uartx);
	if(ret != PL_SUCCESS){
		printf("open uart %d err!\n",i);
		return ERR;
	}

	/*send frame buff*/
	uart->send_frame = (char *)malloc(UART_SEND_FARME_SIZE);
	if(uart->send_frame == NULL){
		return ERR;
	}
	memset(uart->send_frame,0,UART_SEND_FARME_SIZE);
	uart->send_len = 0;
	

	ret = pthread_mutex_init(&uart->mutex,NULL);
	if(ret != OK){
		printf("uart mutex init err! line:%d\n",__LINE__);
		return ERR;
	}

	ret = pthread_mutex_init(&uart->cyclic_lock,NULL);
	if(ret != OK){
		printf("uart cyclic_lock init err! line:%d\n",__LINE__);
		return ERR;
	}


	INIT_LIST_HEAD(&uart->cyclic_query_list_head);
	
	/*thread*/
	uart->is_thread_alive = 1;
	uart->thread = SDL_CreateThread(uart_thread_func, uart);
	if(uart->thread == NULL){
		return ERR;
	}

	uart->send_and_recv = uart_send_and_recv;
//	uart->select_uartx = select_uartx;
	uart->add_cyclic_query = add_cyclic_query_item;
	uart->del_cyclic_query = del_cyclic_query_item;
	uart->add_up_data_item = uart_add_up_data_item;//(void * uart, void * params)
	uart->del_up_data_item = uart_del_up_data_item;//(void * uart, void * params)
	uart->send_broadcast = uart_send_broadcast;
	
	return OK;
}


static int uart_commx_deinit(struct uart_commx *uart,unsigned int i)
{
	int ret;

	if(uart == NULL)
		return ERR_PARA;

	uart->is_thread_alive = 0;
	SDL_WaitThread(uart->thread,0);


	pthread_mutex_destroy(&uart->mutex);

	if(uart->send_frame != NULL){
		free(uart->send_frame);
		uart->send_frame = NULL;
	}

	ret = uart->uartx.UartClose(&uart->uartx);
	if(ret != PL_SUCCESS){
		printf("open uart %d err!\n",i);
		return ERR;
	}

	if(uart->uartx.uart_dev_path != NULL){
		free(uart->uartx.uart_dev_path);
		uart->uartx.uart_dev_path = NULL;
	}
	
	return OK;
}



int uart_commx_array_init(unsigned int serial_num)
{
	int i;
	int ret;
	//unsigned int serial_num;
	uart_commx_t *uart_array;
	uart_array = uart_commx_array;
	
	memset(uart_array,0,sizeof(uart_commx_t) * UART_MAX_NUM);

	//ret = cfg_get_serial_num(&serial_num);
	//if(ret < 0){
	//	return ret;
	//}

	for(i = 0;i < serial_num;i ++){
		//uart_commx_init(uart_commx_array + i,i);
		(uart_array + i)->init = uart_commx_init;
		(uart_array + i)->deinit = uart_commx_deinit;
		(uart_array + i)->init(uart_array + i,i);
	}

	return OK;
}

int uart_commx_array_deinit(unsigned int serial_num)
{
	int i,ret;
	//unsigned int serial_num;

	//ret = cfg_get_serial_num(&serial_num);
	//if(ret < 0){
	//	return ret;
	//}

	for(i = 0;i < serial_num;i ++){
		uart_commx_deinit(uart_commx_array + i,i);
	}

	return OK;
}


int uart_commx_array_restart(unsigned int serial_num)
{
	int i,ret;
	//unsigned int serial_num;

	//ret = cfg_get_serial_num(&serial_num);
	//if(ret < 0){
	//	return ret;
	//}
	for(i = 0;i < serial_num;i ++){
		if(uart_commx_array[i].is_thread_alive == 0){
			ret = uart_commx_array[i].deinit(&uart_commx_array[i],i);
			if(ret == OK)
				uart_commx_array[i].init(&uart_commx_array[i],i);
		}
	}

	return OK;
}



