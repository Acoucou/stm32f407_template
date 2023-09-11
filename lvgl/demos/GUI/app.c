#include "app.h"
#include "lvgl.h"
#include "WS2812.h"

extern uint8_t led_mode, led_light, led_color, led_flag;
extern uint32_t led_red, led_green, led_blue, led_rgb;

extern float pitch,roll,yaw; 		//欧拉角
extern short tempara;					//温度
extern unsigned char pitch_str[10], yaw_str[10], roll_str[10], temp_str[10];

static lv_obj_t *tabview;
static lv_obj_t *tab1, *tab2, *tab3;
static lv_obj_t *red_slider, *green_slider, *blue_slider, *intense_slider, *slider_label;
static lv_obj_t *img1;
static lv_obj_t * label1;
static lv_obj_t * sw_Switch, *sw_RainbowLed, *sw_RunningLed, *sw_HorseLed, *sw_BreathLed, *sw_RhythmLed;
lv_timer_t *timer_time;
lv_obj_t *sensor_label1, *sensor_label2, *sensor_label3, *sensor_label4, *sensor_label5;

static lv_obj_t *create_slider(lv_color_t color);
static void lv_ctr_led(void);
static void home_tab(void);
static void sensor_tab(void);
static void creat_switch(void);
static void creat_laber(void);
static void creat_roller(void);
static void slider_event_cb(lv_event_t * e);
static void slider_event_cb1(lv_event_t * e);
static void sw_event_handler1(lv_event_t * e);
static void sw_event_handler2(lv_event_t * e);
static void sw_event_handler3(lv_event_t * e);
static void sw_event_handler4(lv_event_t * e);
static void sw_event_handler5(lv_event_t * e);
static void sw_event_handler6(lv_event_t * e);
static void roller_event_handler(lv_event_t * e);
static void lv_timer_time_update(lv_timer_t *timer);

// 入口函数
void app_start(void)
{
	lv_color_t c = lv_palette_main(LV_PALETTE_BLUE);
	lv_theme_t * th = lv_theme_default_init(lv_disp_get_default(),  /*Use the DPI, size, etc from this display*/ 
                                        c, lv_color_white(),   /*Primary and secondary palette*/
                                        true,    /*Light or dark mode*/ 
                                         &lv_font_montserrat_26); /*Small, normal, large fonts*/
								
	th = lv_theme_mono_init(lv_disp_get_default(), true, &lv_font_montserrat_26);
                                       
	lv_disp_set_theme(NULL, th); /*Assign the theme to the display*/
	
	
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_BOTTOM, 80);
	
	tab1 = lv_tabview_add_tab(tabview, "LED_CTR");
    tab2 = lv_tabview_add_tab(tabview, "HOME");
    tab3 = lv_tabview_add_tab(tabview, "OTHER");

	//tab1
	lv_ctr_led();	
					
	// tab2		
	home_tab();

    // tab3
    sensor_tab();
}

