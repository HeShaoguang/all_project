#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "24cxx.h"
#include "touch.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_test_theme_2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lv_test_font.h"
#include "usart3.h"
#include "queue.h"
#include "ui_app.h"
#include "string.h"
#include "chart_win.h"
#include "gauge_win.h"
#include "industry_win.h"
#include "usart2.h"
#include "rc522_config.h"
#include "rc522_function.h"

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define MQTT_TASK_PRIO		2
//任务堆栈大小	
#define MQTT_STK_SIZE 		256  
//任务句柄
TaskHandle_t MqttTask_Handler;
//任务函数
void mqtt_task(void *pvParameters);

//任务优先级
#define ZIGBEE_TASK_PRIO		3
//任务堆栈大小	
#define ZIGBEE_STK_SIZE 		256  
//任务句柄
TaskHandle_t ZigbeeTask_Handler;
//任务函数
void zigbee_task(void *pvParameters);

//任务优先级
#define ESP8266_TASK_PRIO		4
//任务堆栈大小	
#define ESP8266_STK_SIZE 		256  
//任务句柄
TaskHandle_t Esp8266Task_Handler;
//任务函数
void esp8266_task(void *pvParameters);

//任务优先级
#define SCREEN_TASK_PRIO		5
//任务堆栈大小	
#define SCREEN_STK_SIZE 		1024  
//任务句柄
TaskHandle_t ScreenTask_Handler;
//任务函数
void screen_task(void *pvParameters);

//任务优先级
#define RFID_TASK_PRIO		5
//任务堆栈大小	
#define RFID_STK_SIZE 		236  
//任务句柄
TaskHandle_t Rfid_Handler;
//任务函数
void rfid_task(void *pvParameters);



#define   MESSAGE_FROMZIGBEE_NUM   6
QueueHandle_t MessageFromZigbee_Queue;

#define   MESSAGE_ESP8266_NUM   10
QueueHandle_t MessageFromEsp8266_Queue;

#define   MQTT_NUM   6
QueueHandle_t Mqtt_Queue;
 int main(void)
 {	 		    
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(9600);	 	//串口初始化为9600
	uart2_init(9600);
	uart3_init(9600);
//	TIM3_Int_Init(999,71);
 	LED_Init();			     //LED端口初始化
	LCD_Init();	
	KEY_Init();	 	
 	tp_dev.init();
	
 	RC522_Init ();//rfid初始化
  PcdReset ();
	GPIO_ResetBits(GPIOE,GPIO_Pin_5);  //开灯
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);  //开灯 
	 
	lv_init();
	 lv_port_disp_init();
	 lv_port_indev_init();
	 
	 //lv_test_theme_2();
	ui_app_start();

	//lv_font_test_start();

			 	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();  						
// 	 while(1)
//	 {
//		 tp_dev.scan(0);
//		 lv_task_handler();
//	 }
}


