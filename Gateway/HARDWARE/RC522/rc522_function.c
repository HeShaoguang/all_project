#include "rc522_function.h"
#include "rc522_config.h"
#include "stm32f10x.h"
#include "delay.h"


#define   RC522_DELAY()  delay_us ( 2 )

 /**
  * @brief  ��RC522����1 Byte ����
  * @param  byte��Ҫ���͵�����
  * @retval RC522���ص�����
  */
	
	//һ��һ��λ�ط��ͣ����������ƣ�׼���÷���һλ��һ����8λһ���ֽ�
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
  * @brief  ��RC522����1 Byte ����
  * @param  ��
  * @retval RC522���ص�����
  */
//һ����λ��ȡ����һ�Ͱ�������λ��1��Ȼ������
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
  * @brief  ��RC522�Ĵ���
  * @param  ucAddress���Ĵ�����ַ
  * @retval �Ĵ����ĵ�ǰֵ
  */
//��ʵ��ucAddress�ǼĴ����ĵ�ַ��һ����4*16=64���Ĵ�����������ͷ�ļ��
//����һ���Ĵ�����ַ��ȥ��rc522�ͻ᷵�ؼĴ�����ֵ�������������Ĵ�����ֵ�ͺ���
uint8_t ReadRawRC ( uint8_t ucAddress )
{
	uint8_t ucAddr, ucReturn;		//�Ĵ�����Ч��ַ���Ĵ�������ֵ
	
	ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;	//�����Ч��ַ��ʽ�����Ϊ0�����λΪ1
	RC522_CS_Enable();
	SPI_RC522_SendByte ( ucAddr );
	ucReturn = SPI_RC522_ReadByte ();
	RC522_CS_Disable();
	
	return ucReturn;	
}

/**
  * @brief  дRC522�Ĵ���
  * @param  ucAddress���Ĵ�����ַ
  * @param  ucValue��д��Ĵ�����ֵ
  * @retval ��
  */
//����һ���Ĵ�����ַ��ȥ���ٷ���Ҫд��ֵ���Ϳ���д���Ĵ���
void WriteRawRC ( uint8_t ucAddress, uint8_t ucValue )
{  
	uint8_t ucAddr;
	
	ucAddr = ( ucAddress << 1 ) & 0x7E;	//�����Ч��ַ��ʽ�����Ϊ0�����λΪ1
	RC522_CS_Enable();	
	SPI_RC522_SendByte ( ucAddr );	
	SPI_RC522_SendByte ( ucValue );
	RC522_CS_Disable();		
}


/**
  * @brief  ��RC522�Ĵ�����λ
  * @param  ucReg���Ĵ�����ַ
  * @param   ucMask����λֵ
  * @retval ��
  */

//�ѼĴ�����ԭ������Ϊ0��λ����Ϊ1���Ҳ�Ӱ������λ
void SetBitMask ( uint8_t ucReg, uint8_t ucMask )  
{
   uint8_t ucTemp;
	
   ucTemp = ReadRawRC ( ucReg );
   WriteRawRC ( ucReg, ucTemp | ucMask );         // set bit mask
}


/**
  * @brief  ��RC522�Ĵ�����λ
  * @param  ucReg���Ĵ�����ַ
  * @param  ucMask����λֵ
  * @retval ��
  */
//�ѼĴ�����ԭ������Ϊ1��λ����Ϊ0���Ҳ�Ӱ������λ
void ClearBitMask ( uint8_t ucReg, uint8_t ucMask )  
{
   uint8_t ucTemp;
	
   ucTemp = ReadRawRC ( ucReg );
   WriteRawRC ( ucReg, ucTemp & ( ~ ucMask) );  // clear bit mask
}


/**
  * @brief  �������� 
  * @param  ��
  * @retval ��
  */
//TxControlReg ���������������Ĺܼ�TX1��TX2���߼����ԣ�0x03�൱��ʹ��
void PcdAntennaOn ( void )
{
   uint8_t uc;
  
   uc = ReadRawRC ( TxControlReg );
   if ( ! ( uc & 0x03 ) )
		SetBitMask(TxControlReg, 0x03);		
}


/**
  * @brief  �ر�����
  * @param  ��
  * @retval ��
  */
//�������һ��������Ӧ���൱�ڹر�ʹ��
void PcdAntennaOff ( void )
{
  ClearBitMask ( TxControlReg, 0x03 );	
}


/**
  * @brief  ��λRC522 
  * @param  ��
  * @retval ��
  */
