/*
 * one_wire.h
 *
 *  Created on: Oct 22, 2016
 *      Author: Matej
 */

#ifndef ONE_WIRE_H_
#define ONE_WIRE_H_

typedef enum{
	OW_idle,
	OW_r,
	OW_w,
	OW_rst
}OW_command_t;


void one_wire_main_int(void);
void OW_read(void);
void OW_write(int data);
void OW_reset();
int OW_busy_get(void);
int OW_get_result(void);

#endif /* ONE_WIRE_H_ */
