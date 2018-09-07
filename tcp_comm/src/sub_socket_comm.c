/******************************************************************************
                                                                            
 File Name:      sub_socket_comm.c                               
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com			
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           		NAME           	DESCRIPTION                               
 20131210		Kevin.Zu		Create 

*******************************************************************************/
#include "sys.h"
#include "sub_socket_comm.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>

#include "log_s.h"



extern cmd_handler_t g_cmd_handler;
extern can_dev_monitor_t g_can_monitor;

static int sub_create_connection(struct sub_sock_para *sub_para,sub_socket_comm_t *sub_sock);
static int sub_destory_connection(sub_socket_comm_t *sub_sock);

static void _sub_sock_close_sockinfo(struct _tcp_sockets *sockinfo)
{
	close(sockinfo->socket);
	memset(sockinfo, 0, sizeof(*sockinfo));
}


static int frame_check(const char *input,unsigned int len)
{
	unsigned int i;
	
	if(len == 0)
		return ERR;

	for(i = 0;i < len;i ++){
		if((unsigned char)*(input + i) == 0x0d)
			return i;
	}
	
	return ERR;
}

static int _sub_sock_send_sockinfo (struct _tcp_sockets *sockinfo, const char *msg, int msglen)
{
	int i;
	_DBG("++++++ %s:%d \n",__FUNCTION__,__LINE__);
	for(i = 0;i < msglen;i ++){
		_DBG("%02x ",*(msg + i));
	}
	_DBG("\n");
	
	while (1) {
		i = send(sockinfo->socket, (const void *) msg, msglen, 0);
		_DBG("tcp send ret:%d\n",i);
		if (i < 0) {
			printf("send err!\n");
			close(sockinfo->socket);
			return ERR;
			//continue;
		} else if (i == 0) {
			break; /* what's the meaning here? */
		} else if (i < msglen) {
			msglen -= i;
			msg += i;
			continue;
		}
		break;
	}
	return OK;
}



static int _sub_sock_recv(sub_socket_comm_t *sub_sock)
{
	//unsigned char frame_buf[RECV_FRAME_BUF_LEN];
	unsigned char *buf;
	struct _tcp_sockets *sockinfo;
	sockinfo = &sub_sock->tcp_sock;
	int r,ret;
	//unsigned int frame_len;
	unsigned int frame_size,diff_size,i;

sub_recv:
	r = recv(sockinfo->socket,sub_sock->buf + SIZE_OF_MSG_LEN/* + sub_sock->buflen*/, sub_sock->bufsize - SIZE_OF_MSG_LEN/*sub_sock->buflen*/, 0);
	//_DBG("$     recv ret:%d\n",r);
	if (r == 0) {
		//_DBG("*** recv ret:%d\n",r);
		return OK;//ERR_UNDEFINED_ERROR;
	} else if (r < 0) {
		_sub_sock_close_sockinfo(sockinfo);
		printf("tcp_tl_read_message -- fdset r = recv < 0\n");
		return ERR_UNDEFINED_ERROR;
	} else {
		//int consumed;
		*(unsigned short *)sub_sock->buf = r;
		pthread_mutex_lock(&sub_sock->recv_q_mutex);
		ret = tcp_process_buf_insert(sub_sock->tcp_recv_buf_q);
		pthread_mutex_unlock(&sub_sock->recv_q_mutex);
		usleep(100);
		if(ret != OK){
			printf("para_process_buf_insert err !  fun: %s \n",__FUNCTION__);
			while(1){printf("e");sleep(1);}
			return ret;
		}
get_new_recv_buf:
		pthread_mutex_lock(&sub_sock->recv_q_mutex);
		buf = get_free_tcp_buf(sub_sock->tcp_recv_buf_q);
		pthread_mutex_unlock(&sub_sock->recv_q_mutex);
		usleep(100);
		if(buf != NULL){
				sub_sock->buf = buf;
				sub_sock->bufsize = RECV_BUF_LEN;
				//sub_sock->buflen = 0;
		}else{
			usleep(500000);
			printf("line:%d .  get recv buf err!\n",__LINE__);
			goto get_new_recv_buf;
		}
	}
}



