/*
 * mainTimer.h
 *
 * Created: 11/11/2016 16:55:34
 *  Author: mmondani
 */ 


#ifndef MAINTIMER_H_
#define MAINTIMER_H_

#include "asf.h"



#define MAIN_TIMER_RESOURCE			TC3


typedef enum {
	TIMER_250US = 0x01,
	TIMER_1MSEG = 0x02,
	TIMER_4MSEG = 0x04,
	TIMER_1SEG  = 0x08,
	TIMER_1MIN  = 0x10,
	TIMER_1HORA  = 0x20
}mainTimer_timer_t;


void mainTimer_init (void);
uint32_t mainTimer_expired (mainTimer_timer_t timer);
void maintTimer_clearExpired (mainTimer_timer_t timer);
uint32_t maintTimer_getRandom1 (void);
uint32_t maintTimer_getRandom2 (void);

#endif /* MAINTIMER_H_ */