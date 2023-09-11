#include "WS2812.h"
#include "math.h"
#include "FreeRTOS.h"
#include "task.h"

// Some Static Colors
const RGBColor_TypeDef RED      = {255,0,0};
const RGBColor_TypeDef GREEN    = {0,255,0};
const RGBColor_TypeDef BLUE     = {0,0,255};
const RGBColor_TypeDef SKY      = {0,255,255};
const RGBColor_TypeDef MAGENTA  = {255,0,255};
const RGBColor_TypeDef YELLOW   = {255,255,0};
const RGBColor_TypeDef ORANGE   = {127,106,0};
const RGBColor_TypeDef BLACK    = {0,0,0};
const RGBColor_TypeDef WHITE    = {255,255,255};
const RGBColor_TypeDef PURPLE   = {65,105,225};

static u8 pixelBuffer[Pixel_S1_NUM][24];                     //����
RGB_Color  rgb_color;
HSV_Color  hsv_color;

/***********************************************************************************************
**     name: WS2812b_Configuration
** function:  WS2812B SPI DMA���߳�ʼ��
**parameter: void
************************************************************************************************/
/*****************************************
 ˵����
 SPI2��
 ���ţ�ʹ�õ���PB15���ţ���TFTLCD�µ�LCD BL
 ʱ�ӣ���������ͼ��SPI2��APB1(42MHz)��Ƶ����
 ****************************************/
void WS2812b_Configuration(void){
	
  GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //ʹ��GPIOBʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//ʹ��SPI1ʱ��
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);   //DMA1ʱ��ʹ�� 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                    //PB15���ù������	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                  //���ù���
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;            //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                  //����
  GPIO_Init(GPIOC, &GPIO_InitStructure);                        //��ʼ��
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource3,GPIO_AF_SPI2);        //PB15����Ϊ SPI2

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;    //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		                      //����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		                  //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		                        //����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	                        //����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		                          //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//42M/8=5.25M
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	                  //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	                            //CRCֵ����Ķ���ʽ
	SPI_Init(SPI2, &SPI_InitStructure);                                   //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
 
	SPI_Cmd(SPI2, ENABLE);                                                //ʹ��SPI����
  
	DMA_DeInit(DMA1_Stream4);

	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}                       //�ȴ�DMA������ 
		
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;                            //ͨ��ѡ��
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;                //DMA�����ַ
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)pixelBuffer;                 //DMA �洢��0��ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                   //�洢��������ģʽ
  DMA_InitStructure.DMA_BufferSize = Pixel_S1_NUM * 24;                     //���ݴ����� 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;          //���������ģʽ
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                   //�洢������ģʽ
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //�������ݳ���:8λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;           //�洢�����ݳ���:8λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                             // ʹ����ͨģʽ 
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                     //�е����ȼ�
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;               //�洢��ͻ�����δ���
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;       //����ͻ�����δ���
  DMA_Init(DMA1_Stream4, &DMA_InitStructure);                               //��ʼ��DMA Stream
  
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);	         // ʹ��SPI2��DMA���� 	
	DMA_Cmd(DMA1_Stream4, DISABLE);                            //�ر�DMA���� 	
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}	       //ȷ��DMA���Ա�����  		
	DMA_SetCurrDataCounter(DMA1_Stream4,Pixel_S1_NUM * 24);    //���ݴ�����  
	DMA_Cmd(DMA1_Stream4, ENABLE); 

}

/***********************************************************************************************
**     name: rgb_SetColor
** function: �趨ĳ��RGB LED����ɫ
**parameter: void
**   return: void
************************************************************************************************/
void rgb_SetColor(u16 LedId, RGBColor_TypeDef Color){
	
 	u16 i;
  
  for(i=0;i<=7;i++){
		pixelBuffer[LedId][i]= ( (Color.G & (1 << (7 -i)) )? (CODE1):CODE0 );
	}
  for(i=8;i<=15;i++){
		pixelBuffer[LedId][i]= ( (Color.R & (1 << (15-i)) )? (CODE1):CODE0 );
	}
  for(i=16;i<=23;i++){
		pixelBuffer[LedId][i]= ( (Color.B & (1 << (23-i)) )? (CODE1):CODE0 );
	}
}

