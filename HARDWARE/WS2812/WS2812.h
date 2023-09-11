#ifndef __WS2812_H
#define __WS2812_H

/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "delay.h"

#define 		SIG_1 		0XF8
#define 		SIG_0  		0XE0

typedef struct
{
  u8 R;
  u8 G;
  u8 B;
} RGBColor_TypeDef;

//       ???     GRB
#define	C_Red		0x00FF00
#define	C_Green		0xFF0000
#define	C_Blue		0x0000FF
#define	C_Yellow   	0xFFFF00
#define	C_Purple	0x00FFFF
#define	C_Orange	0x7DFF00
#define	C_indigo	0xFF00FF
#define	C_white 	0xFFFFFF

typedef struct RGB_Color{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char l;
}RGB_Color;
typedef struct HSV_Color{
	float H;
	float S;
	float V;
}HSV_Color;

enum Color
{
	Red,
	Green,
	Blue,
	Yellow,
	Purple,
	Orange,
	Indigo,
	White,
};

//extern u8 pixelBuffer[][24];
//extern const RGBColor_TypeDef RED ;
//extern const RGBColor_TypeDef GREEN;
//extern const RGBColor_TypeDef BLUE;
//extern const RGBColor_TypeDef SKY;
//extern const RGBColor_TypeDef MAGENTA ;
//extern const RGBColor_TypeDef YELLOW ;
//extern const RGBColor_TypeDef ORANGE;
//extern const RGBColor_TypeDef BLACK;
//extern const RGBColor_TypeDef WHITE;
//extern const RGBColor_TypeDef PURPLE;
/* Exported constants --------------------------------------------------------*/
#define Pixel_S1_NUM 14		//灯珠 RGB数量

/**************************************
 说明：

 WS2812B编码协议(单位：ns)：
       min     typ     max
bit 0
 T0H:  220      -      420
 T0L:  750      -      1600
 
bit 1 
 T1H:  750      -      1600
 T1L:  220      -      420
 
 
 RESET: time > 300us


8分频APB1，42MHz/8 = 5.25MHz
时钟周期为：1/5.25/1e6 = 1.90e-7=190ns
**************************************/

#define CODE0 0xC0 // 0码, 发送的时间 1100 0000  根据不同的SCK适当调整
#define CODE1 0xFC // 1码, 发送的时间 1111 1100

void WS2812b_Configuration(void);

/* Basic Color Effects -------------------------------------------------------*/
void RGB_RED(u16 Pixel_LEN);
//void RGB_GREEN(u16 Pixel_LEN);
//void RGB_BLUE(u16 Pixel_LEN);
//void RGB_YELLOW(u16 Pixel_LEN);
//void RGB_MAGENTA(u16 Pixel_LEN);
//void RGB_BLACK(u16 Pixel_LEN);
//void RGB_WHITE(u16 Pixel_LEN);
//void RGB_SKY(u16 Pixel_LEN);
//void RGB_ORANGE(u16 Pixel_LEN);
//void RGB_PURPLE(u16 Pixel_LEN);
/* Complicated Color Effects -------------------------------------------------*/
void rainbowCycle(u16 Pixel_LEN); // 彩虹循环
void Running_water_lamp( uint8_t red ,uint8_t green ,uint8_t blue, uint16_t interval_time ); // 流水灯
void horse_race_lamp(uint16_t interval_time);  // 跑马灯
void ws2812_All_LED_one_Color_breath(uint16_t interval_time, uint32_t GRB_color); // 呼吸灯
void light_ctr(u8 light, uint32_t GRB_color); // 亮度调节

void ws2812_AllShutOff(void);
void ws2812_AllOpen(uint8_t red ,uint8_t green ,uint8_t blue);  // 开灯
void ws281x_setPixelRGB(uint16_t n ,uint8_t red, uint8_t green, uint8_t blue);
void ws281x_setPixelRGB2(uint16_t n ,uint8_t red, uint8_t green, uint8_t blue);
void __brightnessAdjust(float percent, RGB_Color RGB);

#endif /* __WS2812_H */

