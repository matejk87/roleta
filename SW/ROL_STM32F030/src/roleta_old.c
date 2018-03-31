#include "stm32f0xx_conf.h"
#include "roleta_rele.h"
#include "timer.h"
#include "printf.h"
#include "pin_def.h"

int INP_PINS[6]={PINP0,PINP1,PINP2,PINP3};
int OUT_PINS[6]={POUT0,POUT1,POUT2,POUT3};


unsigned short tipke_states=0,tipke_states_old=0;
unsigned short motor_state=0,motor_state_old=0,triac_state=0,triac_state_old=0;
unsigned int triac=0;

roleta_t rol[MAX_ROL_N];
t_timer rol_timer;

#define MAX_SAMPLES 16 //pow of 2
unsigned short input_samples[MAX_SAMPLES];
unsigned short sample_cnt=0,samplefull=0;

t_timer time_needed(roleta_t *r,unsigned short pos_old,unsigned short pos_new);

void sample_input_states(void)
{
	if(samplefull)printf("\nerror sample_input_states");
	input_samples[sample_cnt%MAX_SAMPLES]=GPIOC->IDR;
	//printf("\n0x%x",input_samples[sample_cnt]);
	sample_cnt++;
	if(sample_cnt>=MAX_SAMPLES)
	{
		sample_cnt = 0;
		samplefull=1;
	}
}

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
	if(samplefull)//new samples of switches? - input readings
	{
		samplefull = 0;
	/*	for(inp=0;inp<N_OF_PINP;inp++)
		{
			samples=0;
			for(sample=0;sample<MAX_SAMPLES;sample++)
			{
				if(input_samples[sample]&INP_PINS[inp])samples++;
			}
			//if(inp<2)printf("\ninp:%d samples:%d",inp,samples);
			if(samples>(MAX_SAMPLES*3/4))
			{
				motor_state &= (~(1<<inp));
				tipke_states &= (~(1<<inp));
			}
			else
			{
				motor_state |= (1<<inp);
				if(((triac_state_old&(1<<inp))== 0) && ((triac_state&(1<<inp))== 0))
				{
					tipke_states |= (1<<inp);
				}
			}
			tipke_states &= ~(triac_state); // if triac is on, tipka is not pressed
		}
		if(motor_state_old!=motor_state)
		{
			//printf("\nmotor_state: 0x%x",motor_state);
		}
		motor_state_old = motor_state;
		if(tipke_states!=tipke_states_old)
		{
			//printf("\ntipke_states: 0x%x , triac_old0x%x , triac0x%x",tipke_states,triac_state_old,triac_state);
			for(inp=0;inp<N_OF_PINP;inp++)
			{
				if((1<<inp)&tipke_states)
				{
					if(!(inp&1))
					{
						rol[inp/2].tstate = tup;
						//rol[inp/2].tipkaup_last
						timeout(&(rol[inp/2].tipkaup_timer),0);
						triac &= ~(2<<(inp/2*2));//izklop kontra smeri
						//printf("\n tipkaup");
					}
					else
					{
						rol[inp/2].tstate = tdown;
						timeout(&(rol[inp/2].tipkadown_timer),0);
						triac &= ~(1<<(inp/2*2));//izklop kontra smeri
						//printf("\n tipkadown");
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
		}*/
		tipke_states_old = tipke_states;
		triac_state_old = triac_state;
	}


	/*
	if(timeout(&rol_timer,15))
	{
		for(roln=0;roln<ROL_N;roln++)
		{
			if(rol[roln].mode == mmanual) //manual
			{
				if(rol[roln].tipkaup_last )
				{
					if((rol[roln].tipkaup_last > SWITCH_MIN_TIME) && (rol[roln].tipkaup_last < SWITCH_MAX_TIME))
					{
						rol[roln].state = up;
						timeout(&(rol[roln].rol_timer),0);
						triac |= (1<<(roln*2));
						//printf("triac: 0x%x",triac);
					}
					else
					{
						rol[roln].state = idle;
					}
					rol[roln].tipkaup_last = 0;
					triac &= ~(2<<(roln*2));//izklop kontra smeri
				}
				if( rol[roln].tipkadown_last)
				{
					if((rol[roln].tipkadown_last > SWITCH_MIN_TIME) && (rol[roln].tipkadown_last < SWITCH_MAX_TIME))
					{

						rol[roln].state = down;
						rol[roln].tipkadown_last = 0;
						timeout(&(rol[roln].rol_timer),0);
						triac |= (2<<(roln*2));
						triac &= ~(1<<(roln*2)); //izklop kontra smeri
						//printf("triac: 0x%x",triac);
					}
					else
					{
						rol[roln].state = idle;
					}
					rol[roln].tipkadown_last = 0;
					triac &= ~(1<<(roln*2));//izklop kontra smeri
				}
				switch(rol[roln].state)
				{
					case idle:
						triac &= ~(3<<(roln*2));
						break;
					case up:
						if(timer_duration(&(rol[roln].rol_timer)) > rol[roln].fulltime_up )
						{
							rol[roln].state = idle;
							triac &= ~(3<<(roln*2));
						}
						break;
					case down:
						if(timer_duration(&(rol[roln].rol_timer)) > rol[roln].fulltime_down )
						{
							rol[roln].state = idle;
							triac &= ~(3<<(roln*2));
						}
						break;
				}
				if(rol[roln].state != rol[roln].state_old)printf("\n rol[%d] state:%d",roln,rol[roln].state);
				rol[roln].state_old = rol[roln].state;
				set_triac(triac);
			}
			else if(rol[roln].mode == mauto)  //auto
			{
				if(rol[roln].tipkaup_last || rol[roln].tipkadown_last)
				{
					rol_set_mode(rol,roln,mmanual);

				}
				switch(rol[roln].astate)
				{
				case idle:
					triac &= ~(3<<(roln*2));
					if(rol[roln].pos_cmd != rol[roln].rol_pos)
					{
						if(rol[roln].pos_cmd > rol[roln].rol_pos) //up
						{
							triac |= (1<<(roln*2));
						}
						else///down
						{
							triac |= (2<<(roln*2));
						}
						rol[roln].rol_end_time = time_needed(&(rol[roln]),rol[roln].rol_pos,rol[roln].pos_cmd);
						timeout(&(rol[roln].rol_timer),0);
						rol[roln].astate = move;
					}
					break;
				case cfg:
					triac |= (2<<(roln*2));
					if(timer_duration(&(rol[roln].rol_timer))>= rol[roln].fulltime_down)
					{
						rol[roln].astate = idle;
						triac &= ~(3<<(roln*2));
						rol[roln].rol_pos = 0;
					}
					break;
				case move:
					if(timer_duration(&(rol[roln].rol_timer))>= rol[roln].rol_end_time)
					{
						rol[roln].rol_end_time = 0;
						triac &= ~(3<<(roln*2));
						rol[roln].rol_pos = rol[roln].pos_cmd;
						rol[roln].astate = idle;
					}
					break;
				case up:
					break;
				case down:
					break;
				case fullup:
					break;
				case fulldown:
					break;
				}
				if(rol[roln].astate != rol[roln].astate_old)printf("\n rol[%d] astate:%d",roln,rol[roln].astate);
				rol[roln].astate_old = rol[roln].astate;
				set_triac(triac);
			}
		}
	}*/
}

