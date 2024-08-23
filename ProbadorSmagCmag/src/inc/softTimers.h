/*
 * softTimers.h
 *
 * Created: 14/11/2016 17:12:30
 *  Author: mmondani
 */ 


#ifndef SOFTTIMERS_H_
#define SOFTTIMERS_H_

#include "asf.h"


typedef struct
{
	uint32_t start;
	uint32_t interval;
	uint32_t state;
}SoftTimer_t;

void softTimer_init (SoftTimer_t* timer, uint32_t interval);
void softTimer_restart (SoftTimer_t* timer);
uint32_t softTimer_expired (SoftTimer_t* timer);
void softTimer_stop (SoftTimer_t* timer);

void softTimer_handler (void);
uint32_t softTimer_getTicks (void);



#endif /* SOFTTIMERS_H_ */