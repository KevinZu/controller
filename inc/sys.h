/******************************************************************************
                                                                            
 File Name:      sys.h                                       
 Copyright:    										
 Version:                                                                
 Description:    
 EMAIL: zukeqiang@gmail.com				
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20131203		Kevin.Zu		Create 

*******************************************************************************/
#ifndef __SYS_H__
#define __SYS_H__

//#define __DBG

#define OK 0
#define ERR -6
#define ERR_NOMEM -1
#define ERR_UNDEFINED_ERROR -2
#define ERR_BADPARAMETER -3
#define ERR_TIME_OUT -4
#define ERR_PARA -5
#define ERR_DELAY -6


////////////////////// RETURN ///////////////////////
#define PL_SUCCESS 0
#define PL_NOMEM -1
#define PL_UNDEFINED_ERROR -2
#define PL_BADPARAMETER -3
#define PL_TIME_OUT -4
#define PL_OPEN_ERR -5
#define PL_CLOSE_ERR -6

#define DATA_MOD(num,mod_size) ((num)%mod_size)



#define SOCK_DISABLE 0
#define SOCK_ENABLE 1


#define NO_ESL_TYPE			0
#define INDICATOR_ESL_TYPE	1
#define SCAN_ESL_TYPE		2
#define LIGHT_ESL_TYPE		3
#define PICK_ESL_TYPE		4


enum{
	NET_TYPE_485 = 1,
	NET_TYPE_CAN
};

#define PARA_BUF_SIZE 200
#define RECV_BUF_SIZE 4096

#define Tranverse16(X)                 ((((unsigned short)(X) & 0xff00) >> 8) |(((unsigned short)(X) & 0x00ff) << 8))

#ifndef NULL
#define NULL 0
#endif

//#define __DBG
#define __LOG_S

#ifdef __DBG
#define _DBG(x...)	printf(x)
#else
#define _DBG(x...)
#endif

#ifdef __DBG
#define IM_LOG(x...)
#else
#define IM_LOG(x...) printf(x)
#endif

#ifndef NO_CYC_LOCK

#define CYC_LOCK(P) pthread_mutex_lock(P)
#define CYC_UNLOCK(P) {pthread_mutex_unlock(P);usleep(10);}

#else

#define CYC_LOCK(P) 
#define CYC_UNLOCK(P)

#endif

enum{
	E_GET_SCAN_DATA = 0,
	E_GET_KEY_STATUS
};


#define __DEV_COM__

//#define __DEBUG__// __DEV_COM__
//#define __MONITOR__


#endif

