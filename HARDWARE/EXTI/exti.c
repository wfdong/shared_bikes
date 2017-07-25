#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "stm32f10x_exti.h"

u32 Flag_Send_data=0;

//�ⲿ�ж�0�������
void EXTIX_Init(void)
{
 		EXTI_InitTypeDef EXTI_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		GPIO_InitTypeDef GPIO_InitStructure;

	  //RCC_APB2PeriphCFGClockCmd(RCC_APB2Periph_GPIOC,ENABLE); //���������˿�PB��ʱ��
		//PA0
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //�˿�����Ϊ��������
		GPIO_Init(GPIOA, &GPIO_InitStructure);	//��ʼ���˿�
		//Pc13
//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
//		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //�˿�����Ϊ��������
//		GPIO_Init(GPIOC, &GPIO_InitStructure);	//��ʼ���˿�
		
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); //���������˿�Pc��ʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��
 	 //GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13); 
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0); 
  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
	  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	

  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 
	
//	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); //���������˿�Pc��ʱ��
//		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��
// 	 //GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13); 
//    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13); 
//  	EXTI_InitStructure.EXTI_Line=EXTI_Line13;
//	  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	

//  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

//  	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
//  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
//  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
//  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
//  	NVIC_Init(&NVIC_InitStructure); 
		
//		EXTI_InitStructure.EXTI_Line=EXTI_Line1;
//	  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���


//  	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
//  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
//  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
//  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
//  	NVIC_Init(&NVIC_InitStructure); 
//		

}
////�ⲿ�ж�0������� 
//void EXTI15_10_IRQHandler(void)
//{
////		OLED_Clear();
////		OLED_ShowString(0,0, "WELCOME",24); 
////	  OLED_ShowString(0,24, "USE",24); 
////		OLED_Refresh_Gram();	
//		Flag_Send_data=1<<11;
//		EXTI_ClearITPendingBit(EXTI_Line13); //���LINE0�ϵ��жϱ�־λ  
//}

 //�ⲿ�ж�0������� 
void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0)!=RESET)
	{
		 Flag_Send_data=1<<7;
	   EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ 
	}
	 
}
