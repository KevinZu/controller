/******************************************************************************
                                                                            
 File Name:      independ_process.h                                             
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com				
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20140709		Kevin.Zu		Create 

*******************************************************************************/
#ifndef __INDEPEND_PROCESS_H__
#define __INDEPEND_PROCESS_H__

#include "sys.h"
#include "SDL.h"
#include "esl_attr.h"

typedef int (*INDEPEND_PROCESS_FUN_T)(void *para);

enum{
	INDEP_PROCESS_SELF_TEST = 0
};

struct process_fun_info{
	INDEPEND_PROCESS_FUN_T func;
	SDL_Thread *thread;
};


typedef struct independ_process{
	unsigned int func_array_len;
	struct process_fun_info *func_info_array;
	esl_attr_hlist_t *esl_attr;

	int (*create_process)(struct independ_process *,unsigned int,void *);
}independ_process_t;



#endif

