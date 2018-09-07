/******************************************************************************
                                                                            
 File Name:      esl_protocol.c                                             
 Copyright:    										
 Version:                                                                
 Description:    
 
 Author:				EMAIL: 
 Kevin.Zu                          zukeqiang@gmail.com		
 
*******************************************************************************
 Revision History
 ------------------------------------------------------------------------- 
 DATE           NAME           	DESCRIPTION                               
 20130621		Kevin.Zu		Create
 20131205		Kevin.Zu		Ported to linux

*******************************************************************************/
#include <string.h>
#include "sys.h"
#include "esl_proto.h"


extern esl_protocol_t g_esl_proto;

#ifndef UINT16
#define UINT16 unsigned short
#endif

#ifndef UInt32
#define UInt32 unsigned int
#endif

#define Tranverse16(X)                 ((((UINT16)(X) & 0xff00) >> 8) |(((UINT16)(X) & 0x00ff) << 8))
#define Tranverse32(X)                 ((((UInt32)(X) & 0xff000000) >> 24) | (((UInt32)(X) & 0x00ff0000) >> 8) | (((UInt32)(X) & 0x0000ff00) << 8) | (((UInt32)(X) & 0x000000ff) << 24))

/*****************************************************                       
 *				 char get_cr(unsigned char *pData,int uLen)
 * Description:	
 * Arguments  :
 * Parameter :	unsigned char *pData£º
				unsigned char uLen£º
 * Returns    :  
 ******************************************************/
static unsigned char get_sm(unsigned char *pData,int uLen)
{
	unsigned char fcc = 0x00;
#if 1 //def __GET_CR__
	unsigned char i = 0;

	for (i = 0;i < uLen; i ++){
		fcc += *(pData + i);
	}

	fcc = 0xff - fcc + 0x01;
#endif

	return fcc;

}

//=====================================================================================
//                                                                           485 CMD process
//=====================================================================================


static unsigned int create_ack_frame(const unsigned char *src,unsigned int len,unsigned char *frame)
{
	unsigned int add_len = 0;
	unsigned int i;

	*frame = _START_CHAR;
	frame ++;
	*frame = _START_CHAR;
	frame ++;

	add_len += 2;

	for(i = 0;i < len; i ++){
		*frame = *(src + i);
		frame ++;
		if(*(src + i) == _START_CHAR || *(src + i) == _STOP_CHAR){
			*frame = _PAD_CHAR;
			frame ++;
			add_len ++;
		}
	}

	*frame = _STOP_CHAR;
	frame ++;
	*frame = _STOP_CHAR;
	frame ++;

	add_len += 2;
	
	return add_len;
}




/***********************build CMD function ****************************/


#define set_frame_head(frame_buf,addr,para_len,cmd_code) \
{ \
	frame_buf[0] = FRAME_START_ID; \
	frame_buf[1] = (unsigned char)(addr >> 8); \
	frame_buf[2] = (unsigned char)addr; \
	frame_buf[3] = cmd_code; \
	frame_buf[4] = para_len; \
}

#define para_len_index (sizeof(cmd_head_t) - 1)


static int modify_addr_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
//	esl_protocol_t *proto;
//	unsigned short addr;
        //unsigned char tbuf[2] = {0x03,0x04};

//	addr = buf[sizeof(cmd_head_t)] * 256 + buf[sizeof(cmd_head_t) + 1];//*(unsigned short *)(buf + sizeof(cmd_head_t));

//	proto = &g_esl_proto;
	//proto->esl_addr = addr;
	

	return OK;
}

static int check_state_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

	return OK;
}


static int key_queue_control_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

	return OK;
}

static int get_key_result_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	return OK;
}


static int system_reset_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	char buf[100];
	unsigned int headlen;
	unsigned int buf_len;
	
	if(frame == NULL)
		return ERR_PARA;

	memset(buf,0,100);

	headlen = sizeof(ack_head_t);
	set_frame_head(buf,id,0,cmd); 
	buf[headlen] = get_sm(buf,headlen);
	buf_len = headlen + 1;
	*frame_len = create_ack_frame(buf,buf_len,frame) + buf_len;
	
	return OK;
}




