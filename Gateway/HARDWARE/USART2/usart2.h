#ifndef __USART2_H
#define __USART2_H
#include "stdio.h"	
#include "sys.h" 

#define USART2_REC_LEN  			30  	//定义最大接收字节数 200
	  	
extern u8  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART2_RX_CNT; 
extern u16 USART2_RX_STA;         		//接收状态标记	

void uart2_init(u32 bound);
void USART2_Send_Data(u8 *buf,u8 len);

#endif


