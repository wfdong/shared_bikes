#include "sim900a.h"
#include "usart.h"		
#include "delay.h"	 
#include "string.h" 
#include "key.h"
#include "usart2.h" 
#include "oled.h"
#include "math.h"
#include "stdio.h"

#define Device_Id "100010002"

//********************************************************************************
//无
//////////////////////////////////////////////////////////////////////////////////	
u8 SIM900_CSQ[3];
u8 Flag_send_message=0;		//1--启动发送经纬度；0--不发送
u8 dtbuf[50];   								//打印缓存器	
/////////////////////////////////////////////////////////////////////////////////
//usmart支持部分 
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART2_RX_STA;
//     1,清零USART2_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		printf("%s",USART2_RX_BUF);	//发送到串口
		if(mode)USART2_RX_STA=0;
	} 
}
//////////////////////////////////////////////////////////////////////////////
//ATK-SIM900A 各项测试(拨号测试、短信测试、GPRS测试)共用代码

//sim900a发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* sim900a_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向sim发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	CLR_Buf2();
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel7->CNDTR!=0);	//等待通道7传输完成   
		USART2->DR=(u32)cmd;
	}else u2_printf("%s\r\n",cmd);//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(sim900a_check_cmd(ack))break;//得到有效数据 
				USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//将1个字符转换为16进制数字
//chr:字符,0~9/A~F/a~F
//返回值:chr对应的16进制数值
u8 sim900a_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
u8 sim900a_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
u8 sim900a_word_test(void)
{
	//通信连接测试
	UART1_SendString("====word test AT 1====\r\n");
	if(sim900a_send_cmd((u8 *)"AT",(u8 *)"OK",100))
	{
		UART1_SendString("====word test AT 2====\r\n");
		if(sim900a_send_cmd((u8 *)"AT",(u8 *)"OK",100))
			return 1;	
	}	
	
	//没有SIM卡
	UART1_SendString("====word test CPIN====\r\n");
	if(sim900a_send_cmd((u8 *)"AT+CPIN?",(u8 *)"READY",200))
		return 2;	
	
	//等待附着到网络
	UART1_SendString("====word test CREG====\r\n");
	if(sim900a_send_cmd((u8 *)"AT+CREG?",(u8 *)"0,1",200))
	{
		 UART1_SendString("====word test CSQ====\r\n");
		 if(!sim900a_send_cmd((u8 *)"AT+CSQ",(u8 *)"OK",100))	
		 {
			 	UART1_SendString("====word test 6====\r\n");
				memcpy(SIM900_CSQ,USART2_RX_BUF+15,2);
		 }
		 return 3;	
	}
	
	//判断电源是否打开
	if(sim900a_send_cmd((u8 *)"AT+CGNSPWR?",(u8 *)"+CGNSPWR: 1",100))
	{
		sim900a_send_cmd((u8 *)"AT+CGNSPWR=1",(u8 *)"OK",100);
		sim900a_send_cmd((u8 *)"AT+CGNSSEQ=\"RMC\"",(u8 *)"OK",100);
		if(sim900a_send_cmd((u8 *)"AT+CGNSPWR?",(u8 *)"+CGNSPWR: 1",100))
			return 4;
	}
	
	return 0;
}


u8 SIM900A_CONNECT_SERVER(u8 *IP_ADD,u8 *COM)
{		
		if(sim900a_send_cmd((u8 *)"AT+CGATT?",(u8 *)": 1",100))	 return 1;
	  if(sim900a_send_cmd((u8 *)"AT+CIPSHUT",(u8 *)"OK",500))	return 2;
		if(sim900a_send_cmd((u8 *)"AT+CSTT",(u8 *)"OK",200))	return 3;
		if(sim900a_send_cmd((u8 *)"AT+CIICR",(u8 *)"OK",600))	return 4;
		if(!sim900a_send_cmd((u8 *)"AT+CIFSR",(u8 *)"ERROR",200))	return 5;		
		//sprintf((char*)dtbuf,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",IP_ADD,COM);
	  //if(sim900a_send_cmd((u8 *)dtbuf,(u8 *)"CONNECT OK",200))	return 6;		
	  return 0;
}	
u8 SIM900A_GPRS_SEND_DATA(u8 *DATA)
{		
	  if(sim900a_send_cmd((u8 *)"AT+CIPSEND",(u8 *)">",100))	 return 1;
	  if(sim900a_send_cmd(DATA,"",0))	return 2;
	  if(sim900a_send_cmd((u8 *)0x1a,(u8 *)"SEND OK",1500))	return 3;		
	  return 0;
}

