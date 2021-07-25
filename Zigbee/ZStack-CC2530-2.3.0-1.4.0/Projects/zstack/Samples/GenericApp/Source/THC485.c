#include <ioCC2530.h>
#include "OnBoard.h"
#include "THC485.h"



uint16 temperture;
uint16 temperture_shi, temperture_ge, temperture_xiao;
uint16 humidity485;
uint16 humidity_shi, humidity_ge, humidity_xiao;
uint16 CO2;
uint16 CO2_qian, CO2_bai, CO2_shi, CO2_ge;

uint8 HATstr[13];


uint16 GetCrcData(uint8 *buffer, uint16 len)
{
  uint16 wcrc = 0XFFFF;//16位crc寄存器预置
  uint8 temp;
  uint16 i = 0, j = 0;//计数
  for (i = 0; i < len; i++)//循环计算每个数据
  {
    temp = *buffer & 0X00FF;//将八位数据与crc寄存器亦或
    buffer++;//指针地址增加，指向下个数据
    wcrc ^= temp;//将数据存入crc寄存器
    for (j = 0; j < 8; j++)//循环计算数据的
    {
      if (wcrc & 0X0001)//判断右移出的是不是1，如果是1则与多项式进行异或。
      {
        wcrc >>= 1;//先将数据右移一位
        wcrc ^= 0XA001;//与上面的多项式进行异或
      }
      else//如果不是1，则直接移出
      {
        wcrc >>= 1;//直接移出
      }
    }
  }
  uint8 CRC_L;//定义数组
  uint8 CRC_H;
  CRC_L = wcrc&0xff;//crc的低八位
  CRC_H = wcrc >> 8;//crc的高八位
  return ((CRC_L << 8) | CRC_H);
}

uint8 isCRC(uint8 *buffer,uint16 len)
{
  if(buffer[len-2] == 0 || buffer[len-1] == 0){
   return 'f'; 
  }
  uint16 itsCRC = GetCrcData(buffer, len-2);
  uint16 buffCRC = ((buffer[len-2]) << 8 | buffer[len-1]);
  if(itsCRC == buffCRC){
    return 't';
  }else{
    return 'f';
  }
}

uint8 getValue(uint8 *buffer, uint8 len)
{
  if(isCRC(buffer,len) == 't')
  {
    if(len == 7)  //CO2
    {
      CO2 = ((buffer[3]) << 8 | buffer[4]);
       if(CO2 <10000 && CO2>0)
       {
          CO2_qian = CO2/1000;
          CO2_bai = (CO2%1000)/100;
          CO2_shi = (CO2%100)/10;
          CO2_ge = CO2%10;
       }else{
         return false; 
       }
    }
    
    if(len == 9)   //温度湿度
    {
      humidity485 = ((buffer[3]) << 8 | buffer[4]);
       if(humidity485 <1000 && humidity485>0)
       {
          humidity_shi = humidity485/100;
          humidity_ge = (humidity485%100)/10;
          humidity_xiao = humidity485%10;
          HATstr[0] = 'h';
          HATstr[1] = humidity_shi+0x30;
          HATstr[2] = humidity_ge+0x30;
          HATstr[3] = '.';
          HATstr[4] = humidity_xiao+0x30;
          HATstr[5] = '%';
          HATstr[6] = '\t';
       }else{
         return false; 
       }
       
       temperture = ((buffer[5]) << 8 | buffer[6]);
       if(temperture <1000 && temperture>0)
       {
          temperture_shi = temperture/100;
          temperture_ge = (temperture%100)/10;
          temperture_xiao = temperture%10;
          HATstr[7] = 't';
          HATstr[8] = temperture_shi+0x30;
          HATstr[9] = temperture_ge+0x30;
          HATstr[10] = '.';
          HATstr[11] = temperture_xiao+0x30;
          HATstr[12] = '\0';
       }else{
         return false; 
       }
    }
    return 1;
  }
  else
  {
   return 0;
  }

}