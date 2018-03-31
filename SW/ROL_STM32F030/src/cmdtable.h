
#ifndef CMDTABLE_H_
#define CMDTABLE_H_

#include "cmdproc.h"



/* declaration of cmd functions*/

int help(int argc,char **argv);
int bla(int argc,char **argv);
void debug(int argc,char **argp);
void cmd_set_triac(int argc,char **argp);
void cmd_rol_print(int argc,char **argp);
int cmd_stats(int argc, char **argv);
int cmd_set_rs485_addr(int argc, char **argv);
int cmd_enter_bl(int argc, char **argv);
int cmd_i2c_read(int argc, char **argv);
//int cmd_setrol(int argc, char **argv);
int cmd_reset(int argc,char ** argv);
//int cmd_rol_auto(int argc, char **argp);
int cmd_ds_debug(int argc, char **argp);
int cmd_i2c_write(int argc, char **argv);
int cmd_set_rol(int argc,char ** argp);
int cmd_print_temp(int argc, char **argv);
//int cmd_setrol_fulltime(int argc, char **argp);
//int cmd_owerwriterolpos(int argc, char **argp);



//int get_inb1(int argc, char ** argp);
/* end of declaration of cmd functions*/
/* CMD table */

// definicija ukazov

const t_cmd_str cmd_table[] =
    {
 /*   "cmd string" , cmd_function,"help string"*/
		{ "help",help,"<-l or cmd> print help"},  // cmdproc.c embedded function
		{ "bla",bla,"test run cmd_process"},
	    /*{ "set_triac",cmd_set_triac,"a"},
		{ "debug",debug,"debug print  cmd_debug"},
		{ "rol_print",cmd_rol_print,"rol debug print"},*/
		{ "set_rs485_addr",cmd_set_rs485_addr,"rs485address"},
		//{ "setrol",cmd_setrol,"rol_n position[%]"},
		//{ "owerwriterolpos",cmd_owerwriterolpos,"rol_n position[%]"},
		//{ "setrol_fulltime",cmd_setrol_fulltime,"rol_n fulltime[ms]"},
		//{ "rol_auto",cmd_rol_auto,"rol_n set to auto"},
		{ "enterBL",cmd_enter_bl,"enter Boot Loader"},
		{ "reset",cmd_reset,"reset device"},
		{ "i2c_read",cmd_i2c_read,"addr reg"},
		{ "i2c_write",cmd_i2c_write,"addr reg data"},
		{ "stats",cmd_stats,"RS485 stats"},
		{ "set_rol",cmd_set_rol,"roln rol_pos"},
		{ "ds_debug",cmd_ds_debug,"debug print"},
		{ "print_temp",cmd_print_temp,"print temperature every n miliseconds"},
		//{"enter_bl",cmd_enter_bl,""}

	//	{ "inpb1",get_inb1," get GPIO PB1"},
    };

#endif /* CMDTABLE_H_ */
