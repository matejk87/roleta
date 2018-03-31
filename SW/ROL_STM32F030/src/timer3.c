/*
 * timer3.c
 *
 *  Created on: Oct 22, 2016
 *      Author: Matej
 */

#include "timer3.h"
#include "roleta_rele.h"
#include "pin_def.h"
#include "printf.h"
#include "one_wire.h"
/*
         (#) Enable TIM clock using
             RCC_APBxPeriphClockCmd(RCC_APBxPeriph_TIMx, ENABLE) function.
         (#) Fill the TIM_TimeBaseInitStruct with the desired parameters.
         (#) Call TIM_TimeBaseInit(TIMx, &TIM_TimeBaseInitStruct) to configure
             the Time Base unit with the corresponding configuration.
         (#) Enable the NVIC if you need to generate the update interrupt.
         (#) Enable the corresponding interrupt using the function
             TIM_ITConfig(TIMx, TIM_IT_Update).
         (#) Call the TIM_Cmd(ENABLE) function to enable the TIM counter.
*/
TIM_TimeBaseInitTypeDef TIM_st;
TIM_OCInitTypeDef TIM_OCInitStructure;
void TIM3_IRQHandler(void)
{
  //if (TIM3->SR & TIM_IT_Update) {
	//LED0_CLR();
	//TIM3->SR = (uint16_t)~TIM_IT_Update;
	TIM3->SR = (uint16_t)~TIM_IT_CC3;
	timer3_stop();
	one_wire_main_int();
	//ONEW_SET();


}


void timer3_init(void)
{
	printf("\n timer 3 init:");

	  NVIC_InitTypeDef NVIC_InitStructure;

	  /* TIM3 clock enable */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	  /* Enable the TIM3 gloabal Interrupt */
	  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);

	  TIM_st.TIM_Prescaler = 47;//0xffff;
	  TIM_st.TIM_CounterMode = TIM_CounterMode_Up;
	  TIM_st.TIM_ClockDivision = TIM_CKD_DIV1;
	  TIM_st.TIM_Period = 10000;
	  TIM_TimeBaseInit(TIM3, &TIM_st);

	  TIM_PrescalerConfig(TIM3, 47, TIM_PSCReloadMode_Immediate);

	  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	  /* Output Compare Timing Mode configuration: Channel3 */
	  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	  TIM_OCInitStructure.TIM_Pulse = 100;

	  TIM_OC3Init(TIM3, &TIM_OCInitStructure);

	  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);

	  /* Output Compare Timing Mode configuration: Channel4 */
	/*  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	  TIM_OCInitStructure.TIM_Pulse = 100;

	  TIM_OC4Init(TIM3, &TIM_OCInitStructure);

	  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);*/


	  TIM_ITConfig(TIM3, TIM_IT_CC3 /*| TIM_IT_CC4*/, ENABLE);
	  /* TIM3 enable counter */

	  TIM_SetCounter(TIM3, 0);

	//  TIM_Cmd(TIM3, ENABLE);

/*	moja koda
  TIM_st.TIM_Prescaler = 47;//0xffff;
	TIM_st.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_st.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_st.TIM_Period = 1000;
	TIM_st.TIM_RepetitionCounter = 0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInit(TIM3, &TIM_st);
	//TIM_Cmd(TIM3,ENABLE);
	//ONEW_CLR(); //todo odstrani

	  NVIC_InitTypeDef NVIC_InitStructure;
	  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPriority = 3; //3 je najveèja prioriteta
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	  NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIM3, TIM_IT_Update,ENABLE);
*/
	printf("OK");
}


void timer3_stop(void)
{
	TIM_Cmd(TIM3,DISABLE);
}


void timer3_set(unsigned long time_us)
{
	 // ONEW_CLR();
	//TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	  TIM_OCInitStructure.TIM_Pulse = time_us;
	  TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	  TIM_SetCounter(TIM3, 0);
	  TIM_Cmd(TIM3, ENABLE);


	//todo krneki
/*	TIM_DeInit(TIM3);

	TIM_st.TIM_Period = time_us; //
	TIM_SetCounter(TIM3, 0);
	TIM_TimeBaseInit(TIM3, &TIM_st);
	//TIM_SetCounter(TIM3, 0);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//	TIM3->SR = (uint16_t)~TIM_IT_Update;
	TIM_Cmd(TIM3,ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update,ENABLE);
*/

//init praktièno




}
