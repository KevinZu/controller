/******************************************************************************
                                                                            
 File Name:      round_robin_queue.c                                                
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
#include <string.h>
#include "round_robin_queue.h"
#include "sys.h"

//extern void ENTER_CRITICAL() ;
//etern void EXIT_CRITICAL() ;





/********************** frame buffer queue ******************************/

int para_q_is_full(PARA_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;
	
	if(q->front == (q->rear + 1) % FRAME_BUF_Q_SIZE) //
		return QUE_TURN;  
	else  
		return QUE_FALSE;  
}

unsigned char *get_free_para_buf(PARA_BUF_QUEUE_T *q)
{
	if(!para_q_is_full(q))
		return q->cmd_para[q->rear];
	else
		return NULL;
}

int para_process_buf_insert(PARA_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;  
	
	if(para_q_is_full(q))
		return QUE_ERR;  

//	IM_LOG("- 0x%x  %d %d \n",q,q->front,q->rear);
	q->rear = (q->rear + 1) % FRAME_BUF_Q_SIZE; //
//	IM_LOG("- 0x%x  %d %d \n",q,q->front,q->rear);
	
	return QUE_OK;  

}

int para_q_is_empty(PARA_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;
	
	if(q->front == q->rear) //
		return QUE_TURN;  
	else  
		return QUE_FALSE;  
}

unsigned char *get_process_para_buf(PARA_BUF_QUEUE_T *q)
{
	if(!para_q_is_empty(q))
		return q->cmd_para[q->front];
	else
		return NULL;
}

int para_process_buf_delete(PARA_BUF_QUEUE_T *q)
{  
	if(q == NULL)
		return QUE_ERR;
	
	if(para_q_is_empty(q))
		return QUE_ERR;
 // 	IM_LOG("+ 0x%x  %d %d \n",q,q->front,q->rear);
	q->front  = (q->front  + 1) % FRAME_BUF_Q_SIZE;  
//	IM_LOG("+ 0x%x  %d %d \n",q,q->front,q->rear);
	return QUE_OK;	
}

/********************** buffer queue ******************************/
M_BUF_QUEUE_T *m_buf_q_init(unsigned int q_size,unsigned int buf_size)
{
	int i;
	M_BUF_QUEUE_T *q;
	
	if(q_size == 0 || buf_size == 0){
		printf("%s: err para!\n",__FUNCTION__);
		return ERR_PARA;
	}
	
	q = (M_BUF_QUEUE_T *)malloc(sizeof(M_BUF_QUEUE_T));
	if(q == NULL){
		printf("%s:  _BUF_QUEUE_T malloc err!\n",__FUNCTION__);
		return NULL;
	}

	q->_buf_size = buf_size;
	q->_q_size = q_size;

	q->_buf_p = (unsigned char **)malloc(sizeof(unsigned char *) * q->_q_size);
	if(q->_buf_p == NULL){
		printf("%s:  q->_buf_p malloc err!\n",__FUNCTION__);
		return NULL;
	}

	for(i = 0;i < q->_q_size;i ++){
		q->_buf_p[i] = (unsigned char *)malloc(q->_buf_size);
		if(q->_buf_p[i] == NULL){
			printf("%s:  buf_q->_buf_p[i] malloc err!\n",__FUNCTION__);
			return NULL;
		}
	}

	q->front = 0;
	q->rear = 0;

	return q;
}

int m_q_buf_is_full(M_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;
	
	if(q->front == (q->rear + 1) % q->_buf_size) //
		return QUE_TURN;  
	else  
		return QUE_FALSE;  
}

unsigned char *get_m_free_q_buf(M_BUF_QUEUE_T *q)
{
	if(!m_q_buf_is_full(q)){
		return q->_buf_p[q->rear];
	}else
		return NULL;
}

int m_q_buf_insert(M_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;  
	
	if(m_q_buf_is_full(q))
		return QUE_ERR;  


	q->rear = (q->rear + 1) % q->_buf_size; //

	
	return QUE_OK;  

}

int m_q_buf_is_empty(M_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;
	
	if(q->front == q->rear) //
		return QUE_TURN;  
	else  
		return QUE_FALSE;  
}

unsigned char *get_m_process_q_buf(M_BUF_QUEUE_T *q)
{
	if(!m_q_buf_is_empty(q)){

		return q->_buf_p[q->front];
	}else
		return NULL;
}

int process_m_q_buf_delete(M_BUF_QUEUE_T *q)
{  
	if(q == NULL)
		return QUE_ERR;
	
	if(m_q_buf_is_empty(q))
		return QUE_ERR;

	q->front  = (q->front  + 1) % q->_buf_size;  


	
	return QUE_OK;	
}

int m_buf_q_deinit(M_BUF_QUEUE_T *q)
{
	int i;
	
	if(q == NULL){
		printf("%s: err para!\n",__FUNCTION__);
		return ERR_PARA;
	}

	for(i = 0;i < q->_q_size;i ++){
		if(q->_buf_p[i] != NULL){
			free(q->_buf_p[i]);
		}
	}

	if(q->_buf_p != NULL){
		free(q->_buf_p);
	}

	free(q);
	q = NULL;

	return OK;
}