//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    
		MessageFromZigbee_Queue = xQueueCreate(MESSAGE_FROMZIGBEE_NUM, USART3_REC_LEN);
		MessageFromEsp8266_Queue = xQueueCreate(MESSAGE_ESP8266_NUM, USART2_REC_LEN);
		Mqtt_Queue = xQueueCreate(MQTT_NUM, 15);
	
	  xTaskCreate((TaskFunction_t )mqtt_task,     	
                (const char*    )"mqtt_task",   	
                (uint16_t       )MQTT_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )MQTT_TASK_PRIO,	
                (TaskHandle_t*  )&MqttTask_Handler); 
								
    xTaskCreate((TaskFunction_t )zigbee_task,     	
                (const char*    )"zigbee_task",   	
                (uint16_t       )ZIGBEE_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )ZIGBEE_TASK_PRIO,	
                (TaskHandle_t*  )&ZigbeeTask_Handler);   
								
		xTaskCreate((TaskFunction_t )esp8266_task,     	
                (const char*    )"esp8266_task",   	
                (uint16_t       )ESP8266_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )ESP8266_TASK_PRIO,	
                (TaskHandle_t*  )&Esp8266Task_Handler);
    
    xTaskCreate((TaskFunction_t )screen_task,     
                (const char*    )"screen_task",   
                (uint16_t       )SCREEN_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )SCREEN_TASK_PRIO,
                (TaskHandle_t*  )&ScreenTask_Handler);     

    xTaskCreate((TaskFunction_t )rfid_task,     
                (const char*    )"rid_task",   
                (uint16_t       )RFID_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )RFID_TASK_PRIO,
                (TaskHandle_t*  )&Rfid_Handler);  
								
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//LED0任务函数 
void zigbee_task(void *pvParameters)
{
	BaseType_t err,err2;
	u8 id,temperture,humitidy,mq,light,temperture2,humitidy2;
	u16 co2;
	u8 ReceiveBuf[USART3_REC_LEN];
	u8 data_buf[15];
	u8 i,data_len;
	
	while(1)
	{
		if(MessageFromZigbee_Queue != NULL)    //获取串口的信息
		{
			err = xQueueReceive(MessageFromZigbee_Queue, ReceiveBuf, portMAX_DELAY);
			if(err == pdTRUE)
			{
				printf("ReceiveBuf:%s\r\n",ReceiveBuf);
				printf("len:%d",strlen(ReceiveBuf));
				
				data_len = 0;
				for(i=0;i<strlen(ReceiveBuf);i++)
				{
					if(ReceiveBuf[i] == 'e')
					{
						if(data_len == 7)
						{
							err2 = xQueueSend(Mqtt_Queue, &data_buf,0);
							if(err2 != pdTRUE)
							{
								printf("Mqtt_Queue send failed");
							}
							//USART2_Send_Data(data_buf,7);
							id = data_buf[0]-'0';
							temperture = (data_buf[1]-'0')*10 + (data_buf[2]-'0');
							humitidy = (data_buf[3]-'0')*10 + (data_buf[4]-'0');
							mq = data_buf[5]-'0';
							light = data_buf[6]-'0';
							if(temperture != 0 && humitidy != 0)
							{
								gauge_update(id, temperture, humitidy,light,mq);
								switch(id)
								{
									case 1:temperture1_add(temperture);humitidy1_add(humitidy);break;
									case 2:temperture2_add(temperture);humitidy2_add(humitidy);break;
									case 3:temperture3_add(temperture);humitidy3_add(humitidy);break;
									default: break;
								}
							}
						}else if(data_len == 11)
						{
							if( data_buf[0] == '1')
							{
								err2 = xQueueSend(Mqtt_Queue, &data_buf,0);
								if(err2 != pdTRUE)
								{
									printf("Mqtt_Queue send failed");
								}
								//USART2_Send_Data(data_buf,11);
								temperture = (data_buf[1]-'0')*10 + (data_buf[2]-'0');
								temperture2 = data_buf[3]-'0';
								humitidy = (data_buf[4]-'0')*10 + (data_buf[5]-'0');
								humitidy2 = data_buf[6]-'0';
								co2 = (data_buf[7]-'0')*1000 + (data_buf[8]-'0')*100 + (data_buf[9]-'0')*10 + (data_buf[10]-'0');
								industry_add(temperture,temperture2,humitidy,humitidy2,co2);
							}
						}
						data_len = 0;
						memset(data_buf, 0, 15);
					}else{
						data_buf[data_len++] = ReceiveBuf[i];
					}
				}
				
//				if(strlen(ReceiveBuf) == 8)
//				{
//					USART2_Send_Data(ReceiveBuf,8);
//					id = ReceiveBuf[0]-'0';
//					temperture = (ReceiveBuf[1]-'0')*10 + (ReceiveBuf[2]-'0');
//					humitidy = (ReceiveBuf[3]-'0')*10 + (ReceiveBuf[4]-'0');
//					mq = ReceiveBuf[5]-'0';
//					light = ReceiveBuf[6]-'0';
//					if(temperture != 0 && humitidy != 0)
//					{
//						gauge_update(id, temperture, humitidy,light,mq);
//						switch(id)
//						{
//							case 1:temperture1_add(temperture);humitidy1_add(humitidy);break;
//							case 2:temperture2_add(temperture);humitidy2_add(humitidy);break;
//							case 3:temperture3_add(temperture);humitidy3_add(humitidy);break;
//							default: break;
//						}
//					}
//				}else if(strlen(ReceiveBuf) == 12)
//				{
//					if( ReceiveBuf[0] == '1')
//					{
//						USART2_Send_Data(ReceiveBuf,12);
//						temperture = (ReceiveBuf[1]-'0')*10 + (ReceiveBuf[2]-'0');
//						temperture2 = ReceiveBuf[3]-'0';
//						humitidy = (ReceiveBuf[4]-'0')*10 + (ReceiveBuf[5]-'0');
//						humitidy2 = ReceiveBuf[6]-'0';
//						co2 = (ReceiveBuf[7]-'0')*1000 + (ReceiveBuf[8]-'0')*100 + (ReceiveBuf[9]-'0')*10 + (ReceiveBuf[10]-'0');
//						industry_add(temperture,temperture2,humitidy,humitidy2,co2);
//					}
//				}
			}else
			{
				printf("receive usart wrong\r\n");
			}
		}
		vTaskDelay(1000);
	}
}   

