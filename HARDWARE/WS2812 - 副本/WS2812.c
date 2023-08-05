
/************************************************************
	* @FileName:     	ws2812.c
	* @Author:				Hermes
	* @Version:				
	* @Date:					
	* @Description:  SPI + DMA 方式实现归零码发送 灯带驱动模块
*************************************************************/
#include "WS2812.h" 
#include <math.h>
#include <stdlib.h>

uint8_t ws2812_data_buffer[WS2812_LED_NUM][24] ;

RGB_Color  rgb_color;
HSV_Color  hsv_color;


/**
 * @Description  	WS2812灯带GPIO初始化
 * @Param     	  {void}
 * @Return    	  {void}
*/
void ws2812_GPIO_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
	
	//GPIOFB,5初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;//PB3~5复用功能输出	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
}
//SPI1 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{		 			 
 
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空  
	
	SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个byte  数据
		
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完一个byte  
 
	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据	
 		    
}
/**
 * @Description  	WS2812灯带SPI初始化
 * @Param     	  {void}
 * @Return    	  {void}
*/
void ws2812_SPI_Init(void){
//	SPI_InitTypeDef  		SPI_InitStructure;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
//	
//	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//	SPI_InitStructure.SPI_CRCPolynomial = 7;
//	SPI_Init(SPI1, &SPI_InitStructure);
// 
//	SPI_Cmd(SPI1, ENABLE);
//  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_InitTypeDef  SPI_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//使能SPI1时钟
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_SPI1); //PB5复用为 SPI1
 
	//这里只针对SPI口初始化
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//复位SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//停止复位SPI1

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI1, ENABLE); //使能SPI外设

	SPI1_ReadWriteByte(0xff);//启动传输			 	
}
 
/**
 * @Description  	WS2812灯带DMA初始化
 * @Param     	  {void}
 * @Return    	  {void}
*/
void ws2812_DMA_Init(void){
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);   //DMA1时钟使能 
	
//	DMA_DeInit(DMA1_Channel3);
//	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(SPI1 -> DR);
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ws2812_data_buffer;
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
//	DMA_InitStructure.DMA_BufferSize = WS2812_LED_NUM * 24;
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	 DMA_InitStructure.DMA_Channel = DMA_Channel_0;                            //通道选择
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI1->DR;                //DMA外设地址
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ws2812_data_buffer;                 //DMA 存储器0地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                   //存储器到外设模式
  DMA_InitStructure.DMA_BufferSize = WS2812_LED_NUM * 24;                     //数据传输量 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;          //外设非增量模式
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                   //存储器增量模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //外设数据长度:8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;           //存储器数据长度:8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                             // 使用普通模式 
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                     //中等优先级
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;               //存储器突发单次传输
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;       //外设突发单次传输
  DMA_Init(DMA1_Stream4, &DMA_InitStructure);

	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);	         // 使能SPI2的DMA发送 	
	DMA_Cmd(DMA1_Stream4, DISABLE);                            //关闭DMA传输 	
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}	       //确保DMA可以被设置  		
	DMA_SetCurrDataCounter(DMA1_Stream4,WS2812_LED_NUM * 24);    //数据传输量  
	DMA_Cmd(DMA1_Stream4, ENABLE); 


//初始化DMA Stream

}

void WS2812b_Configuration(void){
	
 GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //使能GPIOB时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//使能SPI1时钟
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);   //DMA1时钟使能 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                    //PB15复用功能输出	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                  //复用功能
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;            //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                  //上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);                        //初始化
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource3,GPIO_AF_SPI2);        //PB15复用为 SPI2

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;    //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		                      //设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		                  //设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		                        //串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	                        //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		                          //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//42M/8=5.25M
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	                  //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	                            //CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);                                   //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI2, ENABLE);                                                //使能SPI外设
  
	DMA_DeInit(DMA1_Stream4);

	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}                       //等待DMA可配置 
		
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;                            //通道选择
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;                //DMA外设地址
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)ws2812_data_buffer;                 //DMA 存储器0地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                   //存储器到外设模式
  DMA_InitStructure.DMA_BufferSize = WS2812_LED_NUM * 24;                     //数据传输量 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;          //外设非增量模式
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                   //存储器增量模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //外设数据长度:8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;           //存储器数据长度:8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                             // 使用普通模式 
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                     //中等优先级
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;               //存储器突发单次传输
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;       //外设突发单次传输
  DMA_Init(DMA1_Stream4, &DMA_InitStructure);                               //初始化DMA Stream
  
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);	         // 使能SPI2的DMA发送 	
	DMA_Cmd(DMA1_Stream4, DISABLE);                            //关闭DMA传输 	
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}	       //确保DMA可以被设置  		
	DMA_SetCurrDataCounter(DMA1_Stream4,WS2812_LED_NUM * 24);    //数据传输量  
	DMA_Cmd(DMA1_Stream4, ENABLE); 
}