static int change_template_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
//	unsigned char cur_template;

	return OK;
}


static int update_display_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	unsigned int headlen,i;
	char buf[100];
	unsigned int buf_len;
	
	if(frame == NULL)
		return ERR_PARA;

	memset(buf,0,100);
	headlen = sizeof(ack_head_t);
	
	set_frame_head(buf,id,para_len,cmd); 
	buf[headlen] = para[0] - 0x30;

	buf[headlen + 1] = 0x55;
	
	//memcpy(buf + headlen + 1,para + 1,para_len - 1);
	memcpy(buf + headlen + 2,para + 1,para_len - 1);
	buf[headlen + para_len] = get_sm(buf,headlen + para_len);
	buf_len = headlen + para_len + 1;
	 *frame_len = create_ack_frame(buf,buf_len,frame) + buf_len;
	
	return OK;
}


static int lcd_get_item_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	//char data_type;
	unsigned int headlen,i;
	char buf[100];
	unsigned int buf_len;
	unsigned int ParaLen;
	unsigned char data_item_no;

	//data_type = *para;
	if(para == NULL){
		printf("para err! func: %s\n",__FUNCTION__);
		return ERR_PARA;
	}
	data_item_no = *para;

	if(frame == NULL){
		printf("frame err! func: %s\n",__FUNCTION__);
		return ERR_PARA;
	}

	memset(buf,0,100);
	headlen = sizeof(ack_head_t);
	ParaLen = 1;
	
	set_frame_head(buf,id,ParaLen,cmd); 
	buf[headlen] = data_item_no;
	
	buf[headlen + 1] = get_sm(buf,headlen + 1);
	buf_len = headlen + 2;
	 *frame_len = create_ack_frame(buf,buf_len,frame) + buf_len;
	
	return OK;
}


static int get_led_flash_status_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

	return OK;
}

static int led_gr_control_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	return OK;
}

static int select_voice_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	return OK;
}

static int get_scan_code_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	char buf[100];
	unsigned int headlen;
	unsigned int buf_len;
	
	if(frame == NULL)
		return ERR_PARA;

	memset(buf,0,100);

	headlen = sizeof(ack_head_t);
	set_frame_head(buf,id,0,cmd); 
	buf[headlen] = get_sm(buf,headlen);
	buf_len = headlen + 1;
	*frame_len = create_ack_frame(buf,buf_len,frame) + buf_len;
	
	return OK;
}


static int led_ctrl_485(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	unsigned int headlen,i;
	char buf[100];
	unsigned int buf_len;
	
	if(frame == NULL)
		return ERR_PARA;

	memset(buf,0,100);
	headlen = sizeof(ack_head_t);
	
	set_frame_head(buf,id,para_len,cmd); 
	buf[headlen] = para[0];
	
	buf[headlen + para_len] = get_sm(buf,headlen + para_len);
	buf_len = headlen + para_len + 1;
	 *frame_len = create_ack_frame(buf,buf_len,frame) + buf_len;
	
	return OK;
}

/************************* esl build func array **************************/
func_node_t esl_cmd_func_node_array_485[] = 
{
	{CMD_SYS_RESET,system_reset_485,{NULL,NULL}},
	{CMD_MODIFY_ADDR,modify_addr_485,{NULL,NULL}},
	{CMD_UPDATE_DISPLAY,update_display_485,{NULL,NULL}},
	{CMD_LCD_GET_ITEM,lcd_get_item_485,{NULL,NULL}},
	{CMD_CODE_SCAN,get_scan_code_485,{NULL,NULL}},
	{CMD_LED_CONTROL,led_ctrl_485,{NULL,NULL}},

	{0,NULL,{NULL,NULL}}
};

//=====================================================================================
//                                                                       CAN CMD process
//=====================================================================================

/***********************build CMD function ****************************/
static int modify_addr_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	int i;

	if(frame == NULL)
		return ERR_PARA;

	frame[0] = cmd;

	for(i = 0;i < para_len;i ++){
		frame[i + 1] = para[i];
	}
	
	*frame_len = para_len + 1;
	
	return OK;
}

