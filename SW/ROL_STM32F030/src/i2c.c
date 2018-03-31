/*
 * i2c.c
 *
 *  Created on: Jul 16, 2016
 *      Author: Matej
 */
#include "stm32f0xx.h"
#include "stm32f0xx_i2c.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "printf.h"
#include "i2c.h"
#include "pin_def.h"


/* interrupts
I2C_IT_ERRI I2C_IT_TCI I2C_IT_STOPI I2C_IT_NACKI  I2C_IT_ADDRI I2C_IT_RXI I2C_IT_TXI
*/

i2c_t i2c ;

/*
i2c_reg,
i2c_data,
i2c_stop,*/
/* neki dela
void I2C2_IRQHandler(void)
{
	int i2c_isr = I2C2->ISR;
	//printf("\nisr:0x%x",i2c_isr);
	if(i2c_isr & I2C_ISR_NACKF)
	{
		printf("\n i2c nack!");
	}
	if(i2c.read_write == i2c_write)
	{
		switch(i2c.state)
		{
		case i2c_addr:
			if(i2c_isr & I2C_ISR_TXIS)
			{

				I2C_SendData(I2C2, i2c.reg);
				i2c.state = i2c_reg;
			}
			break;
		case i2c_reg:
			if(i2c_isr & I2C_ISR_TCR)
			{

				I2C_AutoEndCmd(I2C2, ENABLE);
				i2c.state = i2c_auto_wait;
			}
			break;
		case i2c_auto_wait:
			if(i2c_isr & I2C_ISR_TXIS)
			{
				I2C_SendData(I2C2, i2c.data_ptr[0]);
				I2C_ReloadCmd(I2C2, DISABLE);
				i2c.state = i2c_stop;
			}
			break;
		case i2c_data:

			break;
		case i2c_stop:
			if(i2c_isr & I2C_ISR_STOPF)
			{
				I2C2->ICR = I2C_ICR_STOPCF;

			}
			break;
		default:
			break;
		}

	}
	else if (i2c.read_write == i2c_read)
	{


	}

	//I2C2->ISR=0;
}*/



