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

// lvglͷ�ļ�
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
// myAPP
#include "app.h"

// rtosͷ�ļ�
#include "FreeRTOS.h"
#include "task.h"


uint8_t led_mode, led_light = 100, led_color, led_flag;
uint32_t led_red, led_green, led_blue, led_rgb = C_Purple;

#define NOISE 50       //��������
u16 max9814_data;
int last_VALUE=0;
int light_ADC=0;
int num;

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define TASK1_TASK_PRIO		2
//�����ջ��С	
#define TASK1_STK_SIZE 		128  
//������
TaskHandle_t Task1Task_Handler;
//������
void task1_task(void *pvParameters);

//�������ȼ�
#define TASK2_TASK_PRIO		3
//�����ջ��С	
#define TASK2_STK_SIZE 		128  
//������
TaskHandle_t Task2Task_Handler;
//������
void task2_task(void *pvParameters);

// �ɶ���
void RtythmLight(void);

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
	usart3_Init(115200);
	WS2812b_Configuration();
	
	LED_Init();					//��ʼ��LED 
 	LCD_Init();					//LCD��ʼ�� 
	KEY_Init(); 				//������ʼ��  
	BEEP_Init();				//��������ʼ��
	Adc_Init();
	TIM3_Int_Init(999,83);	//��ʱ����ʼ��(1ms�ж�),���ڸ�lvgl�ṩ1ms����������
	tp_dev.init();			//��������ʼ��
	//ESP8266_Init();
	//get_weather_str();
	//printf("the data is:\r\n %s\r\n", esp8266_buf);
	
	
	lv_init();						//lvglϵͳ��ʼ��
	lv_port_disp_init();	//lvgl��ʾ�ӿڳ�ʼ��,����lv_init()�ĺ���
	lv_port_indev_init();	//lvgl����ӿڳ�ʼ��,����lv_init()�ĺ���
	
	app_start();
	
	
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
    //����TASK1����
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);   
    //����TASK2����
    xTaskCreate((TaskFunction_t )task2_task,     
                (const char*    )"task2_task",   
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_TASK_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler); 
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//task1������, UIˢ��
void task1_task(void *pvParameters)
{
	while(1)
	{
		tp_dev.scan(0);//����ɨ��
		lv_task_handler();//lvgl������
	}
}

//�ʵƿ���������
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
// �ɶ���
void RtythmLight(){
	last_VALUE=Get_Adc_Average(ADC_Channel_5,15);
		
	//ƽ��ֵ�˲���10��������
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