/***********************************************************************************************
**     name: rgb_SendArray
** function: Configure colors to RGB pixel series.
             RGBColor_TypeDef: pointer to a RGBColor_TypeDef structure that contains the color configuration information for the RGB pixel.
**parameter: void
**   return: void
************************************************************************************************/
void rgb_SendArray(void){
	
	if(DMA_GetFlagStatus(DMA1_Stream4,DMA_FLAG_TCIF4) != RESET){ //�ȴ�DMA1_Steam5�������			
		DMA_ClearFlag(DMA1_Stream4,DMA_FLAG_TCIF4);                //���DMA1_Steam5������ɱ�־����Ԥ��DMA_FLAG_TCIF0���㣬�������Channel		
		SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);	         // ʹ��SPI3��DMA���� 	
		DMA_Cmd(DMA1_Stream4, DISABLE);                            //�ر�DMA���� 	
		while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}	       //ȷ��DMA���Ա�����  		
		DMA_SetCurrDataCounter(DMA1_Stream4,Pixel_S1_NUM * 24);    //���ݴ�����  
		DMA_Cmd(DMA1_Stream4, ENABLE);                             //����DMA���� 
	}
}



/***********************************************************************************************
**     name: RGB_RED
** function: �趨��ɫΪRED
**parameter: Pixel_LEN ������
**   return: void
************************************************************************************************/
void RGB_RED(u16 Pixel_LEN){
	
  u16 i;
	
  for(i = 0; i < Pixel_LEN; i++){  
    rgb_SetColor(i,RED);
	}
  
  rgb_SendArray();
}

/***********************************************************************************************
**     name: Colourful_Wheel
** function: ����ɫת��ΪGRB
**parameter: WheelPos ��ɫ��ֵ
**   return: RGBColor_TypeDef ��ɫGRB
************************************************************************************************/
RGBColor_TypeDef Colourful_Wheel(u8 WheelPos){
	
	RGBColor_TypeDef color;
  WheelPos = 255 - WheelPos;
  
  if(WheelPos < 85){
    color.R = 255 - WheelPos * 3;
    color.G = 0;
    color.B = WheelPos * 3;
		return color;
  }
  if(WheelPos < 170){
    WheelPos -= 85;
    color.R = 0;
    color.G = WheelPos * 3;
    color.B = 255 - WheelPos * 3;
		return color;
  }
  
  WheelPos -= 170;
  color.R = WheelPos * 3; 
  color.G = 255 - WheelPos * 3;
  color.B = 0;
  
  return color;  
}

void rainbowCycle(u16 Pixel_LEN){
	
  u16 i, j = 0;

  for(j = 0; j < 1023; j++){                                                   // 1 cycles of all colors on wheel
    for(i = 0; i < Pixel_LEN; i++){  
      rgb_SetColor(i,Colourful_Wheel(((i * 256 / Pixel_LEN) + j)&255));
	} 
    rgb_SendArray();
    vTaskDelay(20);
  }
}

/**
 * @Description  	�������е�
* @Param     	 NONE
 * @Return    	NONE  
*/
void ws2812_AllOpen(uint8_t red ,uint8_t green ,uint8_t blue)
{
	u16 i, j;
  for(j = 0; j < 14; j++){
	  for(i=0;i<=7;i++){
			pixelBuffer[j][i]= ( (red & (1 << (7 -i)) )? (CODE1):CODE0 );
		}
	  for(i=8;i<=15;i++){
			pixelBuffer[j][i]= ( (green & (1 << (15-i)) )? (CODE1):CODE0 );
		}
	  for(i=16;i<=23;i++){
			pixelBuffer[j][i]= ( (blue & (1 << (23-i)) )? (CODE1):CODE0 );
		}
	}
   rgb_SendArray();
}
/**
 * @Description  	WS2812 ���õ�n���������ɫ
* @Param     n:�ڼ�������   red:0-255   green:0-255    blue:0-255 	   eg:yellow:255 255 0
 * @Return    	  
*/

