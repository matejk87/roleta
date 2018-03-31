/*
 * one_wire.c
 *
 *  Created on: Oct 22, 2016
 *      Author: Matej
 */
#include "stm32f0xx_conf.h"
#include "timer3.h"
#include "one_wire.h"
#include "pin_def.h"
#include "printf.h"

char OW_busy=0;
int n=0,data_cnt;
int data_w=0,data_r=0;
int cmd = OW_idle;

int OW_busy_get(void)
{
	return OW_busy;
}

void OW_reset()  // OK if just using a single permanently connected device
{
 if(OW_busy)return;
 OW_busy = 1;
 n=0;
 cmd = OW_rst;
 ONEW_CLR();
 timer3_set(500); //500
}

void OW_write(int data)
{
	 if(OW_busy)return;
	 OW_busy = 1;
	 n=0;
	 data_w =data;
	 data_cnt=0;
	 cmd = OW_w;
	 timer3_set(2);
}

void OW_read(void)
{
	 if(OW_busy)return;
	 OW_busy = 1;
	 n=0;
	 data_cnt=0;
	 data_r=0;
	 cmd = OW_r;
	 timer3_set(2);
}

int OW_get_result(void)
{
	return data_r;
}

void one_wire_main_int(void)
{
	switch(cmd){
		case OW_rst:
			switch(n){
				case 0:
					ONEW_SET();
					timer3_set(500);
					break;
				case 1:
					ONEW_SET();
					timer3_stop();
					OW_busy=0;
					break;
			}
			n++;
			break;
		case OW_w:
			if(n==0)
			{
				ONEW_CLR();
				timer3_set(2);//10
			}
			else if(n==1)
			{

			      if(data_w&(1<<data_cnt))ONEW_SET();
				  else ONEW_CLR();
				  data_cnt++;
				  timer3_set(60);
			}
			else if(n==2)
			{
				ONEW_SET(); // set 1-wire high again,
				timer3_set(2); //10
			}
			n++;
			if(n>=3)n=0;
			if(data_cnt>=8 && n==0)
			{
				timer3_stop();
				OW_busy=0;
			}
			break;
			/*
			  ONEW_CLR();
			  delay_us( 2 ); // pull 1-wire low to initiate write time-slot.
			  //output_bit(ONE_WIRE_PIN, shift_right(&data,1,0)); // set output bit on 1-wire
			  if(data&(1<<count))ONEW_SET();
			  else ONEW_CLR();
			  delay_us( 60 ); // wait until end of write slot.
			  ONEW_SET(); // set 1-wire high again,
			  delay_us( 2 ); // for more than 1us minimum.

			*/

		case OW_r:
			if(n==0)
			{
				ONEW_CLR();
				timer3_set(2);
			}
			else if(n==1)
			{
				ONEW_SET();
				timer3_set(8);
			}
			else if(n==2)
			{
				if(ONEW_READ())data_r|= (1<<data_cnt);
				data_cnt++;
				timer3_set(120);
			}
			n++;
			if(n>=3)n=0;
			if(data_cnt>=8 && n==0)
			{
				timer3_stop();
				OW_busy=0;
			}
			break;
		default:
		 break;
	}

}

int cmd_ds_debug(int argc, char **argp)
{
	printf("\n n=%d dcnt:%d",n,data_cnt);
	return 0;
}