static int messages_handle(sub_socket_comm_t *sub_sock,const char *frame,unsigned int frame_size)
{
	struct _tcp_sockets *sockinfo;
	int ret;
	cmd_handler_t *cmd;

	char sendbuf[SEND_FRAME_BUF_LEN];
	memset(sendbuf,0,SEND_FRAME_BUF_LEN);
	
	unsigned char *send_buf;

	if(sub_sock == NULL){
		printf("err para! func:%s\n",__FUNCTION__);
		return ERR;
	}

	sockinfo = &sub_sock->tcp_sock;
	cmd = sub_sock->cmd_handler;//&g_cmd_handler;

	cmd->recv_frame = frame;//buf;
	cmd->recv_len = frame_size;//frame_len;
	cmd->send_buf = sendbuf;
	_DBG("frame_size:%d\n",frame_size);
	ret = cmd->cmd_parser(cmd);
	if(ret == OK){
		send_buf = get_free_para_buf(sub_sock->send_frame_q);
		if(send_buf != NULL){
			memset(send_buf,0,PARA_BUF_SIZE);
			send_buf[0] = cmd->send_len;
			memcpy(send_buf + 1,cmd->send_buf,cmd->send_len);
			ret = para_process_buf_insert(sub_sock->send_frame_q);
			if(ret != OK){
				printf("para_process_buf_insert err !  fun: %s \n",__FUNCTION__);
				return ret;
			}
		}		
	}

	return OK;
	//_DBG("handle_messages, buf: %s ---- buflen = %d\n",buf,strlen(buf));
}






int thread_sub_sock1_recv(void *para)
{

#ifdef __DEBUG__
	struct  timeval    tv0;
        struct  timezone   tz0;
	struct  timeval    tv1;
        struct  timezone   tz1;
	unsigned int time_us;
	gettimeofday(&tv0,&tz0);
#endif
	int ret;
	sub_socket_comm_t *sub_sock;
	int sock;
	fd_set rfds, wfds;//, efds; 
	struct timeval timeout;

	
	_DBG("+++ thread: %s \n",__FUNCTION__);
	sub_sock = (sub_socket_comm_t *)para;
	sock = sub_sock->tcp_sock.socket;
	if(sock < 0){
		return ERR_PARA;
	}
	
	timeout.tv_sec=0;
	timeout.tv_usec=200;
	
	FD_ZERO(&rfds);
	FD_SET(sock,&rfds);
	
	while(1){
#ifdef __DEBUG__
		gettimeofday(&tv1,&tz1);
		time_us = 1000000 * (tv1.tv_sec - tv0.tv_sec) + tv1.tv_usec - tv0.tv_usec;
		if(time_us > 1000000){
			gettimeofday(&tv0,&tz0);
			printf("----%s----\n",__FUNCTION__);
		}
#endif

	/********* recv msg **********/
		ret = select(sock + 1, NULL, &rfds, NULL, &timeout);
		if(ret<0){
			//printf("read select err\n");
		}else{
			ret = FD_ISSET(sock,&rfds);
			if(ret != 0){
				_sub_sock_recv(sub_sock);
			}
		}
	
	/********* thread exit ********/
		if(sub_sock->is_recv_thread_alive ==  0)
			return OK;

		usleep(10);
	}
}


extern int log_id1;

