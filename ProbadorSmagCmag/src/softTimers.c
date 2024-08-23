#include "../inc/softTimers.h"


uint32_t systemTicks = 0;

#define STATE_RUNNING       0
#define STATE_STOPPED       1






void softTimer_init (SoftTimer_t* timer, uint32_t interval)
{
	timer->state = STATE_RUNNING;
	timer->start = softTimer_getTicks();
	timer->interval = interval;
}


void softTimer_restart (SoftTimer_t* timer)
{
	timer->state = STATE_RUNNING;
	timer->start = softTimer_getTicks();
}


uint32_t softTimer_expired (SoftTimer_t* timer)
{
	if (timer->state == STATE_RUNNING)
	{
		if (systemTicks >= timer->start)
		return ((systemTicks - timer->start) > timer->interval);
		else
		return (((65535 - timer->start) + systemTicks) > timer->interval);
	}
	else
	{
		return 0;
	}
}


void softTimer_stop (SoftTimer_t* timer)
{
	timer->state = STATE_STOPPED;
}


void softTimer_handler (void)
{
	systemTicks ++;
}


uint32_t softTimer_getTicks (void)
{
	return systemTicks;
}