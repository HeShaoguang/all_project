#ifndef __USART3_H
#define __USART3_H
#include "stdio.h"	
#include "sys.h" 

#define USART3_REC_LEN  			30  	//定义最大接收字节数 30


extern u8  USART3_RX_BUF[USART3_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART3_RX_CNT; 

void uart3_init(u32 bound);
void USART3_Send_Data(u8 *buf,u8 len);

#endif