//����ԭɫ�������ݺϲ�Ϊ24λ����
uint32_t ws281x_color(uint8_t red, uint8_t green, uint8_t blue)
{
  return green << 16 | red << 8 | blue;
}
void ws281x_setPixelRGB2(uint16_t n ,uint8_t red, uint8_t green, uint8_t blue){
	 u16 i;

	if(n < Pixel_S1_NUM)
	  {
		for(i = 0; i < 24; ++i)
		{
		  pixelBuffer[n][i] = (((ws281x_color(red,green,blue) << i) & 0X800000) ? CODE1 : CODE0);
		}
	  }
	rgb_SendArray();
}
//�趨��n���������ɫ
void ws281x_setPixelRGB(uint16_t n ,uint8_t red, uint8_t green, uint8_t blue)
{
  u16 i;

	  for(i=0;i<=7;i++){
			pixelBuffer[n][i]= ( (red & (1 << (7 -i)) )? (CODE1):CODE0 );
		}
	  for(i=8;i<=15;i++){
			pixelBuffer[n][i]= ( (green & (1 << (15-i)) )? (CODE1):CODE0 );
		}
	  for(i=16;i<=23;i++){
			pixelBuffer[n][i]= ( (blue & (1 << (23-i)) )? (CODE1):CODE0 );
	  }

   rgb_SendArray();
}
/**
 * @Description  	WS2812�ر����еƹ�		1. ����WS2812_LED_NUM * 24λ�� 0 ��
																
 * @Param     	  {void}
 * @Return    	  {void}
*/
void ws2812_AllShutOff(void){
	uint16_t i;
  uint8_t j;
  
  for(i = 0; i < Pixel_S1_NUM; i++)
  {
    for(j = 0; j < 24; j++)
    {
      pixelBuffer[i][j] = CODE0;
    }
  }
  
  rgb_SendArray();
}
void set_pixel_rgb(uint16_t n,u8 color)
{
	switch(color)
	{
		case Red: 
			ws281x_setPixelRGB(n,255,0,0);
			break;
		case Green: 
			ws281x_setPixelRGB(n,0,255,0);
			break;
		case Blue: 
			ws281x_setPixelRGB(n,0,0,255);
			break;
		case Yellow: 
			ws281x_setPixelRGB(n,255,255,0);
			break;
		case Purple: 
			ws281x_setPixelRGB(n,255,0,255);
			break;
		case Orange: 
			ws281x_setPixelRGB(n,255,125,0);
			break;
		case Indigo: 
			ws281x_setPixelRGB(n,0,255,255);
			break;
		case White:
			ws281x_setPixelRGB(n,255,255,255);
			break;
	
	}

}
/**
 * @Description  	��ˮ��Ч��
* @Param     interval_time:���ʱ��  red:0-255 green:0-255 blue:0-255
 * @Return    	NONE  
*/
void Running_water_lamp( uint8_t red ,uint8_t green ,uint8_t blue, uint16_t interval_time )
{
	uint16_t i;
  
  for(i = 0; i < Pixel_S1_NUM; i++)
  {
		ws281x_setPixelRGB(i,red,green,blue);
		vTaskDelay(interval_time);
  }
	ws2812_AllShutOff();
	vTaskDelay(interval_time);
}
//���ùرյ�n������
void ws281x_ShutoffPixel(uint16_t n)
{
  uint8_t i;
  
  if(n < Pixel_S1_NUM)
  {
    for(i = 0; i < 24; ++i)
    {
      pixelBuffer[n][i] = CODE0;
    }
  }
	rgb_SendArray();
	vTaskDelay(10);
}
/**
 * @Description  	�����Ч��
* @Param     interval_time:���ʱ��
 * @Return    	NONE  
*/
void horse_race_lamp(uint16_t interval_time)
{
	u8 i,color;

  for(i = 0; i < Pixel_S1_NUM; i++)
  {
//		ws281x_setPixelRGB(i,255,255,0);
		color = rand()%7;
		set_pixel_rgb(i,color);//�����ɫ
		ws281x_ShutoffPixel(i-1);
		vTaskDelay(interval_time);
  }
	ws281x_ShutoffPixel(Pixel_S1_NUM-1);
	vTaskDelay(interval_time);
}

/**
 * @Description  	WS2812����ĳһλ��LED����ɫ ��������
 * @Param     	  {uint16_t LED_index ,uint32_t GRB_color}
 * @Return    	  {void}
*/
void ws2812_Set_one_LED_Color(uint16_t LED_index ,uint32_t GRB_color){
  uint8_t i = 0;
	uint32_t cnt = 0x800000;
  if(LED_index < Pixel_S1_NUM){
    for(i = 0; i < 24; ++i){
			if(GRB_color & cnt){
				pixelBuffer[LED_index][i] = CODE1;
			}
			else{
				pixelBuffer[LED_index][i] = CODE0;
			}
			cnt >>= 1;
    }
  }
}
/**
 * @Description  	WS2812 ��ɫ������ ��->��->��
 * @Param     	  {uint16_t interval_time, uint32_t GRB_color} ������ʱ��
 * @Return    	  {void}
*/
void ws2812_All_LED_one_Color_breath(uint16_t interval_time, uint32_t GRB_color){
	uint8_t i = 0;
	uint16_t j = 0;
	rgb_color.G = GRB_color>>16;
	rgb_color.R = GRB_color>>8;
	rgb_color.B = GRB_color;
	for(i=1;i<=100;i++){
		__brightnessAdjust(i/100.0f, rgb_color);
		for(j=0;j<Pixel_S1_NUM;j++){
			ws2812_Set_one_LED_Color(j, ((rgb_color.G<<16) | (rgb_color.R<<8) | (rgb_color.B)));
		}
		rgb_SendArray();
		vTaskDelay(interval_time);
	}
	for(i=100;i>=1;i--){
		__brightnessAdjust(i/100.0f, rgb_color);
		for(j=0;j<Pixel_S1_NUM;j++){
			ws2812_Set_one_LED_Color(j, ((rgb_color.G<<16) | (rgb_color.R<<8) | (rgb_color.B)));
		}
		rgb_SendArray();
		vTaskDelay(interval_time);
	}
}
void light_ctr(u8 light, uint32_t GRB_color){
	uint16_t j = 0;
	rgb_color.G = GRB_color>>16;
	rgb_color.R = GRB_color>>8;
	rgb_color.B = GRB_color;
	
	__brightnessAdjust(light/100.0f, rgb_color);
	for(j=0;j<Pixel_S1_NUM;j++){
		ws2812_Set_one_LED_Color(j, ((rgb_color.G<<16) | (rgb_color.R<<8) | (rgb_color.B)));
	}
	rgb_SendArray();

}
/***********************************************************
										Private Function
************************************************************/
/**
 * @Description  	����������ֵ
 * @Param     	  {float a,float b}
 * @Return    	  {float}
*/
float __getMaxValue(float a, float b){
	return a>=b?a:b;
}

