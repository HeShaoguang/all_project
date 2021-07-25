#include "rc522_function.h"
#include "rc522_config.h"
#include "stm32f10x.h"
#include "delay.h"


#define   RC522_DELAY()  delay_us ( 2 )

 /**
  * @brief  向RC522发送1 Byte 数据
  * @param  byte，要发送的数据
  * @retval RC522返回的数据
  */
	
	//一个一个位地发送，发送完右移，准备好发下一位。一共发8位一个字节
void SPI_RC522_SendByte ( uint8_t byte )
{
  uint8_t counter;

  for(counter=0;counter<8;counter++)
  {     
    if ( byte & 0x80 )
      RC522_MOSI_1 ();
    else 
      RC522_MOSI_0 ();
    //delay_us ( 3 );
    RC522_DELAY();
    RC522_SCK_0 ();
    //delay_us ( 1 );
    //delay_us ( 3 );
    RC522_DELAY();
    RC522_SCK_1();
    //delay_us ( 3 );
    RC522_DELAY();
    byte <<= 1; 
  } 	
}


/**
  * @brief  从RC522发送1 Byte 数据
  * @param  无
  * @retval RC522返回的数据
  */
//一个个位读取，是一就把最左那位放1，然后左移
uint8_t SPI_RC522_ReadByte ( void )
{
  uint8_t counter;
  uint8_t SPI_Data;

  for(counter=0;counter<8;counter++)
  {
    SPI_Data <<= 1;
    RC522_SCK_0 ();
    //delay_us ( 3 );
    RC522_DELAY();
    if ( RC522_MISO_GET() == 1)
     SPI_Data |= 0x01;
    //delay_us ( 2 );
    //delay_us ( 3 );
    RC522_DELAY();
    RC522_SCK_1 ();
    //delay_us ( 3 );
    RC522_DELAY();
  }
  return SPI_Data;
	
}



/**
  * @brief  读RC522寄存器
  * @param  ucAddress，寄存器地址
  * @retval 寄存器的当前值
  */
//其实的ucAddress是寄存器的地址，一共有4*16=64个寄存器，具体在头文件里。
//发送一个寄存器地址过去，rc522就会返回寄存器的值，这里接受这个寄存器的值就好了
uint8_t ReadRawRC ( uint8_t ucAddress )
{
	uint8_t ucAddr, ucReturn;		//寄存器有效地址，寄存器返回值
	
	ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;	//变成有效地址形式，最低为0，最高位为1
	RC522_CS_Enable();
	SPI_RC522_SendByte ( ucAddr );
	ucReturn = SPI_RC522_ReadByte ();
	RC522_CS_Disable();
	
	return ucReturn;	
}

/**
  * @brief  写RC522寄存器
  * @param  ucAddress，寄存器地址
  * @param  ucValue，写入寄存器的值
  * @retval 无
  */
//发送一个寄存器地址过去，再发送要写的值，就可以写到寄存器
void WriteRawRC ( uint8_t ucAddress, uint8_t ucValue )
{  
	uint8_t ucAddr;
	
	ucAddr = ( ucAddress << 1 ) & 0x7E;	//变成有效地址形式，最低为0，最高位为1
	RC522_CS_Enable();	
	SPI_RC522_SendByte ( ucAddr );	
	SPI_RC522_SendByte ( ucValue );
	RC522_CS_Disable();		
}


/**
  * @brief  对RC522寄存器置位
  * @param  ucReg，寄存器地址
  * @param   ucMask，置位值
  * @retval 无
  */

//把寄存器中原本可能为0的位，置为1，且不影响其他位
void SetBitMask ( uint8_t ucReg, uint8_t ucMask )  
{
   uint8_t ucTemp;
	
   ucTemp = ReadRawRC ( ucReg );
   WriteRawRC ( ucReg, ucTemp | ucMask );         // set bit mask
}


