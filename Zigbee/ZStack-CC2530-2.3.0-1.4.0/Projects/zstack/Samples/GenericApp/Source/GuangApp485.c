/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "GuangApp.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "MT_UART.h"

#include "DHT11.H"
#include "THC485.h"


// This list should be filled with Application specific Cluster IDs.
const cId_t GuangApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};

const SimpleDescriptionFormat_t GuangApp_SimpleDesc =
{
  GENERICAPP_ENDPOINT,              //  int Endpoint;
  GENERICAPP_PROFID,                //  uint16 AppProfId[2];
  GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                 //  int   AppFlags:4;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GuangApp_ClusterList,  //  byte *pAppInClusterList;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GuangApp_ClusterList   //  byte *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in GuangApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t GuangApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
byte GuangApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // GuangApp_Init() is called.
devStates_t GuangApp_NwkState;


byte GuangApp_TransID;  // This is the unique message ID (counter)

afAddrType_t GuangApp_DstAddr;
afAddrType_t GuangApp_Broadcast;   //广播




static uint8 Light_Work_State;   //灯工作状态
static uint8 Fans_Work_State;   //风扇工作状态
static uint8 Feng_Work_State;   //风扇工作状态

#define FANS      P1_5           //风扇
#define FENG      P1_4            //蜂鸣器控制端口
#define MYLED     P1_1            //灯控制端口        
#define MQ_PIN    P0_6            //可燃气体传感器控制端口  
#define LIGHT_PIN P1_6            //光敏传感器控制端口

#define GUANG_APP_TX_MAX  25
static uint8 GuangApp_TxBuf[GUANG_APP_TX_MAX+1];
static uint8 GuangApp_TxLen;
uint8 THCAsk[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x08, 0x44, 0x0c};
uint16 THCnum = 0;
uint8 THCstr[15];
  
extern uint16 temperture;
extern uint16 temperture_shi, temperture_ge, temperture_xiao;
extern uint16 humidity485;
extern uint16 humidity_shi, humidity_ge, humidity_xiao;
extern uint16 CO2;
extern uint16 CO2_qian, CO2_bai, CO2_shi, CO2_ge;
/*********************************************************************
 * LOCAL FUNCTIONS
 */
void GuangApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
void GuangApp_HandleKeys( byte shift, byte keys );
void GuangApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void GuangApp_SendTheMessage( void );
extern uint16 GetCrcData(uint8 *buffer, uint16 len);
/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      GuangApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void GuangApp_Init( byte task_id )
{
  GuangApp_TaskID = task_id;
  GuangApp_NwkState = DEV_INIT;
  GuangApp_TransID = 0;

  //------------------------配置串口---------------------------------TXD----接p0_4  RXD--接P0_5
  MT_UartInit();                    //串口初始化   里面可以修改波特率
  MT_UartRegisterTaskID(task_id);   //注册串口任务
  HalUARTWrite(0,"\n strTemp\n", sizeof("\n strTemp\n"));//串口发送
  //-----------------------------------------------------------------

   //----------初始化IO口-------------------
  P0SEL &= 0x3f;                //P0_6 7定义为通用IO口  0011 1111
  P1SEL &= 0x8d;                //P1_1 4 5 6定义为通用IO口  1000 1101
  P0DIR &= ~0xc0;               //P0_6 7初始化为输入口  0011 1111
  P1DIR |= 0x32;                 //P1_1 4 5初始化为输出口   0011 0010
  P1DIR &= ~0x40;               //P1_6初始化为输入口  
  //----------------------------------------
  Light_Work_State = 0;
  Feng_Work_State = 0;
  Fans_Work_State = 0; //默认风扇 蜂鸣器 和 灯是自动工作状态
  MYLED = 0;                 //熄灭灯    
  FANS = 0;                 //风扇关
  FENG = 0;                 //关蜂鸣器
  
  // Broadcast to everyone 发送模式:广播发送
  GuangApp_Broadcast.addrMode = (afAddrMode_t)AddrBroadcast;//广播
  GuangApp_Broadcast.endPoint = GENERICAPP_ENDPOINT; //指定端点号
  GuangApp_Broadcast.addr.shortAddr = 0xFFFF;//指定目的网络地址为广播地址
  
  GuangApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  GuangApp_DstAddr.endPoint = GENERICAPP_ENDPOINT;
  GuangApp_DstAddr.addr.shortAddr = 0x0000;

  // Fill out the endpoint description.
  GuangApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GuangApp_epDesc.task_id = &GuangApp_TaskID;
  GuangApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GuangApp_SimpleDesc;
  GuangApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &GuangApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( GuangApp_TaskID );

  // Update the display
#if defined ( LCD_SUPPORTED )
    HalLcdWriteString( "GuangApp", HAL_LCD_LINE_1 );
#endif
    
//  ZDO_RegisterForZDOMsg( GuangApp_TaskID, End_Device_Bind_rsp );    ???
//  ZDO_RegisterForZDOMsg( GuangApp_TaskID, Match_Desc_rsp );      ???
}

/*********************************************************************
 * @fn      GuangApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
UINT16 GuangApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;

  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter
  byte lineone[12];
  
  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GuangApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          GuangApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;
          
        case KEY_CHANGE:
          GuangApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
          }
          break;

        case AF_INCOMING_MSG_CMD:
          GuangApp_MessageMSGCB( MSGpkt );
          break;

        case ZDO_STATE_CHANGE:
          
          GuangApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if (GuangApp_NwkState == DEV_END_DEVICE)
          {
            lineone[0] = 'E';
            lineone[1] = 'n';
            lineone[2] = 'd';
            lineone[3] = 'D';

            lineone[4] = 'e';
            lineone[5] = 'v';
            lineone[6] = 'i';
            lineone[7] = 'c';
            lineone[8] = 'e';
            lineone[9] = ':';
            lineone[10] = ZIGBEE_ID;
            lineone[11] = '\0';   
            
          }else if(GuangApp_NwkState == DEV_ROUTER)
          {
            
            lineone[0] = 'R';
            lineone[1] = 'o';
            lineone[2] = 'u';
            lineone[3] = 't';

            lineone[4] = 'e';
            lineone[5] = 'r';
            lineone[6] = ':';
            lineone[7] = ZIGBEE_ID;
            lineone[8] = '\0';
            
          }
          
          
          
          if ( //(GuangApp_NwkState == DEV_ZB_COORD)|| 
              (GuangApp_NwkState == DEV_ROUTER)
              ||(GuangApp_NwkState == DEV_END_DEVICE) )
              {
                LcdClearLine(0,8);  //清空
                LCD_P8x16Str(0, 0, lineone); //id
                LCD_P16x16Ch(16,2,4*16);   //光
                LCD_P16x16Ch(32,2,5*16);  //照
                LCD_P16x16Ch(96,2,6*16);  //燃
                LCD_P16x16Ch(112,2,7*16);  //气
              for(int i=0; i<3; i++) //输出温度、湿度提示字符 
              { 
                if(i==0) 
                { 
                  LCD_P16x16Ch(i*16,4,i*16); 
                  LCD_P16x16Ch(i*16,6,(i+3)*16); 
                } 
                else 
                { 
                  LCD_P16x16Ch(i*16,4,i*16); 
                  LCD_P16x16Ch(i*16,6,i*16); 
                } 
              } 
              
            // Start sending "the" message in a regular interval.
            osal_start_timerEx( GuangApp_TaskID,
                                GENERICAPP_SEND_MSG_EVT,
                              GENERICAPP_SEND_MSG_TIMEOUT );
//          GuangApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
//          if ( //(GuangApp_NwkState == DEV_ZB_COORD)||
//               (GuangApp_NwkState == DEV_ROUTER)
//              || 
//                (GuangApp_NwkState == DEV_END_DEVICE) )
//          {
//            // Start sending "the" message in a regular interval.
//            osal_start_timerEx( GuangApp_TaskID,
//                                GENERICAPP_SEND_MSG_EVT,
//                              GENERICAPP_SEND_MSG_TIMEOUT );
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GuangApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in GuangApp_Init()).
  if ( events & GENERICAPP_SEND_MSG_EVT )
  {
    // Send "the" message
    GuangApp_SendTheMessage();

    // Setup to send message again
    osal_start_timerEx( GuangApp_TaskID,
                        GENERICAPP_SEND_MSG_EVT,
                      GENERICAPP_SEND_MSG_TIMEOUT );

    // return unprocessed events
    return (events ^ GENERICAPP_SEND_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */

/*********************************************************************
 * @fn      GuangApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
void GuangApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
        // Light LED
        HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined(BLINK_LEDS)
      else
      {
        // Flash LED to show failure
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;

    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            GuangApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
            GuangApp_DstAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            GuangApp_DstAddr.endPoint = pRsp->epList[0];

            // Light LED
            HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          osal_mem_free( pRsp );
        }
      }
      break;
  }
}

/*********************************************************************
 * @fn      GuangApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void GuangApp_HandleKeys( byte shift, byte keys )
{
  //zAddrType_t dstAddr;
  
  // Shift is used to make each button/switch dual purpose.
  if ( shift )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }

    if ( keys & HAL_KEY_SW_2 )
    {
//      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
//
//      // Initiate an End Device Bind Request for the mandatory endpoint
//      dstAddr.addrMode = Addr16Bit;
//      dstAddr.addr.shortAddr = 0x0000; // Coordinator
//      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(), 
//                            GuangApp_epDesc.endPoint,
//                            GENERICAPP_PROFID,
//                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GuangApp_ClusterList,
//                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GuangApp_ClusterList,
//                            FALSE );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
    }

    if ( keys & HAL_KEY_SW_4 )
    {
//      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
//      // Initiate a Match Description Request (Service Discovery)
//      dstAddr.addrMode = AddrBroadcast;
//      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
//      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
//                        GENERICAPP_PROFID,
//                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GuangApp_ClusterList,
//                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GuangApp_ClusterList,
//                        FALSE );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      GuangApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void GuangApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
      // "the" message
    //------------添加打印--------------
      HalUARTWrite(0, "Rx:", 3); //提示信息 
      HalUARTWrite(0, pkt->cmd.Data, pkt->cmd.DataLength); //输出接收到的数据 
      HalUARTWrite(0, "\n", 1); //回车换行 
     //------------------------------------
//#if defined( LCD_SUPPORTED )
//      HalLcdWriteScreen( (char*)pkt->cmd.Data, "rcvd" );
//#elif defined( WIN32 )
//      WPRINTSTR( pkt->cmd.Data );
//#endif
      break;
      
      case GENERICAPP_BROADCASTID:   //广播收到信息
      
      HalUARTWrite(0, "Rx:", 3); //提示信息 
      HalUARTWrite(0, pkt->cmd.Data, pkt->cmd.DataLength); //输出接收到的数据 
      HalUARTWrite(0, "\n", 1); //回车换行 
      
      
      if(pkt->cmd.DataLength == 4)//是否符合数据帧
      {
          if((pkt->cmd.Data)[1] == '0')
          {
            Light_Work_State = 0;
            MYLED = LIGHT_PIN;
          }else if((pkt->cmd.Data)[1] == '1')  //开
          {
            Light_Work_State = 1;
            MYLED =1;
          }else if((pkt->cmd.Data)[1] == '2'){  //关
            Light_Work_State = 2;
            MYLED =0;
          }
          
          if((pkt->cmd.Data)[2] == '0')
          {
            Feng_Work_State = 0;
            FENG = !MQ_PIN;
          }else if((pkt->cmd.Data)[2] == '1')  //开
          {
            Feng_Work_State = 1;
            FENG =1;
          }else if((pkt->cmd.Data)[2] == '2'){  //关
            Feng_Work_State = 2;
            FENG =0;
          }
          
          if((pkt->cmd.Data)[3] == '0')
          {
            Fans_Work_State = 0;
            FANS = !MQ_PIN;
          }else if((pkt->cmd.Data)[3] == '1')  //开
          {
            Fans_Work_State = 1;
            FANS =1;
          }else if((pkt->cmd.Data)[3] == '2'){  //关
            Fans_Work_State = 2;
            FANS =0;
          }
      }
      
      break;  
  }
}

/*********************************************************************
 * @fn      GuangApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */
void GuangApp_SendTheMessage( void )
{
  static uint8 send = 0;
  byte temp[3], humidity[3], light_mq[4],strTemp[9]; 
  if(send ==0)
  {
      HalUARTWrite(1, THCAsk, 8);
      send = 1;
  }else{
        DHT11(); //获取温湿度 
  
        //光敏传感器和可燃气体传感器处理
        if(MQ_PIN == 0)         //当浓度高于设定值时 ，执行条件函数        
        {
          if(Feng_Work_State == 0) FENG = 1;          //开蜂鸣器
          //if(Fans_Work_State == 0) FANS = 1;          //开风扇
          light_mq[0] = 0x31;
          LCD_P16x16Ch(80,2,8*16);
        }else
        {
          if(Feng_Work_State == 0) FENG = 0;         //关蜂鸣器
          //if(Fans_Work_State == 0) FANS = 0;          //关风扇
          light_mq[0] = 0x30;
          LCD_P16x16Ch(80,2,9*16);
        }
        
        if(LIGHT_PIN == 1)    //当光敏电阻处于黑暗中时，LED亮
        {
            if(Light_Work_State == 0) MYLED =1;
            light_mq[1] = 0x31;
            LCD_P16x16Ch(0,2,9*16);   //屏幕打印无
        }
        else
        {
            if(Light_Work_State == 0) MYLED = 0;       //检测到光线时，低电平LED关闭
            light_mq[1] = 0x30;
             LCD_P16x16Ch(0,2,8*16);   //屏幕打印有
        }
      light_mq[2] = 'e'; 
      light_mq[3] = '\0'; 
      //ds18b20处理
     
      //风扇开关
      if(Fans_Work_State == 0)
      {
        if( MQ_PIN == 1 && (wendu_shi<2  || (wendu_shi==2 && wendu_ge <= 6)) )
        {
          FANS = 0;
        }else{
          FANS = 1;
        }
      }
        
      //将温湿度的转换成字符串,供 LCD 显示 
      temp[0] = wendu_shi+0x30; 
      temp[1] = wendu_ge+0x30; 
      temp[2] = '\0'; 
      humidity[0] = shidu_shi+0x30; 
      humidity[1] = shidu_ge+0x30; 
      humidity[2] = '\0';
      //将数据整合后方便发给协调器显示 
    
      strTemp[0] = ZIGBEE_ID; 
      osal_memcpy(&strTemp[1], temp, 2); 
      osal_memcpy(&strTemp[3], humidity, 2); 
      osal_memcpy(&strTemp[5], light_mq, 4); 
      //获得的温湿度通过串口输出到电脑显示 
      HalUARTWrite(0, "T&H:", 4); 
      HalUARTWrite(0, strTemp, 7); 
      HalUARTWrite(0, "\n",1); 
      
      //输出到 LCD 显示 
    
      LCD_P8x16Str(48, 4, temp); //LCD 显示温度值 
      LCD_P8x16Str(48, 6, humidity); //LCD 显示湿度值
      
    
      
      //发送给协调器
      if ( AF_DataRequest( &GuangApp_DstAddr, &GuangApp_epDesc,
                           GENERICAPP_CLUSTERID,
                           9,
                           strTemp,
                           &GuangApp_TransID,
                           AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
      {
        // Successfully requested to be sent.
      }
      else
      {
        // Error occurred in request to send.
      }
      send = 0;
  }
}


extern void rxCB(uint8 port,uint8 event)
{

  if (GuangApp_TxLen < GUANG_APP_TX_MAX)
        {
            GuangApp_TxLen = HalUARTRead(port, GuangApp_TxBuf, GUANG_APP_TX_MAX);
            if (GuangApp_TxLen)
            {
            //双串口互错传输
            //串口0发---> 串口1收
            //串口1发---> 串口0收

                if(port==1)
                {
                    //HalUARTWrite(0, GuangApp_TxBuf, GuangApp_TxLen);
                  if(GuangApp_TxBuf[0] == 0x01){//isCRC(GuangApp_TxBuf,20) == 't' && GuangApp_TxBuf[0] == 0x01){
                    HalLedSet(HAL_LED_4,HAL_LED_MODE_TOGGLE);
                     THCnum = 0;
                     memset(THCstr,0,15);
//                     THCstr[THCnum++] = '{';
//                     THCstr[THCnum++] = 'i';
//                     THCstr[THCnum++] = 'd';
//                     THCstr[THCnum++] = ':';
//                     THCstr[THCnum++] = '0';
                     THCstr[THCnum++] = ZIGBEE_ID;
//                     THCstr[THCnum++] = ',';
//                     THCstr[THCnum++] = 't';
//                     THCstr[THCnum++] = 'y';
//                     THCstr[THCnum++] = 'p';
//                     THCstr[THCnum++] = 'e';
//                     THCstr[THCnum++] = ':';
//                     THCstr[THCnum++] = '4';
//                     THCstr[THCnum++] = '8';
//                     THCstr[THCnum++] = '5';
//                     THCstr[THCnum++] = ',';
                     
                     temperture = ((GuangApp_TxBuf[5]) << 8 | GuangApp_TxBuf[6]);
                     if(temperture <1000 && temperture>0){
                        temperture_shi = temperture/100;
                        temperture_ge = (temperture%100)/10;
                        temperture_xiao = temperture%10;
//                         THCstr[THCnum++] = 't';
//                         THCstr[THCnum++] = 'e';
//                         THCstr[THCnum++] = 'm';
//                         THCstr[THCnum++] = ':';
                         THCstr[THCnum++] = temperture_shi+0x30;
                         THCstr[THCnum++] = temperture_ge+0x30;
//                         THCstr[THCnum++] = '.';
                         THCstr[THCnum++] = temperture_xiao+0x30;
//                         THCstr[THCnum++] = ',';
                      }
                     
                     humidity485 = ((GuangApp_TxBuf[3]) << 8 | GuangApp_TxBuf[4]);
                     if(humidity485 <1000 && humidity485>0){
                        humidity_shi = humidity485/100;
                        humidity_ge = (humidity485%100)/10;
                        humidity_xiao = humidity485%10;
                        
//                         THCstr[THCnum++] = 'h';
//                         THCstr[THCnum++] = 'u';
//                         THCstr[THCnum++] = 'm';
//                         THCstr[THCnum++] = ':';
                         THCstr[THCnum++] = humidity_shi+0x30;
                         THCstr[THCnum++] = humidity_ge+0x30;
//                         THCstr[THCnum++] = '.';
                         THCstr[THCnum++] = humidity_xiao+0x30;
//                         THCstr[THCnum++] = ',';
                      }
                     

                   
                   CO2 = ((GuangApp_TxBuf[17]) << 8 | GuangApp_TxBuf[18]);  
                     if(CO2 <10000 && CO2>0){
                        CO2_qian = CO2/1000;
                        CO2_bai = (CO2%1000)/100;
                        CO2_shi = (CO2%100)/10;
                        CO2_ge = CO2%10;
//                        THCstr[THCnum++] = 'c';
//                         THCstr[THCnum++] = 'o';
//                         THCstr[THCnum++] = '2';
//                         THCstr[THCnum++] = ':';
                         THCstr[THCnum++] = CO2_qian+0x30;
                         THCstr[THCnum++] = CO2_bai+0x30;
                         THCstr[THCnum++] = CO2_shi+0x30;
                         THCstr[THCnum++] = CO2_ge+0x30;
//                         THCstr[THCnum++] = ',';
                      } 
                    THCstr[THCnum++] = 'e';
                     HalUARTWrite(0,THCstr, THCnum);
                     AF_DataRequest( &GuangApp_DstAddr, &GuangApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       THCnum,
                       THCstr,
                       &GuangApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS );
                   }
                  THCnum = 0;
                }
                else
                {
                    HalUARTWrite(1, GuangApp_TxBuf, GuangApp_TxLen);
                }

             }            

            GuangApp_TxLen = 0;
        }
}
/*********************************************************************
*********************************************************************/
