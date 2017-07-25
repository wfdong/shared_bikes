/**********************************************************************************
 * ???  :led.c
 * ??    :led ?????         
 * ????:NiRen_TwoHeart???
 * ????:  PB5 -> LED1     
 *             PB6 -> LED2     
 *             PB7 -> LED3    
 *             PB8 -> LED3    
 * ???  :ST_v3.5
**********************************************************************************/

#include "led.h"
	   
/*******************************************************************************
* 函数名  : LED_GPIO_Config
* 描述    : LED IO
* 输入    : 
* 输出    : 
* 返回    : 
* 说明    : LED(1~4): GPIOA -> PA8
*******************************************************************************/
void LED_GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = LED;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	LED_OFF();
}

/*??LED.PA8*/
void LED_ON(void) 
{
  GPIO_ResetBits(LED_PORT,LED);
}

/*??LED.PA8*/
void LED_OFF(void)
{
  GPIO_SetBits(LED_PORT,LED);
}

