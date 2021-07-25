#include "industry_win.h"
#include "usart.h"
#include "ui_app.h"
#include "main_win.h"

lv_obj_t *industry_temperture,*industry_humidity,*industry_co2;

#define  BETWEEN_LABEL   60

#define MY_WEN2    "\xE6\xB8\xA9"
#define MY_DU2    "\xE5\xBA\xA6"
#define MY_SI2    "\xE6\xB9\xBF"

char industry_value[3][10]={"00du","00%","00ppm"}; //0温度，1湿度，2CO2

extern u8 now_screen;
void industry_add(u8 t1,u8 t2,u8 h1,u8 h2, u16 co)
{
	sprintf(industry_value[0],"%d.%ddu",t1,t2);
	sprintf(industry_value[1],"%d.%d%%",h1,h2);
	sprintf(industry_value[2],"%dppm",co);
	if(now_screen == 4)
	{
		lv_label_set_text(industry_temperture,industry_value[0]);//设置文本
		lv_label_set_text(industry_humidity,industry_value[1]);//设置文本
		lv_label_set_text(industry_co2,industry_value[2]);//设置文本
		lv_obj_realign(industry_temperture);
		lv_obj_realign(industry_humidity);
		lv_obj_realign(industry_co2);
	}
}

void industry_win_start(lv_obj_t * parent)
{
    lv_obj_t *win;
    win = general_win_create(parent,MY_JING2 MY_ZHUN MY_SHU MY_JU);



  industry_temperture = lv_label_create(win,NULL);//用标签来模拟按钮
	lv_label_set_text(industry_temperture,industry_value[0]);//设置文本
	lv_obj_align(industry_temperture,win,LV_ALIGN_IN_RIGHT_MID,-30,30);//设置对齐方式

  industry_humidity = lv_label_create(win,NULL);//用标签来模拟按钮
	lv_label_set_text(industry_humidity,industry_value[1]);//设置文本
	lv_obj_align(industry_humidity,industry_temperture,LV_ALIGN_OUT_TOP_MID,0,-BETWEEN_LABEL);//设置对齐方式

   industry_co2 = lv_label_create(win,NULL);//用标签来模拟按钮
	lv_label_set_text(industry_co2,industry_value[2]);//设置文本
	lv_obj_align(industry_co2,industry_temperture,LV_ALIGN_OUT_BOTTOM_MID,0,BETWEEN_LABEL);//设置对齐方式

	lv_obj_t* industry_temperture_label = lv_label_create(win,NULL);
	lv_label_set_text(industry_temperture_label,MY_JING2 MY_ZHUN MY_WEN2 MY_DU2);//设置文本
	lv_obj_align(industry_temperture_label,industry_temperture,LV_ALIGN_OUT_LEFT_MID,-80,0);//设置对齐方式

	lv_obj_t* industry_humidity_label = lv_label_create(win,NULL);
	lv_label_set_text(industry_humidity_label,MY_JING2 MY_ZHUN MY_SI2 MY_DU2);//设置文本
	lv_obj_align(industry_humidity_label,industry_temperture_label,LV_ALIGN_OUT_TOP_MID,0,-BETWEEN_LABEL);//设置对齐方式

	lv_obj_t* industry_co2_label = lv_label_create(win,NULL);
	lv_label_set_text(industry_co2_label,MY_JING2 MY_ZHUN "CO2");//设置文本
	lv_obj_align(industry_co2_label,industry_temperture_label,LV_ALIGN_OUT_BOTTOM_MID,0,BETWEEN_LABEL);//设置对齐方式
}
