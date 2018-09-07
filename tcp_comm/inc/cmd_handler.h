/******************************************************************************
                                                                            
 File Name:      cmd_handler.h                                       
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
#ifndef __CMD_HANDLER_H__
#define __CMD_HANDLER_H__
#include "event.h"
//#include "uart_commx.h"
#include "SDL_mutex.h"
#include "SDL_sysmutex_c.h"
#include "protocol.h"
#include "esl_proto.h"
//#include "esl_attr.h"
//#include "sub_socket_comm.h"



typedef struct cmd_handler{
	char *recv_frame;	//point to recv_frame
	unsigned int recv_len;
	char *send_buf;
	unsigned int send_len;
	
	
	aioi_proto_t *aioi_proto;

	int (*cmd_parser)(struct cmd_handler *cmd_handler);
	int (*get_data)(struct cmd_handler *cmd_handler,char *);
}cmd_handler_t;


#endif

