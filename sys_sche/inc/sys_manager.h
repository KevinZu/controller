/******************************************************************************
                                                                            
 File Name:      sys_manager.h                                       
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
#ifndef __THREAD_H__
#define __THREAD_H__
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "SDL.h"
#include "list.h"
#include "uart_commx.h"
#include "tcp_comm.h"
#include "esl_attr.h"
#include "protocol.h"
#include "can_dev_monitor.h"



typedef struct sys_manager{
	unsigned int com_num;
	unsigned int net_type;
	tcp_comm_t *tcp_com;
	//unsigned int uart_num;
#ifdef __DEV_COM__
//	dev_com_t *comx_array;
#else
//	uart_commx_t *comx_array;
#endif
	esl_attr_hlist_t *esl_attr;
	aioi_proto_t *aioi_proto;
	esl_protocol_t *esl_proto;
	can_dev_monitor_t *can_monitor;

	int (*init)(struct sys_manager *);
	int (*deinit)(struct sys_manager *);
	int (*monitor)(struct sys_manager *);
}sys_manager_t;


#endif/*__SYS_MANAGER_H__*/

