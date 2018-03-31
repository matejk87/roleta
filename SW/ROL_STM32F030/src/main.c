
/* Includes */
#include <stddef.h>
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_conf.h"
#include "pin_def.h"
#include "bootloader.h"
#include "timer.h"
#include "config.h"
#include "printf.h"
#include "usart1.h"
#include "usart2.h"
#include "getline.h"

#include "roleta_rele.h"
//#include "ext_int.h"  // todo odstrani

#include "mybus.h"
#include "i2c.h"
#include "timer3.h"
#include "ds1820.h"
#include "se95.h"




uint32_t timer=0;
uint8_t  timerFlag=0;
unsigned long print_temp_time=0;

char version_str[]="<version_data>" VER_OPIS "</version_data>";
char test_str[]="bb b";

t_timer timer_led,timer_temp_print;

void gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	RCC->AHBENR |= RCC_AHBENR_GPIOCEN; 	// enable the clock to GPIOC			//(RM0091 lists this as IOPCEN, not GPIOCEN)
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; 	// enable the clock to GPIOC			//(RM0091 lists this as IOPCEN, not GPIOCEN)
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; 	// enable the clock to GPIOC			//(RM0091 lists this as IOPCEN, not GPIOCEN)

	//GPIOC->MODER = (1 << 16)|(1 << 0)|(1 << 2);


	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = POUT_RELAY;
	GPIO_Init(POUT_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = POUT_TRIAC0;
	GPIO_Init(POUT_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = POUT_TRIAC1;
	GPIO_Init(POUT_PORT, &GPIO_InitStructure);


//leds
	GPIO_InitStructure.GPIO_Pin = LED0;
	GPIO_Init(LED0_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = LED1;
	GPIO_Init(LED1_PORT, &GPIO_InitStructure);

// one wire open drain
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin = ONEW;
	GPIO_Init(ONEW_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

//inputs
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = PINP0;
	GPIO_Init(PINP_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PINP1;
	GPIO_Init(PINP_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PINP2;
	GPIO_Init(PINP_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PINP3;
	GPIO_Init(PINP_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PINP4;
	GPIO_Init(PINP_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PINP5;
	GPIO_Init(PINP_PORT, &GPIO_InitStructure);
}


/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
  uint32_t ii = 0,tmp;

  copy_vector_table_sram();
  /* TODO - Add your application code here */
 // SysTick_Config(4800);  /* 0.1 ms = 100us if clock frequency 12 MHz */

 // SystemCoreClockUpdate();
 // ii = SystemCoreClock;
 // ii = 0;


  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	gpio_init();
	/*while(1)
	{
	LED0_TOGGLE();
	}*/

	SysTick_Config(SystemCoreClock/1000);
	SystemCoreClockUpdate();
USART_InitTypeDef USART_InitStructure;

RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);
//Configure USART2 pins:  Rx and Tx ----------------------------
GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_3;  //todo odstrani inicializacijo uart 2
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
GPIO_Init(GPIOA, &GPIO_InitStructure);
//Configure USART2 setting:           ----------------------------
USART_InitStructure.USART_BaudRate = 115200;
USART_InitStructure.USART_WordLength = USART_WordLength_8b;
USART_InitStructure.USART_StopBits = USART_StopBits_1;
USART_InitStructure.USART_Parity = USART_Parity_No;
USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
USART_Init(USART2, &USART_InitStructure);
USART_Cmd(USART2,ENABLE);



timer_init(1000);
timeout(&timer_led,0);

usart2_init();
printf("\n\n\n");
printf(version_str);

roleta_init(ROL_N);
mybus_init();
timer3_init();

I2C2_init(); // todo i2c

  while(1)
  {

	  if(timeout(&timer_led,200))
	  {
	  	LED0_TOGGLE();
	  	//RELAY0_TOGGLE();
	  //	POUT0_TOGGLE();

	  }
	  if(print_temp_time)
	  {
		 if(timeout(&timer_temp_print,print_temp_time))
		 {
			 printf("\n temp_DS1820:%d temp_SE95:%d",temp_DS1820.temperature,temp_SE95.temperature);
		 }
	  }
	  roleta_main();
	  getline();
	  mybus_main();
	  se95_main();
	  i2c_main();
  }
}
/*

void debug(int argc,char ** argp)
{
	printf("\nGPIOC->PUPDR = 0x%x",GPIOC->PUPDR);
}

int cmd_reset(int argc,char ** argv)
{
	resetcpu();
	return 1;
}

int cmd_print_temp(int argc, char **argv)
{
	if(argc>1)
	{
		print_temp_time = strtoul(argv[1],0,0);
		printf("\nprint temperature time:%dms",print_temp_time);
	}
	else
	{
		//printf("\n argc error");
		print_temp_time=0;
		 printf("\n temp_DS1820:%d temp_SE95:%d",temp_DS1820.temperature,temp_SE95.temperature);
	}
	return 1;
}*/



int cmd_print_temp(int argc, char **argv)
{
	if(argc>1)
	{
		print_temp_time = strtoul(argv[1],0,0);
		printf("\nprint temperature time:%dms",print_temp_time);
	}
	else
	{
		//printf("\n argc error");
		print_temp_time=0;
		 printf("\n temp_DS1820:%d temp_SE95:%d",temp_DS1820.temperature,temp_SE95.temperature);
	}
	return 1;
}
