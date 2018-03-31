/*
 * i2c.h
 *
 *  Created on: Jul 24, 2016
 *      Author: Matej
 */

#ifndef I2C_H_
#define I2C_H_

#include "timer.h"

typedef enum
{
	i2c_read,
	i2c_write
}i2c_read_write_t;


typedef enum
{
	i2c_addr,
	i2c_reg,
	i2c_auto_wait,
	i2c_data,
	i2c_stop,
	i2c_successful,
	i2c_fail,
}i2c_state_t;


typedef struct
{
	short read_write;
	short state;
	short reg;
	short n_bytes;
	short cnt;
	short address;
	char busy;
	char  *data_ptr;
	t_timer timer;
}i2c_t;

char i2c_last_transaction_valid(void);
void I2C2_init(void);
unsigned char I2C2_Read_Wait(unsigned char address, unsigned char reg);
char I2C_Write(unsigned char address, unsigned char reg, short bytes_n, char *data_ptr);
char I2C_Read(unsigned char address, unsigned char reg, short bytes_n, char *data_ptr);
unsigned char I2C_Read_Wait(unsigned char address, unsigned char reg);
void i2c_main(void);


#endif /* I2C_H_ */
