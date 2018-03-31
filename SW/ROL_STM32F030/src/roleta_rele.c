#include "stm32f0xx_conf.h"
#include "roleta_rele.h"
#include "timer.h"
#include "printf.h"
#include "pin_def.h"
#include "bootloader.h"

int INP_PINS[6]={PINP0,PINP1,PINP2,PINP3};
//int OUT_PINS[6]={POUT0,POUT1,POUT2,POUT3};
int RELAY_PINS[6]={POUT_RELAY};
int TRIAC_PINS[6]={POUT_TRIAC0,POUT_TRIAC1};


unsigned short tipke_states=0,tipke_states_old=0;
unsigned short motor_state=0,motor_state_old=0,triac_state=0,triac_state_old=0;
unsigned int triac=0;

roleta_t rol[MAX_ROL_N];
relay_state_t relay_state[(MAX_ROL_N/2)+1];
t_timer rol_timer;
t_timer test_timer;

#define MAX_SAMPLES 64  // = 2^n
unsigned short input_samples[MAX_SAMPLES];
unsigned short sample_cnt=0,samplefull=0;

t_timer time_needed(roleta_t *r,unsigned short pos_old,unsigned short pos_new);

void sample_input_states(void)
{
	//if(samplefull)printf("\nerror sample_input_states");
	if(samplefull)printf("||");
	input_samples[sample_cnt%MAX_SAMPLES]=(PINP_PORT->IDR);
	//printf("\n0x%x",input_samples[sample_cnt]);
	sample_cnt++;
	if(sample_cnt>=MAX_SAMPLES)
	{
		sample_cnt = 0;
		samplefull=1;
	}
}
/*
void set_triac(unsigned int triac_mask)
{
	int i;
	for(i=0;i<N_OF_POUT;i++)
	{
		if(triac_mask&(1<<i))
		{
			//printf("\n set 1");
			POUT_PORT->ODR |= OUT_PINS[i];
			triac_state |= (1<<i);
		}
		else
		{
			//printf("\n set 0");
			POUT_PORT->ODR &= ~(OUT_PINS[i]);
			triac_state &= ~((1<<i));
		}
	}
}*/

void set_triac(int rol_n,int onoff)
{
	if(onoff) POUT_PORT->ODR |= TRIAC_PINS[rol_n];
	else POUT_PORT->ODR &= ~(TRIAC_PINS[rol_n]);
}



#define RELAY_UP 0
#define RELAY_DOWN 1


void set_relay(int rol_n,int updown)
{
	if(updown) POUT_PORT->ODR |= RELAY_PINS[rol_n/2];
	else POUT_PORT->ODR &= ~(RELAY_PINS[rol_n/2]);
}

unsigned short paired_rol_n(unsigned short n)
{
	if(n==0)return 1;
	else if(n==1)return 0;
	else printf("\n paired_rol_n error");
	return 0;
}
unsigned short paired_rol_active(unsigned short n)
{
	int paired_rol;
	paired_rol = paired_rol_n(n);
	if(rol[paired_rol].mode == mmanual)
	{
		if(rol[paired_rol].m_state== m_idle) return 0;
		else return 1;
	}
	if(rol[paired_rol].mode == mauto)
	{
		if(rol[paired_rol].astate == a_idle) return 0;
		else return 1;
	}
	return 0;
}

/*
relay_state_t paired_rol_relay_state(unsigned short n)
{
	unsigned short paired_rol;
	paired_rol = paired_rol_n(n);
	return rol[paired_rol].relay_state;
}*/

relay_state_t get_rol_relay_state(unsigned short n)
{
	return relay_state[n/2];
}

int rol_relay_free(unsigned short n,int updown)
{

	relay_state_t state;
	state = get_rol_relay_state(n);
	if(updown == ROL_UP)
	{
		if(state == relay_idle || state == relay_up)return 1;
		else return 0;
	}
	else if (updown == ROL_DOWN)
	{
		if(state == relay_idle || state == relay_down)return 1;
		else return 0;
	}
	else printf("\n error!");
	return 0;
}

