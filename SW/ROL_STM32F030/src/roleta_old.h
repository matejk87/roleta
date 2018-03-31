/*
 * roleta.h
 *
 *  Created on: 26. dec. 2015
 *      Author: Matej
 */

#ifndef INC_ROLETA_H_
#define INC_ROLETA_H_

#include "stm32f0xx.h"
#include "timer.h"
#include "ds1820.h"
#include "se95.h" // todo odkomentiraj


#define MAX_ROL_N 4
#define ROL_N 2

#define SWITCH_MIN_TIME 70 //ms
#define SWITCH_MAX_TIME 300 //ms

#define DEFAULT_FULL_SCALE_TIME 15000 //ms

#define ROL_UP 0
#define ROL_DOWN 1

#define ROL_ACT 0
#define ROL_IDLE 1

typedef enum{
	mauto,
	mmanual
}mode_t;

typedef enum{
	tidle,
	tup,
	tdown
}tstate_t;

typedef enum{
	m_idle,
	m_relay_delay,
	m_up,
	m_down
}rol_man_state_t;

typedef enum{
	idle,
	cfg,
	up,
	move,
	down,
	up_man,
	down_man,
	fullup,
	fulldown
}rol_auto_state_t;

typedef struct
{
	int mode;  // mode_t (auto/manual)
	rol_man_state_t m_state,m_state_old; //state var for state machine
	unsigned int switch_mode_pause;

	rol_auto_state_t astate,astate_old;
	unsigned short rol_pos,pos_cmd; // roleta height in % (100% means completely opened)
	tstate_t tstate,outstate;
	t_timer tup_man,tdown_man;
	t_timer rol_timer,rol_end_time;
	t_timer tipkaup_timer,tipkadown_timer;
	t_timer tipkaup_last,tipkadown_last;
	t_timer fulltime_up,fulltime_down;

}roleta_t;

typedef struct
{
	//unsigned char ;  // mode_t (auto/manual)


}roleta_cmd_t;


extern roleta_t rol[MAX_ROL_N];

void sample_input_states(void);
void roleta_init(int n_of_rol);
void roleta_main(void);
int rol_set_mode(roleta_t *r,unsigned short roln,short mode);



#endif /* INC_ROLETA_H_ */