// 传感器界面
void sensor_tab()
{
    sensor_label1 = lv_label_create(tab3);
    lv_label_set_long_mode(sensor_label1, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Break the long lines*/
    lv_obj_set_width(sensor_label1, 250);
	lv_label_set_recolor(sensor_label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(sensor_label1, "#0000ff THIS# #ff00ff IS# #ff0000 LED#  #ffff00 CONTROLER#");
    lv_obj_set_width(sensor_label1, 150);  /*Set smaller width to make the lines wr ap*/
    lv_obj_align(sensor_label1, LV_ALIGN_TOP_MID, 0, 40);
	
	sensor_label2 = lv_label_create(tab3);
	lv_obj_align_to(sensor_label2, tab3, LV_ALIGN_TOP_MID, -60, 200);
    lv_label_set_text_fmt(sensor_label2, "pitch: %s", pitch_str);

	sensor_label3 = lv_label_create(tab3);
    lv_label_set_text_fmt(sensor_label3, "roll: %s", roll_str);
	lv_obj_align_to(sensor_label3, sensor_label2, LV_ALIGN_TOP_LEFT, 0, 45);

	sensor_label4 = lv_label_create(tab3);
	lv_label_set_text_fmt(sensor_label4, "yaw: %s", yaw_str);
	lv_obj_align_to(sensor_label4, sensor_label3, LV_ALIGN_TOP_LEFT, 0, 45);

    sensor_label5 = lv_label_create(tab3);
	lv_label_set_text_fmt(sensor_label5, "tempara: %s", temp_str);
	lv_obj_align_to(sensor_label5, sensor_label4, LV_ALIGN_TOP_LEFT, 0, 60);

    timer_time = lv_timer_create(lv_timer_time_update, 100, 0);
}
static void lv_timer_time_update(lv_timer_t *timer)
{
    lv_label_set_text_fmt(sensor_label2, "pitch: %s", pitch_str);
    lv_label_set_text_fmt(sensor_label3, "roll: %s", roll_str);
    lv_label_set_text_fmt(sensor_label4, "yaw: %s", yaw_str);

    lv_label_set_text_fmt(sensor_label5, "tempara: %s", temp_str);
}

void home_tab(){
	label1 = lv_label_create(tab2);
	
    lv_label_set_text(label1, "12:00");

    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 80);
	
    lv_obj_t * label2 = lv_label_create(tab2);
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_width(label2, 150);
    lv_label_set_text(label2, "It is a circularly scrolling text. ");
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 200);
	
}
// 创建led界面
static void lv_ctr_led(void)
{
	creat_laber();
	creat_switch();
	creat_roller();
	
	lv_obj_t * slider = lv_slider_create(tab1);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_event_cb(slider, slider_event_cb1, LV_EVENT_VALUE_CHANGED, NULL);
    /*Create a label below the slider*/
    slider_label = lv_label_create(tab1);
    lv_label_set_text(slider_label, "0%");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	
    /*Create 4 sliders to adjust RGB color and re-color intensity*/
    red_slider = create_slider(lv_palette_main(LV_PALETTE_RED));
    green_slider = create_slider(lv_palette_main(LV_PALETTE_GREEN));
    blue_slider = create_slider(lv_palette_main(LV_PALETTE_BLUE));
    intense_slider = create_slider(lv_palette_main(LV_PALETTE_GREY));

    lv_slider_set_value(red_slider, LV_OPA_20, LV_ANIM_OFF);
    lv_slider_set_value(green_slider, LV_OPA_90, LV_ANIM_OFF);
    lv_slider_set_value(blue_slider, LV_OPA_60, LV_ANIM_OFF);
    lv_slider_set_value(intense_slider, LV_OPA_0, LV_ANIM_OFF);

    lv_obj_align(red_slider, LV_ALIGN_LEFT_MID, 60, 220);
    lv_obj_align_to(green_slider, red_slider, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_align_to(blue_slider, green_slider, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_align_to(intense_slider, blue_slider, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
	
	LV_IMG_DECLARE(led_img)
    img1 = lv_img_create(tab1);
    lv_img_set_src(img1, &led_img);
    lv_obj_align(img1, LV_ALIGN_CENTER, 100, 220);
	

//	static lv_style_t *style_img;
//	lv_style_init(style_img);
//    lv_style_set_radius(style_img, 50);
//	lv_obj_add_style(img1, style_img, LV_STATE_DEFAULT); 

    lv_event_send(intense_slider, LV_EVENT_VALUE_CHANGED, NULL);
}
//创建标签
void creat_laber(void){
	lv_obj_t * label1 = lv_label_create(tab1);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Break the long lines*/
    lv_obj_set_width(label1, 250);
	lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#0000ff THIS# #ff00ff IS# #ff0000 LED#  #ffff00 CONTROLER#");
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wr ap*/
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 40);
	
	lv_obj_t * label2 = lv_label_create(tab1);
	lv_label_set_text(label2, "SwitchLed");
	lv_obj_align_to(label2, tab1, LV_ALIGN_TOP_LEFT, 40, 100);
	lv_obj_t * label3 = lv_label_create(tab1);
	lv_label_set_text(label3, "RainbowLed");
	lv_obj_align_to(label3, label2, LV_ALIGN_TOP_LEFT, 0, 45);
	lv_obj_t * label4 = lv_label_create(tab1);
	lv_label_set_text(label4, "RunningLed");
	lv_obj_align_to(label4, label3, LV_ALIGN_TOP_LEFT, 0, 45);
	lv_obj_t * label5 = lv_label_create(tab1);
	lv_label_set_text(label5, "HorseLed");
	lv_obj_align_to(label5, label4, LV_ALIGN_TOP_LEFT, 0, 45);
	lv_obj_t * label6 = lv_label_create(tab1);
	lv_label_set_text(label6, "BreathLed");
	lv_obj_align_to(label6, label5, LV_ALIGN_TOP_LEFT, 0, 45);
	
	lv_obj_t * label7 = lv_label_create(tab1);
	lv_label_set_text(label7, "RhythmLed");
	lv_obj_align_to(label7, label6, LV_ALIGN_TOP_LEFT, 0, 45);
}
// 创建开关
void creat_switch(void)
{ 
    sw_Switch = lv_switch_create(tab1);
	lv_obj_align_to(sw_Switch, tab1, LV_ALIGN_TOP_LEFT, 240, 100);
    lv_obj_add_event_cb(sw_Switch, sw_event_handler1, LV_EVENT_ALL, NULL);
	
	sw_RainbowLed = lv_switch_create(tab1);
	lv_obj_align_to(sw_RainbowLed, sw_Switch, LV_ALIGN_TOP_LEFT, 0, 45);
    lv_obj_add_event_cb(sw_RainbowLed, sw_event_handler2, LV_EVENT_ALL, NULL);
	
	sw_RunningLed = lv_switch_create(tab1);
	lv_obj_align_to(sw_RunningLed, sw_RainbowLed, LV_ALIGN_TOP_LEFT, 0, 45);
    lv_obj_add_event_cb(sw_RunningLed, sw_event_handler3, LV_EVENT_ALL, NULL);
	
	sw_HorseLed = lv_switch_create(tab1);
	lv_obj_align_to(sw_HorseLed, sw_RunningLed, LV_ALIGN_TOP_LEFT, 0, 45);
    lv_obj_add_event_cb(sw_HorseLed, sw_event_handler4, LV_EVENT_ALL, NULL);
	
	sw_BreathLed = lv_switch_create(tab1);
	lv_obj_align_to(sw_BreathLed, sw_HorseLed, LV_ALIGN_TOP_LEFT, 0, 45);
    lv_obj_add_event_cb(sw_BreathLed, sw_event_handler5, LV_EVENT_ALL, NULL);
	
	sw_RhythmLed = lv_switch_create(tab1);
	lv_obj_align_to(sw_RhythmLed, sw_BreathLed, LV_ALIGN_TOP_LEFT, 0, 45);
    lv_obj_add_event_cb(sw_RhythmLed, sw_event_handler6, LV_EVENT_ALL, NULL);
}


// 创建颜色滚动条
static lv_obj_t * create_slider(lv_color_t color)
{
    lv_obj_t * slider = lv_slider_create(tab1);
    lv_slider_set_range(slider, 0, 255);
    lv_obj_set_size(slider, 10, 200);
    lv_obj_set_style_bg_color(slider, color, LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider, lv_color_darken(color, LV_OPA_40), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    return slider;
}


// 创建滚动条
void creat_roller(void)
{
    lv_obj_t *roller1 = lv_roller_create(tab1);
    lv_roller_set_options(roller1,
						"Exit\n"
                        "Red\n"
                        "Green\n"
                        "Blue\n"
                        "Yellow\n"
                        "Purple\n"
                        "Orange\n"
                        "White",
                        LV_ROLLER_MODE_INFINITE);

    lv_roller_set_visible_row_count(roller1, 4);
    lv_obj_center(roller1);
	lv_obj_align_to(roller1, tab1, LV_ALIGN_TOP_LEFT, 330, 150);
	
    lv_obj_add_event_cb(roller1, roller_event_handler, LV_EVENT_ALL, NULL);
}


static void slider_event_cb1(lv_event_t * e)  // 百分比控制
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[8];
	led_light = (int)lv_slider_get_value(slider);
    lv_snprintf(buf, sizeof(buf), "%d%%", led_light);
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);	
}
static void sw_event_handler1(lv_event_t * e)  // 开关
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
			led_mode = 1;
		}else{
			led_mode = 0;
		}
    }
}
static void sw_event_handler2(lv_event_t * e){  // 彩虹灯
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
			led_mode = 2;
		}
    }
}
static void sw_event_handler3(lv_event_t * e){  // 流水灯
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
			led_mode = 3;
		}
    }
}
static void sw_event_handler4(lv_event_t * e){  // 跑马灯
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
			led_mode = 4;
		}
    }
}
static void sw_event_handler5(lv_event_t * e){  // 呼吸灯
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
			led_mode = 5;
		}
    }
}
static void sw_event_handler6(lv_event_t * e){  // 律动灯
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
			led_mode = 6;
		}
    }
}