void esp8266_task(void *pvParameters)
{
	BaseType_t err;
	u8 Esp8266Buf[USART2_REC_LEN];
	
	while(1)
	{
		if(MessageFromEsp8266_Queue != NULL)    //获取串口的信息
		{
			err = xQueueReceive(MessageFromEsp8266_Queue, Esp8266Buf, portMAX_DELAY);
			if(err == pdTRUE)
			{
				printf("Esp8266Buf:%s\r\n",Esp8266Buf);
				printf("len:%d",strlen(Esp8266Buf));
				if(strlen(Esp8266Buf) == 4)
				{
					USART3_Send_Data(Esp8266Buf,4); 
				}
			}else
			{
				printf("receive usart wrong\r\n");
			}
		}
		vTaskDelay(1000);
	}
}  

void mqtt_task(void *pvParameters)
{
	BaseType_t err;
	u8 mqtt_buf[15];
	while(1)
	{
		err = xQueueReceive(Mqtt_Queue, mqtt_buf, portMAX_DELAY);
		if(err == pdTRUE)
		{
			printf("send to mqtt");
			USART2_Send_Data(mqtt_buf,strlen(mqtt_buf));
			
		}
		vTaskDelay(1000);
	}
}


void screen_task(void *pvParameters)
{
	
    while(1)
    {
			
			//taskENTER_CRITICAL();           //进入临界区
			tp_dev.scan(0);			
			lv_task_handler();
			//taskEXIT_CRITICAL();            //退出临界区
			vTaskDelay(pdMS_TO_TICKS(1));
    }
}

uint8_t KeyValue[]={0xFF ,0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // 卡A密钥
void rfid_task(void *pvParameters)
{
	uint32_t writeValue = 100;
	uint32_t readValue;
	char cStr [ 30 ];
  uint8_t ucArray_ID [ 4 ];    /*先后存放IC卡的类型和UID(IC卡序列号)*/                                                                                         
	uint8_t ucStatusReturn;      /*返回状态*/    
	while(1)
	{
			/*寻卡，寻找未休眠的卡*/
		if ( ( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK )  
       /*若失败再次寻卡*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );		                                                

		if ( ucStatusReturn == MI_OK  )
		{
      /*防冲撞（当有多张卡进入读写器操作范围时，防冲突机制会从其中选择一张进行操作）*/
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK )                                                                   
			{
				PcdSelect(ucArray_ID);			
				
				sprintf ( cStr, "The Card ID is: %02X%02X%02X%02X",ucArray_ID [0], ucArray_ID [1], ucArray_ID [2],ucArray_ID [3] );
				printf ( "%s\r\n",cStr );  //打印卡片ID
				PcdHalt();
				if(ucArray_ID [0]==0x50 && ucArray_ID [1]==0xA1 && ucArray_ID [2]==0x8B && ucArray_ID [3]==0x43)
				{
					printf ( "resume task\r\n");  //打印卡片ID
					vTaskResume(ScreenTask_Handler);/* 恢复任务！ */
					GPIO_ResetBits(GPIOE,GPIO_Pin_5);  //开灯
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);  //开灯
				}
			}	
    }
		vTaskDelay(100);
	}
}

