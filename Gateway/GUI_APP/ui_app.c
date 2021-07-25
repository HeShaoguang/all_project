#include "ui_app.h"
#include "main_win.h"
#include "usart.h"

#define MY_FAN    "\xE8\xBF\x94"
#define MY_HUI    "\xE5\x9B\x9E"

extern u8 now_screen;
void ui_app_start()
{
	//1.������ʹ������
	//ʹ�ò�ͬ������ʱ,���Ȱ���lv_conf.h�����ļ��е���Ӧ���ʹ����
	lv_theme_t * theme = lv_theme_night_init(210,NULL);
	lv_theme_set_current(theme);

	//2.�򿪵�¼����
	//login_win_create(lv_scr_act());
	now_screen = 0;
	main_win_create(lv_scr_act());
}

static void back_btn_event_handler(lv_obj_t * obj,lv_event_t event)
{
	if(event==LV_EVENT_PRESSED)//�����¼�
	{
		lv_label_set_text(obj,"#444444 <back#");//ʹ���ı���ɫ���,�а��µ�Ч��
		lv_label_set_recolor(obj,true);//ʹ���ػ�ɫ
	}else if(event==LV_EVENT_RELEASED||event==LV_EVENT_PRESS_LOST)//�����¼��򻮳��ؼ���Ч�����¼�
	{
		//ɾ������,��ʵҲ����ɾ��lv_page����
		//��Ϊback_btn����lv_pageҳ���е�,��Ҫ��ȡ2����ܻ�ȡ��������lv_page����
		obj = lv_obj_get_parent(obj);
		obj = lv_obj_get_parent(obj);//��ʱ��objΪlv_pageҳ�����,������
		lv_event_cb_t event_cb = lv_obj_get_event_cb(obj);
		if(event_cb)//�˴����Ƿ��������¼��ص�����,�����˵Ļ������,�������������ͷ���Դ�Ȳ���
			event_cb(obj,USER_EVENT_BACK);
		printf("delete\r\n");
		now_screen = 0;
		printf("now_screen:%d\r\n",now_screen);
		lv_obj_del(obj);//ɾ���˴���,�൱�ڷ��ز���
	}
}

//����һ��ͨ�õĴ���,�����б���ͷ��ذ�ť
//parent:������
//title:����ı���
lv_obj_t * general_win_create(lv_obj_t * parent,const char * title)
{
	#define	TOP_OFFSET	5
	lv_obj_t * win;

	//��������
	win = lv_page_create(parent,NULL);//��lv_pageҳ����ģ�ⴰ��
	lv_obj_set_size(win,lv_obj_get_width(parent),lv_obj_get_height(parent));//���ô���Ĵ�С
	lv_page_set_sb_mode(win,LV_SB_MODE_OFF);//���ù�������ģʽΪ�Զ�
	//lv_page_set_edge_flash(win,true);//ʹ�ܱ�Ե��Բ������Ч��
	lv_page_set_style(win,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);//����scrl��������ʽΪ͸��
	//����һ������
	lv_obj_t * title_label = lv_label_create(win,NULL);
	lv_label_set_text(title_label,title);//���ô������
	lv_obj_align(title_label,NULL,LV_ALIGN_IN_TOP_RIGHT,0,TOP_OFFSET);//���ö��뷽ʽ
	//�������ذ�ť
	lv_obj_t * back_btn = lv_label_create(win,NULL);//�ñ�ǩ��ģ�ⰴť
	lv_label_set_text(back_btn,"< " MY_FAN MY_HUI);//�����ı�
	lv_obj_align(back_btn,NULL,LV_ALIGN_IN_TOP_LEFT,5,TOP_OFFSET);//���ö��뷽ʽ
	lv_obj_set_click(back_btn,true);//ʹ�ܵ������
	lv_obj_set_event_cb(back_btn,back_btn_event_handler);//�����¼��ص�����,���ڹرմ���

	return win;
}
