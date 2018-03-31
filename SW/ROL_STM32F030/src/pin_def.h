/*
 * pin_def.h
 *
 *  Created on: 30. dec. 2015
 *      Author: Matej
 */

#ifndef INC_PIN_DEF_H_
#define INC_PIN_DEF_H_
#include "stm32f0xx.h"


#define N_OF_PINP 4 //todo
#define PINP_PORT GPIOB
#define PINP0 GPIO_Pin_3
#define PINP1 GPIO_Pin_4
#define PINP2 GPIO_Pin_5
#define PINP3 GPIO_Pin_6
#define PINP4 GPIO_Pin_7
#define PINP5 GPIO_Pin_8


#define N_OF_POUT N_OF_PINP
#define POUT_PORT GPIOA

#define POUT_RELAY GPIO_Pin_4
#define POUT_TRIAC0 GPIO_Pin_5
#define POUT_TRIAC1 GPIO_Pin_7

#define RELAY_CLR() ((POUT_PORT->ODR)&=~(POUT_RELAY))
#define RELAY_SET() ((POUT_PORT->ODR)|=(POUT_RELAY))
#define RELAY_TOGGLE() ((POUT_PORT->ODR)^=(POUT_RELAY))
#define RELAY_READ() (((POUT_PORT->IDR)&POUT_RELAY)?1:0)


//#define POUT0_TOGGLE() ((POUT_PORT->ODR)^=(POUT0))

#define LED0 GPIO_Pin_1
#define LED0_PORT GPIOB
#define LED0_CLR() ((LED0_PORT->ODR)&=~(LED0))
#define LED0_SET() ((LED0_PORT->ODR)|=(LED0))
#define LED0_TOGGLE() ((LED0_PORT->ODR)^=(LED0))

#define LED1 GPIO_Pin_2
#define LED1_PORT GPIOB
#define LED1_CLR() ((LED1_PORT->ODR)&=~(LED1))
#define LED1_SET() ((LED1_PORT->ODR)|=(LED1))
#define LED1_TOGGLE() ((LED1_PORT->ODR)^=(LED1))
#define LED1_READ() (((LED1_PORT->IDR)&LED1)?1:0)

#define ONEW GPIO_Pin_9
#define ONEW_PORT GPIOB
#define ONEW_CLR() ((ONEW_PORT->ODR)&=~(ONEW))
#define ONEW_SET() ((ONEW_PORT->ODR)|=(ONEW))
#define ONEW_TOGGLE() ((ONEW_PORT->ODR)^=(ONEW))
#define ONEW_READ() (((ONEW_PORT->IDR)&ONEW)?1:0)



#endif /* INC_PIN_DEF_H_ */