/********************** tcp recv buffer queue ******************************/

int tcp_q_is_full(TCP_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;
	
	if(q->front == (q->rear + 1) % TCP_BUF_Q_SIZE) //
		return QUE_TURN;  
	else  
		return QUE_FALSE;  
}

unsigned char *get_free_tcp_buf(TCP_BUF_QUEUE_T *q)
{
	if(!tcp_q_is_full(q)){
		return q->tcp_buf[q->rear];
	}else
		return NULL;
}

int tcp_process_buf_insert(TCP_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;  
	
	if(tcp_q_is_full(q))
		return QUE_ERR;  


	q->rear = (q->rear + 1) % TCP_BUF_Q_SIZE; //

	
	return QUE_OK;  

}

int tcp_q_is_empty(TCP_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;
	
	if(q->front == q->rear) //
		return QUE_TURN;  
	else  
		return QUE_FALSE;  
}

unsigned char *get_process_tcp_buf(TCP_BUF_QUEUE_T *q)
{
	if(!tcp_q_is_empty(q)){


		return q->tcp_buf[q->front];
	}else
		return NULL;
}

int tcp_process_buf_delete(TCP_BUF_QUEUE_T *q)
{  
	if(q == NULL)
		return QUE_ERR;
	
	if(tcp_q_is_empty(q))
		return QUE_ERR;

	q->front  = (q->front  + 1) % TCP_BUF_Q_SIZE;  


	
	return QUE_OK;	
}



/********************** buffer queue ******************************/
_BUF_QUEUE_T *_buf_q_init(unsigned int q_size,unsigned int buf_size)
{
	int i,ret;
	//unsigned int count = 0;
	unsigned char *buf;
	_BUF_QUEUE_T *q;
	
	if(q_size == 0 || buf_size == 0){
		printf("%s: err para!\n",__FUNCTION__);
		return ERR_PARA;
	}
	
	q = (_BUF_QUEUE_T *)malloc(sizeof(_BUF_QUEUE_T));
	if(q == NULL){
		printf("%s:  _BUF_QUEUE_T malloc err!\n",__FUNCTION__);
		return NULL;
	}

	pthread_mutex_init(&q->mutex,NULL);

	q->_buf_size = buf_size;
	q->_q_size = q_size;

	q->front = 0;
	q->rear = 0;

	q->_buf_p = (unsigned char **)malloc(sizeof(unsigned char *) * q->_q_size);
	if(q->_buf_p == NULL){
		printf("%s:  q->_buf_p malloc err!\n",__FUNCTION__);
		return NULL;
	}

	do{
		buf = (unsigned char *)malloc(q->_buf_size);
		if(buf == NULL){
			printf("%s:  buf_q->_buf_p[i] malloc err!\n",__FUNCTION__);
			return NULL;
		}
		ret = q_buf_insert(q,buf);
		//count ++;
		//_DBG("+++++q_size:%d  count:%d\n",q->_q_size,count);
	}while(ret == QUE_OK);

	free(buf);

	return q;
}

int _q_buf_is_full(_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;
	
	if(q->front == (q->rear + 1) % q->_q_size)
		return QUE_TURN;  
	else  
		return QUE_FALSE;  
}

unsigned char *get_free_q_buf(_BUF_QUEUE_T *q)
{
	if(!_q_buf_is_full(q)){
		return q->_buf_p[q->rear];
	}else
		return NULL;
}

int q_buf_insert(_BUF_QUEUE_T *q,unsigned char *buf)
{
	if(q == NULL)
		return QUE_ERR;  
	
	if(_q_buf_is_full(q))
		return QUE_ERR;  

	PTHREAD_LOCK(&q->mutex);
	q->_buf_p[q->rear] = buf;  
	q->rear = (q->rear + 1) % q->_q_size;
	PTHREAD_UNLOCK(&q->mutex);
	usleep(100);
//	EXIT_CRITICAL();

	
	return QUE_OK;  

}

int _q_buf_is_empty(_BUF_QUEUE_T *q)
{
	if(q == NULL)
		return QUE_ERR;
	
	if(q->front == q->rear) //
		return QUE_TURN;  
	else  
		return QUE_FALSE;  
}



unsigned char *get_q_buf(_BUF_QUEUE_T *q)
{
	unsigned char *buf;
	
	if(q == NULL)
		return NULL;
	
	if(_q_buf_is_empty(q))
		return NULL;

	PTHREAD_LOCK(&q->mutex);
	buf = q->_buf_p[q->front];  
	q->front  = (q->front  + 1) % q->_q_size;
	PTHREAD_UNLOCK(&q->mutex);
	usleep(100);

	
	return buf;	
}

int _buf_q_deinit(_BUF_QUEUE_T *q)
{
	int i;
	
	if(q == NULL){
		printf("%s: err para!\n",__FUNCTION__);
		return ERR_PARA;
	}

	pthread_mutex_destroy(&q->mutex);

	for(i = 0;i < q->_q_size;i ++){
		if(q->_buf_p[i] != NULL){
			free(q->_buf_p[i]);
		}
	}

	if(q->_buf_p != NULL){
		free(q->_buf_p);
	}

	free(q);
	q = NULL;

	return OK;
}



