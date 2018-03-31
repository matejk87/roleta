#include "mybus.h"
#include "crc16.h"
#include "usart1.h"
#include "timer.h"
#include "conf_IAP.h"
#include "config.h"
#include "usart1.h"
#include "printf.h"
#include "bootloader.h"
#include "roleta_rele.h"
#include "pin_def.h"

//#include <string.h>


#define PRINT_TX 0

// common serial functions
#define MB_PING_FUNCT 0x00
#define MB_ACK_FUNCT 0x00
#define MB_NACK_FUNCT 0x01
#define MB_UPTIME_FUNCT 0x7
#define MB_RESET_CPU 0xB
#define MB_SET_ADDR 0x7d
#define MB_CLEAR_RESET_FLAG_FUNCT 0x7e
#define MB_GET_VERSION_FUNCT 0x7f
//#define GET_STATUS_FUNCT                2

// special serial functions
#define MB_SET_ROLETA 0x5
#define MB_GET_ROLETA 0x6
#define MB_SET_ROL_PAR 0x8
#define MB_GET_ROL_PAR 0x9
#define MB_GET_TEMP 0x0A
#define MB_GET_STATUS 0x0C
#define MB_SET_MA 0x0D
#define MB_GET_MA 0x0E


#define MYBUS_RX_BUFF_SIZE 1024
#define MYBUS_TX_BUFF_SIZE 256

t_mybus rx;
t_mybus tx;

t_timer stx_timer;
t_timer stx_rx_timer;
t_timer stx_act_led_timer;
t_timer timerreadmodaddr;

#define RESET_FLAG_MASK 0x01

volatile unsigned char reset_flag = 0x01;

#define mFlags  (reset_flag&RESET_FLAG_MASK)

//const char ver_opis[]={VER_OPIS};

//extern t_timer timer_modul_reset;

int printchr(char c)
{
	usart1_putchar(c);
  return 0;
}

#define MOD_DEFAULT_ADDR  ROL_DEFAULT_ADDR
unsigned short mod_addr;// = BL_485_ADDR;

unsigned char rx_buffer[MYBUS_RX_BUFF_SIZE];
unsigned char tx_buffer[MYBUS_TX_BUFF_SIZE];
unsigned long tmp32,cfg_ok,time_from_start;

int mybus_st_init(t_mybus * dev, unsigned char * data, int len)
{
  dev->state = MYBUS_STX;
  dev->maxlen = 0;
  dev->data = data;
  dev->maxlen = len;
  dev->cnt = 0;
  return 0;
}

int mybus_put_long(unsigned char * tx_data, unsigned long data)
{
  int tx_len = 0;
  tx_data[tx_len++] = data & 0xff;
  tx_data[tx_len++] = (data >> 8) & 0xff;
  tx_data[tx_len++] = (data >> 16) & 0xff;
  tx_data[tx_len++] = (data >> 24) & 0xff;
  return tx_len;
}

int mybus_put_short(unsigned char * tx_data, unsigned short data)
{
  int tx_len = 0;
  tx_data[tx_len++] = data & 0xff;
  tx_data[tx_len++] = (data >> 8) & 0xff;
  return tx_len;
}

int mybus_put_char(unsigned char * tx_data, unsigned char data)
{
  int tx_len = 0;
  tx_data[tx_len++] = data & 0xff;
  return tx_len;
}

#define CHAR  1
#define SHORT 2
#define LONG  4

unsigned long mybus_get(unsigned char * rx_data,int type)
{
  unsigned long tmp=0;
  tmp = rx_data[0];
  if(type == CHAR)return tmp;
  tmp |= (rx_data[1]<<8);
  if(type == SHORT)return tmp;
  tmp |= (rx_data[2]<<16);
  tmp |= (rx_data[3]<<24);
  return tmp;
}

