#ifndef __USART3_H__
#define __USART3_H__
#include "sys.h"

#define USART_DEBUG		USART1		//调试打印所使用的串口组

void usart3_Init(unsigned int baud);//初始化

void Usart_SendString(USART_TypeDef *USARTx, unsigned char *str, unsigned short len);

void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...);
		 				    
#endif