/**
  * @brief  对RC522寄存器清位
  * @param  ucReg，寄存器地址
  * @param  ucMask，清位值
  * @retval 无
  */
//把寄存器中原本可能为1的位，置为0，且不影响其他位
void ClearBitMask ( uint8_t ucReg, uint8_t ucMask )  
{
   uint8_t ucTemp;
	
   ucTemp = ReadRawRC ( ucReg );
   WriteRawRC ( ucReg, ucTemp & ( ~ ucMask) );  // clear bit mask
}


/**
  * @brief  开启天线 
  * @param  无
  * @retval 无
  */
//TxControlReg 控制天线驱动器的管家TX1和TX2的逻辑特性，0x03相当于使能
void PcdAntennaOn ( void )
{
   uint8_t uc;
  
   uc = ReadRawRC ( TxControlReg );
   if ( ! ( uc & 0x03 ) )
		SetBitMask(TxControlReg, 0x03);		
}


/**
  * @brief  关闭天线
  * @param  无
  * @retval 无
  */
//这里跟上一个函数对应，相当于关闭使能
void PcdAntennaOff ( void )
{
  ClearBitMask ( TxControlReg, 0x03 );	
}


/**
  * @brief  复位RC522 
  * @param  无
  * @retval 无
  */
//重置RC522，时钟重置
//RESET数据线重置，CommandReg寄存器写0x0f，重置RC522.
//ModeReg0x3D，设置CRC错误检测码
//设置计时器。TReloadReg重装值，TModeReg分频高位，TPrescalerReg分频低位
void PcdReset ( void )
{
	RC522_Reset_Disable();
	
	delay_us ( 1 );
	
	RC522_Reset_Enable();
	
	delay_us ( 1 );
	
	RC522_Reset_Disable();
	
	delay_us ( 1 );
	
	WriteRawRC ( CommandReg, 0x0f );
	
	while ( ReadRawRC ( CommandReg ) & 0x10 );
	
	delay_us ( 1 );
	
	
  WriteRawRC ( ModeReg, 0x3D );            //定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363
	
  WriteRawRC ( TReloadRegL, 30 );          //16位定时器低位    
	WriteRawRC ( TReloadRegH, 0 );			     //16位定时器高位
	
  WriteRawRC ( TModeReg, 0x8D );				   //定义内部定时器的设置
	
  WriteRawRC ( TPrescalerReg, 0x3E );			 //设置定时器分频系数
	
	WriteRawRC ( TxAutoReg, 0x40 );				   //调制发送信号为100%ASK	
	
}



/**
  * @brief  设置RC522的工作方式
  * @param  ucType，工作方式
  * @retval 无
  */
//开启读写器模式
//Status2Reg第三位清零，读写器重置
void M500PcdConfigISOType ( uint8_t ucType )
{
	if ( ucType == 'A')                     //ISO14443_A
  {
		ClearBitMask ( Status2Reg, 0x08 );
		
    WriteRawRC ( ModeReg, 0x3D );//3F
		//CRC检错码初始值位0x6363
		
		WriteRawRC ( RxSelReg, 0x86 );//84 
		//86 前四位8，表示内部模拟调制信号
		//选择内部接收器设置，内部模拟部分调制信号，发送数据后延迟6个位时钟接收，这个延迟是用来帧保护
		
		WriteRawRC( RFCfgReg, 0x7F );   //4F 配置接收器 48dB最大增益
		
		//下面四个函数是重置计时器
		WriteRawRC( TReloadRegL, 30 );//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		
		WriteRawRC ( TReloadRegH, 0 );
		
		WriteRawRC ( TModeReg, 0x8D );
		
		WriteRawRC ( TPrescalerReg, 0x3E );
		
		delay_us ( 2 );
		
		PcdAntennaOn ();//开天线
		
   }	 
}



