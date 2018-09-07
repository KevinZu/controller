/******************************************************************************
                                                                            
 File Name:      inirw.c                      
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
#include <stdarg.h>
//include <varargs.h>
#include <stdio.h>
#include <string.h>
#include "inirw.h"

#define CONFMAXLINELEN	2048
#define CONFMAXVALUELEN	2048

//ȥ����β�ո���Ʊ��
static char * trim(char * pstr)
{
		int len;
		char * pRet;
		pRet = pstr;
		
		//ȥ���ײ��ո�
		while ( ((*pRet)==' ') || ((*pRet)=='\t') )
			pRet ++;
		
		//ȥ��β���ո�
		len = strlen(pRet);
		while (len>0){
			if ( (pRet[len-1]==' ') || (pRet[len-1]=='\t')  || (pRet[len-1]=='\r')  ||(pRet[len-1]=='\n') )
				len --;
			else
				break;
		}
		
		*(pRet + len) = 0;
		return pRet;
}


//int GetProfileItem(const char * lpcstrFileName, const char * lpcstrSectName, const char * lpcstrItemName, const char * lpcstrFormat, ...)
int get_profile_item(const char * lpcstrFileName, const char * lpcstrSectName, const char * lpcstrItemName, const char * lpcstrFormat, ...)
{
	FILE * pFile;
	char linebuf[CONFMAXLINELEN];
	char * pItem;
	char * pValue;
	char * pLine;
	int bSectOk;

	bSectOk = 0;
	/*���ļ�*/
	pFile = fopen(lpcstrFileName, "rt");
	if (pFile==NULL)goto errOpenFile;
	/*���ļ��еõ�ָ�����ֵ*/
	while (pLine = fgets(linebuf, CONFMAXLINELEN-1, pFile))
	{
		/*ȥ�����׵Ŀո���Ʊ��*/
		pLine = trim(pLine);		
		/*�Ƿ�Ϊ����*/
		if (strlen(pLine)<3)continue;
		/*�Ƿ�Ϊע����*/
		if ( (*pLine)=='#' )
			continue;
		/*��С������*/
		if ( ((*pLine)=='[' ) && (*(pLine + strlen(pLine)-1)==']') )
		{
			pLine ++;
			*(pLine + strlen(pLine)-1) = '\0';
			pLine = trim(pLine);
			if (strcasecmp(pLine, lpcstrSectName)==0)
			{
				bSectOk = 1;
			}
			else
			{
				bSectOk = 0;
			}
		}
		/*�ڸ�����С����*/
		if (bSectOk)
		{
			/*�� '=' ��*/
			pItem = pLine;
			if ((pValue = strchr(pLine, '=')) == NULL)
			{
				if ((pValue = strchr(pLine, ' ')) == NULL)
				{
					if ((pValue = strchr(pLine, '\t')) == NULL)
					{
						if ((pValue = strchr(pLine, '\0')) == NULL)continue;
					}
	//				else printf("mytest %s\n",pLine);
				}
			}
			/*�����������ֵ*/
			pLine = pValue;
			pValue ++;
			*pLine = '\0';
			pItem = trim(pItem);
			/*�Ǹ���������*/
			if (strcasecmp(pItem, lpcstrItemName)==0)
			{
				pValue = trim(pValue);
				break;
			}
		}
	}
	/*�ر��ļ�*/
	fclose(pFile);
	/*��ʽ������*/
	if (pLine && pValue)
	{
		va_list ap;
		int ret;
  		va_start( ap,  lpcstrFormat );
  		ret = vsscanf(pValue , lpcstrFormat, ap );
  		va_end( ap );
		return ret;
	}
	return 0;
errOpenFile:
	return -1;
}