int
tx_data(t_mybus * dev, int
(*fputchar)(char))
{
  unsigned char cval;
  int n;
  fputchar(STX);
#if PRINT_TX
  printf("\nTX:%02x",STX);
#endif

  dev->crc = 0;

  cval = dev->src & 0xFF;
  crc16_n(&(dev->crc), (char *) &cval, 1);
  fputchar(cval);
#if PRINT_TX
  printf(" %02x",cval);
#endif

  cval = (dev->src >> 8) & 0xFF;
  crc16_n(&(dev->crc), (char *) &cval, 1);
  fputchar(cval);
#if PRINT_TX
  printf(" %02x",cval);
#endif

  cval = dev->dest & 0xFF;
  crc16_n(&(dev->crc), (char *) &cval, 1);
  fputchar(cval);
#if PRINT_TX
  printf(" %02x",cval);
#endif

  cval = (dev->dest >> 8) & 0xFF;
  crc16_n(&(dev->crc), (char *) &cval, 1);
  fputchar(cval);
#if PRINT_TX
  printf(" %02x",cval);
#endif

  fputchar(dev->msg_number);
  crc16_n(&(dev->crc), (char *) &dev->msg_number, 1);
#if PRINT_TX
  printf(" %02x",dev->msg_number);
#endif

  fputchar(dev->funct);
  crc16_n(&(dev->crc), (char *) &dev->funct, 1);
#if PRINT_TX
  printf(" %02x",dev->funct);
#endif

  fputchar(dev->flags);
#if PRINT_TX
  printf(" %02x",dev->flags);
#endif

  crc16_n(&(dev->crc), (char *) &dev->flags, 1);
  fputchar(dev->datalen);
#if PRINT_TX
  printf(" %02x",dev->datalen);
#endif

  crc16_n(&(dev->crc), (char *) &dev->datalen, 1);
  for (n = 0; n < (dev->datalen); n++)
    {
      fputchar(dev->data[n]);
#if PRINT_TX
      printf(" %02x",dev->data[n]);
#endif

      crc16_n(&(dev->crc), (char *) &dev->data[n], 1);
    }
  fputchar(dev->crc & 0xff);
#if PRINT_TX
  printf(" %02x",dev->crc & 0xff);
#endif

  fputchar((dev->crc >> 8) & 0xff);
#if PRINT_TX
  printf(" %02x",(dev->crc >> 8) & 0xff);
#endif

  fputchar(ETX);
#if PRINT_TX
  printf(" %02x",ETX);
#endif
  dev->stats.tx_msg_cnt++; // update stats
  return 0;
}

void mybus_init(void)
{
  printf("\n mybus init");
  usart1_init();
  printf(":OK");
  if(get_RS485addr_from_flash() >= 0xffff /*|| get_RS485addr_from_flash() <= 1*/)
  {
	  //printf("\n\rwriting default addr to flash");
	  write_RS485addr_to_flash(MOD_DEFAULT_ADDR);
  }
  mod_addr=get_RS485addr_from_flash();
  printf("\n\rRS485 addr:0x%x",mod_addr);
  // printf("\nmod addr:0x%04x", mod_addr);
  mybus_st_init(&rx, rx_buffer, MYBUS_RX_BUFF_SIZE);
  mybus_st_init(&tx, tx_buffer, MYBUS_TX_BUFF_SIZE);

}

void mybus_make_tx_header(t_mybus * rxdev, t_mybus * txdev)
{
  txdev->src = mod_addr;
  txdev->dest = rxdev->src;
  txdev->flags = mFlags;
  txdev->msg_number = rxdev->msg_number;
  //printf("\n txdev->src:%x",txdev->src);
}

