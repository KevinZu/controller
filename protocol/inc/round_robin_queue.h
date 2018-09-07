/******************************************************************************
                                                                            
 File Name:      round_robin_queue.h                                                 
 Copyright:      									
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com
                   
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20130515		Kevin.Zu		Create 
 20131205		Kevin.Zu		Ported to linux

*******************************************************************************/

#ifndef __ROUND_ROBIN__
#define __ROUND_ROBIN__

#include <pthread.h> 
  
#define MAXSIZE 300  

#define QUE_OK 0
#define QUE_ERR -1

#define QUE_TURN 1
#define QUE_FALSE 0


#define PTHREAD_LOCK(u)  //pthread_mutex_lock(u)
#define PTHREAD_UNLOCK(u) //pthread_mutex_unlock(u)

//typedef char ElemType;
typedef unsigned char ElemType;

#define FRAME_BUF_Q_SIZE 6
#define TCP_BUF_Q_SIZE 700//400



#ifndef NULL
#define NULL 0 
#endif

typedef struct{
	unsigned char *cmd_para[FRAME_BUF_Q_SIZE];
	int front;
	int rear;
}PARA_BUF_QUEUE_T;

int para_q_is_full(PARA_BUF_QUEUE_T *q);
unsigned char *get_free_para_buf(PARA_BUF_QUEUE_T *q);
int para_process_buf_insert(PARA_BUF_QUEUE_T *q);
int para_q_is_empty(PARA_BUF_QUEUE_T *q);
unsigned char *get_process_para_buf(PARA_BUF_QUEUE_T *q);
int para_process_buf_delete(PARA_BUF_QUEUE_T *q);


typedef struct{
	unsigned char *tcp_buf[TCP_BUF_Q_SIZE];
	int front;
	int rear;
}TCP_BUF_QUEUE_T;


int tcp_q_is_full(TCP_BUF_QUEUE_T *q);
unsigned char *get_free_tcp_buf(TCP_BUF_QUEUE_T *q);
int tcp_process_buf_insert(TCP_BUF_QUEUE_T *q);
int tcp_q_is_empty(TCP_BUF_QUEUE_T *q);
unsigned char *get_process_tcp_buf(TCP_BUF_QUEUE_T *q);
int tcp_process_buf_delete(TCP_BUF_QUEUE_T *q);

typedef struct{
	unsigned int _q_size;
	unsigned _buf_size;
	unsigned char **_buf_p;
	int front;
	int rear;
}M_BUF_QUEUE_T;

M_BUF_QUEUE_T *m_buf_q_init(unsigned int q_size,unsigned int buf_size);
int m_q_buf_is_full(M_BUF_QUEUE_T *q);
unsigned char *get_m_free_q_buf(M_BUF_QUEUE_T *q);
int m_q_buf_insert(M_BUF_QUEUE_T *q);
int m_q_buf_is_empty(M_BUF_QUEUE_T *q);
unsigned char *get_m_process_q_buf(M_BUF_QUEUE_T *q);
int process_m_q_buf_delete(M_BUF_QUEUE_T *q);
int m_buf_q_deinit(M_BUF_QUEUE_T *q);

//====================================================

typedef struct{
	pthread_mutex_t mutex;
	unsigned int _q_size;
	unsigned int _buf_size;
	unsigned char **_buf_p;
	int front;
	int rear;
}_BUF_QUEUE_T;

_BUF_QUEUE_T *_buf_q_init(unsigned int q_size,unsigned int buf_size);
int _q_buf_is_full(_BUF_QUEUE_T *q);
unsigned char *get_free_q_buf(_BUF_QUEUE_T *q);
int q_buf_insert(_BUF_QUEUE_T *q,unsigned char *buf);
int _q_buf_is_empty(_BUF_QUEUE_T *q);
unsigned char *get_q_buf(_BUF_QUEUE_T *q);
int _buf_q_deinit(_BUF_QUEUE_T *q);

#endif
  
