#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include "asf.h"

typedef struct
{
	uint8_t* buffer;
	uint32_t len;
	uint32_t ptrIn;
	uint32_t ptrOut;
	volatile uint32_t count;
}ringBuffer_t;

void ringBuffer_init (ringBuffer_t* rb, uint8_t* buffer, uint32_t len);
uint32_t ringBuffer_getFreeSpace (ringBuffer_t* rb);
uint32_t ringBuffer_getPending (ringBuffer_t* rb);
void ringBuffer_put (ringBuffer_t* rb, uint8_t data);
uint8_t ringBuffer_remove (ringBuffer_t* rb);
uint8_t ringBuffer_peek (ringBuffer_t* rb);
void ringBuffer_flush (ringBuffer_t* rb);

#endif /* RINGBUFFER_H_ */