void I2C2_IRQHandler(void)
{
	int i2c_isr = I2C2->ISR;
	//printf("\nisr:0x%x",i2c_isr);
	if(i2c_isr & I2C_ISR_NACKF)
	{
		printf("\n i2c nack addr:0x%x",i2c.address);
		i2c.busy=0;

		I2C2->ICR =0x2fff;
		I2C_ITConfig(I2C2,I2C_IT_TXI | I2C_IT_TCI | I2C_IT_NACKI |I2C_IT_STOPI | I2C_IT_RXI , DISABLE);
		LED1_SET();
		i2c.state= i2c_fail;
	}
	if(i2c.read_write == i2c_write)
	{
		switch(i2c.state)
		{
		case i2c_addr:
			if(i2c_isr & I2C_ISR_TXIS)
			{

				I2C_SendData(I2C2, i2c.reg);
				i2c.state = i2c_reg;
			}
			break;
		case i2c_reg:
			if(i2c_isr & I2C_ISR_TCR)
			{

				I2C_AutoEndCmd(I2C2, ENABLE);
				i2c.state = i2c_auto_wait;
			}
			break;
		case i2c_auto_wait:
			if(i2c_isr & I2C_ISR_TXIS)
			{
				I2C_SendData(I2C2, i2c.data_ptr[i2c.cnt]);
				i2c.cnt++;
				if(i2c.n_bytes==1)
				{
					i2c.state = i2c_stop;
					I2C_ReloadCmd(I2C2, DISABLE);
				}
				else i2c.state = i2c_data;
			}
			break;
		case i2c_data:
			if((i2c_isr & I2C_ISR_TXIS) || ( i2c_isr & I2C_ISR_TCR))
			{

				I2C_SendData(I2C2, i2c.data_ptr[i2c.cnt]);
				i2c.cnt++;
				if(i2c.n_bytes <= i2c.cnt)
				{

					I2C_ReloadCmd(I2C2, DISABLE);
					i2c.state = i2c_stop;
				}
			}
			break;
		case i2c_stop:
			if(i2c_isr & I2C_ISR_STOPF)
			{
				//LED1_SET();
				I2C2->ICR =/* 0x2fff;*/ I2C_ICR_STOPCF;
				i2c.busy = 0;
				I2C_ITConfig(I2C2,I2C_IT_TXI | I2C_IT_TCI | I2C_IT_NACKI |I2C_IT_STOPI , DISABLE); //todo: je tole ok??
				i2c.state= i2c_successful;
			}
			break;
		default:
			break;
		}
	}
	else if (i2c.read_write == i2c_read)
	{
		switch(i2c.state)
		{
		case i2c_addr:
			if(i2c_isr & I2C_ISR_TXIS)
			{

				I2C_SendData(I2C2, i2c.reg);
				i2c.state = i2c_reg;
			}
			break;
		case i2c_reg:
			if(i2c_isr & I2C_ISR_TC)
			{

				I2C_TransferHandling(I2C2, i2c.address<<1, i2c.n_bytes, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
				i2c.state = i2c_data;
				//if(i2c.n_bytes <=1 )i2c.state = i2c_stop;
			}
			break;
		case i2c_data:
			if((i2c_isr & I2C_ISR_RXNE))
			{
				i2c.data_ptr[i2c.cnt++] = I2C_ReceiveData(I2C2);
				if(i2c.cnt >= i2c.n_bytes)
				{
					i2c.state = i2c_stop;
				}

			}
			break;
		case i2c_stop:
			if(i2c_isr & I2C_ISR_STOPF)
			{
				I2C2->ICR =/* 0x2fff;*/ I2C_ICR_STOPCF;
				I2C_ITConfig(I2C2,I2C_IT_TXI | I2C_IT_TCI | I2C_IT_NACKI |I2C_IT_STOPI | I2C_IT_RXI , DISABLE); //todo: je tole ok??
				i2c.busy = 0;
				i2c.state= i2c_successful;
			}
			break;
		default:
			break;
		}

	}

	//I2C2->ISR=0;
}

static void NVIC_Config_i2c2(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = I2C2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
 /* NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1 ; //todo on m3 and m4 devices
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;*/
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
};


void I2C2_init(void){
	printf("\n i2c2 init");
	I2C_DeInit(I2C2);
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    /* Configure the I2C clock source. The clock is derived from the HSI */
  //  RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

    //Configure pins: SCL and SDA ------------------
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_1);


    I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    I2C_InitStructure.I2C_DigitalFilter = 0x00;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00; // MPU6050 7-bit adress = 0x68, 8-bit adress = 0xD0;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
   // I2C_InitStructure.I2C_Timing = 0xA0120227;
    I2C_InitStructure.I2C_Timing = 0x10805E89; //100kHz
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    I2C_Init(I2C2, &I2C_InitStructure);
  // possible interrupts I2C_IT_ERRI I2C_IT_TCI I2C_IT_STOPI I2C_IT_NACKI  I2C_IT_ADDRI I2C_IT_RXI I2C_IT_TXI

    //I2C_ITConfig(I2C2,I2C_IT_TCI |I2C_IT_STOPI|I2C_IT_NACKI | I2C_IT_RXI | I2C_IT_TXI, ENABLE); //todo odkomentiraj

    NVIC_Config_i2c2();

    I2C_Cmd(I2C2, ENABLE);
    i2c.busy = 0;

}


