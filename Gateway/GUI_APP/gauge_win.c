#include "gauge_win.h"
#include "ui_app.h"
#include "string.h"
#include "main_win.h"
static lv_obj_t *table1,*gauge_tabview;
static lv_obj_t *gauge1,*gauge2;
static lv_obj_t *gauge1_label,*gauge2_label;
static lv_color_t needle_colors[3];
//u8 light_state, mq_state; 

#define MY_ZHI2    "\xE5\x80\xBC"
#define MY_HAO     "\xE5\x8F\xB7"
#define MY_GUANG    "\xE5\x85\x89"
#define MY_ZHAO    "\xE7\x85\xA7"
#define MY_KE    "\xE5\x8F\xAF"
#define MY_RANG    "\xE7\x87\x83"
#define MY_QI    "\xE6\xB0\x94"
#define MY_YOU    "\xE6\x9C\x89"
#define MY_WU    "\xE6\x97\xA0"

#define MY_PING    "\xE5\xB9\xB3"
#define MY_JUN    "\xE5\x9D\x87"
#define MY_WEN    "\xE6\xB8\xA9"
#define MY_DU    "\xE5\xBA\xA6"
#define MY_SI    "\xE6\xB9\xBF"

#define ARRAY_LEN   3
u8 temperture_array[ARRAY_LEN] = {0,0,0};
u8 humidity_array[ARRAY_LEN] = {0,0,0};

lv_style_t cell1_style;
lv_style_t cell2_style;
lv_style_t cell3_style;
char* table_value[3][4] = { {MY_ZHI2,"1" MY_HAO,"2" MY_HAO,"3" MY_HAO},//第一行,作为标题行
                                    {MY_GUANG MY_ZHAO,MY_WU,MY_WU,MY_WU},//数据行
                                    {MY_KE MY_RANG MY_QI,MY_WU,MY_WU,MY_WU}};//数据行
//每一列的宽度
const uint16_t TABLE_COL_WIDTH[4] = {55,55,55,55};

extern u8 now_screen;

void update_label()
{
	u8 tnum = 0,hnum = 0;
	u32 sum_temperture = 0 ,sum_humidity = 0 ;
	u8 temper_str[20],humid_str[20],light_str[20],mq_str[20];
	u8 average_temperture = 0, average_humidity = 0;
	
	//温湿度
	for(int i=0;i<ARRAY_LEN;i++)
	{
		sum_temperture+=temperture_array[i];
		sum_humidity+=humidity_array[i];
		if(temperture_array[i] != 0 && tnum !=ARRAY_LEN) tnum++;
		if(humidity_array[i] != 0 && hnum !=ARRAY_LEN) hnum++;
	}
	if(tnum != 0) average_temperture = sum_temperture/tnum;
	if(hnum != 0) average_humidity = sum_humidity/hnum;
	
	if(average_temperture > 26)
	{
		sprintf(temper_str,"#CC3333 " MY_WEN MY_DU ":%d#",average_temperture);  //红
		//sprintf(humid_str,"#CC3333 " MY_PING MY_JUN MY_SI MY_DU ":%d%%#",average_temperture);  //红
	}else{
		sprintf(temper_str,"#99CC66 " MY_WEN MY_DU ":%d#",average_temperture);  //绿
		//sprintf(humid_str,"#CC3333 " MY_PING MY_JUN MY_SI MY_DU ":%d%%#",average_temperture);  //红
	}
	lv_label_set_text(gauge1_label,(char *)temper_str);
	
	if(average_humidity > 75)
	{
		sprintf(humid_str,"#CC3333 " MY_SI MY_DU ":%d%%#",average_humidity);  //红
	}else{
		sprintf(humid_str,"#99CC66 " MY_SI MY_DU ":%d%%#",average_humidity);  //绿
	}
	
	
	lv_label_set_text(gauge2_label,(char *)humid_str);	

	
	//光照与可燃气
	
}

