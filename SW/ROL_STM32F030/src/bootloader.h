#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

// bootloader commands
#define PING_BL                         0xf1
#define GET_APP_VERSION                 0xf2
#define PROGRAM_DATA                    0xf3
#define GET_APP_OFFSET                  0xf4
#define GET_CRC                         0xf5
#define EXIT_BL                         0xf6
#define START_BL 						0xf0


void copy_vector_table_sram(void);
void resetcpu(void);
void enter_bootloader(unsigned short rs485_addr);
int write_RS485addr_to_flash(unsigned short rs485_addr);
unsigned int get_RS485addr_from_flash(void);


#endif /* BOOT_LOADER_H_ */
