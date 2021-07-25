#include "sys.h"
#include "usart2.h"	 
#include "FreeRTOS.h"
#include "queue.h"
#include "string.h"

u8  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
u8  USART2_RX_CNT=0; 
u16 USART2_RX_STA=0;         		//接收状态标记	
extern QueueHandle_t MessageFromEsp8266_Queue;

void uart2_init(u32 bound)
{
	GPIO_InitTypeDef gs;
	USART_InitTypeDef us;
	NVIC_InitTypeDef ns;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	gs.GPIO_Pin=GPIO_Pin_2;
	gs.GPIO_Mode=GPIO_Mode_AF_PP;
	gs.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gs);
	
	gs.GPIO_Pin=GPIO_Pin_3;
	gs.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	gs.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gs);	
	
	us.USART_BaudRate=bound;
	us.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	us.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	us.USART_Parity=USART_Parity_No;
	us.USART_StopBits=USART_StopBits_1;
	us.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART2,&us);
	USART_Cmd(USART2,ENABLE);
	
	USART_ITConfig(USART2,USART_IT_RXNE, ENABLE);
	USART_ITConfig (USART2, USART_IT_IDLE, ENABLE );
	
	ns.NVIC_IRQChannel=USART2_IRQn;
	ns.NVIC_IRQChannelCmd=ENABLE;
	ns.NVIC_IRQChannelPreemptionPriority=7;
	ns.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&ns);
}

void USART2_Send_Data(u8 *buf,u8 len)
{
	u8 t;

  for(t=0;t<len;t++)		//循环发送数据
	{		   
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART2,buf[t]);
	}	 
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		
}



void USART2_IRQHandler(void)
{
	u8 ret;
	BaseType_t err;
	BaseType_t xHigherPriorityTaskWoken;
	
	if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET )
	{
		ret=USART_ReceiveData(USART2);
		if(USART2_RX_CNT<USART2_REC_LEN)
		{
			USART2_RX_BUF[USART2_RX_CNT++]=ret;		//记录接收到的值
		}
	}
	
	if(USART_GetITStatus(USART2,USART_IT_IDLE) == SET )
	{
		err = xQueueSendFromISR(MessageFromEsp8266_Queue, &USART2_RX_BUF, &xHigherPriorityTaskWoken);
		if(err != pdTRUE)
		{
			printf("send failed");
		}

		memset(USART2_RX_BUF, 0, USART2_REC_LEN);
		USART2_RX_CNT = 0;
		ret = USART_ReceiveData(USART2);
		
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		
	}
	
}

