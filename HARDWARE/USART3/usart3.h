#ifndef __USART3_H__
#define __USART3_H__
#include "sys.h"

#define USART_DEBUG		USART1		//���Դ�ӡ��ʹ�õĴ�����

void usart3_Init(unsigned int baud);//��ʼ��

void Usart_SendString(USART_TypeDef *USARTx, unsigned char *str, unsigned short len);

void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...);
		 				    
#endif
