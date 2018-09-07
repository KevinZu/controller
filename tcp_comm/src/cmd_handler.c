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
#include "sys.h"
#include "cmd_handler.h"
#include "uart_commx.h"




static int cmd_parser(struct cmd_handler *cmd)
{
	int ret;
	unsigned int i;
	char cmd_code;
	
	if(cmd == NULL || cmd->recv_frame == NULL)
		return ERR_PARA;

/*test*/
	_DBG("cmd_parser.........\n");
	cmd_code = cmd->recv_frame[0];
	ret = cmd->aioi_proto->process(cmd->aioi_proto,cmd->recv_frame,cmd->recv_len,cmd->send_buf,&cmd->send_len);
	
	
	return OK;
}



static int cmd_get_data(struct cmd_handler *cmd,char *buf)
{
	unsigned int len = 0;
	cmd->aioi_proto->get_up_data(cmd->aioi_proto,buf,&len);
	
	return len;
}

extern aioi_proto_t g_aioi_proto;


cmd_handler_t g_cmd_handler = {
	.recv_frame = NULL,
	.recv_len = 0,
	
	.aioi_proto = &g_aioi_proto,
	
	.cmd_parser = cmd_parser,
	.get_data = cmd_get_data
};
