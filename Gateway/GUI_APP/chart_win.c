#include "chart_win.h"
#include "ui_app.h"
#include "main_win.h"

#define MY_HAO    "\xE5\x8F\xB7"

static lv_obj_t *chart_tabview;
static lv_obj_t *chart_tab_page1,*chart_tab_page2,*chart_tab_page3;
static lv_obj_t *chart1,*chart2,*chart3;
static lv_chart_series_t *temperture1,*humitidy1,*temperture2,*humitidy2,*temperture3,*humitidy3;
static lv_obj_t *  sw1;
#define TEMPERTURE_COUNT   6  //ÿ�������������е����ݵ����
lv_coord_t temperture1_y[TEMPERTURE_COUNT] = {0,0,0,0,0,0};
lv_coord_t humitidy1_y[TEMPERTURE_COUNT] = {0,0,0,0,0,0};
lv_coord_t temperture2_y[TEMPERTURE_COUNT] = {0,0,0,0,0,0};
lv_coord_t humitidy2_y[TEMPERTURE_COUNT] = {0,0,0,0,0,0};
lv_coord_t temperture3_y[TEMPERTURE_COUNT] = {0,0,0,0,0,0};
lv_coord_t humitidy3_y[TEMPERTURE_COUNT] = {0,0,0,0,0,0};
extern u8 now_screen;

//�¼��ص�����
//static void event_handler(lv_obj_t * obj,lv_event_t event)
//{

//}

