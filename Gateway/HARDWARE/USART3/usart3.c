#include "sys.h"
#include "usart3.h"	 
#include "FreeRTOS.h"
#include "queue.h"
#include "string.h"

u8  USART3_RX_BUF[USART3_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
u8  USART3_RX_CNT=0; 

extern QueueHandle_t MessageFromZigbee_Queue;

void uart3_init(u32 bound)
{
	GPIO_InitTypeDef gs;
	USART_InitTypeDef us;
	NVIC_InitTypeDef ns;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
	gs.GPIO_Pin=GPIO_Pin_10;
	gs.GPIO_Mode=GPIO_Mode_AF_PP;
	gs.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gs);
	
	gs.GPIO_Pin=GPIO_Pin_11;
	gs.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	gs.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gs);	
	
	us.USART_BaudRate=bound;
	us.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	us.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	us.USART_Parity=USART_Parity_No;
	us.USART_StopBits=USART_StopBits_1;
	us.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART3,&us);
	USART_Cmd(USART3,ENABLE);
	
	USART_ITConfig(USART3,USART_IT_RXNE, ENABLE);
	USART_ITConfig (USART3, USART_IT_IDLE, ENABLE );
	
	ns.NVIC_IRQChannel=USART3_IRQn;
	ns.NVIC_IRQChannelCmd=ENABLE;
	ns.NVIC_IRQChannelPreemptionPriority=6;
	ns.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&ns);
}

void USART3_Send_Data(u8 *buf,u8 len)
{
	u8 t;

  for(t=0;t<len;t++)		//循环发送数据
	{		   
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART3,buf[t]);
	}	 
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);		
}



void USART3_IRQHandler(void)
{
	u8 ret;

	BaseType_t err;
	BaseType_t xHigherPriorityTaskWoken;
	
	if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET )
	{
		ret=USART_ReceiveData(USART3);
		if(USART3_RX_CNT<USART3_REC_LEN)
		{
			USART3_RX_BUF[USART3_RX_CNT++]=ret;		//记录接收到的值
		}
		//USART_SendData(USART3,ret);
	}
	
	if(USART_GetITStatus(USART3,USART_IT_IDLE) == SET )
	{
//		data_len = 0;
//		for(i=0;i<USART3_RX_CNT;i++)
//		{
//			if(USART3_RX_BUF[i] == 'e')
//			{
//				data_len = 0;
//				err = xQueueSendFromISR(MessageFromZigbee_Queue, &DATA_BUF, &xHigherPriorityTaskWoken);
//				if(err != pdTRUE)
//				{
//					printf("send failed");
//				}
//				memset(DATA_BUF, 0, DATA_MAX_LEN);
//			}else{
//				DATA_BUF[data_len++] = USART3_RX_BUF[i];
//			}
//		}
		err = xQueueSendFromISR(MessageFromZigbee_Queue, &USART3_RX_BUF, &xHigherPriorityTaskWoken);
		if(err != pdTRUE)
		{
			printf("send failed");
		}
		
		memset(USART3_RX_BUF, 0, USART3_REC_LEN);
		USART3_RX_CNT = 0;
		ret=USART_ReceiveData(USART3);
		
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		

		
	}
	
}

