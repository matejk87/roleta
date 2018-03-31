/*
 * se95.c
 *
 *  Created on: Oct 25, 2016
 *      Author: Matej
 */

#include "timer.h"
#include "se95.h"
#include "i2c.h"
#include "printf.h"


temperature_t temp_SE95;

int temp=0;
t_timer temp_timer;
char data[]={0xaa,0xaa};
short sensor_addr=0x48;

void se95_main(void)
{

	 if(timeout(&temp_timer,1000))
	 {
		// printf("\n temp conversion");
		// I2C_Write(sensor_addr, 0, 2, data);
		 temp = data[0]<<8;
		 temp |= data[1];
		 /*temp = temp>>7;
		 temp = temp *50;*/
		 temp = temp>>3;
		 temp = temp *3125;
		 temp = temp/1000;
		 temp_SE95.temperature = temp;
		 //printf("\n data %x %x  temp:%d",data[0],data[1],temp);
		 if(i2c_last_transaction_valid())temp_SE95.temperature_valid =1;
		 else temp_SE95.temperature_valid =0;
		 I2C_Read(0x48, 0, 2, data);
	 }

}