/**
  * @brief  通过RC522和ISO14443卡通讯
  * @param  ucCommand，RC522命令字
  * @param  pInData，通过RC522发送到卡片的数据，数据内容，写进FIFOdata（FIFO缓冲区）
  * @param  ucInLenByte，发送数据的字节长度，数据长度
  * @param  pOutData，接收到的卡片返回数据，接受到数据放到这个数组里
  * @param  pOutLenBit，返回数据的位长度
  * @retval 状态值= MI_OK，成功
  */
char PcdComMF522 ( uint8_t ucCommand, 
                    uint8_t * pInData,
                    uint8_t ucInLenByte, 
                    uint8_t * pOutData, 
                    uint32_t * pOutLenBit )		
{
  char cStatus = MI_ERR;     //返回值，先是默认错误
  uint8_t ucIrqEn   = 0x00;		//用来开启允许的中断，讲允许的中断使能
  uint8_t ucWaitFor = 0x00;  
  uint8_t ucLastBits;
  uint8_t ucN;
  uint32_t ul;

  switch ( ucCommand )
  {
     case PCD_AUTHENT:		//Mifare认证  ，验证密钥
        ucIrqEn   = 0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
        ucWaitFor = 0x10;		//认证寻卡等待时候 查询空闲中断标志位
        break;
     
     case PCD_TRANSCEIVE:		//接收发送 发送接收
        ucIrqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        ucWaitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与 空闲中断标志位
        break;
     
     default:
       break;     
  }
	//1.中断初始化
  WriteRawRC ( ComIEnReg, ucIrqEn | 0x80 );		//ComIEnReg中断使能寄存器。IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反 
  ClearBitMask ( ComIrqReg, 0x80 );			//ComIrqReg中断请求寄存器。Set1该位清零时，CommIRqReg的屏蔽位清零
  WriteRawRC ( CommandReg, PCD_IDLE );		//写空闲命令。CommandReg后四位是命令
  SetBitMask ( FIFOLevelReg, 0x80 );			//置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除
	//2.写数据写指令
  for ( ul = 0; ul < ucInLenByte; ul ++ )
    WriteRawRC ( FIFODataReg, pInData [ ul ] );    		//写数据进FIFOdata（FIFO缓冲区）。
    
  WriteRawRC ( CommandReg, ucCommand );					//写命令。CommandReg后四位是命令

	//3.发数据
  if ( ucCommand == PCD_TRANSCEIVE )
    SetBitMask(BitFramingReg,0x80);  				//StartSend置位启动数据发送 该位与收发命令使用时才有效
	//4.等待触发中断
  ul = 1000;//根据时钟频率调整，操作M1卡最大等待时间25ms

  do 														//认证 与寻卡等待时间	
  {
       ucN = ReadRawRC ( ComIrqReg );							//查询事件中断
       ul --;
  } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );		//退出条件i=0,定时器中断，与写空闲命令

  ClearBitMask ( BitFramingReg, 0x80 );					//清理允许StartSend位，StartSend位是启动数据的收发（发完数据了）

	
	//5.接收数据。下面这部分，先排除一些错误，然后计算有多少位数据pOutLenBit，并把数据放到pOutData
  if ( ul != 0 )	//等待超时
  {
    if ( ! ( ReadRawRC ( ErrorReg ) & 0x1B ) )			//读错误标志寄存器BufferOfI（FIFO缓冲区满了） CollErr（检测到冲突） ParityErr（奇偶校验错了） ProtocolErr（SOF出错）
    {
      cStatus = MI_OK;
      
      if ( ucN & ucIrqEn & 0x01 )					//是否发生定时器中断
        cStatus = MI_NOTAGERR;   
        
      if ( ucCommand == PCD_TRANSCEIVE )  //发送并接收数据命令
      {
        ucN = ReadRawRC ( FIFOLevelReg );			//FIFOLevelReg，FIFOdata（FIFO缓冲区）保存多少个字节数  读FIFO中保存的字节数
        
        ucLastBits = ReadRawRC ( ControlReg ) & 0x07;	//最后接收到得字节的有效位数
        
        if ( ucLastBits )
          * pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	//N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
        else
          * pOutLenBit = ucN * 8;   					//最后接收到的字节整个字节有效
        
        if ( ucN == 0 )		
          ucN = 1;    
        
        if ( ucN > MAXRLEN )
          ucN = MAXRLEN;   
        
        for ( ul = 0; ul < ucN; ul ++ )
          pOutData [ ul ] = ReadRawRC ( FIFODataReg );   
        
        }        
    }   
    else
      cStatus = MI_ERR;       
  }

  SetBitMask ( ControlReg, 0x80 );           // stop timer now
  WriteRawRC ( CommandReg, PCD_IDLE ); 
   
  return cStatus;
}

