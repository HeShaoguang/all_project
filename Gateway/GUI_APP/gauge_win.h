#ifndef __GAUGE_WIN_H__
#define __GAUGE_WIN_H__
#include "sys.h"
#include "lvgl.h"

void guege_win_start(lv_obj_t * parent);
void gauge_update(u8 theid,u8 temper,u8 humit,u8 thelight,u8 themq);
#endif // __TEST_H__
