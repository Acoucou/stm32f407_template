#include "sys.h"
#include "esp8266.h"
#include "delay.h"
#include "usart.h"
#include "usart3.h"

#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"CouCou\",\"12345678\"\r\n"
#define mqtt_cfg				"AT+MQTTUSERCFG=0,1,\"id\",\"\",\"\",0,0,\"\"\r\n"
#define ESP8266_ONENET_INFO		"AT+MQTTCONN=0,\"1.15.236.13\",1883,1\r\n"

unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

//	�������ƣ�	ESP8266_Clear
//	�������ܣ�	��ջ���
void ESP8266_Clear(void)
{
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//	�������ƣ�	ESP8266_WaitRecive
//	�������ܣ�	�ȴ��������
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//	˵����		ѭ�����ü���Ƿ�������
_Bool ESP8266_WaitRecive(void)
{
	if(esp8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
	
	if(esp8266_cnt == esp8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		esp8266_cnt = 0;							//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	esp8266_cntPre = esp8266_cnt;					//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־
}

//	�������ƣ�	ESP8266_SendCmd
//	�������ܣ�	��������
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//	���ز�����	0-�ɹ�	1-ʧ��
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	unsigned char timeOut = 200;

	Usart_SendString(USART3, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//����յ�����
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
			{
				ESP8266_Clear();									//��ջ���
				
				return 0;
			}
		}
		
		delay_ms(10);
	}
	return 1;
}

//	�������ܣ�	��������
//	��ڲ�����	data������
//				len������
void ESP8266_SendData(unsigned char *data, unsigned short len)
{
	UsartPrintf(USART_DEBUG, "���Ϳ�ʼ\r\n");
	while(ESP8266_SendCmd("AT+MQTTPUB=0,\"topic\",\"text\",1,0\r\n", "OK"))
		delay_ms(500);
	UsartPrintf(USART_DEBUG, "���ͳɹ�\r\n");
}

//	�������ƣ�	ESP8266_GetIPD
//	�������ܣ�	��ȡƽ̨���ص�����
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//	���ز�����	ƽ̨���ص�ԭʼ����
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
unsigned char *ESP8266_GetIPD(unsigned short timeOut, char *topic)
{
	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//����������
		{
			ptrIPD = strstr((char *)esp8266_buf, topic);				//������msg��ͷ
			if(ptrIPD == NULL)											//���û�ҵ���������msgͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
			{
				UsartPrintf(USART_DEBUG, "\"topic\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ',');							//�ҵ�':'
				ptrIPD++;
				ptrIPD = strchr(ptrIPD, ',');							//�ҵ�':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}
		
		delay_ms(5);													//��ʱ�ȴ�
	} while(timeOut--);
	
	return NULL;														//��ʱ��δ�ҵ������ؿ�ָ��
}

//	�������ƣ�	ESP8266_Init
//	�������ܣ�	��ʼ��ESP8266
void ESP8266_Init(void)
{
	ESP8266_Clear();

	UsartPrintf(USART_DEBUG, " 1. AT\r\n");
	while(ESP8266_SendCmd("AT\r\n", "OK"))
		delay_ms(500);
	
	while(ESP8266_SendCmd("AT+RST\r\n", "OK"))
		delay_ms(500);
	
	while(ESP8266_SendCmd("AT\r\n", "OK"))
		delay_ms(500);

	UsartPrintf(USART_DEBUG, "2. CWMODE\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		delay_ms(500);

	UsartPrintf(USART_DEBUG, "3. CWJAP\r\n");
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
		delay_ms(500);

	while(ESP8266_SendCmd(mqtt_cfg, "OK"))
		delay_ms(500);
	
	UsartPrintf(USART_DEBUG, "5. CIPSTART\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		delay_ms(500);
	
	UsartPrintf(USART_DEBUG, "6.AT+MQTTSUB=0,\"led\",1\r\n");
	while(ESP8266_SendCmd("AT+MQTTSUB=0,\"led\",1\r\n", "OK"))
		delay_ms(500);

	UsartPrintf(USART_DEBUG, "7. ESP8266 Init OK\r\n");
}

//	�������ƣ�	USART3_IRQHandler
//	�������ܣ�	����3�շ��ж�
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //�����ж�
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //��ֹ���ڱ�ˢ��
	
		esp8266_buf[esp8266_cnt++] = USART3->DR;
		
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}
}