//����RC522��ʱ������
//RESET���������ã�CommandReg�Ĵ���д0x0f������RC522.
//ModeReg0x3D������CRC��������
//���ü�ʱ����TReloadReg��װֵ��TModeReg��Ƶ��λ��TPrescalerReg��Ƶ��λ
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
	
	
  WriteRawRC ( ModeReg, 0x3D );            //���巢�ͺͽ��ճ���ģʽ ��Mifare��ͨѶ��CRC��ʼֵ0x6363
	
  WriteRawRC ( TReloadRegL, 30 );          //16λ��ʱ����λ    
	WriteRawRC ( TReloadRegH, 0 );			     //16λ��ʱ����λ
	
  WriteRawRC ( TModeReg, 0x8D );				   //�����ڲ���ʱ��������
	
  WriteRawRC ( TPrescalerReg, 0x3E );			 //���ö�ʱ����Ƶϵ��
	
	WriteRawRC ( TxAutoReg, 0x40 );				   //���Ʒ����ź�Ϊ100%ASK	
	
}



/**
  * @brief  ����RC522�Ĺ�����ʽ
  * @param  ucType��������ʽ
  * @retval ��
  */
//������д��ģʽ
//Status2Reg����λ���㣬��д������
void M500PcdConfigISOType ( uint8_t ucType )
{
	if ( ucType == 'A')                     //ISO14443_A
  {
		ClearBitMask ( Status2Reg, 0x08 );
		
    WriteRawRC ( ModeReg, 0x3D );//3F
		//CRC������ʼֵλ0x6363
		
		WriteRawRC ( RxSelReg, 0x86 );//84 
		//86 ǰ��λ8����ʾ�ڲ�ģ������ź�
		//ѡ���ڲ����������ã��ڲ�ģ�ⲿ�ֵ����źţ��������ݺ��ӳ�6��λʱ�ӽ��գ�����ӳ�������֡����
		
		WriteRawRC( RFCfgReg, 0x7F );   //4F ���ý����� 48dB�������
		
		//�����ĸ����������ü�ʱ��
		WriteRawRC( TReloadRegL, 30 );//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		
		WriteRawRC ( TReloadRegH, 0 );
		
		WriteRawRC ( TModeReg, 0x8D );
		
		WriteRawRC ( TPrescalerReg, 0x3E );
		
		delay_us ( 2 );
		
		PcdAntennaOn ();//������
		
   }	 
}



