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

//去掉首尾空格或制表符
static char * trim(char * pstr)
{
		int len;
		char * pRet;
		pRet = pstr;
		
		//去掉首部空格
		while ( ((*pRet)==' ') || ((*pRet)=='\t') )
			pRet ++;
		
		//去掉尾部空格
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
	/*打开文件*/
	pFile = fopen(lpcstrFileName, "rt");
	if (pFile==NULL)goto errOpenFile;
	/*从文件中得到指定项的值*/
	while (pLine = fgets(linebuf, CONFMAXLINELEN-1, pFile))
	{
		/*去掉行首的空格或制表符*/
		pLine = trim(pLine);		
		/*是否为空行*/
		if (strlen(pLine)<3)continue;
		/*是否为注释行*/
		if ( (*pLine)=='#' )
			continue;
		/*是小节名吗*/
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
		/*在给定的小节中*/
		if (bSectOk)
		{
			/*有 '=' 吗？*/
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
			/*分离出项名和值*/
			pLine = pValue;
			pValue ++;
			*pLine = '\0';
			pItem = trim(pItem);
			/*是给定的项吗*/
			if (strcasecmp(pItem, lpcstrItemName)==0)
			{
				pValue = trim(pValue);
				break;
			}
		}
	}
	/*关闭文件*/
	fclose(pFile);
	/*格式化返回*/
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
	//生成输出字符串
	outputlinebuf[0] = 0;
	bSectOk = 0;
	bItemComplete = 0;{
		va_list ap;
  		va_start( ap,  str_format );
	 	vsnprintf( outputlinebuf, sizeof( outputlinebuf ), str_format, ap );
  		va_end( ap );
	}
	
	//打开文件
	pFile = fopen(str_file_name, "rt");

	//生成临时文件
	pTempFile = fopen("/tmp/tmp", "wt");
	//pTempFile = fopen("tmp", "rt");
	if (pTempFile==NULL)goto errOpenTempFile;

	//替换新的配置项
	if (pFile){
		while (pLine = fgets(linebuf, CONFMAXLINELEN-1, pFile)){
			char tempbuf[CONFMAXLINELEN];
			//如果项已经写入，则简单将后面的内容复制到临时文件中
			if (bItemComplete){
				fprintf(pTempFile,"%s", pLine);
			    	continue;
			}
			strcpy(tempbuf, pLine);
			//去掉行首的空格或制表符
			pLine = trim(pLine);		
			//是否为空行
			if (strlen(pLine)<3){
				fprintf(pTempFile,"%s",  tempbuf);
			    	continue;
			}
			//是否为注释行
			if ( (*pLine)=='#' ){
				fprintf(pTempFile,"%s",  tempbuf);
			    	continue;
			}
			//是小节名吗
			if ( ((*pLine)=='[' ) && (*(pLine + strlen(pLine)-1)==']') ){
				//目前在小节中,马上要切换到另外一小节，但是没有找到项，此时增加一项
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
			//在给定的小节中
			if (!bSectOk){
				fprintf(pTempFile,"%s",  tempbuf);
			    	continue;
			}else{
				//有 '=' 吗？
				pItem = pLine;
				if ((pValue = strchr(pLine, '=')) == NULL){
					if ((pValue = strchr(pLine, ' ')) == NULL){
						if ((pValue = strchr(pLine, '\0')) == NULL){
								fprintf(pTempFile, "%s", tempbuf);
						    		continue;
							}
					}
				}
				//分离出项名和值
				pLine = pValue;
				pValue ++;
				*pLine = '\0';
				pItem = trim(pItem);
				//是给定的项吗
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
	//给定的项是否写入？
	if (!bItemComplete){
		//指定的小节没有则生成一个小节
		if (!bSectOk){
			fprintf(pTempFile, "[%s]\n", str_sect_name);
		}
		if(is_have)
		  fprintf(pTempFile, "%s = %s\n", str_item_name, outputlinebuf);
		else
		  fprintf(pTempFile, "%s %s\n", str_item_name, outputlinebuf);

	}
	//关闭临时文件
	fclose(pTempFile);	
	//关闭配置文件
	if (pFile)fclose(pFile);
	//将临时文件复制配置文件中
//	rename("/tmp/tmp", str_file_name);
	system("wr cp \"\/tmp\/tmp\" \"\/root\/ini\/S0.ini\"");
	return 0;
errOpenTempFile:
	return -1;
}

