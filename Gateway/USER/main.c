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

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define MQTT_TASK_PRIO		2
//�����ջ��С	
#define MQTT_STK_SIZE 		256  
//������
TaskHandle_t MqttTask_Handler;
//������
void mqtt_task(void *pvParameters);

//�������ȼ�
#define ZIGBEE_TASK_PRIO		3
//�����ջ��С	
#define ZIGBEE_STK_SIZE 		256  
//������
TaskHandle_t ZigbeeTask_Handler;
//������
void zigbee_task(void *pvParameters);

//�������ȼ�
#define ESP8266_TASK_PRIO		4
//�����ջ��С	
#define ESP8266_STK_SIZE 		256  
//������
TaskHandle_t Esp8266Task_Handler;
//������
void esp8266_task(void *pvParameters);

//�������ȼ�
#define SCREEN_TASK_PRIO		5
//�����ջ��С	
#define SCREEN_STK_SIZE 		1024  
//������
TaskHandle_t ScreenTask_Handler;
//������
void screen_task(void *pvParameters);

//�������ȼ�
#define RFID_TASK_PRIO		5
//�����ջ��С	
#define RFID_STK_SIZE 		236  
//������
TaskHandle_t Rfid_Handler;
//������
void rfid_task(void *pvParameters);



#define   MESSAGE_FROMZIGBEE_NUM   6
QueueHandle_t MessageFromZigbee_Queue;

#define   MESSAGE_ESP8266_NUM   10
QueueHandle_t MessageFromEsp8266_Queue;

#define   MQTT_NUM   6
QueueHandle_t Mqtt_Queue;
 int main(void)
 {	 		    
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 	//���ڳ�ʼ��Ϊ9600
	uart2_init(9600);
	uart3_init(9600);
//	TIM3_Int_Init(999,71);
 	LED_Init();			     //LED�˿ڳ�ʼ��
	LCD_Init();	
	KEY_Init();	 	
 	tp_dev.init();
	
 	RC522_Init ();//rfid��ʼ��
  PcdReset ();
	GPIO_ResetBits(GPIOE,GPIO_Pin_5);  //����
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);  //���� 
	 
	lv_init();
	 lv_port_disp_init();
	 lv_port_indev_init();
	 
	 //lv_test_theme_2();
	ui_app_start();

	//lv_font_test_start();

			 	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();  						
// 	 while(1)
//	 {
//		 tp_dev.scan(0);
//		 lv_task_handler();
//	 }
}


//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
    
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
								
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//LED0������ 
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
		if(MessageFromZigbee_Queue != NULL)    //��ȡ���ڵ���Ϣ
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
		if(MessageFromEsp8266_Queue != NULL)    //��ȡ���ڵ���Ϣ
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
			
			//taskENTER_CRITICAL();           //�����ٽ���
			tp_dev.scan(0);			
			lv_task_handler();
			//taskEXIT_CRITICAL();            //�˳��ٽ���
			vTaskDelay(pdMS_TO_TICKS(1));
    }
}

uint8_t KeyValue[]={0xFF ,0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // ��A��Կ
void rfid_task(void *pvParameters)
{
	uint32_t writeValue = 100;
	uint32_t readValue;
	char cStr [ 30 ];
  uint8_t ucArray_ID [ 4 ];    /*�Ⱥ���IC�������ͺ�UID(IC�����к�)*/                                                                                         
	uint8_t ucStatusReturn;      /*����״̬*/    
	while(1)
	{
			/*Ѱ����Ѱ��δ���ߵĿ�*/
		if ( ( ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID ) ) != MI_OK )  
       /*��ʧ���ٴ�Ѱ��*/
			ucStatusReturn = PcdRequest ( PICC_REQIDL, ucArray_ID );		                                                

		if ( ucStatusReturn == MI_OK  )
		{
      /*����ײ�����ж��ſ������д��������Χʱ������ͻ���ƻ������ѡ��һ�Ž��в�����*/
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK )                                                                   
			{
				PcdSelect(ucArray_ID);			
				
				sprintf ( cStr, "The Card ID is: %02X%02X%02X%02X",ucArray_ID [0], ucArray_ID [1], ucArray_ID [2],ucArray_ID [3] );
				printf ( "%s\r\n",cStr );  //��ӡ��ƬID
				PcdHalt();
				if(ucArray_ID [0]==0x50 && ucArray_ID [1]==0xA1 && ucArray_ID [2]==0x8B && ucArray_ID [3]==0x43)
				{
					printf ( "resume task\r\n");  //��ӡ��ƬID
					vTaskResume(ScreenTask_Handler);/* �ָ����� */
					GPIO_ResetBits(GPIOE,GPIO_Pin_5);  //����
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);  //����
				}
			}	
    }
		vTaskDelay(100);
	}
}

