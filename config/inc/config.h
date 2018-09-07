/******************************************************************************
                                                                            
 File Name:      config.h                                       
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com			
                                                                            
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20131205		Kevin.Zu		Create 

*******************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__


#define CONFIG_FILE_NAME "/root/ini/S0.ini"
#define CONFIG_CACHE_FILE "/tmp/tmp"

#define _MAX_COM_NUM 3
#define _MAX_ESL_NUM 150


int cfg_get_net_type(void);
int cfg_get_dev_name(char *dev_name);
int cfg_get_serial_num(unsigned int *num);
int cfg_get_esl_num(const unsigned char comx,unsigned int *num);
int cfg_get_spec_com_attr(const unsigned int comx,const unsigned int no,unsigned short *id,unsigned char *type,unsigned int *shelf_no);
int cfg_get_dev_name_can(char *dev_name);
int cfg_get_dev_num_can(unsigned int *num);


//int cfg_set_net_type(void);
//int cfg_set_dev_name(char *dev_name);
//int cfg_set_serial_num(unsigned int *num);
int cfg_set_esl_num(const unsigned char comx,unsigned int num);
int cfg_set_spec_com_attr(const unsigned int comx,const unsigned int no,unsigned short id,unsigned char type,unsigned int shelf_no);
//int cfg_set_dev_name_can(char *dev_name);
int cfg_set_dev_num_can(unsigned int num);

#endif

