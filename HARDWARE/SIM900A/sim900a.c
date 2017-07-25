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
//��
//////////////////////////////////////////////////////////////////////////////////	
u8 SIM900_CSQ[3];
u8 Flag_send_message=0;		//1--�������;�γ�ȣ�0--������
u8 dtbuf[50];   								//��ӡ������	
/////////////////////////////////////////////////////////////////////////////////
//usmart֧�ֲ��� 
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART2_RX_STA;
//     1,����USART2_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		printf("%s",USART2_RX_BUF);	//���͵�����
		if(mode)USART2_RX_STA=0;
	} 
}
//////////////////////////////////////////////////////////////////////////////
//ATK-SIM900A �������(���Ų��ԡ����Ų��ԡ�GPRS����)���ô���

//sim900a���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* sim900a_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//��sim��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	CLR_Buf2();
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel7->CNDTR!=0);	//�ȴ�ͨ��7�������   
		USART2->DR=(u32)cmd;
	}else u2_printf("%s\r\n",cmd);//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(sim900a_check_cmd(ack))break;//�õ���Ч���� 
				USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//��1���ַ�ת��Ϊ16��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��16������ֵ
u8 sim900a_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//��1��16��������ת��Ϊ�ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
u8 sim900a_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
u8 sim900a_word_test(void)
{
	//ͨ�����Ӳ���
	UART1_SendString("====word test AT 1====\r\n");
	if(sim900a_send_cmd((u8 *)"AT",(u8 *)"OK",100))
	{
		UART1_SendString("====word test AT 2====\r\n");
		if(sim900a_send_cmd((u8 *)"AT",(u8 *)"OK",100))
			return 1;	
	}	
	
	//û��SIM��
	UART1_SendString("====word test CPIN====\r\n");
	if(sim900a_send_cmd((u8 *)"AT+CPIN?",(u8 *)"READY",200))
		return 2;	
	
	//�ȴ����ŵ�����
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
	
	//�жϵ�Դ�Ƿ��
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

//SIM900A����Ӣ�Ķ���ģʽ����
//0--TESTӢ��1--����
u8 SIM900A_MESSAGE_MODE(u8 MODE)
{
	if(MODE)	//����
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
	return 0;	//�ɹ�
}

//���Ͷ���
//0--�ɹ�������-ʧ��
u8 SIM900A_SEND_MESSAGE(u8 *numerb,u8 *data)
{
	u8 CMGS[25];	
	sprintf((char*)CMGS,"AT+CMGS=\"%s\"",(u8*)numerb); 	
	if(sim900a_send_cmd((u8 *)CMGS,(u8*)">",300))	return 1;	
	if(sim900a_send_cmd((u8 *)data,(u8 *)"",0))	return 2;
	if(sim900a_send_cmd((u8 *)0x1A,(u8 *)"+CMGS:",1500))	return 3;
	return 0;
}

//SIM808 GPS���ݽ������쳣����
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
				p2=(char*)strtok(NULL,",");	//��ȡ�Ƿ�λ��ʾ
				
				//�������
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

				if(*p2=='1')		//��λ
				{
						sta=0;	//��ЧGPS����
				}
				else 	//δ��λ
				{
					if(sim900a_send_cmd((u8 *)"AT+CGNSPWR?",(u8 *)"+CGNSPWR: 1",100))//�жϵ�Դ�Ƿ��
					{
						sim900a_send_cmd((u8 *)"AT+CGNSPWR=1",(u8 *)"OK",100);
						sim900a_send_cmd((u8 *)"AT+CGNSSEQ=\"RMC\"",(u8 *)"OK",100);
					}
					sta=1;	//����ЧGPS����
				}					
			}
		}			
	}
	else 
	{
		sprintf((char*)message,"%s","http://35.166.81.188:9000/dov1?time=20170101010101&curlatitude=0&curlongitude=0&");	//��ʱ
		res=1;
		while(res)
		{
			res=sim900a_word_test();	//���¼��	
			//BEEP_MODE(1);
		}
		sta=2;	//��GPS����
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