int thread_sub_sock1_send(void *para)
{

#ifdef __DEBUG__
	struct  timeval    tv0;
        struct  timezone   tz0;
	struct  timeval    tv1;
        struct  timezone   tz1;
	unsigned int time_us;
	
	gettimeofday(&tv0,&tz0);
#endif
	int ret;
	unsigned short recv_para_len;
	sub_socket_comm_t *sub_sock;
	unsigned char *send_buf,*recv_buf;
	unsigned int send_len,frame_len;
	static unsigned int recv_frame_len = 0,RecvBufHandSize = 0;
	//char SendBuf[SEND_FRAME_BUF_LEN],*_send_buf;
	char *_send_buf;
	
	char RecvQBuf[RECV_FRAME_BUF_LEN];

	sub_sock = (sub_socket_comm_t *)para;
	//memset(SendBuf,0,SEND_FRAME_BUF_LEN);
	memset(RecvQBuf,0,RECV_FRAME_BUF_LEN);

	_DBG("+++ thread: %s \n",__FUNCTION__);
	//recv_frame_len = 0;
	//RecvBufHandSize = 0;
	//_send_buf = SendBuf;
	_send_buf = sub_sock->sendbuf;

	while(1){
#ifdef __DEBUG__
		gettimeofday(&tv1,&tz1);
		time_us = 1000000 * (tv1.tv_sec - tv0.tv_sec) + tv1.tv_usec - tv0.tv_usec;
		if(time_us > 1000000){
			gettimeofday(&tv0,&tz0);
			printf("----%s----\n",__FUNCTION__);
		}
		
#endif
		usleep(10);
		//send_len = sub_sock->cmd_handler->get_data(sub_sock->cmd_handler,SendBuf);

		// up data
		send_len = sub_sock->cmd_handler->get_data(sub_sock->cmd_handler,_send_buf);
		
		if(send_len > 0){
			_DBG("%%%  %s:%d            send_len: \n",__FUNCTION__,__LINE__,send_len);
			//#########################################################
			while(*_send_buf == 0x00 && send_len > 0){
				_send_buf ++;
				send_len --;
			}
			//ret = _sub_sock_send_sockinfo(&sub_sock->tcp_sock,SendBuf,send_len);
			ret = _sub_sock_send_sockinfo(&sub_sock->tcp_sock,_send_buf,send_len);
			if(ret != OK){
		//		IM_LOG("line:%d send err!\n",__LINE__);
#ifdef __MONITOR__
				if(g_can_monitor.thread != NULL)
					g_can_monitor.del_thread(&g_can_monitor);
#endif
				return ERR;
			}
		}

		// answer
		send_buf = get_process_para_buf(sub_sock->send_frame_q);
		if(send_buf != NULL){
			_DBG("=========   len: %d\n",send_buf[0]);
			int i;
			for(i = 1;i <= send_buf[0];i ++)
				_DBG("%02hhx ",*(send_buf + i));
			_DBG("\n");
			/********* send msg **********/
			frame_len = send_buf[0];
			ret = _sub_sock_send_sockinfo(&sub_sock->tcp_sock,send_buf + 1,frame_len);
			if(ret != OK){
				IM_LOG("line:%d send err!\n",__LINE__);
			}
			
			para_process_buf_delete(sub_sock->send_frame_q);
		}


		//recv msg q. process
		pthread_mutex_lock(&sub_sock->recv_q_mutex);
		recv_buf = get_process_tcp_buf(sub_sock->tcp_recv_buf_q);
		pthread_mutex_unlock(&sub_sock->recv_q_mutex);
		usleep(100);
		if(recv_buf != NULL){
			recv_para_len = *(unsigned short *)recv_buf;
			_DBG("---- recv_para_len = %d\n",recv_para_len);
			ret = frame_check(recv_buf + SIZE_OF_MSG_LEN,recv_para_len);
			_DBG("---- ret: %d\n",ret);
			if(ret >= 0){
				memcpy(RecvQBuf + sub_sock->buflen,recv_buf + SIZE_OF_MSG_LEN,ret + 1);
				pthread_mutex_lock(&sub_sock->recv_q_mutex);
				tcp_process_buf_delete(sub_sock->tcp_recv_buf_q);
				pthread_mutex_unlock(&sub_sock->recv_q_mutex);
				usleep(100);
				
				_DBG("---- ret: %d    --------messages_handle\n",ret);
				LOG_PRINT(log_id1,"---- ret: %d    --------messages_handle\n",ret);
				messages_handle(sub_sock,RecvQBuf,sub_sock->buflen + ret + 1);

				if(recv_para_len > ret + 1){
					memcpy(RecvQBuf,recv_buf + SIZE_OF_MSG_LEN + ret + 1,recv_para_len - ret - 1);
					sub_sock->buflen = recv_para_len - ret - 1;
				}else{
					sub_sock->buflen = 0;
				}
			}else{
				memcpy(RecvQBuf + sub_sock->buflen,recv_buf + SIZE_OF_MSG_LEN,recv_para_len);
				sub_sock->buflen += recv_para_len;
				pthread_mutex_lock(&sub_sock->recv_q_mutex);
				tcp_process_buf_delete(sub_sock->tcp_recv_buf_q);
				pthread_mutex_unlock(&sub_sock->recv_q_mutex);
				usleep(100);
			}
		}

		/********* thread exit ********/
		if(sub_sock->is_send_thread_alive ==  0)
			return OK;
	}
}



