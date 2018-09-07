#if 0
/******************************************************************************
                                                                            
 File Name:      serial.h                                      
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

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <fcntl.h>


int serial_init(int *fd);
int serial_de_init(int *fd);

#endif
#endif
/******************************************************************************
                                                                            
 File Name:      uart.h   --> serial.h                                              
 Copyright:      DediProg										
 Version:                                                                
 Description:    
			Uart Operation module definition
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           		NAME           	DESCRIPTION                               
 20121214		Kevin.Zu		Create 
 20131205		Kevin.Zu		Add to ELS Controller project
 
*******************************************************************************/
#ifndef _UART_H_
#define _UART_H_


#ifdef   __cplusplus
    extern   "C" 
    {
#endif

typedef  struct DpUart{
	int fd_p;
	/*** dev ***/
	char *uart_dev_path;
	int uart_flags;
	/*** set ***/
	int speed;
	int nDatabits;
	int nStopbits;
	int nParity;
	int c_cc_vtime;
	int c_cc_vmin;
	/*** operation ***/
	int (*UartOpen)(struct DpUart *uart);
	int (*UartClose)(struct DpUart *uart);
	int (*UartRecv)(int fd, char *buf, int len);
	int (*UartSend)(int fd, char *buf, int len);
}DpUart_t;


int uart_open(DpUart_t *p_uart);
int uart_close(DpUart_t *p_uart);
int uart_recv(int fd, char *buf, int len);
int uart_send(int fd, char *buf, int len);


#ifdef   __cplusplus
	}
#endif

#endif /* _UART_H_ */