void set_rol_relay_state(unsigned short n,relay_state_t state)
{
	relay_state[n/2]= state;


}

void roleta_init(int n_of_rol)
{
	int i=0;
	triac=0;
	for(i=0;i<n_of_rol;i++) //todo samo zacasen fulltime_up
	{
		rol[i].fulltime_up = DEFAULT_FULL_SCALE_TIME;
		rol[i].fulltime_down = DEFAULT_FULL_SCALE_TIME;
	}
	printf("\n roleta init");

}



void roleta_main(void)
{
	int inp,sample=0,samples,roln;

	/*
	static int test_cnt=0;
	if(timeout(&test_timer,1000))
	{
		if(test_cnt == 1)
		{
			rol[0].tipkaup_last = 200;
		}
		else if(test_cnt == 51)
		{
			rol[0].tipkadown_last = 200;
		}
		else if(test_cnt == 100) test_cnt = 0;
		test_cnt++;
	}*/

	if(samplefull)//new samples of switches? - input readings
	{
		samplefull = 0;
		for(inp=0;inp<N_OF_PINP;inp++)
		{
			samples=0;
			for(sample=0;sample<MAX_SAMPLES;sample++)
			{
				if(input_samples[sample]&INP_PINS[inp])samples++;
			}
			//if(inp<2)printf("\ninp:%d samples:%d",inp,samples);
			if(samples>(MAX_SAMPLES*3/4))
			{
				tipke_states &= (~(1<<inp));
			}
			else
			{
				tipke_states |= (1<<inp);
			}

		}
		if(tipke_states!=tipke_states_old)
		{
			printf("\n tipke:0x%x",tipke_states);
			for(inp=0;inp<N_OF_PINP;inp++)
			{
				if((1<<inp)&tipke_states)
				{
					if(!(inp&1))
					{
						rol[inp/2].tstate = tup;
						//rol[inp/2].tipkaup_last
						timeout(&(rol[inp/2].tipkaup_timer),0);
						//triac &= ~(2<<(inp/2*2));//izklop kontra smeri
						printf("\n tipkaup");
					}
					else
					{
						rol[inp/2].tstate = tdown;
						timeout(&(rol[inp/2].tipkadown_timer),0);
						//triac &= ~(1<<(inp/2*2));//izklop kontra smeri
						printf("\n tipkadown");
					}
				}
			}
			for(inp=0;inp<(N_OF_PINP);inp+=2)
			{
				if(((tipke_states>>inp)&3)!=((tipke_states_old>>inp)&3))
				{
					if(((tipke_states>>inp)&3)==0)
					{
						if((tipke_states_old>>inp)&1)
						{
							rol[inp/2].tstate = tidle;
							rol[inp/2].tipkaup_last = timer_duration(&(rol[inp/2].tipkaup_timer));
							printf("\n tipkaup: %ums",rol[inp/2].tipkaup_last);
						}
						else if((tipke_states_old>>inp)&2)
						{
							rol[inp/2].tstate = tidle;
							rol[inp/2].tipkadown_last = timer_duration(&(rol[inp/2].tipkadown_timer));
							printf("\n tipkadown: %ums",rol[inp/2].tipkadown_last);
						}
						else printf("\n error 2");
					}
				}
			}
		}
		for(inp=0;inp<(N_OF_PINP);inp+=2)
		{
			if((tipke_states>>inp)&3)
			{
				if((tipke_states>>inp)&1)
				{
					if(timer_duration(&(rol[inp/2].tipkaup_timer)) > SWITCH_MAX_TIME)
					{
						//printf("\n tipka up long");
						rol[inp/2].up_man = 1; //znak da je tipka pritisnjena dlje od SWITCH_MAX_TIME
						rol[inp/2].down_man = 0;
						rol[inp/2].tipkaup_last=0;
					}
				}
				else if((tipke_states>>inp)&2)
				{
					if(timer_duration(&(rol[inp/2].tipkadown_timer)) > SWITCH_MAX_TIME)
					{
						//printf("\n tipka down long");
						rol[inp/2].down_man = 1; //znak da je tipka pritisnjena dlje od SWITCH_MAX_TIME
						rol[inp/2].up_man = 0;
						rol[inp/2].tipkadown_last=0;
					}
				}
			}
			else
			{
				rol[inp/2].up_man = 0;
				rol[inp/2].down_man = 0;
			}
		}
		//for(inp=0;inp<(N_OF_PINP);inp+=2)


		tipke_states_old = tipke_states;
		triac_state_old = triac_state;
	}
	if(timeout(&rol_timer,15))
	{
		for(roln=0;roln<ROL_N;roln++)
		{
			//if(rol[roln].down_man || rol[roln].up_man || rol[roln].tipkadown_last || rol[roln].tipkaup_last) rol[roln].mode = mmanual; // preklop v manual èe se pritisne tipka
			if(rol[roln].mode == mmanual) //manual
			{
				switch(rol[roln].m_state)
				{
					case m_idle:
						if(paired_rol_active(roln)){break;}
						if(rol[roln].mode_set_auto_cmd == 1)
						{
							rol[roln].mode_set_auto_cmd = 0;
							rol[roln].mode = mauto;
							break;
						}
						if((rol[roln].tipkaup_last > SWITCH_MIN_TIME) && (rol[roln].tipkaup_last < SWITCH_MAX_TIME))
						{
								rol[roln].m_state = m_relay_delay;
								timeout(&(rol[roln].relay_timer),0);
								set_relay(roln,RELAY_UP);
								//set_rol_relay_state(roln,relay_up);
						}
						else if((rol[roln].tipkadown_last > SWITCH_MIN_TIME) && (rol[roln].tipkadown_last < SWITCH_MAX_TIME))
						{

								rol[roln].m_state = m_relay_delay;
								timeout(&(rol[roln].relay_timer),0);
								set_relay(roln,RELAY_DOWN);
								//set_rol_relay_state(roln,relay_down);
						}
						else if(rol[roln].up_man)
						{
								rol[roln].m_state = m_relay_delay;
								timeout(&(rol[roln].relay_timer),0);
								set_relay(roln,RELAY_UP);
								//set_rol_relay_state(roln,relay_up);
						}
						else if(rol[roln].down_man)
						{
								rol[roln].m_state = m_relay_delay;
								timeout(&(rol[roln].relay_timer),0);
								set_relay(roln,RELAY_DOWN);
								//set_rol_relay_state(roln,relay_down);
						}
						break;
					case m_relay_delay:
						if(timeout(&(rol[roln].relay_timer),RELAY_DELAY_TIME))
						{

							if((rol[roln].tipkaup_last > SWITCH_MIN_TIME) && (rol[roln].tipkaup_last < SWITCH_MAX_TIME))
							{
								rol[roln].m_state = m_up_full;
								timeout(&(rol[roln].rol_timer),0);
								set_triac(roln,1);
							}
							else if((rol[roln].tipkadown_last > SWITCH_MIN_TIME) && (rol[roln].tipkadown_last < SWITCH_MAX_TIME))
							{
								rol[roln].m_state = m_down_full;
								timeout(&(rol[roln].rol_timer),0);
								set_triac(roln,1);
							}
							else if(rol[roln].up_man)
							{
								rol[roln].m_state = m_up;
								set_triac(roln,1);
							}
							else if(rol[roln].down_man)
							{
								rol[roln].m_state = m_down;
								set_triac(roln,1);

							}
							else // todo
							{
								//printf("\n problem 1 v 0!");
								set_relay(roln,0);
								rol[roln].m_state = m_relay_delay_off;

							}
							//set_triac(roln,1);
						}
						break;
					case m_relay_delay_off:
						if(timeout(&(rol[roln].rol_timer),100))
						{
							rol[roln].m_state = m_idle;
							rol[roln].tipkaup_last = 0;
							rol[roln].tipkadown_last = 0;
						}
						break;
					case m_triac_delay:
						if(timeout(&(rol[roln].rol_timer),100))
						{
							set_relay(roln,0);
							rol[roln].m_state = m_relay_delay_off;
						}
						break;
					case m_up_full:
						set_triac(roln,1);
						if(timer_duration(&(rol[roln].rol_timer)) > rol[roln].fulltime_up )
						{
							set_triac(roln,0);
							rol[roln].m_state = m_triac_delay;
							//rol[roln].tipkaup_last = 0;
							timeout(&(rol[roln].rol_timer),0);
						}
						break;
					case m_up:
						set_triac(roln,1);
						if(!rol[roln].up_man)
						{
							printf("\n set triac 0");
							set_triac(roln,0);
							rol[roln].m_state = m_triac_delay;
							//rol[roln].tipkaup_last = 0;
							timeout(&(rol[roln].rol_timer),0);
						}
						break;
					case m_down_full:
						set_triac(roln,1);
						if(timer_duration(&(rol[roln].rol_timer)) > rol[roln].fulltime_down )
						{
							set_triac(roln,0);
							rol[roln].m_state = m_triac_delay;
							//rol[roln].tipkadown_last = 0;
							timeout(&(rol[roln].rol_timer),0);
						}
					break;
					case m_down:
						set_triac(roln,1);
						if(!rol[roln].down_man)
						{
							set_triac(roln,0);
							rol[roln].m_state = m_triac_delay;
							//rol[roln].tipkadown_last = 0;
							timeout(&(rol[roln].rol_timer),0);
						}
						break;

				}
				//printf("\n rol[%d] state:%d up_man:%d down_man:%d tipkaup_last:%d tipkadown_last:%d",roln,rol[roln].m_state,rol[roln].up_man,rol[roln].down_man,rol[roln].tipkaup_last,rol[roln].tipkadown_last);
				if(rol[roln].m_state != rol[roln].m_state_old)printf("\n rol[%d] state:%d up_man:%d down_man:%d tipkaup_last:%d tipkadown_last:%d",roln,rol[roln].m_state,rol[roln].up_man,rol[roln].down_man,rol[roln].tipkaup_last,rol[roln].tipkadown_last);
				rol[roln].m_state_old = rol[roln].m_state;
			}
			else if(rol[roln].mode == mauto)  //auto
			{
				/*if(rol[roln].tipkaup_last || rol[roln].tipkadown_last)
				{
					rol_set_mode(rol,roln,mmanual);

				}*/
				switch(rol[roln].astate)
				{
				case a_idle:
					if(paired_rol_active(roln)){break;}
					if(rol[roln].down_man || rol[roln].up_man || rol[roln].tipkadown_last || rol[roln].tipkaup_last)
					{
						rol[roln].mode = mmanual;
						break;
					}
					if(rol[roln].pos_cmd != rol[roln].rol_pos)
					{
						if(rol[roln].pos_cmd > rol[roln].rol_pos) //up
						{
							set_relay(roln,RELAY_UP);
						}
						else///down
						{
							set_relay(roln,RELAY_DOWN);
						}
						rol[roln].astate = a_relay_delay_on;
						timeout(&(rol[roln].relay_timer),0);

						/*timeout(&(rol[roln].rol_timer),0);
						rol[roln].astate = a_move;*/
					}
					break;
				case a_relay_delay_on:
					if(timeout(&(rol[roln].relay_timer),RELAY_DELAY_TIME))
					{
						rol[roln].rol_end_time = time_needed(&(rol[roln]),rol[roln].rol_pos,rol[roln].pos_cmd);
						set_triac(roln,1);
						timeout(&(rol[roln].rol_timer),0);
						rol[roln].astate = a_move;
					}
					break;
				case a_cfg:
				/*	triac |= (2<<(roln*2));
					if(timer_duration(&(rol[roln].rol_timer))>= rol[roln].fulltime_down)
					{
						rol[roln].astate = a_idle;
						triac &= ~(3<<(roln*2));
						rol[roln].rol_pos = 0;
					}*/
					break;
				case a_move:
					if(timer_duration(&(rol[roln].rol_timer))>= rol[roln].rol_end_time)
					{
						rol[roln].rol_end_time = 0;
						set_triac(roln,0);
						rol[roln].rol_pos = rol[roln].pos_cmd;
						timeout(&(rol[roln].rol_timer),0);
						rol[roln].astate = a_triac_delay_off;
					}
					break;
				case a_triac_delay_off:
					if(timeout(&(rol[roln].rol_timer),100))
					{
						//printf("\n123");
						set_relay(roln,0);
						rol[roln].astate = a_relay_delay_off;
					}
					break;
				case a_relay_delay_off:
					if(timeout(&(rol[roln].rol_timer),100))
					{
						//printf("\n123456");
						rol[roln].astate = a_idle;
					}
					break;

				case a_up:
					break;
				case a_down:
					break;
				case a_fullup:
					break;
				case a_fulldown:
					break;
				}
				if(rol[roln].astate != rol[roln].astate_old)printf("\n rol[%d] astate:%d",roln,rol[roln].astate);
				rol[roln].astate_old = rol[roln].astate;
				//set_triac(triac);
			}
			if(rol[roln].mode != rol[roln].mode_old)printf("\nmode:%d",rol[roln].mode);
			rol[roln].mode_old = rol[roln].mode;
		}
	}
}