static int check_state_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

	return OK;
}


static int key_queue_control_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

	return OK;
}

static int get_key_result_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	return OK;
}


static int system_reset_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

#if 1
//	unsigned int buf_len;

	_DBG("++++++ %s:%d \n",__FUNCTION__,__LINE__);


	if(frame == NULL)
		return ERR_PARA;

	frame[0] = cmd;
	*frame_len = 1;

	return OK;
#else
	char buf[100];
	unsigned int headlen;
	unsigned int buf_len;
	
	if(frame == NULL)
		return ERR_PARA;

	memset(buf,0,100);

	headlen = sizeof(ack_head_t);
	set_frame_head(buf,id,0,cmd); 
	buf[headlen] = get_sm(buf,headlen);
	buf_len = headlen + 1;
	*frame_len = create_ack_frame(buf,buf_len,frame) + buf_len;

	return OK;
#endif
}






static int change_template_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
//	unsigned char cur_template;

//	cur_template =  buf[sizeof(cmd_head_t)];

	return OK;
}


static int update_display_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	int i;
	
	if(frame == NULL)
		return ERR_PARA;

	frame[0] = cmd;

	for(i = 0;i < para_len;i ++){
		frame[i + 1] = para[i];
	}
	
	*frame_len = para_len + 1;
	
	return OK;
}


static int lcd_get_item_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	//char data_type;
	unsigned int headlen,i;
	char buf[100];
	unsigned int buf_len;
	unsigned int ParaLen;
	unsigned char data_item_no;

	//data_type = *para;
	if(para == NULL){
		printf("para err! func: %s\n",__FUNCTION__);
		return ERR_PARA;
	}
	data_item_no = *para;

	if(frame == NULL){
		printf("frame err! func: %s\n",__FUNCTION__);
		return ERR_PARA;
	}

	memset(buf,0,100);
	headlen = sizeof(ack_head_t);
	ParaLen = 1;
	
	set_frame_head(buf,id,ParaLen,cmd); 
	buf[headlen] = data_item_no;
	
	buf[headlen + 1] = get_sm(buf,headlen + 1);
	buf_len = headlen + 2;
	 *frame_len = create_ack_frame(buf,buf_len,frame) + buf_len;
	
	return OK;
}


static int get_led_flash_status_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

	return OK;
}

static int led_gr_control_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	if(para == NULL){
		printf("para err! func: %s\n",__FUNCTION__);
		return ERR_PARA;
	}
		
	if(frame == NULL)
		return ERR_PARA;

	
	frame[0] = cmd;
	frame[1] = 0x01;
	frame[2] = para[0];

	*frame_len = 3;
	
	return OK;
}

static int select_voice_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	return OK;
}

static int get_scan_code_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	char buf[100];
	unsigned int headlen;
	unsigned int buf_len;
	
	if(frame == NULL)
		return ERR_PARA;

	memset(buf,0,100);

	headlen = sizeof(ack_head_t);
	set_frame_head(buf,id,0,cmd); 
	buf[headlen] = get_sm(buf,headlen);
	buf_len = headlen + 1;
	*frame_len = create_ack_frame(buf,buf_len,frame) + buf_len;
	
	return OK;
}

static int id_alloc_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	//unsigned short addr;

	//addr = para[0] * 1000 + para[1] * 100 + para[2] * 10 + para[3] ;
	
	frame[0] = cmd;
	frame[1] = para[1];//(char)(id >> 8);
	frame[2] = para[0];//(char)(id & 0x00ff);

	IM_LOG("para0: 0x%x para1:0x%x\n",para[0],para[1]);

	*frame_len = 3;

	return OK;
}

static int scan_switch_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	if(para == NULL){
		printf("para err! func: %s\n",__FUNCTION__);
		return ERR_PARA;
	}
		
	if(frame == NULL)
		return ERR_PARA;

	
	*frame = cmd;
	
	if(para[0] == 0){ //OPEN
		*(frame + 1) = 0x01;
	}else if(para[0] == 1){//close
		*(frame + 1) = 0x00;
	}else{
		printf("wrong para!\n");
		return ERR;
	}

	*frame_len = 2;
	
	return OK;
}

