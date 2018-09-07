
/******************************************************************************
                                                                            
 File Name:      esl_proto.h                                             
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com					
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20130621		Kevin.Zu		Create 
 20131205		Kevin.Zu		Ported to linux
 							The file name changed to "esl.proto.h",The old file name is 
 							"pick_protocol.h"

*******************************************************************************/

#ifndef __ESL_PROTO_H__
#define __ESL_PROTO_H__

#include "func_hlist.h"

/*************************** function ****************************/

typedef int (*CMD_BUILD_T)(short id,char cmd,char *para,unsigned int para_len,char *frame,unsigned int *frame_len);



/****************************** cmd code *************************/

#define CMD_CHECK_STATE 0x01
#define CMD_INDICATOR_LIGHT_CONTROL 0x02
#define CMD_KEY_QUEUE_CONTROL 0x03
#define CMD_GET_LED_FLASH_STATUS 0x04
#define CMD_MODIFY_ADDR 0x06
#define CMD_SYS_RESET 0x07

#define CMD_LCD_UPDATE 0x05
#define CMD_UPDATE_DISPLAY 0x05
#define CMD_LCD_GET_ITEM 0x09
#define CMD_SCAN_SWITCH 0x10
#define CMD_GET_DATA_OK 0x11
#define CMD_SELECT_VOICE 0x12
#define CMD_ID_ALLOC 0x08

#define CMD_CODE_SCAN 0x60
#define CMD_LED_CONTROL 0x62

#define CMD_SELF_TEST 0x09



/**************** ack para *****************/

struct ack_query_state_para{
	unsigned char state1;
}__attribute__((packed));

struct ack_query_addr_para{
	unsigned char addr;
}__attribute__((packed));

struct ack_motion_state_para{
	unsigned short motion_type;
	unsigned char motion_state;
}__attribute__((packed));

struct cmd_query_state_para{
	unsigned char state1;
}__attribute__((packed));

struct cmd_query_addr_para{
	unsigned char addr;
}__attribute__((packed));

struct cmd_motion_state_para{
	unsigned short motion_type;
}__attribute__((packed));

struct cmd_lcd_update_item{
	unsigned char item_id;
}__attribute__((packed));

/****************************************************************/
//extern unsigned char test_id[2];
//#define RECV_BUF_LEN 200
#define UART_SEND_BUF_LEN 300
#define UART_RECV_BUF_LEN 300
#define BROADCAST_ADDR_485 0xFFFF
//#define LOCAL_ADDR_485(proto)  (proto->esl_addr)//((unsigned short)(test_id[1] * 256 + test_id[0]))
#define FRAME_START_ID (0x0d)

#define FRAME_START_FLAG 0xfefe
#define FRAME_STOP_FLAG 0xfcfc
#define _START_CHAR 0xfe
#define _STOP_CHAR 0xfc
#define _PAD_CHAR 0x55

#define _LED_ON 0x01
#define _LED_OUT 0x02
#define _KEY_QUEUE_OPEN 0x01
#define _KEY_QUEUE_CLOSE 0x02
#define _KEY_IS_DONE 0x01
#define _KEY_IS_NOT_DONE 0x02

enum{
	FRAME_IDLE,
	FRAME_START_0,
	FRAME_START,
	FRAME_COMPLETE_0,
	FRAME_COMPLETE
};


typedef struct cmd_head{
	unsigned char start_id;
	//unsigned char esl_addr;
	unsigned short esl_addr;
	unsigned char cmd_code;
	unsigned char  cmd_len;
}__attribute__((packed))cmd_head_t;



typedef struct ask_head{
	unsigned char start_id;
	unsigned short esl_addr;
	unsigned char cmd_code;
	unsigned char  cmd_len;
}__attribute__((packed))ack_head_t;




typedef struct esl_protocol{
	func_hlist_t *fun_h;  //cmd_build_func
	

	int (*init)(struct esl_protocol *,unsigned int);
	int (*deinit)(struct esl_protocol *);
	int (*cmd_build)(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len);
	int (*cmd_analysis)(const char *frame,unsigned int len,int *ret,char *para); 
}esl_protocol_t;


#define CR_LEN 1



#endif