char I2C_Read(unsigned char address, unsigned char reg, short bytes_n, char *data_ptr)
{
	if(i2c.busy)
	{
		printf("\n i2c busy");
		return 0;
	}
	timeout(&i2c.timer,0);
	i2c.busy = 1;
	i2c.address = address;
	i2c.reg = reg;
	i2c.n_bytes = bytes_n;
	i2c.data_ptr = data_ptr;
	i2c.read_write = i2c_read;
	i2c.state =i2c_addr;
	i2c.cnt=0;
	I2C_TransferHandling(I2C2, address<<1,1 , I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
	I2C_ITConfig(I2C2,I2C_IT_TXI | I2C_IT_TCI | I2C_IT_NACKI |I2C_IT_STOPI | I2C_IT_RXI , ENABLE);
	return 1;
}

char I2C_Write(unsigned char address, unsigned char reg, short bytes_n, char *data_ptr)
{
	if(i2c.busy)
	{
		printf("\n i2c busy");
		return 0;
	}
	timeout(&i2c.timer,0);
	i2c.busy = 1;
	i2c.address = address;
	i2c.reg = reg;
	i2c.n_bytes = bytes_n;
	i2c.data_ptr = data_ptr;
	i2c.read_write = i2c_write;
	i2c.state = i2c_addr;
	i2c.cnt=0;
	I2C_TransferHandling(I2C2,i2c.address<<1, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);
	I2C_ITConfig(I2C2,I2C_IT_TXI | I2C_IT_TCI | I2C_IT_NACKI |I2C_IT_STOPI , ENABLE);
	return 1;
}



void i2c_main(void)
{
	if(i2c.busy)
	{
		if(timeout(&i2c.timer,50))
		{
			printf("\n i2c timeout: reinit!");
			I2C2_init();
		}
	}
}
char i2c_last_transaction_valid(void)
{
	if(i2c.state == i2c_successful) return 1;
	return 0;
}


unsigned char I2C_Read_Wait(unsigned char address, unsigned char reg){

	  uint8_t tmp = 0xFF;

	//I2C1->CR2 = (uint32_t)(LIS302DL_ADDR) | (uint32_t)(1 << 16) | I2C_CR2_START ;
	  I2C_TransferHandling(I2C2, address<<1, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
	  while(!(I2C2->ISR & I2C_ISR_TXIS)){;}

	//I2C1->TXDR = (uint8_t)(addr);
	  I2C_SendData(I2C2, reg);
	  while(!(I2C2->ISR & I2C_ISR_TC)){;}

	//I2C1->CR2 = (uint32_t)(LIS302DL_ADDR) | (uint32_t)(1 << 16) | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START;
	  I2C_TransferHandling(I2C2, address<<1, 1, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
	  while(!(I2C2->ISR & I2C_ISR_RXNE)){;}

	//tmp = (uint8_t)I2C1->RXDR;
	  tmp = I2C_ReceiveData(I2C2);

      while(!(I2C2->ISR & I2C_ISR_STOPF)){;}  //todo odkomentiraj
	  I2C2->ICR = I2C_ICR_STOPCF;

	  return(tmp);
}

int I2C_Write_Wait(unsigned char address, unsigned char reg,unsigned char data){

	//I2C1->CR2 |= (uint32_t)(LIS302DL_ADDR) | (uint32_t)(1 << 16) | I2C_CR2_RELOAD | I2C_CR2_START ;
	  I2C_TransferHandling(I2C2, address<<1, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);
	  while(!(I2C2->ISR & I2C_ISR_TXIS)){;}

	  I2C_SendData(I2C2, reg);
	  while(!(I2C2->ISR & I2C_ISR_TCR)){;}

	  I2C_AutoEndCmd(I2C2, ENABLE);
	  while(!(I2C2->ISR & I2C_ISR_TXIS)){;}

	  I2C_SendData(I2C2, data);

	  I2C_ReloadCmd(I2C2, DISABLE);

	  while(!(I2C2->ISR & I2C_ISR_STOPF)){;}
	  I2C2->ICR = I2C_ICR_STOPCF;

}


int cmd_i2c_write(int argc, char **argv) // se95 1001000
{
	unsigned char addr=0;
	unsigned char reg=0,data=0;
	if(argc>3)
	{
		addr = strtoul(argv[1],0,0);
		reg = strtoul(argv[2],0,0);
		data = strtoul(argv[3],0,0);
		I2C_Write_Wait(addr,reg,data);
		printf("\ni2c write addr:0x%x reg:0x%x data:0x%x",addr,reg,data);
	}
	else
	{
		printf("\n argc error");
	}
	return 1;
}


int cmd_i2c_read(int argc, char **argv) // se95 1001000
{
	unsigned char addr=0;
	unsigned char reg=0,data=0;
	if(argc>2)
	{
		addr = strtoul(argv[1],0,0);
		reg = strtoul(argv[2],0,0);
		data=I2C_Read_Wait(addr,reg);
		printf("\ni2c read addr:0x%x reg:0x%x reg_value:0x%x",addr,reg,data);
	}
	else
	{
		printf("\n argc error");
	}
	return 1;
}