static int self_test_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

	frame[0] = cmd;

	*frame_len = 1;
	return OK;
}

#if 0
static int led_ctrl_can(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{

	return OK;
}
#endif


func_node_t esl_cmd_func_node_array_can[] = 
{
	{CMD_SYS_RESET,system_reset_can,{NULL,NULL}},
	{CMD_MODIFY_ADDR,modify_addr_can,{NULL,NULL}},
	{CMD_UPDATE_DISPLAY,update_display_can,{NULL,NULL}},
	{CMD_LCD_GET_ITEM,lcd_get_item_can,{NULL,NULL}},
	{CMD_CODE_SCAN,get_scan_code_can,{NULL,NULL}},
	{CMD_LED_CONTROL,led_gr_control_can,{NULL,NULL}},
	{CMD_SCAN_SWITCH,scan_switch_can,{NULL,NULL}},
	{CMD_ID_ALLOC,id_alloc_can,{NULL,NULL}},
	{CMD_SELF_TEST,self_test_can,{NULL,NULL}},
//	{'z',cmd_Z_process,{NULL,NULL}},
	{0,NULL,{NULL,NULL}}
};

/*********************************************************************************/



static int add_func_hlist(func_node_t *func_node_array,func_hlist_t *func_hlist)
{
	func_node_t *pos;
	
	if(func_node_array == NULL || func_hlist == NULL)
		return ERR;
	
	for(pos = func_node_array;pos->param != NULL;pos ++){
		func_hlist->add_func_node(func_hlist,pos);
	}

	return OK;
}


static int esl_init(struct esl_protocol *proto,unsigned int net_type)
{
	unsigned char i;

	
	if(proto == NULL ){
		return ERR_PARA;
	}


	proto->fun_h->init(proto->fun_h);

	switch(net_type){
		case NET_TYPE_485:
			init_func_hlist(esl_cmd_func_node_array_485,proto->fun_h);
			break;
		case NET_TYPE_CAN:
			init_func_hlist(esl_cmd_func_node_array_can,proto->fun_h);
			break;
		default:
			printf("%s: err net_type!\n",__FUNCTION__);
			return ERR;
	}
	
	
	
	
	return OK;
}

static int esl_deinit(struct esl_protocol *proto)
{
	int i;
	
	return OK;                                                                                                       
}

static int esl_cmd_build(short id,char cmd,const char *para,unsigned int para_len,char *frame,unsigned int *frame_len)
{
	int ret;
	struct esl_protocol *proto;
	struct func_node *fnode;
	
	
	proto = &g_esl_proto;


	fnode = proto->fun_h->get_func_node(proto->fun_h,cmd);
	if(fnode == NULL){
		printf("func:%s()  cmd build func node get err!\n",__FUNCTION__);
		return ERR;
	}


	ret = ((CMD_BUILD_T)fnode->param)(id,cmd,para,para_len,frame,frame_len);
	if(ret != OK){
		printf("func:%s()  cmd build err!\n",__FUNCTION__);
		return ERR;
	}

	_DBG("++++++ %s:%d \n",__FUNCTION__,__LINE__);
	
	return OK;
}

static int esl_cmd_analysis(const char *frame,unsigned int len,int *ret,char *para)   //When ret>0 "ret" is length of para.
{
	char *cmd;
	ack_head_t *a_h;

	cmd = frame + 2;
	a_h = (ack_head_t *)cmd;
	if(a_h->start_id != 0x0d){
		*ret = ERR;
		goto ret_end;
	}
	
	if(a_h->cmd_len & 0x80){
		*ret = ERR;
	}else{
		*ret = OK;
	}
ret_end:
	return OK;
}


extern func_hlist_t g_esl_func;


esl_protocol_t g_esl_proto = {
	.fun_h = &g_esl_func,
	
	.init = esl_init,
	.deinit = esl_deinit,
	.cmd_build = esl_cmd_build,
	.cmd_analysis = esl_cmd_analysis
};




