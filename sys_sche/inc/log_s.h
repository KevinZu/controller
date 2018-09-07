#ifndef __LOG_S__
#define __LOG_S__

#define BUF_SIZE 1024  
#define LOG_FILE_SIZE 4096

#define __NOPRINT_NOCTEAT_NOSAVE	0
#define __NOPRINT_SAVE				1
#define __PRINT_SAVE					2
#define __OTHER						3


#define MAX_LOG_FILE_NUM 5

typedef struct _log_st log_st;  
struct _log_st  
{  
    char path[128];
    int fd;
    int size;
    int level;
    int num;
};  

#if 0
log_st *log_init(char *path, int size, int level, int num);
void log_debug(log_st *log, const char *msg, ...);
void conflicting(log_st *log);
#endif

int log_init(char *path, int size, int level, int num);
void log_debug(int index,const char *msg, ...);
void conflicting(int index);


#ifdef __LOG_S
#define LOG_INIT(path,size,level,num)  log_init(path,size,level,num)
#define LOG_PRINT(x...) log_debug(x)
#define LOG_CHECK(index) conflicting(index)
#else
#define LOG_INIT(path,size,level,num) (NULL)
#define LOG_PRINT(x...)
#define LOG_CHECK(index)
#endif

#endif