/**
  * @brief  ͨ��RC522��ISO14443��ͨѶ
  * @param  ucCommand��RC522������
  * @param  pInData��ͨ��RC522���͵���Ƭ�����ݣ��������ݣ�д��FIFOdata��FIFO��������
  * @param  ucInLenByte���������ݵ��ֽڳ��ȣ����ݳ���
  * @param  pOutData�����յ��Ŀ�Ƭ�������ݣ����ܵ����ݷŵ����������
  * @param  pOutLenBit���������ݵ�λ����
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdComMF522 ( uint8_t ucCommand, 
                    uint8_t * pInData,
                    uint8_t ucInLenByte, 
                    uint8_t * pOutData, 
                    uint32_t * pOutLenBit )		
{
  char cStatus = MI_ERR;     //����ֵ������Ĭ�ϴ���
  uint8_t ucIrqEn   = 0x00;		//��������������жϣ���������ж�ʹ��
  uint8_t ucWaitFor = 0x00;  
  uint8_t ucLastBits;
  uint8_t ucN;
  uint32_t ul;

  switch ( ucCommand )
  {
     case PCD_AUTHENT:		//Mifare��֤  ����֤��Կ
        ucIrqEn   = 0x12;		//��������ж�����ErrIEn  ��������ж�IdleIEn
        ucWaitFor = 0x10;		//��֤Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ
        break;
     
     case PCD_TRANSCEIVE:		//���շ��� ���ͽ���
        ucIrqEn   = 0x77;		//����TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        ucWaitFor = 0x30;		//Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ�� �����жϱ�־λ
        break;
     
     default:
       break;     
  }
	//1.�жϳ�ʼ��
  WriteRawRC ( ComIEnReg, ucIrqEn | 0x80 );		//ComIEnReg�ж�ʹ�ܼĴ�����IRqInv��λ�ܽ�IRQ��Status1Reg��IRqλ��ֵ�෴ 
  ClearBitMask ( ComIrqReg, 0x80 );			//ComIrqReg�ж�����Ĵ�����Set1��λ����ʱ��CommIRqReg������λ����
  WriteRawRC ( CommandReg, PCD_IDLE );		//д�������CommandReg����λ������
  SetBitMask ( FIFOLevelReg, 0x80 );			//��λFlushBuffer����ڲ�FIFO�Ķ���дָ���Լ�ErrReg��BufferOvfl��־λ�����
	//2.д����дָ��
  for ( ul = 0; ul < ucInLenByte; ul ++ )
    WriteRawRC ( FIFODataReg, pInData [ ul ] );    		//д���ݽ�FIFOdata��FIFO����������
    
  WriteRawRC ( CommandReg, ucCommand );					//д���CommandReg����λ������

	//3.������
  if ( ucCommand == PCD_TRANSCEIVE )
    SetBitMask(BitFramingReg,0x80);  				//StartSend��λ�������ݷ��� ��λ���շ�����ʹ��ʱ����Ч
	//4.�ȴ������ж�
  ul = 1000;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms

  do 														//��֤ ��Ѱ���ȴ�ʱ��	
  {
       ucN = ReadRawRC ( ComIrqReg );							//��ѯ�¼��ж�
       ul --;
  } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );		//�˳�����i=0,��ʱ���жϣ���д��������

  ClearBitMask ( BitFramingReg, 0x80 );					//��������StartSendλ��StartSendλ���������ݵ��շ������������ˣ�

	
	//5.�������ݡ������ⲿ�֣����ų�һЩ����Ȼ������ж���λ����pOutLenBit���������ݷŵ�pOutData
  if ( ul != 0 )	//�ȴ���ʱ
  {
    if ( ! ( ReadRawRC ( ErrorReg ) & 0x1B ) )			//�������־�Ĵ���BufferOfI��FIFO���������ˣ� CollErr����⵽��ͻ�� ParityErr����żУ����ˣ� ProtocolErr��SOF����
    {
      cStatus = MI_OK;
      
      if ( ucN & ucIrqEn & 0x01 )					//�Ƿ�����ʱ���ж�
        cStatus = MI_NOTAGERR;   
        
      if ( ucCommand == PCD_TRANSCEIVE )  //���Ͳ�������������
      {
        ucN = ReadRawRC ( FIFOLevelReg );			//FIFOLevelReg��FIFOdata��FIFO��������������ٸ��ֽ���  ��FIFO�б�����ֽ���
        
        ucLastBits = ReadRawRC ( ControlReg ) & 0x07;	//�����յ����ֽڵ���Чλ��
        
        if ( ucLastBits )
          * pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	//N���ֽ�����ȥ1�����һ���ֽڣ�+���һλ��λ�� ��ȡ����������λ��
        else
          * pOutLenBit = ucN * 8;   					//�����յ����ֽ������ֽ���Ч
        
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
  * @brief  Ѱ��
  * @param  ucReq_code��Ѱ����ʽ = 0x52��Ѱ��Ӧ�������з���14443A��׼�Ŀ���Ѱ����ʽ= 0x26��Ѱδ��������״̬�Ŀ�
  * @param  pTagType����Ƭ���ʹ��룬����ֵҲ��������飬+1����ʾ����ֵ
             = 0x4400��Mifare_UltraLight
             = 0x0400��Mifare_One(S50)
             = 0x0200��Mifare_One(S70)
             = 0x0800��Mifare_Pro(X))
             = 0x4403��Mifare_DESFire
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdRequest ( uint8_t ucReq_code, uint8_t * pTagType )
{
  char cStatus;  
  uint8_t ucComMF522Buf [ MAXRLEN ];  //���ͺͽ��ܶ����������ucComMF522Buf
  uint32_t ulLen;


  ClearBitMask ( Status2Reg, 0x08 );	//����ָʾMIFARECyptol��Ԫ��ͨ�Լ����п�������ͨ�ű����ܵ����
  WriteRawRC ( BitFramingReg, 0x07 );	//	���͵����һ���ֽڵ� ��λ
  SetBitMask ( TxControlReg, 0x03 );	//TX1,TX2�ܽŵ�����źŴ��ݾ����͵��Ƶ�13.56�������ز��ź�

  ucComMF522Buf [ 0 ] = ucReq_code;		//���� ��Ƭ������

  cStatus = PcdComMF522 ( PCD_TRANSCEIVE,	ucComMF522Buf, 1, ucComMF522Buf, & ulLen );	//Ѱ��  

  if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//Ѱ���ɹ����ؿ����� 
  {    
     * pTagType = ucComMF522Buf [ 0 ];
     * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
  }

  else
   cStatus = MI_ERR;

  return cStatus;	 
}

/**
  * @brief  ����ײ
  * @param  pSnr����Ƭ���кţ�4�ֽڡ�����ֵҲ���������
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdAnticoll ( uint8_t * pSnr )
{
  char cStatus;
  uint8_t uc, ucSnr_check = 0;
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;
  

  ClearBitMask ( Status2Reg, 0x08 );		 //Status2Reg�շ�״̬�Ĵ�������MFCryptol Onλ ֻ�гɹ�ִ��MFAuthent����󣬸�λ������λ
  WriteRawRC ( BitFramingReg, 0x00);		//BitFramingReg��3λ��ʾ���һ���ֽ�λ��������Ĵ��� ֹͣ�շ�
  ClearBitMask ( CollReg, 0x80 );			 //��ValuesAfterColl���н��յ�λ�ڳ�ͻ�����
 
  ucComMF522Buf [ 0 ] = 0x93;	//��Ƭ����ͻ����
  ucComMF522Buf [ 1 ] = 0x20;
 
  cStatus = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//�뿨Ƭͨ��

  if ( cStatus == MI_OK)		//ͨ�ųɹ�
  {
    for ( uc = 0; uc < 4; uc ++ )
    {
       * ( pSnr + uc )  = ucComMF522Buf [ uc ];			//����UID
       ucSnr_check ^= ucComMF522Buf [ uc ];  //^������
    } 
    
    if ( ucSnr_check != ucComMF522Buf [ uc ] )
      cStatus = MI_ERR;    				 
  }
  
  SetBitMask ( CollReg, 0x80 );
      
  return cStatus;		
}


/**
  * @brief  ��RC522����CRC16
  * @param  pIndata������CRC16������
  * @param  ucLen������CRC16�������ֽڳ���
  * @param   pOutData����ż�������ŵ��׵�ַ
  * @retval ��
  */