void mybus_main(void)
{
  char rxchar = 0;
  static unsigned long tmp=0;
  int j;
  if (rx.state != MYBUS_END)
    {
      if (usart1_rxlen())
        {
          rxchar = usart1_getchar();
       //   usart2_putchar(rxchar); //todo odstrani
        /*  if(rx.state == MYBUS_STX)printf("\n");
          printf("0x%x ",rxchar); //todo odstrani*/
          switch (rx.state)
            {
          case MYBUS_STX:
            timeout(&stx_rx_timer, 0);
            time_from_start = timetick;
            if (rxchar == 0x55)
              rx.state = MYBUS_SRC0;
           //  printf("\ns:");
            break;
          case MYBUS_SRC0:
           // printf("\nSRC0:%02x",rxchar);
            rx.crc_calc = 0;
            rx.src = rxchar & 0x00FF;
            crc16_n(&(rx.crc_calc), &rxchar, 1);
            rx.state = MYBUS_SRC1;
            break;
          case MYBUS_SRC1:
        	  //printf("\nSRC1:%02x",rxchar);
            rx.src |= (rxchar << 8) & 0xFF00;
            crc16_n(&(rx.crc_calc), &rxchar, 1);
            rx.state = MYBUS_DEST0;
            break;
          case MYBUS_DEST0:
        	  //printf("\nDEST0:%02x",rxchar);
            rx.state = MYBUS_DEST1;
            crc16_n(&(rx.crc_calc), &rxchar, 1);
            rx.dest = rxchar & 0x00FF;
            break;
          case MYBUS_DEST1:
        	  //printf("\nDEST1:%02x",rxchar);
            rx.state = MYBUS_MSG_NUMBER;
            crc16_n(&(rx.crc_calc), &rxchar, 1);
            //   printf("dest:%04x",rx.dest );
            rx.dest |= (rxchar << 8) & 0xFF00;
            break;
          case MYBUS_MSG_NUMBER:
            rx.state = MYBUS_FUNCT;
            crc16_n(&(rx.crc_calc), &rxchar, 1);
            rx.msg_number = rxchar;
            break;
          case MYBUS_FUNCT:
            rx.state = MYBUS_FLAGS;
            crc16_n(&(rx.crc_calc), &rxchar, 1);
            rx.funct = rxchar;
            break;
          case MYBUS_FLAGS:
        	  //printf("\nFLAGS0:%02x",rxchar);
            rx.state = MYBUS_DATALEN;
            crc16_n(&(rx.crc_calc), &rxchar, 1);
            rx.flags = rxchar;
            break;
          case MYBUS_DATALEN:
        	  //printf("\nDLEN:%02x",rxchar);
            crc16_n(&(rx.crc_calc), &rxchar, 1);
            rx.datalen = rxchar;
            rx.cnt = 0;
            if (rx.datalen)
              rx.state = MYBUS_DATA;
            else
              rx.state = MYBUS_CRC1;
            break;
          case MYBUS_DATA:
            rx.data[rx.cnt] = rxchar;
            crc16_n(&(rx.crc_calc),
                (char *) &(rx.data[rx.cnt]), 1);
            rx.cnt++;
            if (rx.cnt == rx.datalen)
              {
                rx.state = MYBUS_CRC1;
              }
            break;
          case MYBUS_CRC1:
            rx.state = MYBUS_CRC2;
            rx.crc = rxchar & 0xFFUL;
            break;
          case MYBUS_CRC2:
        	  //printf("\ncrc:%02x",rxchar);
            rx.state = MYBUS_ETX;
            rx.crc |= (rxchar & 0xFFUL) << 8;
            break;
          case MYBUS_ETX:
        	  //printf("\netx");
            if (rxchar == ETX)
              { // ali je etx ok
                if (rx.crc_calc == rx.crc)
                  { // preveri crc

                    rx.state = MYBUS_END;
                    rx.stats.rx_total_msg_cnt++; // update stats
                    timeout(&stx_timer,0);

                   //printf("\nRX OK");
                    // return 1;
                  }
                else // CRC ERROR
                  {
                    rx.stats.rx_crc_err_cnt++; // update stats
                    rx.state = MYBUS_STX;
                    printf("\n\r_crc_error");
                    //printf(" sr:%04x de:%04x", rx.src, rx.dest);
                    //return -2;
                  }
              }
            else // etx error
              {
                rx.stats.rx_etx_err_cnt++; // update stats
                rx.state = MYBUS_STX;
                printf("\n\retx_error (rx_char:0x%x) dest:0x%x",rxchar,rx.dest);
                // return -1;
              }
            break;
            }
        }
      else if ((rx.state != MYBUS_STX) && timeout(&stx_rx_timer,1000))
        {
    	  printf("\n\rtimeout");
          rx.state = MYBUS_STX;
          rx.stats.rx_timeout_cnt++;

        }
    }
  else
    {
     // printf("\nrxok src:%04x dest:%04x",rx.src,rx.dest);
      if ((rx.dest == mod_addr) && (rx.src != mod_addr))
        {
    	  LED1_SET();
          if (!timeout(&stx_timer,5))
            return;

          rx.stats.rx_msg_cnt++;
         // printf("\nrxok dest:%04x",rx.dest);
          //  printf("\nrx:%02x",rx.funct);
          timeout(&stx_act_led_timer, 0);
          //timeout(&timer_modul_reset, 0);
         // tmr_reset_timer();
          mybus_make_tx_header(&rx, &tx);
          switch (rx.funct)
            {
          case MB_PING_FUNCT:
           // printf("\nping/ack");
            tx.funct = MB_ACK_FUNCT;
            tx.datalen = 0;

            tx_data(&tx, printchr);
            break;
       /*   case GET_STATUS_FUNCT:
            tx.funct = GET_STATUS_FUNCT;
            tx.datalen = 0;

            tx_data(&tx, printchr);
            break;*/
          case MB_UPTIME_FUNCT:
            tx.funct = MB_UPTIME_FUNCT;
            tx.datalen = 0;
            tx.datalen += mybus_put_long( &tx.data[tx.datalen], timesec);
            tx_data(&tx, printchr);
            break;
          case MB_CLEAR_RESET_FLAG_FUNCT:
            reset_flag = 0x00;
            tx.flags = mFlags;
            tx.funct = MB_ACK_FUNCT;
            tx.datalen = 0;
            tx_data(&tx, printchr);
            break;

          case MB_GET_VERSION_FUNCT:
            tx.funct = MB_GET_VERSION_FUNCT;
            tx.datalen = 0;
            for(j=0;j<strlen(VER_OPIS);j++)
              {
                       tx.datalen += mybus_put_char(&tx.data[tx.datalen], VER_OPIS[j]); // opis
              }
            tx.datalen += mybus_put_char(&tx.data[tx.datalen], 0);
            tx_data(&tx, printchr);
            break;
          case START_BL:
            tx.datalen=0;
            tx.funct = START_BL;
            tx_data(&tx, printchr);
            delay(15);
            enter_bootloader(mod_addr); //todo add a bootloader function
            break;
          case MB_RESET_CPU:
        	tx.funct = MB_RESET_CPU;
        	tx.datalen = 0;
        	tx_data(&tx, printchr);
        	delay(15);
        	resetcpu();
        	break;
          case MB_SET_ADDR:
            tx.funct = MB_SET_ADDR;
            tx.datalen = 0;
            printf("\nRS485 set addr");
            write_RS485addr_to_flash(mybus_get(rx.data,SHORT));
            mod_addr = mybus_get(rx.data,SHORT);
            tx_data(&tx, printchr);
            break;
          case MB_SET_ROLETA:
            tx.funct = MB_SET_ROLETA;
            tx.datalen = 0;
            for(j=0;j<(rx.datalen) &&  j<ROL_N;j++)
            {
            	rol_set_auto_pos(j,rx.data[j]);
            }
            tx_data(&tx, printchr);
            break;
          case MB_GET_ROLETA:
            tx.funct = MB_GET_ROLETA;
            tx.datalen = 0;
            for(j=0;j<(rx.datalen) &&  j<ROL_N;j++)
            {
            	tx.datalen += mybus_put_char( &tx.data[tx.datalen],rol[j].rol_pos);
            }
            tx_data(&tx, printchr);
            break;
          case MB_SET_ROL_PAR:
            tx.funct = MB_SET_ROL_PAR;
            tx.datalen = 0;
            for(j=0;j<(rx.datalen) &&  j<ROL_N;j++)
            {
            	rol[j].fulltime_up = mybus_get((unsigned char *)(rx.data+(j*5)),SHORT);
            	rol[j].fulltime_down = mybus_get((unsigned char *)(rx.data+SHORT+(j*5)),SHORT);
            	rol_set_mode(rol,j,rx.data[j*5+4]);
            }
            tx_data(&tx, printchr);
            break;
          case MB_GET_ROL_PAR:
            tx.funct = MB_GET_ROL_PAR;
            tx.datalen = 0;
            for(j=0; j<ROL_N;j++)
            {
            	tx.datalen += mybus_put_short( &tx.data[tx.datalen],(unsigned short)(rol[j].fulltime_up));
            	tx.datalen += mybus_put_short( &tx.data[tx.datalen],(unsigned short)(rol[j].fulltime_down));
            	tx.datalen += mybus_put_char( &tx.data[tx.datalen],rol[j].mode);
            }
            tx_data(&tx, printchr);
            break;
          case MB_GET_STATUS:
            tx.funct = MB_GET_STATUS;
            tx.datalen = 0;
            tx.datalen += mybus_put_char( &tx.data[tx.datalen],temp_SE95.temperature_valid);
            tx.datalen += mybus_put_short( &tx.data[tx.datalen],temp_SE95.temperature);
            for(j=0; j<ROL_N;j++)
            {
            	tx.datalen += mybus_put_char( &tx.data[tx.datalen],rol[j].mode);
            	tx.datalen += mybus_put_char( &tx.data[tx.datalen],rol[j].rol_pos);
            }
            tx_data(&tx, printchr);
            break;
          case MB_SET_MA:
            tx.funct = MB_SET_MA;
            tx.datalen = 0;
            rol_set_mode(rol,0,rx.data[0]);
            rol_set_mode(rol,1,rx.data[0]);
            tx_data(&tx, printchr);
            break;
          case MB_GET_MA:
            tx.funct = MB_GET_MA;
            tx.datalen = 0;
            	tx.datalen += mybus_put_char( &tx.data[tx.datalen],rol[0].mode); // todo drugi kanal?
            tx_data(&tx, printchr);
            break;
          case MB_GET_TEMP:
              tx.funct = MB_GET_TEMP;
              tx.datalen = 0;
              //temp_SE95.temperature_valid
              tx.datalen += mybus_put_char( &tx.data[tx.datalen],temp_SE95.temperature_valid);
              tx.datalen += mybus_put_short( &tx.data[tx.datalen],temp_SE95.temperature);
              tx_data(&tx, printchr);
        	  break;

          default:
        	printf("\n nack - function number:%d",rx.funct);
            tx.funct = 0x01; // nack
            tx.datalen = 0;
            tx_data(&tx, printchr);
            break;
            }
          rx.state = MYBUS_STX; // rx complette
          LED1_CLR();

        } //addr ok
      else
        {
          rx.state = MYBUS_STX;
        }
    }// end // RX OK
}

int cmd_stats(int argc, char **argv)
{
  printf("\nmybus statistics");
  printf("\nAddr:%02x",/*MODUL_DEFAULT_ADDR+*/mod_addr);
  printf("\nrx_tot_msg_cnt:%d", rx.stats.rx_total_msg_cnt);
  printf("\nrx_msg_cnt    :%d", rx.stats.rx_msg_cnt);
  printf("\nrx_etx_err_cnt:%d", rx.stats.rx_etx_err_cnt);
  printf("\nrx_timeout_cnt:%d", rx.stats.rx_timeout_cnt);
  printf("\nrx_crc_err_cnt:%d", rx.stats.rx_crc_err_cnt);
  printf("\nrx_etx_err_cnt:%d", rx.stats.rx_etx_err_cnt);
  return 0;
}

int cmd_enter_bl(int argc, char **argv)
{
	enter_bootloader(mod_addr);
	return 0;
};

