#include "main_win.h"
#include "usart.h"
#include "chart_win.h"
#include "gauge_win.h"
#include "control_win.h"
#include "industry_win.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
u8 now_screen;
lv_obj_t *main_win;
lv_obj_t *gauge_btn,*chart_btn,*control_btn,*industry_btn,*close_btn;
LV_FONT_DECLARE(myzigbee_font);//申明字体

extern TaskHandle_t ScreenTask_Handler;

//事件回调函数
static void event_handler(lv_obj_t * obj,lv_event_t event)
{

	if(obj==gauge_btn)
	{
		if(event==LV_EVENT_CLICKED)
		{
			now_screen = 1;
			guege_win_start(lv_scr_act());//跳转到仪表窗体
		}
	}else if(obj==chart_btn)
	{
		if(event==LV_EVENT_CLICKED)
		{
			now_screen = 2;
			char_win_start(lv_scr_act());//跳转到图表窗体
		}
			
	}else if(obj==control_btn)
	{
		if(event==LV_EVENT_CLICKED)
		{
			now_screen = 3;
			control_win_start(lv_scr_act());//跳转到控制窗体
		}
			
	}else if(obj==industry_btn)
	{
		if(event==LV_EVENT_CLICKED)
		{
			now_screen = 4;
			industry_win_start(lv_scr_act());//跳转到控制窗体
		}
			
	}else if(obj==close_btn)
	{
		if(event==LV_EVENT_CLICKED)
			vTaskSuspend(ScreenTask_Handler);/* 挂起任务 */
			GPIO_SetBits(GPIOE,GPIO_Pin_5);  //关灯
			GPIO_SetBits(GPIOB,GPIO_Pin_5);  //关灯
	}
}


void main_win_create(lv_obj_t * parent)
{
    main_win = lv_page_create(parent, NULL);
    lv_obj_set_size(main_win, 240, 320);   // !!!!
    lv_page_set_sb_mode(main_win, LV_SB_MODE_DRAG);
    lv_page_set_edge_flash(main_win,true);
    lv_page_set_style(main_win,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);//设置scrl容器的样式为透明


    //创建标题
	static lv_style_t title_style;
	lv_obj_t * title_label = lv_label_create(main_win,NULL);
	lv_style_copy(&title_style,&lv_style_plain_color);//复制标签之前的主题样式
	title_style.text.font = &myzigbee_font;//改变字体
	//title_style.text.color = LV_COLOR_WHITE;
	lv_label_set_style(title_label,LV_LABEL_STYLE_MAIN,&title_style);//设置新的样式
	lv_label_set_text(title_label,MY_HUAN MY_JING MY_XIAO MY_GUAN MY_JIA);    ////!!!
	lv_obj_align(title_label,NULL,LV_ALIGN_IN_TOP_MID,0,10);

	//创建列表
	lv_obj_t * list = lv_list_create(main_win,NULL);
    lv_obj_set_size(list,lv_obj_get_width(main_win)*0.9f,lv_obj_get_height(main_win)-60);
	lv_obj_align(list,title_label,LV_ALIGN_OUT_BOTTOM_MID,0,10);
	lv_list_set_sb_mode(list,LV_SB_MODE_AUTO);
	//添加列表项1
    gauge_btn = lv_list_add_btn(list,NULL,MY_YI MY_BIAO MY_TU);   // !!!
    lv_obj_t * item_label1 = lv_list_get_btn_label(gauge_btn);
    lv_label_set_align(item_label1, LV_LABEL_ALIGN_CENTER);   //居中
	lv_obj_set_event_cb(gauge_btn,event_handler);//设置事件回调函数
	//添加列表项2
    chart_btn = lv_list_add_btn(list,NULL,MY_ZHE MY_XIAN MY_TU);  // !!!
    lv_obj_t * item_label2 = lv_list_get_btn_label(chart_btn);
    lv_label_set_align(item_label2, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_event_cb(chart_btn,event_handler);//设置事件回调函数
	//添加列表项3

	industry_btn = lv_list_add_btn(list,NULL,MY_JING2 MY_ZHUN MY_SHU MY_JU);// !!!
    lv_obj_t * item_label4 = lv_list_get_btn_label(industry_btn);
    lv_label_set_align(item_label4, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_event_cb(industry_btn,event_handler);//设置事件回调函数
	//添加列表项4
	control_btn = lv_list_add_btn(list,NULL, MY_ZONG MY_KONG MY_ZHI);// !!!
    lv_obj_t * item_label3 = lv_list_get_btn_label(control_btn);
    lv_label_set_align(item_label3, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_event_cb(control_btn,event_handler);//设置事件回调函数
	
	//添加列表项5
	close_btn = lv_list_add_btn(list,NULL, MY_GUANGUAN MY_KONG MY_ZHI);// !!!
    lv_obj_t * item_label5 = lv_list_get_btn_label(close_btn);
    lv_label_set_align(item_label5, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_event_cb(close_btn,event_handler);//设置事件回调函数

}