//��������CRCУ����
// ͬPcdComMF522�������ƣ�д���ݵ�FIFODataReg�Ĵ�����Ȼ���ͣ�Ȼ����ܷ���ֵ���ŵ�����Ĵ�����
void CalulateCRC ( uint8_t * pIndata, uint8_t ucLen, uint8_t * pOutData )
{
  uint8_t uc, ucN;


  ClearBitMask(DivIrqReg,0x04);

  WriteRawRC(CommandReg,PCD_IDLE);  //CommandReg����λ���ȡ����ǰ����

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
  * @brief  ѡ����Ƭ
  * @param  pSnr����Ƭ���кţ�4�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
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
	//��ָ������ݷŵ�Ҫ���͵�������
  for ( uc = 0; uc < 4; uc ++ )
  {
    ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
    ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
  }
  //������У����ŵ�Ҫ���͵�������
  CalulateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );

  ClearBitMask ( Status2Reg, 0x08 );
	//���͵ȴ�����
  ucN = PcdComMF522 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );
  
  if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
    ucN = MI_OK;  
  else
    ucN = MI_ERR;    
  
  return ucN;		
}



/**
  * @brief  ��֤��Ƭ����
  * @param  ucAuth_mode��������֤ģʽ= 0x60����֤A��Կ��������֤ģʽ= 0x61����֤B��Կ
  * @param  uint8_t ucAddr�����ַ
  * @param  pKey������ 
  * @param  pSnr����Ƭ���кţ�4�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
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

  if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC ( Status2Reg ) & 0x08 ) ) )  //�ɹ�ִ��MFAuthent����
    cStatus = MI_ERR;   
    
  return cStatus;
}


/**
  * @brief  д���ݵ�M1��һ��
  * @param  uint8_t ucAddr�����ַ (0x10-0x20)
  * @param  pData��д������ݣ�16�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
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
  * @brief  ��ȡM1��һ������
  * @param  ucAddr�����ַ (0x10-0x20)
  * @param  pData�����������ݣ�16�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
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
  * @brief  ���Ƭ��������״̬
  * @param  ��
  * @retval ״ֵ̬= MI_OK���ɹ�
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
//��    �ܣ�д��Ǯ�����
//����˵��: ucAddr[IN]�����ַ
//          pData��д��Ľ��
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���ȡǮ�����
//����˵��: ucAddr[IN]�����ַ
//          *pData�������Ľ��
//��    ��: �ɹ�����MI_OK
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
		if((ucComMF522Buf[j] != ucComMF522Buf[j+8]) && (ucComMF522Buf[j] != ~ucComMF522Buf[j+4]))//��֤һ���ǲ���Ǯ��������
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