//SIM900A发送英文短信模式配置
//0--TEST英文1--中文
u8 SIM900A_MESSAGE_MODE(u8 MODE)
{
	if(MODE)	//中文
	{
		if(sim900a_send_cmd((u8 *)"AT+CMGF=1",(u8 *)"OK",100))	return 1;
		if(sim900a_send_cmd((u8 *)"AT+CSCS=\"UCS2\"",(u8 *)"OK",100))	return 2;
		if(sim900a_send_cmd((u8 *)"AT+CSMP=17,167,0,25",(u8 *)"OK",100))	return 3;		
	}
	else
	{
		if(sim900a_send_cmd((u8 *)"AT+CMGF=1",(u8 *)"OK",100))	return 1;
		if(sim900a_send_cmd((u8 *)"AT+CSCS=\"GSM\"",(u8 *)"OK",100))	return 2;
		if(sim900a_send_cmd((u8 *)"AT+CSMP=17,167,0,241",(u8 *)"OK",100))	return 3;
	}
	if(sim900a_send_cmd((u8 *)"AT+CSCA?",(u8 *)"OK",200))	return 4;
	return 0;	//成功
}

//发送短信
//0--成功，其他-失败
u8 SIM900A_SEND_MESSAGE(u8 *numerb,u8 *data)
{
	u8 CMGS[25];	
	sprintf((char*)CMGS,"AT+CMGS=\"%s\"",(u8*)numerb); 	
	if(sim900a_send_cmd((u8 *)CMGS,(u8*)">",300))	return 1;	
	if(sim900a_send_cmd((u8 *)data,(u8 *)"",0))	return 2;
	if(sim900a_send_cmd((u8 *)0x1A,(u8 *)"+CMGS:",1500))	return 3;
	return 0;
}

//SIM808 GPS数据解析及异常处理
u8 SIM808_GPS_HANDLE(u8 * message)
{
	u8 res,sta;	
	char *p1,*p2;
	
	if(!sim900a_send_cmd((u8 *)"AT+CGNSINF",(u8 *)"OK",300))
	{
		UART1_SendString("===  cgnsinf ====\r\n");
		UART1_SendString((char *)USART2_RX_BUF);UART1_SendLR();
		
		if((p1=(char*)strstr((const char*)USART2_RX_BUF,"+CGNSINF: ")),(p1!=NULL))//
		{		
			if((p2=(char*)strstr((const char*)p1,"OK")),(p2!=NULL))//?????
			{
				*p2=0;//?????
				p2=strtok((p1),",");
				p2=(char*)strtok(NULL,",");	//获取是否定位标示
				
				//填充数据
				//message[0]='0';
				
				//1. UTC TIME
				p2=(char*)strtok(NULL,",");
				//res=strlen(p2);
				//sprintf(message,%s,p2);//memcpy(message,p2,strlen(p2));
				//sprintf((char*)message+1,",%s,%s",(u8*)Device_Id,(u8*)p2);
				sprintf((char*)message,"http://35.166.81.188:9000/dov1?time=%s&",(u8*)p2);

				//2. LAT
				p2=(char*)strtok(NULL,",");
				//memcpy(message+strlen(message),p2,strlen(p2));
				//sprintf((char*)message+strlen((char *)message),",%s",(u8*)p2);	
				sprintf((char*)message+strlen((char *)message),"curlatitude=%s&",(u8*)p2);
				
				//3. LONG
				p2=(char*)strtok(NULL,",");
				//memcpy(message+strlen(message),p2,strlen(p2));
				//sprintf((char*)message+strlen((char *)message),",%s,0001",(u8*)p2);
        sprintf((char*)message+strlen((char *)message),"curlongitude=%s&",(u8*)p2);

				if(*p2=='1')		//定位
				{
						sta=0;	//有效GPS数据
				}
				else 	//未定位
				{
					if(sim900a_send_cmd((u8 *)"AT+CGNSPWR?",(u8 *)"+CGNSPWR: 1",100))//判断电源是否打开
					{
						sim900a_send_cmd((u8 *)"AT+CGNSPWR=1",(u8 *)"OK",100);
						sim900a_send_cmd((u8 *)"AT+CGNSSEQ=\"RMC\"",(u8 *)"OK",100);
					}
					sta=1;	//无有效GPS数据
				}					
			}
		}			
	}
	else 
	{
		sprintf((char*)message,"%s","http://35.166.81.188:9000/dov1?time=20170101010101&curlatitude=0&curlongitude=0&");	//超时
		res=1;
		while(res)
		{
			res=sim900a_word_test();	//重新检测	
			//BEEP_MODE(1);
		}
		sta=2;	//无GPS数据
	}
	return sta;
}

void add_more_info(u8 * request, u8 type, u8 state)
{
	//Add device_id, type, and state
	//type : 0 -> update gps info; 
	//       1 -> lock and update
	//state: 0 -> open; 
	//       1 -> closed
	sprintf((char*)request+strlen((char *)request),"id=%s&type=%d&state=%d",(u8*)Device_Id, type, state);
}


