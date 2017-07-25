//******************************************************************************/
#include "sim900a.h"
#include "usart2.h"
#include "delay.h"
#include "sys.h"
#include "bmp.h"
#include "usart.h"
#include "timer.h"
#include "exti.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_dma.h"
#include "string.h"
#include "led.h"

void CLR_Message(void);
void CLR_HTTP(void);

//需要按原理图要求焊接好R26~R28
#define Buf2_Max 300
#define Message_Max 200

char ip[]="nirenelec.oicp.net";//服务器IP或域名
char port[]="38293";//端口
char HTTP_Request[Message_Max]; 
char message[Message_Max];

int main(void)
 {
	u8 res=1;
	u8 http_res = 0;
	u8 lock_flag = 0;
	u8 type = 0;
	u8 n = 0;
	u8 lock_off_flag = 0;
	u8 curr_lock_flag = 0;
	 
	LED_GPIO_Config(); //LED端口初始化

	delay_init();	    	       //延时函数初始化	  
	NVIC_Configuration(); 	   //设置NVIC中断分组2:2位抢占优先级，2位响应优先级 	

	uart_init(115200);
	USART2_Init(115200);	     //初始化串口2
	 

	TIM2_NVIC_Configuration(); // TIM2 定时配置 
	TIM2_Configuration(); 		
	EXTIX_Init();
	 
	UART1_SendString("====main testing====\r\n");
	LED_OFF();
  lock_off_flag = GPIO_ReadOutputDataBit(LED_PORT, LED);
  curr_lock_flag = GPIO_ReadOutputDataBit(LED_PORT, LED);

	sprintf((char*)message,"lock_off_flag: %d; current-led-status: %d", lock_off_flag, curr_lock_flag);
	UART1_SendString((char*)message);UART1_SendLR();
	CLR_Message();
			
	while(res)
	{
		res=sim900a_word_test();
		
		switch(res)
		{
			case 1:	//无通信失败
	      UART1_SendString("====res 1====\r\n");			 
				break;
			case 2:	//无SIM卡
	      UART1_SendString("====res 2====\r\n");	
				break;
			case 3:	//等待注册到网络
	      UART1_SendString("====res 3====\r\n");	
				break;
			case 4:	//开启GPS失败
	      UART1_SendString("====res 4====\r\n");
			  break;
			default:
	      UART1_SendString("====res default====\r\n");	
				break;				
		}
	}	 //系统检测完毕
/*
	res=1;
	while(res)
	{		
		res=SIM900A_CONNECT_SERVER((u8*)ip,(u8*)port);
	}
*/
	//delay_ms(1000);
  init_network();
	
	while(1)
	{
		//LED_OFF();
		//delay_ms(5000);
    //LED_ON();
		
		if((Flag_Send_data&0x8080))
		{
			UART1_SendLR();UART1_SendLR();UART1_SendLR();UART1_SendLR();
      UART1_SendString("===  begining====\r\n");
		
		  CLR_Buf2();
		  CLR_Message();
      CLR_HTTP();

			//curr_lock_flag  = GPIOA->IDR & GPIO_Pin_8;
			curr_lock_flag = GPIO_ReadOutputDataBit(LED_PORT, LED);

			sprintf((char*)message,"current-led-status: %d",curr_lock_flag);
			UART1_SendString((char*)message);UART1_SendLR();
		  CLR_Message();
			
			res=SIM808_GPS_HANDLE((u8 *)message);
			
			if (curr_lock_flag == lock_off_flag)
			{
				 UART1_SendString("===led=== lock ===\r\n");
				 type = 0;
			   lock_flag = 0;
			}
			else
			{
				UART1_SendString("===led=== unlock ===\r\n");
				//lock_flag = 1;
				n++;
				if (n==5)
				{
					LED_OFF();

					if (lock_flag == 1)
						type = 1;
				  lock_flag = 0;
					n = 0;
			  }
			 }
			
			/*
			if(Flag_Send_data&0x8000)	
				message[0]='0';
			//else if(Flag_Send_data&0x0800)	message[0]='1';
			else if(Flag_Send_data&0x0080)	
				message[0]='1';
			*/
			Flag_Send_data=0;
			START_TIME;
			
			// send http request
			add_more_info((u8 *)message, type, lock_flag);
			sprintf((char*)HTTP_Request,"AT+HTTPPARA=\"URL\",\"%s\"",(u8 *)message);
			
			UART1_SendString("HTTP_Request:");UART1_SendString((char *)HTTP_Request);UART1_SendLR();
			http_res = send_http_request((u8 *)HTTP_Request);
			
			switch (http_res)
			{
				case 0:
					UART1_SendString("===http-response: 0===\r\n");
					break;
				case 1:
					UART1_SendString("===http-response: 1===\r\n");
					LED_ON();
				  lock_flag = 1;
				  break;
				default:
					
					/*
          while(res)
					{
						res=SIM900A_CONNECT_SERVER((u8*)ip,(u8*)port);	//123.166.16.208	
						if(!res) 
							init_network();
					}
				*/
					break;
			}

		}
		
	}// end while
}



void CLR_Message(void)
{
	u16 k;
	for(k=0;k<Message_Max;k++)
	{
		message[k] = 0x00;
	}
}

void CLR_HTTP(void)
{
	u16 k;
	for(k=0;k<Message_Max;k++)
	{
		HTTP_Request[k] = 0x00;
	}
}
