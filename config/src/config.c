/******************************************************************************
                                                                            
 File Name:      config.c                                    
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
#include <string.h>
#include "sys.h"
#include "config.h"
#include "inirw.h"

int cfg_get_net_type(void)
{
	int ret;
	char net_name[50];

	memset(net_name,0,50);
	
	ret = get_profile_item(CONFIG_FILE_NAME,"NET","TYPE","%s",net_name);
	if(ret < 0){
		printf("!! Get net name err! ret:%d\n",ret);
		return ret;
	}
	ret = strcmp(net_name,"CAN");
	if(ret == OK){
		return NET_TYPE_CAN;
	}

	ret = strcmp(net_name,"can");
	if(ret == OK){
		return NET_TYPE_CAN;
	}

	ret = strcmp(net_name,"485");
	if(ret == OK){
		return NET_TYPE_485;
	}

	return ERR;
}


int cfg_get_dev_name(char *dev_name)
{
	if(dev_name == NULL)
		return ERR_PARA;
	
	return get_profile_item(CONFIG_FILE_NAME,"SERIAL","DEVICE","%s",dev_name);
}


int cfg_get_dev_name_can(char *dev_name)
{
	if(dev_name == NULL)
		return ERR_PARA;
	
	return get_profile_item(CONFIG_FILE_NAME,"CAN","DEVICE","%s",dev_name);
}

int cfg_get_dev_num_can(unsigned int *num)
{
	if(num == NULL)
		return ERR_PARA;
	
	return get_profile_item(CONFIG_FILE_NAME,"CAN","CAN_NUM","%u",num);
}

int cfg_get_serial_num(unsigned int *num)
{
	
	if(num == NULL)
		return ERR_PARA;
	
	return get_profile_item(CONFIG_FILE_NAME,"SERIAL","SERIAL_NUM","%u",num);
}

int cfg_get_esl_num(const unsigned char comx,unsigned int *num)
{
	char sect_name[100];
	
	if(num == NULL)
		return ERR_PARA;

	memset(sect_name,0,100);
	sprintf(sect_name,"COM%d",comx);
	
	return get_profile_item(CONFIG_FILE_NAME,sect_name,"ESL_NUM","%u",num);
}

int cfg_get_spec_com_attr(const unsigned int comx,const unsigned int no,unsigned short *id,unsigned char *type,unsigned int *shelf_no)
{
	int ret;
	char sect_name[100];
	char type_str[100];
	char esl_no[100];

	char id_str[10];
	char shelf_str[10];
	unsigned int tt;

	if(id == NULL || type == NULL || shelf_no == NULL)
		return ERR_PARA;

	memset(sect_name,0,100);
	sprintf(sect_name,"COM%d",comx);
	
	memset(type_str,0,100);
	memset(id_str,0,10);
	memset(shelf_str,0,10);
	
	
	memset(esl_no,0,100);
	sprintf(esl_no,"%d",no);
	
	
	ret = get_profile_item(CONFIG_FILE_NAME,sect_name,esl_no,"%d %s %d",&tt,type_str,shelf_no);
	if(ret < 0)
		return ret;
	
	_DBG("+++ type:%s     shelf:%d  tt:%d\n",type_str,*shelf_no,tt);
	*id = (unsigned short)tt;
	
	if(strcmp(type_str,"DK") == 0)
		*type = LIGHT_ESL_TYPE;
	else if(strcmp(type_str,"SM") == 0)
		*type = SCAN_ESL_TYPE;
	else if(strcmp(type_str,"ZS") == 0)
		*type = INDICATOR_ESL_TYPE;
	else if(strcmp(type_str,"JH") == 0)
		*type = PICK_ESL_TYPE;
	else
		*type = NO_ESL_TYPE;
	return OK;
}



//--------------- seting ------------------

//int cfg_set_net_type(void)
//{

//}


//int cfg_set_dev_name(char *dev_name)
//{
	
//}


//int cfg_set_com_num(unsigned char *num)
//{

//}


int cfg_set_esl_num(unsigned char comx,unsigned int num)
{
	//write_profile_item();
	int ret;
	char sect_name[100];
	char str[20];


	memset(sect_name,0,100);
	memset(str,0,20);

	if(comx > _MAX_COM_NUM - 1)
		comx = _MAX_COM_NUM - 1;

	if(num > _MAX_ESL_NUM - 1)
		num =  _MAX_ESL_NUM - 1;

	sprintf(sect_name,"COM%d",comx);
	sprintf(str,"%d",num);

	return write_profile_item(CONFIG_FILE_NAME,sect_name,"ESL_NUM",1,"%s",str);
	
}


int cfg_set_spec_com_attr(unsigned int comx,unsigned int no,unsigned short id,unsigned char type,unsigned int shelf_no)
{
	int ret;
	char sect_name[100];
	char type_str[100];
	char esl_no[100];

	if(id > 9999)
		id = 9999;

	if(no > _MAX_ESL_NUM - 1)
		no =  _MAX_ESL_NUM - 1;

	if(comx > _MAX_COM_NUM - 1)
		comx = _MAX_COM_NUM - 1;

	memset(sect_name,0,100);
	sprintf(sect_name,"COM%d",comx);
	
	memset(type_str,0,100);
	
	
	memset(esl_no,0,100);
	sprintf(esl_no,"%d",no);

	switch(type){
		case LIGHT_ESL_TYPE:
			sprintf(type_str,"DK");
			break;
		case SCAN_ESL_TYPE:
			sprintf(type_str,"SM");
			break;
		case PICK_ESL_TYPE:
			sprintf(type_str,"JH");
			break;
		case INDICATOR_ESL_TYPE:
			sprintf(type_str,"ZS");
			break;
		default:
			break;
	}
	
	
	ret = write_profile_item(CONFIG_FILE_NAME,sect_name,esl_no,0,"%d %s %d",id,type_str,shelf_no);
	if(ret < 0)
		return ret;
}


//int cfg_set_dev_name_can(char *dev_name)
//{

//}


int cfg_set_dev_num_can(unsigned int num)
{
	char str[20];
	
	memset(str,0,20);
	if(num > _MAX_COM_NUM)
		num = _MAX_COM_NUM;
	
	sprintf(str,"%d",num);
	
	return write_profile_item(CONFIG_FILE_NAME,"CAN","CAN_NUM",1,"%s",str);
}





