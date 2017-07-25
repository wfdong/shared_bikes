/**********************************************************************************
 * �ļ���  ��BEEP.c
 * ����    ��BEEP Ӧ�ú�����         
 * Ӳ�����ӣ�       
 *             PB7 -> BEEP     
 *               
 * ��汾  ��ST_v3.5
**********************************************************************************/
#include "BEEP.h"	   
#include "delay.h"	   
/*******************************************************************************
* ������  : GPIO_Config
* ����    : BEEP
* ����    : ��
* ���    : ��
* ����    : �� 
*******************************************************************************/
void BEEP_GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;				//����һ��GPIO_InitTypeDef���͵�GPIO��ʼ���ṹ��
	
	RCC_APB2PeriphClockCmd(BEEP_RCC, ENABLE);			//ʹ��GPIOB������ʱ��	
	
	GPIO_InitStructure.GPIO_Pin = BEEP;	//ѡ��Ҫ��ʼ����GPIOB����(PB5,PB6,PB7,PB8,PB9)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//�������Ź���ģʽΪͨ��������� 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
	GPIO_Init(BEEP_PORT, &GPIO_InitStructure);			//���ÿ⺯���е�GPIO��ʼ����������ʼ��GPIOB�е�PB5,PB6,PB7,PB8,PB9����			 
	BEEP_OFF();
}

//����BEEP.PB7
void BEEP_ON(void) 
{
	GPIO_SetBits(BEEP_PORT,BEEP);
}
//�ر�BEEP.PB7
void BEEP_OFF(void)
{
	GPIO_ResetBits(BEEP_PORT,BEEP);
}
void  BEEP_MODE(u8 mode)
{
	switch(mode)	
	{
		case 0:	//��ʼ���ɹ�
			BEEP_ON();
		  delay_ms(1000);
			BEEP_OFF();
		  break;
		case 1:	//�Լ�ⲻ��
			BEEP_ON();
		  delay_ms(200);
			BEEP_OFF();
		  break;
		case 2:	//������			
			BEEP_ON();
		  delay_ms(500);
			BEEP_OFF();
		  break;
		case 3:	//���ŷ���ʧ��
			BEEP_ON();
		  delay_ms(200);
			BEEP_OFF();
			delay_ms(200);
		default:
			break;
	}
}
