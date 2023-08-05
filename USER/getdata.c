#include "getdata.h"
#include "usart3.h"
#include "string.h"

void get_weather_str(){
	unsigned char *cmd = "GET https://api.seniverse.com/v3/weather/now.json?key=ScuqSTA5ihQt3Oyhf&location=zhanjiang&language=zh-Hans&unit=c\r\n";
	
	ESP8266_Clear();
	
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		delay_ms(500);

	while(ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK"))
		delay_ms(500);

	while(ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"api.seniverse.com\",80\r\n", "OK"))
		delay_ms(500);
	
	UsartPrintf(USART_DEBUG, "5\r\n");
	while(ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK"))
		delay_ms(500);
	
	UsartPrintf(USART_DEBUG, "6\r\n");
	while(ESP8266_SendCmd("AT+CIPSEND\r\n", "OK"))
		delay_ms(500);
	
	Usart_SendString(USART3, (unsigned char *)cmd, strlen((const char *)cmd));
	delay_ms(1000);
	
	Usart_SendString(USART3, "+++", 3);

	// 断开服务器连接
	while(ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK"))

	UsartPrintf(USART_DEBUG, "success\r\n");
}




///* 字符转十进制 */
//unsigned int Char_To_Decimalism(unsigned char Char){
//    if(Char == '0')         return 0;
//    else    if(Char == '1') return 1;
//    else    if(Char == '2') return 2;
//    else    if(Char == '3') return 3;
//    else    if(Char == '4') return 4;
//    else    if(Char == '5') return 5;
//    else    if(Char == '6') return 6;
//    else    if(Char == '7') return 7;
//    else    if(Char == '8') return 8;
//    else    if(Char == '9') return 9;
//    else                    return 0;

//}

//void Get_Weather_Data(unsigned int *code,unsigned int *temp){
//    unsigned char temp_h,temp_l;
//    unsigned int data1,data2;

//	if(strstr((const char *)esp8266_buf, "\"code\"") != NULL){
//		WeatherPtr = strstr((const char *)esp8266_buf, "\"code\"");
//		temp_h = (*(WeatherPtr+8));
//        temp_l = (*(WeatherPtr+9));
//        if(temp_l == '"'){
//            data1 = Char_To_Decimalism(temp_h);
//            *code = data1;
//        }  
//        else{
//            data1 = Char_To_Decimalism(temp_h);
//            data2 = Char_To_Decimalism(temp_l);
//            *code = data1*10+data2;
//        }   
//	}
//	if(strstr((const char *)esp8266_buf, "\"temperature\"") != NULL){
//		WeatherPtr = strstr((const char *)esp8266_buf, "\"temperature\"");
//		temp_h = (*(WeatherPtr+15));
//        temp_l = (*(WeatherPtr+16));
//		if(temp_l == '"'){
//            data1 = Char_To_Decimalism(temp_h);
//            *temp = data1;
//        }  
//        else{
//            data1 = Char_To_Decimalism(temp_h);
//            data2 = Char_To_Decimalism(temp_l);
//            *temp = data1*10+data2;
//        }
//	}
//}

//void Get_Net_Time(){
//    if(strstr((const char *)esp8266_buf, "TIME:") != NULL){
//		TimePtr = strstr((const char *)esp8266_buf, "TIME:");
//		printf("%s\r\n",TimePtr+5);
//        sprintf(time_buf,"%.8s",TimePtr+16);
//        memset(esp8266_buf,0,sizeof(esp8266_buf));
//	}
//}

