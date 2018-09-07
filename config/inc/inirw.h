/******************************************************************************
                                                                            
 File Name:      inirw.h                
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 shichen
 Kevin.Zu                         zukeqiang@gmail.com			
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE			NAME           	DESCRIPTION                               
 20060201		shichen		Create 
 20131205		Kevin.Zu		Add to ELS Controller project

*******************************************************************************/
#ifndef __INIRW_H__
#define __INIRW_H__


typedef struct cfg_file_operation{
	char *file_name;

	int (*init)(struct cfg_file_operation *op);
	int (*deinit)(struct cfg_file_operation *op);
	int (*write_item)(struct cfg_file_operation *op,const char * lpcstrSectionName, const char * lpcstrItemName, const char * lpcstrFormat, ...);
	int (*get_item)(struct cfg_file_operation *op,const char * lpcstrSectName, const char * lpcstrItemName, int is_have, const char * lpcstrFormat, ...);
}cfg_file_operation_t;

int get_profile_item(const char * lpcstrFileName, const char * lpcstrSectionName, const char * lpcstrItemName, const char * lpcstrFormat, ...);
int write_profile_item(const char * lpcstrFileName, const char * lpcstrSectName, const char * lpcstrItemName, int is_have, const char * lpcstrFormat, ...);



#endif