/**
 * @Description  	WS2812灯带初始化主调函数
 * @Param     	  {void}
 * @Return    	  {void}
*/
void ws2812_Init(void){
	ws2812_GPIO_Init();
	ws2812_SPI_Init();
	ws2812_DMA_Init();
  ws2812_AllShutOff();
  delay_ms(WS2812_LED_NUM * 10);
}


/**
 * @Description  	WS2812 启动DMA传输
 * @Param     	  {void}
 * @Return    	  {void}
*/
void ws2812_Send_Data(void){
//	DMA_Cmd(DMA1_Channel3, DISABLE );
//  DMA_ClearFlag(DMA1_FLAG_TC3);    
// 	DMA_SetCurrDataCounter(DMA1_Channel3,24 * WS2812_LED_NUM );
// 	DMA_Cmd(DMA1_Channel3, ENABLE);
	
	DMA_ClearFlag(DMA1_Stream4,DMA_FLAG_TCIF4);                //清除DMA1_Steam5传输完成标志，先预想DMA_FLAG_TCIF0的零，代表的是Channel		
		SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);	         // 使能SPI3的DMA发送 	
		DMA_Cmd(DMA1_Stream4, DISABLE);                            //关闭DMA传输 	
		while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}	       //确保DMA可以被设置  		
		DMA_SetCurrDataCounter(DMA1_Stream4,WS2812_LED_NUM * 24);    //数据传输量  
		DMA_Cmd(DMA1_Stream4, ENABLE);                             //开启DMA传输 
}

//将三原色单独数据合并为24位数据
uint32_t ws281x_color(uint8_t red, uint8_t green, uint8_t blue)
{
  return green << 16 | red << 8 | blue;
}

/**
 * @Description  	WS2812 设置第n个灯珠的颜色
* @Param     n:第几个灯珠   red:0-255   green:0-255    blue:0-255 	   eg:yellow:255 255 0
 * @Return    	  
*/

//设定第n个灯珠的颜色
void ws281x_setPixelRGB(uint16_t n ,uint8_t red, uint8_t green, uint8_t blue)
{
  uint8_t i;
  
  if(n < WS2812_LED_NUM)
  {
    for(i = 0; i < 24; ++i)
    {
      ws2812_data_buffer[n][i] = (((ws281x_color(red,green,blue) << i) & 0X800000) ? SIG_1 : SIG_0);
    }
  }
	ws2812_Send_Data();
	delay_ms(10);
}


/**
 * @Description  	WS2812 设置灯珠颜色（固定的）
* @Param       		 n:第几个灯珠   color:哪种颜色（0-7）
 * @Return    	  
*/
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



//设置关闭第n个灯珠
void ws281x_ShutoffPixel(uint16_t n)
{
  uint8_t i;
  
  if(n < WS2812_LED_NUM)
  {
    for(i = 0; i < 24; ++i)
    {
      ws2812_data_buffer[n][i] = SIG_0;
    }
  }
	ws2812_Send_Data();
	delay_ms(10);
}



/**
 * @Description  	WS2812关闭所有灯光		1. 发送WS2812_LED_NUM * 24位的 0 码
																
 * @Param     	  {void}
 * @Return    	  {void}
*/
void ws2812_AllShutOff(void){
	uint16_t i;
  uint8_t j;
  
  for(i = 0; i < WS2812_LED_NUM; i++)
  {
    for(j = 0; j < 24; j++)
    {
      ws2812_data_buffer[i][j] = SIG_0;
    }
  }
  ws2812_Send_Data();
	delay_ms(10*WS2812_LED_NUM);
}


/**
 * @Description  	WS2812设置某一位的LED的颜色 但不发送
 * @Param     	  {uint16_t LED_index ,uint32_t GRB_color}
 * @Return    	  {void}
*/
void ws2812_Set_one_LED_Color(uint16_t LED_index ,uint32_t GRB_color){
  uint8_t i = 0;
	uint32_t cnt = 0x800000;
  if(LED_index < WS2812_LED_NUM){
    for(i = 0; i < 24; ++i){
			if(GRB_color & cnt){
				ws2812_data_buffer[LED_index][i] = SIG_1;
			}
			else{
				ws2812_data_buffer[LED_index][i] = SIG_0;
			}
			cnt >>= 1;
    }
  }
}


/**
 * @Description  	WS2812 色环转化 0-255灰度值转换为GRB值
 * @Param     	  {uint8_t LED_gray}
 * @Return    	  {uint32_t}
*/
uint32_t ws2812_LED_Gray2GRB(uint8_t LED_gray){
	LED_gray = 0xFF - LED_gray;
	if(LED_gray < 85){
		return (((0xFF - 3 * LED_gray)<<8) | (3 * LED_gray));
	}
	if(LED_gray < 170){
		LED_gray = LED_gray - 85;
		return (((3 * LED_gray)<<16) | (0xFF - 3 * LED_gray));
	}
	LED_gray = LED_gray - 170;
	return (((0xFF - 3 * LED_gray)<<16) | ((3 * LED_gray)<<8));
}


