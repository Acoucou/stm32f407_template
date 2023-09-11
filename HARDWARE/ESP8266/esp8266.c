#include "sys.h"
#include "esp8266.h"
#include "delay.h"
#include "usart.h"
#include "usart3.h"

#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"CouCou\",\"12345678\"\r\n"
#define mqtt_cfg				"AT+MQTTUSERCFG=0,1,\"id\",\"\",\"\",0,0,\"\"\r\n"
#define ESP8266_ONENET_INFO		"AT+MQTTCONN=0,\"81.71.154.91\",1883,1\r\n"

unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
char cmdBuf[100];

extern unsigned char pitch_str[10], yaw_str[10], roll_str[10], temp_str[10];

//	函数名称：	ESP8266_Clear
//	函数功能：	清空缓存
void ESP8266_Clear(void)
{
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//	函数名称：	ESP8266_WaitRecive
//	函数功能：	等待接收完成
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//	说明：		循环调用检测是否接收完成
_Bool ESP8266_WaitRecive(void)
{
	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
	
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志
}

//	函数名称：	ESP8266_SendCmd
//	函数功能：	发送命令
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//	返回参数：	0-成功	1-失败
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	unsigned char timeOut = 200;

	Usart_SendString(USART3, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		delay_ms(10);
	}
	return 1;
}

//	函数功能：	订阅主题
//	入口参数：	data：数据
//				len：长度
void ESP8266_SendData(unsigned char *data, unsigned short len)
{
	UsartPrintf(USART_DEBUG, "发送开始\r\n");
	while(ESP8266_SendCmd("AT+MQTTPUB=0,\"topic\",\"text\",1,0\r\n", "OK"))
		delay_ms(500);
	UsartPrintf(USART_DEBUG, "发送成功\r\n");
}

//	函数名称：	ESP8266_GetIPD
//	函数功能：	获取平台返回的数据
//	入口参数：	等待的时间(乘以10ms)
//	返回参数：	平台返回的原始数据
//	说明：		不同网络设备返回的格式不同，需要去调试
unsigned char *ESP8266_GetIPD(unsigned short timeOut, char *topic)
{
	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, topic);				//搜索“msg”头
			if(ptrIPD == NULL)											//如果没找到，可能是msg头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				UsartPrintf(USART_DEBUG, "\"topic\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ',');							//找到':'
				ptrIPD++;
				ptrIPD = strchr(ptrIPD, ',');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}
		
		delay_ms(5);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针
}

//	函数名称：	ESP8266_Init
//	函数功能：	初始化ESP8266
void ESP8266_Init(void)
{
	ESP8266_Clear();
	
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
	
	UsartPrintf(USART_DEBUG, "6.AT+MQTTSUB=0,\"sensor\",1\r\n");
	while(ESP8266_SendCmd("AT+MQTTSUB=0,\"sensor\",1\r\n", "OK"))
		delay_ms(500);

	UsartPrintf(USART_DEBUG, "7. ESP8266 Init OK\r\n");
}

//	函数名称：	USART3_IRQHandler
//	函数功能：	串口3收发中断
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //接收中断
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
	
		esp8266_buf[esp8266_cnt++] = USART3->DR;
		
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}
}
int fingString(char *pcBuf, char *left, char *right, char *pcRes){
	char *pcBegin = NULL;
	char *pcEnd = NULL;

	pcBegin = strstr(pcBuf, left);

	pcEnd = strstr(pcBegin + strlen(left) + 4, right);

	if(pcBegin == NULL || pcEnd == NULL || pcBegin >= pcEnd){
		return 0;
	}else{
		pcBegin += strlen(left);
		memcpy(pcRes, pcBegin + 4, pcEnd - pcBegin - 4);
		return 1;
	}
}

void SendSensorDate()
{
	

	sprintf(cmdBuf, "AT+MQTTPUB=0,\"sensor\",\"{\\\"tempara\\\": \\\"%s\\\"\\\,\\\"pitch\\\": \\\"%s\\\"\\\,\\\"roll\\\": \\\"%s\\\"\\\,\\\"yaw\\\": \\\"%s\\\"}\",1,0\r\n", temp_str, pitch_str, roll_str, yaw_str);
	//printf("%s", cmdBuf);

	while (ESP8266_SendCmd(cmdBuf, "OK"))
		;
	
	ESP8266_Clear(); //清空接收缓存
}