int rol_set_mode(roleta_t *r,unsigned short roln,short mode)
{
	if(roln > MAX_ROL_N)return 0;
	if(mode == mmanual)
	{
		r[roln].mode = mmanual;
		r[roln].astate = idle;
		r[roln].state = idle;
		triac &= ~(3<<(roln*2));
		printf("\n rol[%d].mode -> manual!",roln);
		return 1;
	}
	else if(mode == mauto)
	{
		r[roln].mode = mauto;
		r[roln].astate = cfg;
		timeout(&(r[roln].rol_timer),0);
		r[roln].state = idle;
		triac &= ~(3<<(roln*2));
		printf("\n rol[%d].mode -> auto!",roln);
		return 1;
	}
	else printf("\n error 2148cf");
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
	if(rol[rol_n].astate == idle)
	{
		printf("R[%d]:%d% ",rol_n,pos);
		rol[rol_n].pos_cmd = pos/10*10;
	}
	return 0;
}



void cmd_set_triac(int argc,char ** argp)
{
	unsigned long outputs=0;
	if(argc>1)
	{
		outputs=strtol(argp[1],0,0);

		set_triac(outputs);
	}
	printf("\nset_triac:0x%x",outputs);
}

void cmd_rol_print(int argc,char ** argp)
{
	int roln=0;
	for(roln=0;roln<ROL_N;roln++)
	{
		printf("\n rol[%d]: mode:%d pos:%d",roln,rol[roln].mode,rol[roln].rol_pos);
	}

}