/**
 * @Description  	WS2812 灰度值驱动渐变效果 沿色环转动
 * @Param     	  {uint16_t interval_time} 渐变间隔时间
 * @Return    	  {void}
*/
void ws2812_Roll_on_Color_Ring(uint16_t interval_time){
	uint8_t i = 0;
	uint16_t j = 0;
	for(i = 0;i <= 255;i++){
		for(j = 0;j < WS2812_LED_NUM;j++){
			ws2812_Set_one_LED_Color(j, ws2812_LED_Gray2GRB(i));
		}
		ws2812_Send_Data();
		delay_ms(interval_time);
	}
}

/**
 * @Description  	WS2812 单色呼吸灯 暗->亮->暗
 * @Param     	  {uint16_t interval_time, uint32_t GRB_color} 渐变间隔时间
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
		for(j=0;j<WS2812_LED_NUM;j++){
			ws2812_Set_one_LED_Color(j, ((rgb_color.G<<16) | (rgb_color.R<<8) | (rgb_color.B)));
		}
		ws2812_Send_Data();
		delay_ms(interval_time);
	}
	for(i=100;i>=1;i--){
		__brightnessAdjust(i/100.0f, rgb_color);
		for(j=0;j<WS2812_LED_NUM;j++){
			ws2812_Set_one_LED_Color(j, ((rgb_color.G<<16) | (rgb_color.R<<8) | (rgb_color.B)));
		}
		ws2812_Send_Data();
		delay_ms(interval_time);
	}
}

/**
 * @Description  	跑马灯效果
* @Param     interval_time:间隔时间
 * @Return    	NONE  
*/
void horse_race_lamp(uint16_t interval_time)
{
	u8 i,color;
	
	
  for(i = 0; i < WS2812_LED_NUM; i++)
  {
//		ws281x_setPixelRGB(i,255,255,0);
		color = rand()%7;
		set_pixel_rgb(i,color);//随机颜色
		ws281x_ShutoffPixel(i-1);
		delay_ms(interval_time);
  }
	ws281x_ShutoffPixel(WS2812_LED_NUM-1);
	delay_ms(interval_time);
}


/**
 * @Description  	流水灯效果
* @Param     interval_time:间隔时间  red:0-255 green:0-255 blue:0-255
 * @Return    	NONE  
*/
void Running_water_lamp( uint8_t red ,uint8_t green ,uint8_t blue, uint16_t interval_time )
{
	uint16_t i;
  
  for(i = 0; i < WS2812_LED_NUM; i++)
  {
		ws281x_setPixelRGB(i,red,green,blue);
		delay_ms(interval_time);
  }
	ws2812_AllShutOff();
	delay_ms(interval_time);
}
/**
 * @Description  	点亮所有灯
* @Param     	 NONE
 * @Return    	NONE  
*/
void ws2812_AllOpen(uint8_t red ,uint8_t green ,uint8_t blue)
{
	uint16_t i,j;
  
	for(j = 0;j<WS2812_LED_NUM;j++)
  {
    for(i = 0; i < 24; ++i)
    {
      ws2812_data_buffer[j][i] = (((ws281x_color(red,green,blue) << i) & 0X800000) ? SIG_1 : SIG_0);
    }
  }
	ws2812_Send_Data();
	delay_ms(10);
}



/**
 * @Description  	随机点亮RGB灯
* @Param     interval_time:间隔时间
 * @Return    	NONE  
*/
uint8_t tmp_flag[WS2812_LED_NUM];


void srand_lamp(uint16_t interval_time)
{
	static uint8_t tmp,i;
	uint8_t k,color;

	tmp = rand()%(WS2812_LED_NUM);
	color = rand()%7;
	if(i==0) //只做一次
	{
		memset(tmp_flag,50,WS2812_LED_NUM);
		tmp_flag[i] = tmp;
		set_pixel_rgb(tmp,color);
		delay_ms(interval_time);
		i++;
	
	}
	else if(i>=WS2812_LED_NUM)
	{
		return ;
	}
		
	for(k=0;k<i;k++)
	{
		if(tmp == tmp_flag[k])//相同就退出
		{
			return ;
		}
		
	}

	//遍历完成
	tmp_flag[i] = tmp;
	set_pixel_rgb(tmp,color);
	delay_ms(interval_time);
	i++;


}






/***********************************************************
										Private Function
************************************************************/
/**
 * @Description  	获得两数最大值
 * @Param     	  {float a,float b}
 * @Return    	  {float}
*/
float __getMaxValue(float a, float b){
	return a>=b?a:b;
}

/**
 * @Description  	获得两数最小值
 * @Param     	  {void}
 * @Return    	  {void}
*/
float __getMinValue(float a, float b){
	return a<=b?a:b;
}


/**
 * @Description  	RGB 转为 HSV
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
 * @Description  	HSV 转为 RGB
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
 * @Description  	亮度调节
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
/************************************************************
														EOF
*************************************************************/




