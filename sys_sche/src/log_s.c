#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "sys.h"

#include "log_s.h"



//log_st *log = NULL;
log_st *log_array[MAX_LOG_FILE_NUM] = {NULL};

/*
 * path——您要存储的文件路径；
 * size——单个文件的最大大小，如果超过该大小则新建新的文件用来存储；
 * level——日志输出方式:
		建议在上层限制其值的范围为0到3，
			0表示日志既不输出到屏幕也不创建文件和保存到文件，
			1表示日志保存到文件但不输出到屏幕，
			2表示日志既输出到屏幕也保存到文件，
			3表示日志只输出到文件而不创建文件和存入文件
 * num——日志文件命名方式
		非0表示以(int)time(NULL)作为文件名来保存文件，文件数量随着日志量的递增而递增；
		0表示以“.new”和“.bak”为文件名来保存文件，文件数量不超过两个，随着日志量的递增，旧的日志文件将被新的覆盖，更直观的说就是说.new”和“.bak”文件只保存最近的日志。
 */

//log_st *log_init(char *path, int size, int level, int num)
int log_init(char *path, int size, int level, int num)  
{  
	int index;
	log_st *log;
	
	char new_path[128] = {0};  
//	if (NULL == path || 0 == level) return NULL;  
	if (NULL == path || 0 == level) return -1;
//    log_st *log = (log_st *)malloc(sizeof(log_st));
	for(index = 0;index <MAX_LOG_FILE_NUM;index ++){
		if(log_array[index] == NULL)
			break;
	}
	if(index == MAX_LOG_FILE_NUM){
		printf("Log file num is out of range!\n");
		return -1;
	}
	else
	log_array[index] = (log_st *)malloc(sizeof(log_st));
	log = log_array[index];
	
	memset(log, 0, sizeof(log_st));  
	if (level != 3){  
        //the num use to control file naming  
		log->num = num;  
		if(num)  
			snprintf(new_path, 128, "%s%d", path, (int)time(NULL));  
		else  
			snprintf(new_path, 128, "%s.new", path);  
		IM_LOG("new_path:%s\n",new_path);
		if(-1 == (log->fd = open(new_path, O_RDWR|O_APPEND|O_CREAT|O_SYNC, S_IRUSR|S_IWUSR|S_IROTH))){  
			free(log);  
			log = NULL;  
			//return NULL;  
			return -1;
		}  
	}  
	strncpy(log->path, path, 128);  
	log->size = (size > 0 ? size:0);  
	log->level = (level > 0 ? level:0);  
	//return log;  
	return index;
}  


/*
 *
 */


void log_debug(/*log_st *log,*/int index,const char *msg, ...)  
{  
	log_st *log;
	va_list ap;  
	time_t now;  
	char *pos;  
	char _n = '\n';  
	char message[BUF_SIZE] = {0};  
	int nMessageLen = 0;
	int sz;

	if(index < 0)
		return;
	
	log = log_array[index];
	
	if(NULL == log || 0 == log->level) return;  
	now = time(NULL);  
	pos = ctime(&now);  
	sz = strlen(pos);  
	pos[sz-1]=']';  
	snprintf(message, BUF_SIZE, "[%s ", pos);  
	for (pos = message; *pos; pos++);  
	sz = pos - message;  
	va_start(ap, msg);  
	nMessageLen = vsnprintf(pos, BUF_SIZE - sz, msg, ap);  
	va_end(ap);  
	if (nMessageLen <= 0) return;  
	if (3 == log->level){  
		printf("%s\n", message);  
		return;  
	}  
	if (2 == log->level)  
		printf("%s\n", message);  
	write(log->fd, message, strlen(message));  
	write(log->fd, &_n, 1);  
	fsync(log->fd);  
}  



/*
 *
 */

void log_checksize(int index)  
{  
	log_st *log;
	struct stat stat_buf;  
	char new_path[128] = {0};  
	char bak_path[128] = {0};  

	if(index < 0)
		return;
	
	log = log_array[index];
	
	if(NULL == log || 3 == log->level || '\0' == log->path[0]) return;  
	memset(&stat_buf, 0, sizeof(struct stat));  
	fstat(log->fd, &stat_buf);  
	if(stat_buf.st_size > log->size){
		close(log->fd);  
		if(log->num)  
			snprintf(new_path, 128, "%s%d", log->path, (int)time(NULL));  
		else{  
			snprintf(bak_path, 128, "%s.bak", log->path);
			snprintf(new_path, 128, "%s.new", log->path);
			remove(bak_path); //delete the file *.bak first
			rename(new_path, bak_path); //change the name of the file *.new to *.bak  
		}  
	        //create a new file  
		log->fd = open(new_path, O_RDWR|O_APPEND|O_CREAT|O_SYNC, S_IRUSR|S_IWUSR|S_IROTH);  
	}  
}  

/*
 *
 */