int rol_set_mode(roleta_t *r,unsigned short roln,short mode)
{
	if(roln > MAX_ROL_N)return 0; //todo
	if(mode == mauto) r[roln].mode_set_auto_cmd = 1;


/*
	if(mode == mmanual)
	{
		r[roln].mode = mmanual;
		r[roln].astate = a_idle;
		r[roln].m_state = m_idle;
		triac &= ~(3<<(roln*2));
		printf("\n rol[%d].mode -> manual!",roln);
		return 1;
	}
	else if(mode == mauto)
	{
		r[roln].mode = mauto;
		r[roln].astate = a_cfg;
		timeout(&(r[roln].rol_timer),0);
		r[roln].m_state = m_idle;
		triac &= ~(3<<(roln*2));
		printf("\n rol[%d].mode -> auto!",roln);
		return 1;
	}
	else printf("\n error 2148cf");
	*/
	return 0;

}


t_timer time_needed(roleta_t *r,unsigned short pos_old,unsigned short pos_new)
{
	t_timer time=0;
	if(pos_new > pos_old)
	{
		time = ((r->fulltime_up)/10)*(((pos_new-pos_old)/10));
	}
	else if(pos_new < pos_old)
	{
		time = ((r->fulltime_up)/10)*(((pos_old-pos_new)/10));
	}
	else printf("\n err23425");
	return time;
}

