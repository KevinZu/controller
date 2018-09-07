/******************************************************************************
 File Name:      uart.c   --> serial.c                                                 
 Copyright:      							
 Version:                                                                
 Description:    
			Uart Operation module

 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com	
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           		NAME           	DESCRIPTION                               
 20121214		Kevin.Zu		Create
 20131205		Kevin.Zu		Add to ESL Controller project

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <termios.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "sys.h"
#include "serial.h"


#ifdef   __cplusplus
    extern   "C" 
    {
#endif

static int uart_set_attribution(DpUart_t *p_uart);

static int SpeedArr[15] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
                    B38400, B19200, B9600, B4800, B2400, B1200, B300};

static int NameArr[15] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300,
                   38400, 19200, 9600, 4800, 2400, 1200, 300};

int uart_open(DpUart_t *p_uart)
{
	p_uart->fd_p = open(p_uart->uart_dev_path,p_uart->uart_flags);
	if (p_uart->fd_p == -1) {
		perror("uart_open");
		return PL_OPEN_ERR;
	}
	return uart_set_attribution(p_uart);
}

int uart_close(DpUart_t *p_uart)
{
	int ret;
	if(p_uart->fd_p == -1){
		printf("err fd!\n");
		return PL_BADPARAMETER;
	}
	ret = close(p_uart->fd_p);
	if(ret != 0)
		return PL_CLOSE_ERR;
	p_uart->fd_p = -1;
	return PL_SUCCESS;
}

static int uart_set_attribution(DpUart_t *p_uart)
{	
	int i;
	int num;
	int status;
	struct termios Opt;

	tcgetattr(p_uart->fd_p, &Opt);

    /* set speed */
	num = sizeof(SpeedArr) / sizeof(int);
	for (i=0; i<num; i++){
		if (p_uart->speed == NameArr[i]){
			tcflush(p_uart->fd_p, TCIOFLUSH);
			cfsetispeed(&Opt, SpeedArr[i]);
			cfsetospeed(&Opt, SpeedArr[i]);
			break;
		}
	}
	
	if (i > num){
		return PL_BADPARAMETER;
	}

	Opt.c_iflag &=~(IGNBRK|IGNCR|INLCR|ICRNL|IUCLC|IXANY|IXON|IXOFF|INPCK|ISTRIP);
	Opt.c_iflag |= (BRKINT|IGNPAR);
	Opt.c_oflag &=~OPOST;
	Opt.c_lflag &=~(XCASE|ECHONL|NOFLSH);
	Opt.c_lflag &=~(ICANON|ISIG|ECHO);
	Opt.c_cflag |= (CLOCAL|CREAD);

    /* set data bits */
	switch (p_uart->nDatabits){
		case 7:
			Opt.c_cflag |= CS7;
			break;
		case 8:
			Opt.c_cflag |= CS8;
			break;
		default:
			perror("Unsupported data bits");
			return PL_BADPARAMETER;
	}
    
    /* set parity */
	switch (p_uart->nParity){
		case 'n':
		case 'N':
			Opt.c_cflag &= ~PARENB;
			Opt.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':
			Opt.c_cflag |= (PARODD | PARENB);
			Opt.c_iflag |= INPCK;
			break;
		case 'e':
 		case 'E':
			Opt.c_cflag |= PARENB;
			Opt.c_cflag &= ~PARODD;
			Opt.c_iflag |= INPCK;
			break;
		case 's':
		case 'S':
			Opt.c_cflag &= ~PARENB;
			Opt.c_cflag &= ~CSTOPB;
			Opt.c_iflag |= INPCK;
			break;
		default:
			perror("Unsupported parity");
			return PL_BADPARAMETER;
	}

	/* set stop bits */
	switch (p_uart->nStopbits){
		case 1:
			Opt.c_cflag &= ~CSTOPB;
			break;
		case 2:
			Opt.c_cflag |= CSTOPB;
			break;
		default:
			perror("Unsupported stop bits");
			return PL_BADPARAMETER;
	}

	tcflush(p_uart->fd_p, TCIFLUSH);
	//tcflush(fd, TCIOFLUSH);

	Opt.c_cc[VTIME] = p_uart->c_cc_vtime;
	Opt.c_cc[VMIN]  = p_uart->c_cc_vmin;

	if (tcsetattr(p_uart->fd_p, TCSANOW, &Opt) != 0){
		perror("tcsetattr err");
		return PL_BADPARAMETER;
	}

	ioctl(p_uart->fd_p, TIOCMGET, &status);
	status |= TIOCM_RTS;
	ioctl(p_uart->fd_p, TIOCMSET, &status);
	//fcntl(fd, F_SETFL, FNDELAY);


	return PL_SUCCESS;
}

int uart_recv(int fd, char *buf, int len)
{
	return read(fd, buf, len);
}


int uart_send(int fd, char *buf, int len)
{
    return write(fd, buf, len);
}

#ifdef   __cplusplus
	}
#endif


