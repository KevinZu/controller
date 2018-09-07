/******************************************************************************

 File Name:      can_bus.c
 Copyright:
 Version:
 Description:
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com	

*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20140219		Kevin.Zu		Create 

*******************************************************************************/
#include <stdio.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/socket.h>
#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "sys.h"
#include "can_bus.h"
#include "config.h"

#ifdef   __cplusplus
    extern   "C" 
    {
#endif

PARA_BUF_QUEUE_T can_updata_q;
PARA_BUF_QUEUE_T can_answer_q;


static void print_frame(struct can_frame *fr)
{
	int i;
	static unsigned int loop_cnt = 0;
#if 0
	IM_LOG("rece data : \r\n");
	IM_LOG("ID = %08x %08x\n", fr->can_id & CAN_EFF_MASK, loop_cnt);
	
	IM_LOG("type = %02x,", _id_get_type(fr->can_id));
	IM_LOG("addr = %04x,", _id_get_addr(fr->can_id));
	IM_LOG("dir = %02x,", _id_get_dir(fr->can_id));
	IM_LOG("attr = %02x,", _id_get_attr(fr->can_id));
	IM_LOG("index = %02x,", _id_get_index(fr->can_id));
	IM_LOG("ack = %02x\n", _id_get_ack(fr->can_id));
	
	IM_LOG("DLC = %d ", fr->can_dlc);
	IM_LOG("DATA = ");
#else
	_DBG("rece data : \r\n");
	_DBG("ID = %08x %08x\n", fr->can_id & CAN_EFF_MASK, loop_cnt);
	
	_DBG("type = %02x,", _id_get_type(fr->can_id));	
	_DBG("addr = %04x,", _id_get_addr(fr->can_id));	
	_DBG("dir = %02x,", _id_get_dir(fr->can_id));		
	_DBG("attr = %02x,", _id_get_attr(fr->can_id));	
	_DBG("index = %02x,", _id_get_index(fr->can_id));	
	_DBG("ack = %02x\n", _id_get_ack(fr->can_id));	
	
	_DBG("DLC = %d ", fr->can_dlc);
	_DBG("DATA = ");
#endif
	for (i = 0; i < fr->can_dlc; i++)
#if 0
		IM_LOG("%02x ", fr->data[i]);
	IM_LOG("\n");
	IM_LOG("\n");
#else
		_DBG("%02x ", fr->data[i]);
	_DBG("\n");
	_DBG("\n");
#endif

	loop_cnt++;
}

#define errout(_s)	fprintf(stderr, "error class: %s\n", (_s))
#define errcode(_d) fprintf(stderr, "error code: %02x\n", (_d))

static void handle_err_frame(const struct can_frame *fr)
{
	if (fr->can_id & CAN_ERR_TX_TIMEOUT) {
		errout("CAN_ERR_TX_TIMEOUT");
	}
	if (fr->can_id & CAN_ERR_LOSTARB) {
		errout("CAN_ERR_LOSTARB");
		errcode(fr->data[0]);
	}
	if (fr->can_id & CAN_ERR_CRTL) {
		errout("CAN_ERR_CRTL");
		errcode(fr->data[1]);
	}
	if (fr->can_id & CAN_ERR_PROT) {
		errout("CAN_ERR_PROT");
		errcode(fr->data[2]);
		errcode(fr->data[3]);
	}
	if (fr->can_id & CAN_ERR_TRX) {
		errout("CAN_ERR_TRX");
		errcode(fr->data[4]);
	}
	if (fr->can_id & CAN_ERR_ACK) {
		errout("CAN_ERR_ACK");
	}
	if (fr->can_id & CAN_ERR_BUSOFF) {
		errout("CAN_ERR_BUSOFF");
	}
	if (fr->can_id & CAN_ERR_BUSERROR) {
		errout("CAN_ERR_BUSERROR");
	}
	if (fr->can_id & CAN_ERR_RESTARTED) {
		errout("CAN_ERR_RESTARTED");
	}
}
#define myerr(str)	fprintf(stderr, "%s, %s, %d: %s\n", __FILE__, __func__, __LINE__, str)



static int can_recv(struct can_bus *can,struct can_frame *frame)
{
	int ret;

	if(can == NULL || frame == NULL)
		return ERR_PARA;

	_DBG("++++++ %s:%d \n",__FUNCTION__,__LINE__);
	ret = select(can->socket+1,&can->rset, NULL, NULL, NULL);
	if (ret == 0) {
		printf("select time out\n");
		return 0;
	}

	//_DBG("++++++ %s:%d \n",__FUNCTION__,__LINE__);
	ret = read(can->socket, frame, sizeof(struct can_frame));
	if (ret < sizeof(struct can_frame)) {
		myerr("read failed");
		return -1;
	}
	
	if (frame->can_id & CAN_ERR_FLAG) { /* å‡ºé”™è®¾å¤‡é”™è¯¯ */
		handle_err_frame(frame);
		myerr("CAN device error");
		return ERR;
	}
	print_frame(frame);

	return frame->can_dlc;
}



static int can_send(struct can_bus *can,can_data_t  *can_data)
{
	int ret, i;
	struct can_frame frdup;
	struct timeval tv;
	fd_set rset;

	if(can == NULL)
		return ERR_PARA;
	frdup.can_id = can_data->id;//_create_can_id(can_data->id.type,can_data->id.addr,can_data->id.dir,can_data->id.attr,can_data->id.index,can_data->id.ack);
	frdup.can_dlc = can_data->can_dlc;
	for(i = 0;i < 8;i ++){
		frdup.data[i] = *(can_data->data + i);
	}

	_DBG("*************** socket:%d\n",can->socket);
	ret = write(can->socket, &frdup, sizeof(frdup));
	if (ret < 0) {
		myerr("write failed");
		return -1;
	}
	//_DBG("o ");
	//usleep(3000);
	usleep(8000);
	return OK;
}

static int can_bus_deinit(struct can_bus *can,unsigned int no)
{
	int i;
	
	pthread_mutex_destroy(&can->mutex_recv);
	pthread_mutex_destroy(&can->mutex_send);
	
	if(can->dev_name != NULL)
		free(can->dev_name);

	
	return OK;
}


int can_bus_init(struct can_bus *can,unsigned int no)
{
    	int ret,i;
    	struct sockaddr_can addr;
    	struct ifreq ifr;
	char dev_name[10];
	struct timeval tv;
	struct can_filter filter[1];
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	if(can == NULL)
		return ERR_PARA;


	can->recv = can_recv;
	can->send = can_send;
	can->deinit = can_bus_deinit;
	
	FD_ZERO(&can->rset);
	FD_SET(can->socket,&can->rset);
	
	ret = pthread_mutex_init(&can->mutex_recv,NULL);
	if(ret != OK){
		printf("can recv mutex init err! line:%d\n",__LINE__);
		return ERR;
	}

	ret = pthread_mutex_init(&can->mutex_send,NULL);
	if(ret != OK){
		printf("can send mutex init err! line:%d\n",__LINE__);
		return ERR;
	}

	memset(dev_name,0,10);


	can->dev_name = (char *)malloc(_DEV_NAME_LEN);
	memset(can->dev_name,0,_DEV_NAME_LEN);

	ret = cfg_get_dev_name_can(&dev_name);
	if(ret < 0){
		printf("Can bus get dev name err!\n");
		return ERR;
	}
	sprintf(can->dev_name,"%s%d",dev_name,no);


	srand(time(NULL));
	can->socket = socket(PF_CAN, SOCK_RAW, CAN_RAW); //´´½¨Ì×½Ó×Ö
	if (can->socket < 0) {
        	perror("socket PF_CAN failed");
        	return 1;
    	}

	IM_LOG("Can dev name:%s\n",can->dev_name);
	
	strcpy(ifr.ifr_name,can->dev_name);
	
	ret = ioctl(can->socket, SIOCGIFINDEX, &ifr);
    	if (ret < 0) {
        	perror("ioctl failed");
        	return 1;
    	}
	
	addr.can_family = AF_CAN;
    	addr.can_ifindex = ifr.ifr_ifindex;
    	ret = bind(can->socket, (struct sockaddr *)&addr, sizeof(addr));
    	if (ret < 0) {
        	perror("bind failed");
        	return 1;
    	}

	ret = setsockopt(can->socket, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));
	if (ret < 0) {
		printf("### setsockopt failed\n");
		return ERR;
	}

	return OK;
}





#ifdef   __cplusplus
	}
#endif


