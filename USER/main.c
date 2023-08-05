#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "beep.h"
#include "timer.h"
#include "touch.h" 
#include "esp8266.h"
#include "getdata.h"
#include "usart3.h"   
#include "WS2812.h"
#include "Adc.h"

// lvgl头文件
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
// myAPP
#include "app.h"

// rtos头文件
#include "FreeRTOS.h"
#include "task.h"


uint8_t led_mode, led_light = 100, led_color, led_flag;
uint32_t led_red, led_green, led_blue, led_rgb = C_Purple;

#define NOISE 50       //噪音底线
u16 max9814_data;
int last_VALUE=0;
int light_ADC=0;
int num;

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define TASK1_TASK_PRIO		2
//任务堆栈大小	
#define TASK1_STK_SIZE 		128  
//任务句柄
TaskHandle_t Task1Task_Handler;
//任务函数
void task1_task(void *pvParameters);

//任务优先级
#define TASK2_TASK_PRIO		3
//任务堆栈大小	
#define TASK2_STK_SIZE 		128  
//任务句柄
TaskHandle_t Task2Task_Handler;
//任务函数
void task2_task(void *pvParameters);

// 律动灯
void RtythmLight(void);

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	usart3_Init(115200);
	WS2812b_Configuration();
	
	LED_Init();					//初始化LED 
 	LCD_Init();					//LCD初始化 
	KEY_Init(); 				//按键初始化  
	BEEP_Init();				//蜂鸣器初始化
	Adc_Init();
	TIM3_Int_Init(999,83);	//定时器初始化(1ms中断),用于给lvgl提供1ms的心跳节拍
	tp_dev.init();			//触摸屏初始化
	//ESP8266_Init();
	//get_weather_str();
	//printf("the data is:\r\n %s\r\n", esp8266_buf);
	
	
	lv_init();						//lvgl系统初始化
	lv_port_disp_init();	//lvgl显示接口初始化,放在lv_init()的后面
	lv_port_indev_init();	//lvgl输入接口初始化,放在lv_init()的后面
	
	app_start();
	
	
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建TASK1任务
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);   
    //创建TASK2任务
    xTaskCreate((TaskFunction_t )task2_task,     
                (const char*    )"task2_task",   
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_TASK_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler); 
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//task1任务函数, UI刷新
void task1_task(void *pvParameters)
{
	while(1)
	{
		tp_dev.scan(0);//触摸扫描
		lv_task_handler();//lvgl事务处理
	}
}

//彩灯控制任务函数
void task2_task(void *pvParameters)
{
	while(1)
	{
		if(!led_flag)	
			led_rgb = (led_green << 16) | (led_red << 8) | led_blue;
		
		switch(led_mode){
			case 1:
				light_ctr(led_light, led_rgb);
			break;
			case 2:
				rainbowCycle(14);
			break;
			case 3:
				Running_water_lamp(led_red, led_green, led_blue, 100);
				break;
			case 4:
				horse_race_lamp(100);
				break;
			case 5:
				ws2812_All_LED_one_Color_breath(20, led_rgb);
			case 6:
				RtythmLight();
			default:
				ws2812_AllShutOff();
				break;
		}
		vTaskDelay(1);
//		printf("led_mode = %d\r\n", led_mode);
//        LED1=!LED1;
	}
}
// 律动灯
void RtythmLight(){
	last_VALUE=Get_Adc_Average(ADC_Channel_5,15);
		
	//平均值滤波，10个，上面
	light_ADC=last_VALUE;
	light_ADC=abs(last_VALUE-1500);
	
	printf("light_ADC = %d\r\n", light_ADC);
	
	light_ADC = (light_ADC <= NOISE) ? 0 :(light_ADC - 30); 
	num=light_ADC>>3;
	
	printf("num = %d\r\n", num);
	if(num >= 30)
		num=10 + (num >> 3);
	else if(num>=14)
		num=10 + (num >> 2); 
	
	ws281x_setPixelRGB2(0, 50,255,0);
	for(int i=1;i<num;i++){
	  if(i<=4){
		  ws281x_setPixelRGB2(i, 50,255-i*25,i*10 - 5); // Moderately bright green color. 
		  vTaskDelay(20);
	  }
	  else{
		  ws281x_setPixelRGB2(i, 50,5,255-(i-10)*25); // Moderately bright green color. 
		
		  vTaskDelay(20);
	  }
	}
	for(int i=num;i>=1;i--){
		delay_ms(10);
		ws281x_setPixelRGB2(i, 0,0,0); // Moderately bright green color. 
	}
}

