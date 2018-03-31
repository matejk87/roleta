/*
 * ds1820.c
 *
 *  Created on: Oct 22, 2016
 *      Author: Matej
 */


#include "one_wire.h"
#include "ds1820.h"
#include "timer.h"
#include "printf.h"

temperature_t temp_DS1820;

t_timer timer_temp_conversion,timer_delay;
int ds_state=0;
char temp1,temp2;
short temp3,temperatura = 0;
int busy=0,busy_old=0;
//unsigned long print_temp_time=0;

void ds1820_main(void)
{
	 /*busy = OW_busy_get();
	 if(busy!= busy_old)
	 {
		// printf("\n busy => %d",busy);
		// busy_old=busy;
	 }*/

	 if(timeout(&timer_temp_conversion,3000))
	 {
		 //printf("\n ds_reset");
		 OW_reset();
		 ds_state=1;  //todo odkomentiraj
	 }
	 if(ds_state)
	 {
		 switch(ds_state)
		 {
		 	case 1:
		 		if(!OW_busy_get())
		 		{
		 			//printf("\n->2");
		 			ds_state++;
		 			OW_write(0xCC);
		 		}
		 		 break;
		 	case 2:
		 		if(!OW_busy_get())
		 		{
		 			//printf("\n->3");
		 			ds_state++;
		 			OW_write(0x44);
		 			timeout(&timer_delay,0);
		 		}
		 		 break;
		 	case 3:
		 		if(timeout(&timer_delay,700))
		 		{
		 			//printf("\n->4");
		 			ds_state++;
		 			OW_reset();
		 		}
		  		 break;
		 	case 4:
		 		if(!OW_busy_get())
		 		{
		 			//printf("\n->5");
		 			ds_state++;
		 			OW_write(0xCC);
		 		}
		  		 break;
		    case 5:
		 		if(!OW_busy_get())
		 		{
		 			//printf("\n->6");
		 			ds_state++;
		 			OW_write(0xBE);
		 		}
		 		 break;
		 	case 6:
		 		if(!OW_busy_get())
		 		{
		 			//printf("\n->7");
		 			ds_state++;
		 			OW_read();
		 		}
		 		 break;
		 	case 7:
		 		if(!OW_busy_get())
		 		{
		 			//printf("\n->8");
		 			ds_state++;
		 			temp1=OW_get_result();
		 			OW_read();
		 		}
		  		 break;
		 	case 8:
		 		if(!OW_busy_get())
		 		{
		 			//printf("\n->0");

		 			temp2=OW_get_result();
		 			 temp3=0;
		 			 temp3|=(temp2<<8);
		 			 temp3|=(temp1);

		 			temperatura = temp3*50;   //Calculation for DS18S20 with 0.5 deg C resolution
		 			// printf("\n temp:%d ",temperatura);
		 			temp_DS1820.temperature=temperatura;
		 			ds_state=0;
		 		}
		  		 break;

		 }

	 }




}







