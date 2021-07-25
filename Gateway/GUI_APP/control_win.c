#include "control_win.h"
#include "usart.h"
#include "ui_app.h"
#include "usart3.h"
#include "main_win.h"


#define MY_DA    "\xE5\xA4\xA7"  //E5AEB6
#define MY_DENG    "\xE7\x81\xAF"  //E5AEB6
#define MY_JING3    "\xE8\xAD\xA6"  //E5AEB6
#define MY_BAO    "\xE6\x8A\xA5"  //E5AEB6
#define MY_FENG    "\xE9\xA3\x8E"  //E5AEB6
#define MY_SAN    "\xE6\x89\x87"  //E5AEB6

#define MY_ZI2    "\xE8\x87\xAA"
#define MY_DONG2    "\xE5\x8A\xA8"
#define MY_KAI    "\xE5\xBC\x80"  //E887AA E58AA8 E5BC80 E585B3
#define MY_GUAN2    "\xE5\x85\xB3"  //E5AEB6

#define ORDER_LEN  4
static lv_obj_t *control1_label, *control2_label, *control3_label;
lv_obj_t *roller1,*roller2,*roller3;
static u8 order[ORDER_LEN];

static void event_handler(lv_obj_t * obj,lv_event_t event)
{
    if(event==LV_EVENT_VALUE_CHANGED)
    {
			u8 roller1_selected = 0,roller2_selected = 0, roller3_selected = 0;

			roller1_selected = lv_roller_get_selected(roller1);
			roller2_selected = lv_roller_get_selected(roller2);
			roller3_selected = lv_roller_get_selected(roller3);
			
			order[0] = '0';
			order[1] = roller1_selected+'0';
			order[2] = roller2_selected+'0';
			order[3] = roller3_selected+'0';
			
			USART3_Send_Data(order,ORDER_LEN);
    }
}

void control_win_start(lv_obj_t * parent)
{
    lv_obj_t *win;
		win = general_win_create(parent,MY_ZONG MY_KONG MY_ZHI);

    roller1 = lv_roller_create(win,NULL);
    lv_roller_set_options(roller1, MY_ZI2 MY_DONG2 "\n" MY_KAI "\n" MY_GUAN2,LV_ROLLER_MODE_INIFINITE);//设置所有的选项值,循环滚动模式
    lv_roller_set_selected(roller1,0,LV_ANIM_OFF);//设置默认选中值为 Guangzhou
    lv_roller_set_fix_width(roller1,140);//设置固定宽度
    lv_roller_set_visible_row_count(roller1,3);//设置可见的行数
    lv_obj_align(roller1,NULL,LV_ALIGN_IN_TOP_RIGHT,-5,40);//设置与屏幕居中对齐
    lv_obj_set_event_cb(roller1,event_handler);//注册事件回调函数

    control1_label = lv_label_create(win,NULL);
		lv_label_set_long_mode(control1_label,LV_LABEL_LONG_SROLL);//设置长文本模式
		lv_obj_set_width(control1_label,70);//设置宽度
		lv_label_set_align(control1_label,LV_LABEL_ALIGN_LEFT);//设置文本居中对齐
		lv_obj_align(control1_label,roller1,LV_ALIGN_OUT_LEFT_MID,0,0);//设置对齐方式
		lv_label_set_text(control1_label,MY_DA MY_DENG);//设置文本


    roller2 = lv_roller_create(win,NULL);
    lv_roller_set_options(roller2, MY_ZI2 MY_DONG2 "\n" MY_KAI "\n" MY_GUAN2,LV_ROLLER_MODE_INIFINITE);//设置所有的选项值,循环滚动模式
    lv_roller_set_selected(roller2,0,LV_ANIM_OFF);//设置默认选中值为 Guangzhou
    lv_roller_set_fix_width(roller2,140);//设置固定宽度
    lv_roller_set_visible_row_count(roller2,3);//设置可见的行数
    lv_obj_align(roller2,roller1,LV_ALIGN_OUT_BOTTOM_MID,0,20);//设置与屏幕居中对齐
    lv_obj_set_event_cb(roller2,event_handler);//注册事件回调函数

    control2_label = lv_label_create(win,NULL);
		lv_label_set_long_mode(control2_label,LV_LABEL_LONG_SROLL);//设置长文本模式
		lv_obj_set_width(control2_label,70);//设置宽度
		lv_label_set_align(control2_label,LV_LABEL_ALIGN_LEFT);//设置文本居中对齐
		lv_obj_align(control2_label,roller2,LV_ALIGN_OUT_LEFT_MID,0,0);//设置对齐方式
		lv_label_set_text(control2_label,MY_JING3 MY_BAO);//设置文本
		
		roller3 = lv_roller_create(win,NULL);
    lv_roller_set_options(roller3, MY_ZI2 MY_DONG2 "\n" MY_KAI "\n" MY_GUAN2,LV_ROLLER_MODE_INIFINITE);//设置所有的选项值,循环滚动模式
    lv_roller_set_selected(roller3,0,LV_ANIM_OFF);//设置默认选中值为 Guangzhou
    lv_roller_set_fix_width(roller3,140);//设置固定宽度
    lv_roller_set_visible_row_count(roller3,3);//设置可见的行数
    lv_obj_align(roller3,roller2,LV_ALIGN_OUT_BOTTOM_MID,0,20);//设置与屏幕居中对齐
    lv_obj_set_event_cb(roller3,event_handler);//注册事件回调函数

    control3_label = lv_label_create(win,NULL);
		lv_label_set_long_mode(control3_label,LV_LABEL_LONG_SROLL);//设置长文本模式
		lv_obj_set_width(control3_label,70);//设置宽度
		lv_label_set_align(control3_label,LV_LABEL_ALIGN_LEFT);//设置文本居中对齐
		lv_obj_align(control3_label,roller3,LV_ALIGN_OUT_LEFT_MID,0,0);//设置对齐方式
		lv_label_set_text(control3_label,MY_FENG MY_SAN);//设置文本
}