/**
 * @Description  	���������Сֵ
 * @Param     	  {void}
 * @Return    	  {void}
*/
float __getMinValue(float a, float b){
	return a<=b?a:b;
}


/**
 * @Description  	RGB תΪ HSV
 * @Param     	  {RGB_Color RGB, HSV_Color *HSV}
 * @Return    	  {void}
*/
void __RGB_2_HSV(RGB_Color RGB, HSV_Color *HSV){
	float r,g,b,minRGB,maxRGB,deltaRGB;
	
	r = RGB.R/255.0f;
	g = RGB.G/255.0f;
	b = RGB.B/255.0f;
	maxRGB = __getMaxValue(r, __getMaxValue(g,b));
	minRGB = __getMinValue(r, __getMinValue(g,b));
	deltaRGB = maxRGB - minRGB;
	
	HSV->V = deltaRGB;
	if(maxRGB != 0.0f){
		HSV->S = deltaRGB / maxRGB;
	}
	else{
		HSV->S = 0.0f;
	}
	if(HSV->S <= 0.0f){
		HSV->H = 0.0f;
	}
	else{
		if(r == maxRGB){
			HSV->H = (g-b)/deltaRGB;
    }
    else{
			if(g == maxRGB){
        HSV->H = 2.0f + (b-r)/deltaRGB;
      }
      else{
				if (b == maxRGB){
					HSV->H = 4.0f + (r-g)/deltaRGB;
        }
      }
    }
    HSV->H = HSV->H * 60.0f;
    if (HSV->H < 0.0f){
			HSV->H += 360;
    }
    HSV->H /= 360;
  }
}


/**
 * @Description  	HSV תΪ RGB
 * @Param     	  {void}
 * @Return    	  {void}
*/
void __HSV_2_RGB(HSV_Color HSV, RGB_Color *RGB){
	float R,G,B,aa,bb,cc,f;
  int k;
  if (HSV.S <= 0.0f)
		R = G = B = HSV.V;
  else{
		if (HSV.H == 1.0f){
			HSV.H = 0.0f;
		}
    HSV.H *= 6.0f;
    k = (int)floor(HSV.H);
    f = HSV.H - k;
    aa = HSV.V * (1.0f - HSV.S);
    bb = HSV.V * (1.0f - HSV.S * f);
    cc = HSV.V * (1.0f -(HSV.S * (1.0f - f)));
    switch(k){
      case 0:
       R = HSV.V; 
       G = cc; 
       B =aa;
       break;
      case 1:
       R = bb; 
       G = HSV.V;
       B = aa;
       break;
      case 2:
       R =aa;
       G = HSV.V;
       B = cc;
       break;
      case 3:
       R = aa;
       G = bb;
       B = HSV.V;
       break;
      case 4:
       R = cc;
       G = aa;
       B = HSV.V;
       break;
      case 5:
       R = HSV.V;
       G = aa;
       B = bb;
       break;
    }
  }
  RGB->R = (unsigned char)(R * 255);
  RGB->G = (unsigned char)(G * 255);
  RGB->B = (unsigned char)(B * 255);
}


/**
 * @Description  	���ȵ���
 * @Param     	  {void}
 * @Return    	  {void}
*/
void __brightnessAdjust(float percent, RGB_Color RGB){
	if(percent < 0.01f){
		percent = 0.01f;
	}
	if(percent > 1.0f){
		percent = 1.0f;
	}
	__RGB_2_HSV(RGB, &hsv_color);
	hsv_color.V = percent;
	__HSV_2_RGB(hsv_color, &rgb_color);
}
