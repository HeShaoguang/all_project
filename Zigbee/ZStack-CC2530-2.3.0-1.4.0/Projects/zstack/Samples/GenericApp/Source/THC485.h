#ifndef __DHT11_H__
#define __DHT11_H__

extern uint8 Rx1_buff[10];
extern uint8 Rx1_num=0;

extern uint16 temperture;
extern uint16 temperture_shi, temperture_ge, temperture_xiao;
extern uint16 humidity485;
extern uint16 humidity_shi, humidity_ge, humidity_xiao;
extern uint16 CO2;
extern uint16 CO2_qian, CO2_bai, CO2_shi, CO2_ge;

extern uint8 isCRC(uint8 *buffer,uint16 len);
extern uint16 GetCrcData(uint8 *buffer, uint16 len);
extern uint8 getValue(uint8 *buffer, uint8 len);



#endif