void gauge_update(u8 theid,u8 temper,u8 humid,u8 thelight,u8 themq)
{
	temperture_array[theid-1] = temper;
	humidity_array[theid-1] = humid;
	
	if(thelight == 1)
	{
		table_value[1][theid] = MY_YOU;
	}else{
		table_value[1][theid] = MY_WU;
	}
	
	if(themq == 1)
	{
		table_value[2][theid] = MY_YOU;
	}else{
		table_value[2][theid] = MY_WU;
	}
	
	if(now_screen == 1)
	{
		lv_gauge_set_value(gauge1,theid-1,temper);
		lv_gauge_set_value(gauge2,theid-1,humid);
		if(thelight == 1)
		{
			lv_table_set_cell_value(table1,1,theid,MY_YOU);
		}else{
			lv_table_set_cell_value(table1,1,theid,MY_WU);
		}
		
		if(themq == 1)
		{
			lv_table_set_cell_value(table1,2,theid,MY_YOU);
			lv_table_set_cell_type(table1,2,theid,3);
		}else{
			lv_table_set_cell_value(table1,2,theid,MY_WU);
			lv_table_set_cell_type(table1,2,theid,1);
		}
		
		update_label();   //更新数据

	}
	
	
}	



void guege_win_start(lv_obj_t * parent)
{
		uint8_t row,col;
    static lv_style_t bg_style,indic_style,btn_bg_style,btn_rel_style,btn_pr_style,btn_tgl_rel_style,btn_tgl_pr_style;
    needle_colors[0] = LV_COLOR_RED;
    needle_colors[1] = LV_COLOR_YELLOW;
    needle_colors[2] = LV_COLOR_BLUE;
    //1.创建样式
    //1.1  创建背景样式
    lv_style_copy(&bg_style,&lv_style_plain);
    bg_style.body.main_color = LV_COLOR_MAKE(39,42,45);//纯色背景
    bg_style.body.grad_color = bg_style.body.main_color;
    bg_style.body.border.color = LV_COLOR_MAKE(150,150,150);//边框颜色
    bg_style.body.border.width = 0;//边框宽度
    bg_style.text.color = LV_COLOR_WHITE;
    //1.2  创建页面指示器的样式
    lv_style_copy(&indic_style,&lv_style_plain_color);
    indic_style.body.main_color = LV_COLOR_MAKE(42,212,66);//指示器的颜色,绿色 indic_style.body.grad_color = indic_style.body.main_color;
    indic_style.body.padding.inner = 0;//设置指示器的高度
    //1.3  创建页面选择按钮栏的背景样式
    //lv_style_transp_tight 样式中的 inner,left,top,right,bottom 等内间距值都为  0,这是为了 //让页面选择按钮能够紧挨在一起
    lv_style_copy(&btn_bg_style,&lv_style_transp_tight);
    //1.4  创建按钮正常态下的松手样式
    lv_style_copy(&btn_rel_style,&lv_style_plain_color);
    btn_rel_style.body.main_color = LV_COLOR_MAKE(102,102,102);
    btn_rel_style.body.grad_color = btn_rel_style.body.main_color;
    btn_rel_style.body.border.color = LV_COLOR_MAKE(150,150,150);//边框颜色
    btn_rel_style.body.border.width = 0;
    btn_rel_style.text.color = LV_COLOR_WHITE;//字体颜色
    //1.5  创建按钮正常态下的按下样式
    lv_style_copy(&btn_pr_style,&btn_rel_style);
    btn_pr_style.body.main_color = LV_COLOR_MAKE(56,60,63);
    btn_pr_style.body.grad_color = btn_pr_style.body.main_color;
    //1.6  创建按钮切换态下的松手样式
    lv_style_copy(&btn_tgl_rel_style,&btn_rel_style);
    btn_tgl_rel_style.body.main_color = LV_COLOR_MAKE(13,14,15);//和主背景颜色一致
    btn_tgl_rel_style.body.grad_color = btn_tgl_rel_style.body.main_color;
    //1.7  创建按钮切换态下的按下样式
    //保持和 btn_tgl_rel_style 一样就行了
    lv_style_copy(&btn_tgl_pr_style,&btn_tgl_rel_style);

    lv_obj_t *win;
		win = general_win_create(parent,MY_YI MY_BIAO);


    //1.创建样式
    //1.1  创建第一种单元格样式,绿字
    lv_style_copy(&cell1_style,&lv_style_plain_color);
    cell1_style.body.border.width = 1;
    cell1_style.body.border.color = LV_COLOR_BLACK;
    cell1_style.body.main_color =  LV_COLOR_MAKE(111,111,111);//银色的背景
    cell1_style.text.color = LV_COLOR_MAKE(153,204,102);
    cell1_style.body.grad_color = LV_COLOR_MAKE(111,111,111);
    //1.2  创建第二种单元格样式,标题
    lv_style_copy(&cell2_style,&lv_style_plain_color);
    cell2_style.body.border.width = 1;
    cell2_style.body.border.color = LV_COLOR_BLACK;
    cell2_style.body.main_color = LV_COLOR_MAKE(70,70,70);//银色的背景
    cell2_style.body.grad_color = LV_COLOR_MAKE(70,70,70);
    cell2_style.text.color = LV_COLOR_BLACK;
    //1.3  创建第三种单元格样式,红字
    lv_style_copy(&cell3_style,&lv_style_plain_color);
    cell3_style.body.border.width = 1;
    cell3_style.body.main_color =  LV_COLOR_MAKE(111,111,111);//银色的背景
    cell3_style.body.grad_color =LV_COLOR_MAKE(111,111,111);
    cell3_style.body.border.color = LV_COLOR_BLACK;
    cell3_style.text.color = LV_COLOR_MAKE(204,51,51);//文本颜色为红色,高亮显示

    table1 = lv_table_create(win,NULL);

    //设置表格的背景样式,为透明
    lv_table_set_style(table1,LV_TABLE_STYLE_BG,&lv_style_transp_tight);
    //设置第一种单元格的样式
    lv_table_set_style(table1,LV_TABLE_STYLE_CELL1,&cell1_style);
    //设置第二种单元格的样式
    lv_table_set_style(table1,LV_TABLE_STYLE_CELL2,&cell2_style);
    //设置第二种单元格的样式
    lv_table_set_style(table1,LV_TABLE_STYLE_CELL3,&cell3_style);

    lv_table_set_col_cnt(table1,4);//设置表格的总列数
    lv_table_set_row_cnt(table1,3);//设置表格的总行数

    for(row=0;row<2;row++)
    {
        for(col=0;col<4;col++)
        {
						lv_table_set_cell_type(table1,row,col,1);
            lv_table_set_cell_align(table1,row,col,LV_LABEL_ALIGN_CENTER);
            lv_table_set_cell_value(table1,row,col,table_value [row][col]);//设置文本内容
        }
    }

		for(col=0;col<4;col++)
		{
			if(table_value[2][col] == MY_YOU)
			{
				lv_table_set_cell_type(table1,2,col,3);
			}else{
				lv_table_set_cell_type(table1,2,col,1);
			}
				
				lv_table_set_cell_align(table1,2,col,LV_LABEL_ALIGN_CENTER);
				lv_table_set_cell_value(table1,2,col,table_value[2][col]);//设置文本内容
		}
		
    for(col=0;col<4;col++)
    {
        //设置每一列的宽度
        lv_table_set_col_width(table1,col,TABLE_COL_WIDTH[col]);
        //标题行的文本内容居中对齐
        lv_table_set_cell_type(table1,0,col,2);//设置为第二种单元格类型
    }

    for(row=0;row<3;row++)
    {
        lv_table_set_cell_type(table1,row,0,2);//设置为第二种单元格类型
    }

   
    //设置所有单元格的文本内容
		lv_obj_align(table1,win,LV_ALIGN_IN_TOP_MID,0,40);//设置表格与屏幕居中对齐


	gauge_tabview = lv_tabview_create(win,NULL);
	lv_tabview_set_btns_pos(gauge_tabview,LV_TABVIEW_BTNS_POS_BOTTOM);
	lv_tabview_set_style(gauge_tabview,LV_TABVIEW_STYLE_BG,&bg_style);//设置背景样式 //设置页面指示器的样式
    lv_tabview_set_style(gauge_tabview,LV_TABVIEW_STYLE_INDIC,&indic_style);//设置页面选择按钮栏的背景样式
    lv_tabview_set_style(gauge_tabview,LV_TABVIEW_STYLE_BTN_BG,&btn_bg_style); //设置按钮正常态下的松手样式
    lv_tabview_set_style(gauge_tabview,LV_TABVIEW_STYLE_BTN_REL,&btn_rel_style);//设置按钮正常态下的按下样式
    lv_tabview_set_style(gauge_tabview,LV_TABVIEW_STYLE_BTN_PR,&btn_pr_style) ;
    lv_tabview_set_style(gauge_tabview,LV_TABVIEW_STYLE_BTN_TGL_REL,&btn_tgl_rel_style);//设置按钮切换态下的松手样式
    lv_tabview_set_style(gauge_tabview,LV_TABVIEW_STYLE_BTN_TGL_PR,&btn_tgl_pr_style);//设置按钮切换态下的按下样式
    //选项卡按钮位于底部
    lv_obj_set_size(gauge_tabview, lv_obj_get_width(win), lv_obj_get_width(win)-35);
    lv_obj_align(gauge_tabview, win, LV_ALIGN_IN_BOTTOM_MID,0,0);

    lv_obj_t *gauge_tab_page1 = lv_tabview_add_tab(gauge_tabview, MY_WEN MY_DU MY_BIAO);
    lv_obj_t *gauge_tab_page2 = lv_tabview_add_tab(gauge_tabview, MY_SI MY_DU MY_BIAO);
    lv_page_set_sb_mode(gauge_tab_page1,LV_SB_MODE_DRAG);
    lv_page_set_sb_mode(gauge_tab_page2,LV_SB_MODE_DRAG);
    static lv_style_t gauge_style;

	//创建仪表盘
	gauge1 = lv_gauge_create(gauge_tab_page1, NULL);
	lv_style_copy(&gauge_style,lv_obj_get_style(gauge1));//复制标签之前的主题样式
	gauge_style.line.color = LV_COLOR_RED;//改变字体
	lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &gauge_style);
	lv_obj_set_size(gauge1,170,170);//设置大小
	lv_gauge_set_range(gauge1,0,50);//设置仪表盘的范围
	lv_gauge_set_needle_count(gauge1,sizeof(needle_colors)/sizeof(needle_colors[0]),needle_colors);
	lv_gauge_set_value(gauge1,0,temperture_array[0]); //设置指针1指向的数值
	lv_gauge_set_value(gauge1,1,temperture_array[1]); //设置指针2指向的数值
	lv_gauge_set_value(gauge1,2,temperture_array[2]); //设置指针2指向的数值
	lv_gauge_set_critical_value(gauge1,30);//设置关键数值点
	lv_gauge_set_scale(gauge1,240,31,6);//设置角度,刻度线的数量,数值标签的数量
	lv_obj_align(gauge1,gauge_tab_page1,LV_ALIGN_CENTER,0,10);//设置对齐方式


	//创建一个标签,用来显示仪表盘上指针1的数值
	gauge1_label = lv_label_create(gauge_tab_page1,NULL);
	lv_label_set_long_mode(gauge1_label,LV_LABEL_LONG_BREAK);//设置长文本模式
	lv_obj_set_width(gauge1_label,80);//设置宽度
	lv_label_set_align(gauge1_label,LV_LABEL_ALIGN_CENTER);//设置文本居中对齐
	//lv_label_set_body_draw(gauge1_label,true);//使能背景重绘
	lv_obj_align(gauge1_label,gauge1,LV_ALIGN_CENTER,0,50);//设置对齐方式
	//lv_label_set_text(gauge1_label,"temp:0du");//设置文本
	lv_label_set_recolor(gauge1_label,true);//使能文本重绘色

	//创建仪表盘
	gauge2 = lv_gauge_create(gauge_tab_page2, gauge1);
	lv_obj_set_size(gauge2,170,170);//设置大小
	lv_gauge_set_range(gauge2,0,100);//设置仪表盘的范围
	lv_gauge_set_needle_count(gauge2,sizeof(needle_colors)/sizeof(needle_colors[0]),needle_colors);
	lv_gauge_set_value(gauge2,0,humidity_array[0]);    //设置指针1指向的数值
	lv_gauge_set_value(gauge2,1,humidity_array[1]);    //设置指针2指向的数值
	lv_gauge_set_value(gauge2,2,humidity_array[2]);    //设置指针2指向的数值
	lv_gauge_set_critical_value(gauge2,80);//设置关键数值点
	lv_gauge_set_scale(gauge2,240,31,6);//设置角度,刻度线的数量,数值标签的数量
	lv_obj_align(gauge2,gauge_tab_page2,LV_ALIGN_CENTER,0,10);//设置对齐方式

	//创建一个标签,用来显示仪表盘上指针1的数值
	gauge2_label = lv_label_create(gauge_tab_page2,NULL);
	lv_label_set_long_mode(gauge2_label,LV_LABEL_LONG_BREAK);//设置长文本模式
	lv_obj_set_width(gauge2_label,120);//设置宽度
	lv_label_set_align(gauge2_label,LV_LABEL_ALIGN_CENTER);//设置文本居中对齐
	//lv_label_set_body_draw(gauge2_label,true);//使能背景重绘
	lv_obj_align(gauge2_label,gauge2,LV_ALIGN_CENTER,0,50);//设置对齐方式
	//lv_label_set_text(gauge2_label,"humi:100du");//设置文本
	lv_label_set_recolor(gauge2_label,true);//使能文本重绘色
	
	update_label();
}