//pos 0-100% opened
int rol_set_auto_pos(unsigned short rol_n,unsigned short pos)
{
	if(pos>100)pos=100;
	pos -= pos%10;
	if(rol[rol_n].astate == a_idle)
	{
		printf("R[%d]:%d% ",rol_n,pos);
		rol[rol_n].pos_cmd = pos/10*10;
	}
	return 0;
}



int cmd_set_triac(int argc,char ** argp)
{
	/*unsigned long outputs=0;
	if(argc>1)
	{
		outputs=strtol(argp[1],0,0);

		set_triac(outputs);
	}
	printf("\nset_triac:0x%x",outputs);*/
	return 1;
}

int cmd_set_rol(int argc,char ** argp)
{
	unsigned long rol_n,rol_pos;
	if(argc>2)
	{
		rol_n=strtol(argp[1],0,0);
		rol_pos=strtol(argp[2],0,0);
		printf("\n set_rol- rol_n:%d rol_pos_cmd:%d rol_pos_old:%d",rol_n,rol_pos,rol[rol_n].rol_pos);
		rol[rol_n].pos_cmd = rol_pos;
	}else printf("\n argc error");
	return 1;

}





void cmd_rol_print(int argc,char ** argp)
{
	int roln=0;
	for(roln=0;roln<ROL_N;roln++)
	{
		printf("\n rol[%d]: mode:%d pos:%d",roln,rol[roln].mode,rol[roln].rol_pos);
	}

}
int cmd_reset(int argc,char ** argv)
{
	resetcpu();
}