u8 init_network(void)
{
	UART1_SendString("===init_network CGATT======\r\n");
	sim900a_send_cmd((u8 *)"AT+CGATT?",(u8 *)": 1",200);
	
	UART1_SendString("===init_network CONTYPE======\r\n");
	sim900a_send_cmd((u8 *)"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",(u8 *)"OK",200);
	
	UART1_SendString("===init_network APN======\r\n");
	sim900a_send_cmd((u8 *)"AT+SAPBR=3,1,\"APN\",\"CMNET\"",(u8 *)"OK",200);

	UART1_SendString("===init_network 1,1======\r\n");
	if(sim900a_send_cmd((u8 *)"AT+SAPBR=1,1",(u8 *)"OK",200))
	{
		UART1_SendString("===init_network error======\r\n");
		sim900a_send_cmd((u8 *)"AT+SAPBR=0,1",(u8 *)"OK",200);
		sim900a_send_cmd((u8 *)"AT+SAPBR=1,1",(u8 *)"OK",200);
	}
	
	UART1_SendString("===init_network done======\r\n");
	return 0;
}

// 0 - success; 
// 1 - open;
// 2 - set para error
// 3 - action error
// 4 - read error
// 5 - content error
u8 send_http_request(u8 * Request)
{
/*
1 AT+CGATT?
2 AT+SAPBR=3,1,"CONTYPE","GPRS" 
3 AT+SAPBR=3,1,"APN","CMNET"
4 AT+SAPBR=1,1  
5 AT+HTTPINIT
6 AT+HTTPPARA="URL","www.baidu.com/img/baidu_logo.gif"
7 AT+HTTPACTION=0
8 AT+HTTPREAD
9 AT+HTTPTERM
*/
  char *p1;
	char y='0';
	char x[5];
	u8 res = 0;

	UART1_SendString("===HTTP-INIT==============\r\n");
	if(sim900a_send_cmd((u8 *)"AT+HTTPINIT",(u8 *)"OK",100))
	{
		UART1_SendString("===HTTPINIT-error==============\r\n");
		//print_buf();
		sim900a_send_cmd((u8 *)"AT+HTTPTERM",(u8 *)"OK",200);
		sim900a_send_cmd((u8 *)"AT+HTTPINIT",(u8 *)"OK",200);
	}
	
	UART1_SendString("===HTTP-CID==============\r\n");
	//print_buf();
	sim900a_send_cmd((u8 *)"AT+HTTPPARA=\"CID\",1",(u8 *)"OK",100);

  UART1_SendString("===HTTP-PARA===========\r\n");
	if(!sim900a_send_cmd((u8 *)Request,(u8 *)"OK", 600))
	//if(!sim900a_send_cmd((u8 *)"AT+HTTPPARA=\"URL\",\"http://35.166.81.188:9000/dov1?type=0&id=123&state=0&curlatitude=31.2292900000&curlongitude=121.3659000000&time=20170610010203\"",(u8 *)"OK",100))
	{
	  //print_buf();
			
		UART1_SendString("===HTTP-ACTION==================\r\n");
		if(!sim900a_send_cmd((u8 *)"AT+HTTPACTION=0",(u8 *)"+HTTPACTION: 0", 1000))
		{
			//delay_ms(1000);
			//print_buf();
			UART1_SendString("===HTTP-READ==================\r\n");
			if(!sim900a_send_cmd((u8 *)"AT+HTTPREAD=0,1",(u8 *)"+HTTPREAD: 1", 500))
			{
				UART1_SendString("===HTTP-READ-content-success==================\r\n");
        //delay_ms(1000);
        //print_buf();
					
				if((p1=(char*)strstr((const char*)USART2_RX_BUF,"+HTTPREAD: 1")),(p1!=NULL))//
        {
					y = *(p1+14);// the 14th char is the return value
					
					if (y == '1')
					{
						res = 1;
					}
					else
					{
						res = 0;
					}
												
					//x[0]=*(p1+11);//
				  //UART1_SendString("===HTTP-READ_RES: get/post (11th)===============\r\n");
					//UART1_SendString((char*)x);UART1_SendLR();
						
					x[0]=*(p1+14);//
					UART1_SendString("===HTTP-READ_RES: value (14th)===============\r\n");
					UART1_SendString((char*)x);UART1_SendLR();
				}
				else
				{
          UART1_SendString("===HTTP-READ-content-failed==================\r\n");					
					res = 5;
				}
			}
			else
			{
        UART1_SendString("===HTTP-READ-failed==================\r\n");
				res = 4;
			}
		}
		else
		{
      UART1_SendString("===HTTP-ACTION-failed==================\r\n");
			res = 3;
		}
	}
	else
	{
		UART1_SendString("===HTTP-PARA-failed===========\r\n");
		res = 2;
	}
	UART1_SendString("===Current Buf Message===========\r\n");
	print_buf();
	UART1_SendString("===HTTP-done (TERM)===========\r\n");
	sim900a_send_cmd((u8 *)"AT+HTTPTERM",(u8 *)"OK",500);
	return res;
}

void print_buf(void)
{
	UART1_SendString((char *)USART2_RX_BUF);UART1_SendLR();
}

void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<USART2_MAX_RECV_LEN;k++)
	{
		USART2_RX_BUF[k] = 0x00;
	}
}