/**
  * @brief  寻卡
  * @param  ucReq_code，寻卡方式 = 0x52，寻感应区内所有符合14443A标准的卡；寻卡方式= 0x26，寻未进入休眠状态的卡
  * @param  pTagType，卡片类型代码，返回值也在这个数组，+1是显示返回值
             = 0x4400，Mifare_UltraLight
             = 0x0400，Mifare_One(S50)
             = 0x0200，Mifare_One(S70)
             = 0x0800，Mifare_Pro(X))
             = 0x4403，Mifare_DESFire
  * @retval 状态值= MI_OK，成功
  */
char PcdRequest ( uint8_t ucReq_code, uint8_t * pTagType )
{
  char cStatus;  
  uint8_t ucComMF522Buf [ MAXRLEN ];  //发送和接受都用这个数组ucComMF522Buf
  uint32_t ulLen;


  ClearBitMask ( Status2Reg, 0x08 );	//清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
  WriteRawRC ( BitFramingReg, 0x07 );	//	发送的最后一个字节的 七位
  SetBitMask ( TxControlReg, 0x03 );	//TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号

  ucComMF522Buf [ 0 ] = ucReq_code;		//存入 卡片命令字

  cStatus = PcdComMF522 ( PCD_TRANSCEIVE,	ucComMF522Buf, 1, ucComMF522Buf, & ulLen );	//寻卡  

  if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//寻卡成功返回卡类型 
  {    
     * pTagType = ucComMF522Buf [ 0 ];
     * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
  }

  else
   cStatus = MI_ERR;

  return cStatus;	 
}

/**
  * @brief  防冲撞
  * @param  pSnr，卡片序列号，4字节。返回值也在这个数组
  * @retval 状态值= MI_OK，成功
  */
char PcdAnticoll ( uint8_t * pSnr )
{
  char cStatus;
  uint8_t uc, ucSnr_check = 0;
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;
  

  ClearBitMask ( Status2Reg, 0x08 );		 //Status2Reg收发状态寄存器，清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
  WriteRawRC ( BitFramingReg, 0x00);		//BitFramingReg低3位表示最后一个字节位数，清理寄存器 停止收发
  ClearBitMask ( CollReg, 0x80 );			 //清ValuesAfterColl所有接收的位在冲突后被清除
 
  ucComMF522Buf [ 0 ] = 0x93;	//卡片防冲突命令
  ucComMF522Buf [ 1 ] = 0x20;
 
  cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//与卡片通信

  if ( cStatus == MI_OK)		//通信成功
  {
    for ( uc = 0; uc < 4; uc ++ )
    {
       * ( pSnr + uc )  = ucComMF522Buf [ uc ];			//读出UID
       ucSnr_check ^= ucComMF522Buf [ uc ];  //^异或符号
    } 
    
    if ( ucSnr_check != ucComMF522Buf [ uc ] )
      cStatus = MI_ERR;    				 
  }
  
  SetBitMask ( CollReg, 0x80 );
      
  return cStatus;		
}


/**
  * @brief  用RC522计算CRC16
  * @param  pIndata，计算CRC16的数组
  * @param  ucLen，计算CRC16的数组字节长度
  * @param   pOutData，存放计算结果存放的首地址
  * @retval 无
  */