int write_profile_item(const char * str_file_name, const char * str_sect_name, const char * str_item_name, int is_have, const char * str_format, ...)
{
	char outputlinebuf[CONFMAXLINELEN];
	char linebuf[CONFMAXLINELEN];
	char * pItem;
	char * pValue;
	char * pLine;
	int  bSectOk;
	int  bItemComplete;
	FILE * pFile, *pTempFile;
	//��������ַ���
	outputlinebuf[0] = 0;
	bSectOk = 0;
	bItemComplete = 0;{
		va_list ap;
  		va_start( ap,  str_format );
	 	vsnprintf( outputlinebuf, sizeof( outputlinebuf ), str_format, ap );
  		va_end( ap );
	}
	
	//���ļ�
	pFile = fopen(str_file_name, "rt");

	//������ʱ�ļ�
	pTempFile = fopen("/tmp/tmp", "wt");
	//pTempFile = fopen("tmp", "rt");
	if (pTempFile==NULL)goto errOpenTempFile;

	//�滻�µ�������
	if (pFile){
		while (pLine = fgets(linebuf, CONFMAXLINELEN-1, pFile)){
			char tempbuf[CONFMAXLINELEN];
			//������Ѿ�д�룬��򵥽���������ݸ��Ƶ���ʱ�ļ���
			if (bItemComplete){
				fprintf(pTempFile,"%s", pLine);
			    	continue;
			}
			strcpy(tempbuf, pLine);
			//ȥ�����׵Ŀո���Ʊ��
			pLine = trim(pLine);		
			//�Ƿ�Ϊ����
			if (strlen(pLine)<3){
				fprintf(pTempFile,"%s",  tempbuf);
			    	continue;
			}
			//�Ƿ�Ϊע����
			if ( (*pLine)=='#' ){
				fprintf(pTempFile,"%s",  tempbuf);
			    	continue;
			}
			//��С������
			if ( ((*pLine)=='[' ) && (*(pLine + strlen(pLine)-1)==']') ){
				//Ŀǰ��С����,����Ҫ�л�������һС�ڣ�����û���ҵ����ʱ����һ��
				if (bSectOk){
					if(is_have)
					  fprintf(pTempFile, "%s = %s\n", str_item_name, outputlinebuf);
					else
					  fprintf(pTempFile, "%s %s\n", str_item_name, outputlinebuf);
					bItemComplete = 1;
					fprintf(pTempFile, "%s", tempbuf);
			    		continue;
				}
				pLine ++;
				*(pLine + strlen(pLine)-1) = '\0';
				pLine = trim(pLine);
				if (strcasecmp(pLine, str_sect_name)==0){
					bSectOk = 1;
				}else{
					bSectOk = 0;
				}
			}
			//�ڸ�����С����
			if (!bSectOk){
				fprintf(pTempFile,"%s",  tempbuf);
			    	continue;
			}else{
				//�� '=' ��
				pItem = pLine;
				if ((pValue = strchr(pLine, '=')) == NULL){
					if ((pValue = strchr(pLine, ' ')) == NULL){
						if ((pValue = strchr(pLine, '\0')) == NULL){
								fprintf(pTempFile, "%s", tempbuf);
						    		continue;
							}
					}
				}
				//�����������ֵ
				pLine = pValue;
				pValue ++;
				*pLine = '\0';
				pItem = trim(pItem);
				//�Ǹ���������
				if (strcasecmp(pItem, str_item_name)==0){
					if(is_have)
					  fprintf(pTempFile, "%s = %s\n", str_item_name, outputlinebuf);
					else
					  fprintf(pTempFile, "%s %s\n", str_item_name, outputlinebuf);
					bItemComplete = 1;
				}else{
					fprintf(pTempFile,"%s",  tempbuf);
				}
			}
		}
	}
	//���������Ƿ�д�룿
	if (!bItemComplete){
		//ָ����С��û��������һ��С��
		if (!bSectOk){
			fprintf(pTempFile, "[%s]\n", str_sect_name);
		}
		if(is_have)
		  fprintf(pTempFile, "%s = %s\n", str_item_name, outputlinebuf);
		else
		  fprintf(pTempFile, "%s %s\n", str_item_name, outputlinebuf);

	}
	//�ر���ʱ�ļ�
	fclose(pTempFile);	
	//�ر������ļ�
	if (pFile)fclose(pFile);
	//����ʱ�ļ����������ļ���
//	rename("/tmp/tmp", str_file_name);
	system("wr cp \"\/tmp\/tmp\" \"\/root\/ini\/S0.ini\"");
	return 0;
errOpenTempFile:
	return -1;
}