void temperture1_add(u8 value)
{
	if(now_screen == 2)
	{
		lv_chart_set_next(chart1,temperture1,value);
	}else
	{
		for(int i=0;i < TEMPERTURE_COUNT-1;i++)
		{
			temperture1_y[i] = temperture1_y[i+1];
		}
		temperture1_y[TEMPERTURE_COUNT-1] = value;
	}
}
void humitidy1_add(u8 value)
{
	if(now_screen == 2)
	{
		lv_chart_set_next(chart1,humitidy1,value);
	}else
	{
		for(int i=0;i < TEMPERTURE_COUNT-1;i++)
		{
			humitidy1_y[i] = humitidy1_y[i+1];
		}
		humitidy1_y[TEMPERTURE_COUNT-1] = value;
	}	
}
//ͼ2
void temperture2_add(u8 value)
{
	if(now_screen == 2)
	{
		lv_chart_set_next(chart2,temperture2,value);
	}else
	{
		for(int i=0;i < TEMPERTURE_COUNT-1;i++)
		{
			temperture2_y[i] = temperture2_y[i+1];
		}
		temperture2_y[TEMPERTURE_COUNT-1] = value;
	}
}
void humitidy2_add(u8 value)
{
	if(now_screen == 2)
	{
		lv_chart_set_next(chart2,humitidy2,value);
	}else
	{
		for(int i=0;i < TEMPERTURE_COUNT-1;i++)
		{
			humitidy2_y[i] = humitidy2_y[i+1];
		}
		humitidy2_y[TEMPERTURE_COUNT-1] = value;
	}	
}
//ͼ3
void temperture3_add(u8 value)
{
	if(now_screen == 2)
	{
		lv_chart_set_next(chart3,temperture3,value);
	}else
	{
		for(int i=0;i < TEMPERTURE_COUNT-1;i++)
		{
			temperture3_y[i] = temperture3_y[i+1];
		}
		temperture3_y[TEMPERTURE_COUNT-1] = value;
	}
}
void humitidy3_add(u8 value)
{
	if(now_screen == 2)
	{
		lv_chart_set_next(chart3,humitidy3,value);
	}else
	{
		for(int i=0;i < TEMPERTURE_COUNT-1;i++)
		{
			humitidy3_y[i] = humitidy3_y[i+1];
		}
		humitidy3_y[TEMPERTURE_COUNT-1] = value;
	}	
}
void char_win_start(lv_obj_t * parent)
{
    static lv_style_t bg_style,indic_style,btn_bg_style,btn_rel_style,btn_pr_style,btn_tgl_rel_style,btn_tgl_pr_style;
    //1.������ʽ
    //1.1  ����������ʽ
    lv_style_copy(&bg_style,&lv_style_plain);
    bg_style.body.main_color = LV_COLOR_MAKE(39,42,45);//��ɫ����
    bg_style.body.grad_color = bg_style.body.main_color;
    bg_style.body.border.color = LV_COLOR_MAKE(150,150,150);//�߿���ɫ
    bg_style.body.border.width = 0;//�߿���
    bg_style.text.color = LV_COLOR_WHITE;
    //1.2  ����ҳ��ָʾ������ʽ
    lv_style_copy(&indic_style,&lv_style_plain_color);
    indic_style.body.main_color = LV_COLOR_MAKE(42,212,66);//ָʾ������ɫ,��ɫ indic_style.body.grad_color = indic_style.body.main_color;
    indic_style.body.padding.inner = 0;//����ָʾ���ĸ߶�
    //1.3  ����ҳ��ѡ��ť���ı�����ʽ
    //lv_style_transp_tight ��ʽ�е� inner,left,top,right,bottom ���ڼ��ֵ��Ϊ  0,����Ϊ�� //��ҳ��ѡ��ť�ܹ�������һ��
    lv_style_copy(&btn_bg_style,&lv_style_transp_tight);
    //1.4  ������ť����̬�µ�������ʽ
    lv_style_copy(&btn_rel_style,&lv_style_plain_color);
    btn_rel_style.body.main_color = LV_COLOR_MAKE(102,102,102);
    btn_rel_style.body.grad_color = btn_rel_style.body.main_color;
    btn_rel_style.body.border.color = LV_COLOR_MAKE(150,150,150);//�߿���ɫ
    btn_rel_style.body.border.width = 0;
    btn_rel_style.text.color = LV_COLOR_WHITE;//������ɫ
    //1.5  ������ť����̬�µİ�����ʽ
    lv_style_copy(&btn_pr_style,&btn_rel_style);
    btn_pr_style.body.main_color = LV_COLOR_MAKE(56,60,63);
    btn_pr_style.body.grad_color = btn_pr_style.body.main_color;
    //1.6  ������ť�л�̬�µ�������ʽ
    lv_style_copy(&btn_tgl_rel_style,&btn_rel_style);
    btn_tgl_rel_style.body.main_color = LV_COLOR_MAKE(13,14,15);//����������ɫһ��
    btn_tgl_rel_style.body.grad_color = btn_tgl_rel_style.body.main_color;
    //1.7  ������ť�л�̬�µİ�����ʽ
    //���ֺ� btn_tgl_rel_style һ��������
    lv_style_copy(&btn_tgl_pr_style,&btn_tgl_rel_style);

    lv_obj_t *win;
    win = general_win_create(parent,MY_TU MY_BIAO);

    chart_tabview = lv_tabview_create(win,NULL);

    lv_tabview_set_style(chart_tabview,LV_TABVIEW_STYLE_BG,&bg_style);//���ñ�����ʽ //����ҳ��ָʾ������ʽ
    lv_tabview_set_style(chart_tabview,LV_TABVIEW_STYLE_INDIC,&indic_style);//����ҳ��ѡ��ť���ı�����ʽ
    lv_tabview_set_style(chart_tabview,LV_TABVIEW_STYLE_BTN_BG,&btn_bg_style); //���ð�ť����̬�µ�������ʽ
    lv_tabview_set_style(chart_tabview,LV_TABVIEW_STYLE_BTN_REL,&btn_rel_style);//���ð�ť����̬�µİ�����ʽ
    lv_tabview_set_style(chart_tabview,LV_TABVIEW_STYLE_BTN_PR,&btn_pr_style) ;
    lv_tabview_set_style(chart_tabview,LV_TABVIEW_STYLE_BTN_TGL_REL,&btn_tgl_rel_style);//���ð�ť�л�̬�µ�������ʽ
    lv_tabview_set_style(chart_tabview,LV_TABVIEW_STYLE_BTN_TGL_PR,&btn_tgl_pr_style);//���ð�ť�л�̬�µİ�����ʽ
    lv_tabview_set_btns_pos(chart_tabview,LV_TABVIEW_BTNS_POS_BOTTOM);//ѡ���ťλ�ڵײ�
    lv_obj_set_size(chart_tabview, lv_obj_get_width(win), lv_obj_get_height(win)-30);
    lv_obj_align(chart_tabview, win, LV_ALIGN_IN_BOTTOM_MID,0,0);
		lv_tabview_set_sliding(chart_tabview,false);
		lv_tabview_set_anim_time(chart_tabview,100);
		
    chart_tab_page1 = lv_tabview_add_tab(chart_tabview, "1" MY_HAO);
    chart_tab_page2 = lv_tabview_add_tab(chart_tabview, "2" MY_HAO);
    chart_tab_page3 = lv_tabview_add_tab(chart_tabview, "3" MY_HAO);
    //����ͼ��1
    chart1 = lv_chart_create(chart_tab_page1,NULL);
    lv_obj_set_size(chart1,lv_obj_get_width(chart_tab_page1)*0.75f,lv_obj_get_height(chart_tab_page1)*0.8f);//���ô�С
    lv_obj_align(chart1,chart_tab_page1,LV_ALIGN_CENTER,15,-3);//���ö��뷽ʽ
    lv_chart_set_type(chart1,LV_CHART_TYPE_POINT|LV_CHART_TYPE_LINE);//����Ϊɢ������ߵ����
    lv_chart_set_series_opa(chart1,LV_OPA_80);//���������ߵ�͸����
    lv_chart_set_series_width(chart1,4);//���������ߵĿ��
    lv_chart_set_series_darking(chart1,LV_OPA_80);//���������ߵĺ���ӰЧ��
    lv_chart_set_point_count(chart1,6);//�������ݵ�ĸ���
    lv_chart_set_div_line_count(chart1,4,2);//����ˮƽ�ʹ�ֱ�ָ���
    lv_chart_set_range(chart1,0,100);//����y�����ֵ��Χ
    lv_chart_set_y_tick_length(chart1,5,2);
    lv_chart_set_y_tick_texts(chart1,"100\n80\n60\n40\n20\n0",2,LV_CHART_AXIS_DRAW_LAST_TICK);
    lv_chart_set_x_tick_length(chart1,5,2);
    lv_chart_set_x_tick_texts(chart1,"0\n2\n4\n6",2,LV_CHART_AXIS_DRAW_LAST_TICK);
    lv_chart_set_margin(chart1,40);
    lv_chart_set_update_mode(chart1, LV_CHART_UPDATE_MODE_SHIFT);
    //��ͼ�������������
    temperture1 = lv_chart_add_series(chart1,LV_COLOR_RED);//ָ��Ϊ��ɫ
    lv_chart_set_points(chart1,temperture1,(lv_coord_t*)temperture1_y);//��ʼ�����ݵ��ֵ
    humitidy1 = lv_chart_add_series(chart1,LV_COLOR_BLUE);
    lv_chart_set_points(chart1,humitidy1,(lv_coord_t*)humitidy1_y);//��ʼ�����ݵ��ֵ


    //����ͼ��2
    chart2 = lv_chart_create(chart_tab_page2,NULL);
    lv_obj_set_size(chart2,lv_obj_get_width(chart_tab_page2)*0.75f,lv_obj_get_height(chart_tab_page2)*0.8f);//���ô�С
    lv_obj_align(chart2,chart_tab_page2,LV_ALIGN_CENTER,15,-3);//���ö��뷽ʽ
    lv_chart_set_type(chart2,LV_CHART_TYPE_POINT|LV_CHART_TYPE_LINE);//����Ϊɢ������ߵ����
    lv_chart_set_series_opa(chart2,LV_OPA_80);//���������ߵ�͸����
    lv_chart_set_series_width(chart2,4);//���������ߵĿ��
    lv_chart_set_series_darking(chart2,LV_OPA_80);//���������ߵĺ���ӰЧ��
    lv_chart_set_point_count(chart2,6);//�������ݵ�ĸ���
    lv_chart_set_div_line_count(chart2,4,2);//����ˮƽ�ʹ�ֱ�ָ���
    lv_chart_set_range(chart2,0,100);//����y�����ֵ��Χ
    lv_chart_set_y_tick_length(chart2,5,2);
    lv_chart_set_y_tick_texts(chart2,"100\n80\n60\n40\n20\n0",2,LV_CHART_AXIS_DRAW_LAST_TICK);
    lv_chart_set_x_tick_length(chart2,5,2);
    lv_chart_set_x_tick_texts(chart2,"0\n2\n4\n6",2,LV_CHART_AXIS_DRAW_LAST_TICK);
    lv_chart_set_margin(chart2,40);
    lv_chart_set_update_mode(chart2, LV_CHART_UPDATE_MODE_SHIFT);
    //��ͼ�������������
		temperture2 = lv_chart_add_series(chart2,LV_COLOR_RED);//ָ��Ϊ��ɫ
    lv_chart_set_points(chart2,temperture2,(lv_coord_t*)temperture2_y);//��ʼ�����ݵ��ֵ
    humitidy2 = lv_chart_add_series(chart2,LV_COLOR_BLUE);
    lv_chart_set_points(chart2,humitidy2,(lv_coord_t*)humitidy2_y);//��ʼ�����ݵ��ֵ

    //����ͼ��3
    chart3 = lv_chart_create(chart_tab_page3,NULL);
    lv_obj_set_size(chart3,lv_obj_get_width(chart_tab_page3)*0.75f,lv_obj_get_height(chart_tab_page3)*0.8f);//���ô�С
    lv_obj_align(chart3,chart_tab_page3,LV_ALIGN_CENTER,15,-3);//���ö��뷽ʽ
    lv_chart_set_type(chart3,LV_CHART_TYPE_POINT|LV_CHART_TYPE_LINE);//����Ϊɢ������ߵ����
    lv_chart_set_series_opa(chart3,LV_OPA_80);//���������ߵ�͸����
    lv_chart_set_series_width(chart3,4);//���������ߵĿ��
    lv_chart_set_series_darking(chart3,LV_OPA_80);//���������ߵĺ���ӰЧ��
    lv_chart_set_point_count(chart3,6);//�������ݵ�ĸ���
    lv_chart_set_div_line_count(chart3,4,2);//����ˮƽ�ʹ�ֱ�ָ���
    lv_chart_set_range(chart3,0,100);//����y�����ֵ��Χ
    lv_chart_set_y_tick_length(chart3,5,2);
    lv_chart_set_y_tick_texts(chart3,"100\n80\n60\n40\n20\n0",2,LV_CHART_AXIS_DRAW_LAST_TICK);
    lv_chart_set_x_tick_length(chart3,5,2);
    lv_chart_set_x_tick_texts(chart3,"0\n2\n4\n6",2,LV_CHART_AXIS_DRAW_LAST_TICK);
    lv_chart_set_margin(chart3,40);
    lv_chart_set_update_mode(chart3, LV_CHART_UPDATE_MODE_SHIFT);
    //��ͼ�������������
		temperture3 = lv_chart_add_series(chart3,LV_COLOR_RED);//ָ��Ϊ��ɫ
    lv_chart_set_points(chart3,temperture3,(lv_coord_t*)temperture3_y);//��ʼ�����ݵ��ֵ
    humitidy3 = lv_chart_add_series(chart3,LV_COLOR_BLUE);
    lv_chart_set_points(chart3,humitidy3,(lv_coord_t*)humitidy3_y);//��ʼ�����ݵ��ֵ

    //��ǩ
//    lv_obj_t *swlabel = lv_label_create(win,NULL);
//    lv_label_set_text(swlabel, "qiehuantu");
//    lv_obj_align(swlabel,chart3,LV_ALIGN_OUT_BOTTOM_LEFT,0,55);
    //��������
//	sw1 = lv_sw_create(win,NULL);
//	lv_obj_set_size(sw1,50,30);//���ô�С
//	lv_obj_align(sw1,swlabel,LV_ALIGN_OUT_RIGHT_MID,80,0);//���ö��뷽ʽ
//	lv_sw_off(sw1,LV_ANIM_ON);//Ĭ��Ϊ��״̬
//	lv_obj_set_event_cb(sw1,event_handler);//�����¼��ص�����
}