static void slider_event_cb(lv_event_t * e)  // 控制颜色
{
    LV_UNUSED(e);

    /*Recolor the image based on the sliders' values*/
	led_red = lv_slider_get_value(red_slider);
	led_green = lv_slider_get_value(green_slider);
	led_blue = lv_slider_get_value(blue_slider);
    lv_color_t color  = lv_color_make(led_red, led_green, led_blue);
	
    lv_opa_t intense = lv_slider_get_value(intense_slider);
    lv_obj_set_style_img_recolor_opa(img1, intense, 0);
    lv_obj_set_style_img_recolor(img1, color, 0);
}
static void roller_event_handler(lv_event_t * e)  // 颜色选择
{
	
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
	
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
		
		if(!strcmp(buf, "Exit"))	led_flag = 0;
		else led_flag = 1;
		
        if(!strcmp(buf, "Red"))		led_rgb = C_Red;
		if(!strcmp(buf, "Green"))	led_rgb = C_Green;
		if(!strcmp(buf, "Blue"))	led_rgb = C_Blue;
		if(!strcmp(buf, "Yellow"))	led_rgb = C_Yellow;
		if(!strcmp(buf, "Purple"))	led_rgb = C_Purple;
		if(!strcmp(buf, "Orange"))	led_rgb = C_Orange;
		if(!strcmp(buf, "Indigo"))	led_rgb = C_indigo;
		if(!strcmp(buf, "White"))	led_rgb = C_white;
    }
}