//用来生成CRC校验码
// 同PcdComMF522函数类似，写数据到FIFODataReg寄存器，然后发送，然后接受返回值，放到结果寄存器那
void CalulateCRC ( uint8_t * pIndata, uint8_t ucLen, uint8_t * pOutData )
{
  uint8_t uc, ucN;


  ClearBitMask(DivIrqReg,0x04);

  WriteRawRC(CommandReg,PCD_IDLE);  //CommandReg后四位命令，取消当前命令

  SetBitMask(FIFOLevelReg,0x80);

  for ( uc = 0; uc < ucLen; uc ++)
    WriteRawRC ( FIFODataReg, * ( pIndata + uc ) );   

  WriteRawRC ( CommandReg, PCD_CALCCRC );

  uc = 0xFF;

  do 
  {
      ucN = ReadRawRC ( DivIrqReg );
      uc --;
  } while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );
  
  pOutData [ 0 ] = ReadRawRC ( CRCResultRegL );
  pOutData [ 1 ] = ReadRawRC ( CRCResultRegM );		
}


/**
  * @brief  选定卡片
  * @param  pSnr，卡片序列号，4字节
  * @retval 状态值= MI_OK，成功
  */
char PcdSelect ( uint8_t * pSnr )
{
  char ucN;
  uint8_t uc;
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t  ulLen;
  
  
  ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
  ucComMF522Buf [ 1 ] = 0x70;
  ucComMF522Buf [ 6 ] = 0;
	//把指令和内容放到要发送的数组里
  for ( uc = 0; uc < 4; uc ++ )
  {
    ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
    ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
  }
  //把生成校验码放到要发送的数组里
  CalulateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );

  ClearBitMask ( Status2Reg, 0x08 );
	//发送等待接受
  ucN = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );
  
  if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
    ucN = MI_OK;  
  else
    ucN = MI_ERR;    
  
  return ucN;		
}



/**
  * @brief  验证卡片密码
  * @param  ucAuth_mode，密码验证模式= 0x60，验证A密钥，密码验证模式= 0x61，验证B密钥
  * @param  uint8_t ucAddr，块地址
  * @param  pKey，密码 
  * @param  pSnr，卡片序列号，4字节
  * @retval 状态值= MI_OK，成功
  */
char PcdAuthState ( uint8_t ucAuth_mode, uint8_t ucAddr, uint8_t * pKey, uint8_t * pSnr )
{
  char cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
  uint32_t ulLen;
  

  ucComMF522Buf [ 0 ] = ucAuth_mode;
  ucComMF522Buf [ 1 ] = ucAddr;

  for ( uc = 0; uc < 6; uc ++ )
    ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );   

  for ( uc = 0; uc < 6; uc ++ )
    ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );   

  cStatus = PcdComMF522 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, & ulLen );

  if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC ( Status2Reg ) & 0x08 ) ) )  //成功执行MFAuthent命令
    cStatus = MI_ERR;   
    
  return cStatus;
}


/**
  * @brief  写数据到M1卡一块
  * @param  uint8_t ucAddr，块地址 (0x10-0x20)
  * @param  pData，写入的数据，16字节
  * @retval 状态值= MI_OK，成功
  */
char PcdWrite ( uint8_t ucAddr, uint8_t * pData )
{
  char cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
  uint32_t ulLen;
   
  
  ucComMF522Buf [ 0 ] = PICC_WRITE;
  ucComMF522Buf [ 1 ] = ucAddr;

  CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

  cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

  if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
    cStatus = MI_ERR;   
      
  if ( cStatus == MI_OK )
  {
    //memcpy(ucComMF522Buf, pData, 16);
    for ( uc = 0; uc < 16; uc ++ )
      ucComMF522Buf [ uc ] = * ( pData + uc );  
    
    CalulateCRC ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );
    
    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
      cStatus = MI_ERR;   
    
  } 	
  return cStatus;		
}