static int sub_create_connection(struct sub_sock_para *sub_para,sub_socket_comm_t *sub_sock)
{
	int ret;
	unsigned char *buf;
	can_dev_monitor_t *monitor = &g_can_monitor;
	
	sub_sock->is_recv_thread_alive = 1;
	sub_sock->is_send_thread_alive = 1;


	sub_sock->tcp_sock.socket = sub_para->socket;
	strncpy(sub_sock->tcp_sock.remote_ip, sub_para->remote_ip,REMOTE_IP_LEN - 1);
	sub_sock->tcp_sock.remote_port = sub_para->remote_port;

	pthread_mutex_lock(&sub_sock->recv_q_mutex);
	buf = get_free_tcp_buf(sub_sock->tcp_recv_buf_q);
	pthread_mutex_unlock(&sub_sock->recv_q_mutex);
	usleep(100);
	if(buf != NULL){
		sub_sock->buf = buf;
		sub_sock->bufsize = RECV_BUF_LEN;
		sub_sock->buflen = 0;
	}else{
		return NULL;
	}

	sub_sock->recv_thread = SDL_CreateThread(thread_sub_sock1_recv, sub_sock);
	if(sub_sock->recv_thread == NULL)
		return NULL;

	sub_sock->send_thread = SDL_CreateThread(thread_sub_sock1_send, sub_sock);
	if(sub_sock->send_thread == NULL)
		return NULL;

#ifdef __MONITOR__
	IM_LOG("\n\nmonitor thread creat!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
	ret = monitor->create_thread(monitor);
	if(ret != OK){
		printf("can monitor thread creat err!\n");
		return ret;
	}
#endif
	
	return OK;
}


static int sub_destory_connection(sub_socket_comm_t *sub_sock)
{
	int ret;
//	can_dev_monitor_t *monitor = &g_can_monitor;

	if(sub_sock == NULL)
		return ERR_PARA;

#if 0
	if(monitor->thread != NULL){
		ret = monitor->del_thread(monitor);
		if(ret != OK){
			printf("can monitor thread del err!\n");
			return ret;
		}
	}
#endif

	/*kill thread*/
	sub_sock->is_recv_thread_alive = 0;
	if(sub_sock->recv_thread != NULL){
		SDL_WaitThread(sub_sock->recv_thread,0);
		sub_sock->recv_thread = NULL;
	}

	sub_sock->is_send_thread_alive = 0;
	if(sub_sock->send_thread != NULL){
		SDL_WaitThread(sub_sock->send_thread,0);
		sub_sock->send_thread = NULL;
	}
	
	if(sub_sock->tcp_sock.socket > 0)
		close(sub_sock->tcp_sock.socket);

	return OK;
}


//#undef __DEBUG__



extern cmd_handler_t g_cmd_handler;
extern can_dev_monitor_t g_can_monitor;
extern sub_socket_comm_t g_sub_socket ;
sub_socket_comm_t *creat_sub_socket_comm(void *data) //data --- sub_socket
{	
	int ret,i;
	unsigned char *send_buf,*buf,*recv_buf;
	sub_socket_comm_t *sub_sock;
	//struct sub_sock_para *sub_para;

	_DBG("+++ creat sub socket comm: %s \n",__FUNCTION__);
	

	//sub_para = (struct sub_sock_para *)data;
#if 0
	sub_sock = (sub_socket_comm_t *)malloc(sizeof(sub_socket_comm_t));
	if(sub_sock == NULL){
		printf("line:%d fun:%s \n",__LINE__,__FUNCTION__);
		return sub_sock;
	}
#endif 
	sub_sock = &g_sub_socket;
	memset(sub_sock,0,sizeof(sub_socket_comm_t));

	sub_sock->create_connection = sub_create_connection;
	sub_sock->destory_connection = sub_destory_connection;

#ifdef __MONITOR__
	/*********** can monitor ************/
	sub_sock->can_monitor = &g_can_monitor;
#endif

	sub_sock->send_frame_q = (PARA_BUF_QUEUE_T *)malloc(sizeof(PARA_BUF_QUEUE_T));
	if(sub_sock->send_frame_q == NULL){
		printf("send_frame_q malloc err!\n");
		return ERR;
	}

	sub_sock->send_frame_q->front = 0;
	sub_sock->send_frame_q->rear = 0;

	for(i = 0;i < FRAME_BUF_Q_SIZE;i ++){
		send_buf = (unsigned char *)malloc(PARA_BUF_SIZE);
		if(send_buf == NULL){
			printf("send [%d] buf malloc err!line:%d\n",i,__LINE__);
			return ERR;
		}
		memset(send_buf,0,PARA_BUF_SIZE);
		sub_sock->send_frame_q->cmd_para[i] = send_buf;

	}

	sub_sock->tcp_recv_buf_q = (TCP_BUF_QUEUE_T *)malloc(sizeof(TCP_BUF_QUEUE_T));
	if(sub_sock->tcp_recv_buf_q == NULL){
		printf("TCP_BUF_QUEUE malloc err!\n");
		return ERR;
	}

	sub_sock->tcp_recv_buf_q->front = 0;
	sub_sock->tcp_recv_buf_q->rear = 0;

	for(i = 0;i < TCP_BUF_Q_SIZE;i ++){

		recv_buf = (unsigned char *)malloc(RECV_BUF_LEN);
		if(recv_buf == NULL){
			printf("$$$ recv [%d] buf malloc err!line:%d\n",i,__LINE__);
			return ERR;
		}
		memset(recv_buf,0,RECV_BUF_LEN);
		sub_sock->tcp_recv_buf_q->tcp_buf[i] = recv_buf;
	}

	ret = pthread_mutex_init(&sub_sock->recv_q_mutex,NULL);
	if(ret != OK){
		printf("sub_sock->recv_q_mutex init err! line:%d\n",__LINE__);
		return ERR;
	}


//	sub_sock->copy_count = 0;

	
	sub_sock->sendbuf = (char *)malloc(SEND_FRAME_BUF_LEN);
	if(sub_sock->sendbuf == NULL){
		printf("------------sub_sock->sendbuf malloc err!");
		return NULL;
	}
	sub_sock->sendbuflen = 0;
	
	sub_sock->cmd_handler = &g_cmd_handler;

	
	return sub_sock;
}
int destory_sub_socket_comm(sub_socket_comm_t *sub_sock)
{
	int ret,i;
	unsigned char *send_buf,*recv_buf;


	if(sub_sock->sendbuf != NULL){
		free(sub_sock->sendbuf);
		sub_sock->sendbuf = NULL;
	}


	for(i = 0;i < FRAME_BUF_Q_SIZE;i ++){
		send_buf = sub_sock->send_frame_q->cmd_para[i];
		if(send_buf != NULL){
			free(send_buf);
			send_buf = NULL;
		}

	}


	if(sub_sock->send_frame_q != NULL){
		free(sub_sock->send_frame_q);
		sub_sock->send_frame_q = NULL;
	}


	for(i = 0;i < TCP_BUF_Q_SIZE;i ++){
		recv_buf = sub_sock->tcp_recv_buf_q->tcp_buf[i];
		if(recv_buf != NULL){
			free(recv_buf);
			recv_buf = NULL;
		}
	}


	if(sub_sock->tcp_recv_buf_q != NULL){
		free(sub_sock->tcp_recv_buf_q);
		sub_sock->tcp_recv_buf_q = NULL;
	}

#if 0
	if(sub_sock != NULL){
		free(sub_sock);
		sub_sock = NULL;
	}
#endif
	
	return OK;
}


sub_socket_comm_t g_sub_socket = {
	
};


