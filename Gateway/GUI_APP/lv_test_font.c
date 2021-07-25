//#include "lv_test_font.h"
//#include "lvgl.h"

//LV_FONT_DECLARE(my_font3);//申明字体

////定义图标
////#define MY_ICON_WIFI	"\xEE\x9F\xA0" //wifi图标
////#define MY_ICON_USB		"\xEE\xB5\xAA" //usb图标
//#define MY_ICON_WIFI    "\xEE\x98\xa5"   //0xee98a5
//#define MY_ICON_HOME    "\xEE\x98\x8e"   //0xee988e

////例程入口函数
//void lv_font_test_start()
//{
//	lv_obj_t* src = lv_scr_act();//获取当前活跃的屏幕对象

//	static lv_style_t my_style;
//	lv_style_copy(&my_style,&lv_style_plain_color);//样式拷贝
//	my_style.text.font = &my_font3;//在样式中使用字体

//	lv_obj_t* label = lv_label_create(src,NULL);//创建标签控件
//	lv_label_set_style(label,LV_LABEL_STYLE_MAIN,&my_style);//设置样式
//	lv_label_set_text(label,"肥屁仔abcxzy190"MY_ICON_WIFI MY_ICON_HOME);//设置文本
//	lv_label_set_body_draw(label,true);//使能绘制背景
//	lv_obj_align(label,NULL,LV_ALIGN_CENTER,0,0);//标签与屏幕保持居中对齐
//}