/**
  * @brief  读取M1卡一块数据
  * @param  ucAddr，块地址 (0x10-0x20)
  * @param  pData，读出的数据，16字节
  * @retval 状态值= MI_OK，成功
  */
char PcdRead ( uint8_t ucAddr, uint8_t * pData )
{
  char cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;
  
  ucComMF522Buf [ 0 ] = PICC_READ;
  ucComMF522Buf [ 1 ] = ucAddr;

  CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 
  cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

  if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
  {
    for ( uc = 0; uc < 16; uc ++ )
      * ( pData + uc ) = ucComMF522Buf [ uc ];   
  }
  
  else
    cStatus = MI_ERR;   
   
  return cStatus;		
}


/**
  * @brief  命令卡片进入休眠状态
  * @param  无
  * @retval 状态值= MI_OK，成功
  */
char PcdHalt( void )
{
	uint8_t ucComMF522Buf [ MAXRLEN ]; 
	uint32_t  ulLen;
  

  ucComMF522Buf [ 0 ] = PICC_HALT;
  ucComMF522Buf [ 1 ] = 0;
	
  CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 	PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

  return MI_OK;	
}

/////////////////////////////////////////////////////////////////////
//功    能：写入钱包金额
//参数说明: ucAddr[IN]：块地址
//          pData：写入的金额
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char WriteAmount( uint8_t ucAddr, uint32_t pData )
{
	char status;
	uint8_t ucComMF522Buf[16];
	ucComMF522Buf[0] = (pData&((uint32_t)0x000000ff));
	ucComMF522Buf[1] = (pData&((uint32_t)0x0000ff00))>>8;
	ucComMF522Buf[2] = (pData&((uint32_t)0x00ff0000))>>16;
	ucComMF522Buf[3] = (pData&((uint32_t)0xff000000))>>24;	
	
	ucComMF522Buf[4] = ~(pData&((uint32_t)0x000000ff));
	ucComMF522Buf[5] = ~(pData&((uint32_t)0x0000ff00))>>8;
	ucComMF522Buf[6] = ~(pData&((uint32_t)0x00ff0000))>>16;
	ucComMF522Buf[7] = ~(pData&((uint32_t)0xff000000))>>24;	
	
	ucComMF522Buf[8] = (pData&((uint32_t)0x000000ff));
	ucComMF522Buf[9] = (pData&((uint32_t)0x0000ff00))>>8;
	ucComMF522Buf[10] = (pData&((uint32_t)0x00ff0000))>>16;
	ucComMF522Buf[11] = (pData&((uint32_t)0xff000000))>>24;	
	
	ucComMF522Buf[12] = ucAddr;
	ucComMF522Buf[13] = ~ucAddr;
	ucComMF522Buf[14] = ucAddr;
	ucComMF522Buf[15] = ~ucAddr;
  status = PcdWrite(ucAddr,ucComMF522Buf);
	return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：读取钱包金额
//参数说明: ucAddr[IN]：块地址
//          *pData：读出的金额
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char ReadAmount( uint8_t ucAddr, uint32_t *pData )
{
	
	char status = MI_ERR;
	uint8_t j;
	uint8_t ucComMF522Buf[16];
  status = PcdRead(ucAddr,ucComMF522Buf);
	if(status != MI_OK)
		return status;
	for(j=0;j<4;j++)
	{
		if((ucComMF522Buf[j] != ucComMF522Buf[j+8]) && (ucComMF522Buf[j] != ~ucComMF522Buf[j+4]))//验证一下是不是钱包的数据
		break;
	}
	if(j == 4)
	{
		  status = MI_OK;
			*pData = ucComMF522Buf[0] + (ucComMF522Buf[1]<<8) + (ucComMF522Buf[2]<<16) + (ucComMF522Buf[3]<<24);
	}
	else
	{
		status = MI_ERR;
		*pData = 0;
	}
  return status;	
}
