#ifndef __LED_H__
#define __LED_H__
	 
#include "stm32f10x.h"

//LED????
#define LED_RCC     RCC_APB2Periph_GPIOA                            
#define LED_PORT	  GPIOA    
#define LED         GPIO_Pin_8    

void LED_GPIO_Config(void);
void LED_ON(void);
void LED_OFF(void);		 				    
#endif
