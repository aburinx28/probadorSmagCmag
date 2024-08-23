/*
 * SerialserialMessagePoolManager.h
 *
 * Created: 24/11/2020 09:36:01
 *  Author: mmondani
 */ 


#ifndef SERIALserialMessagePoolMANAGER_H_
#define SERIALserialMessagePoolMANAGER_H_


#include "asf.h"


#define		OUTPUT_INTERFACE_PC						0
#define		OUTPUT_INTERFACE_4038				1


#define		SERIAL_MESSAGE_POOL_POOL_LEN			10
#define		SERIAL_MESSAGE_POOL_OUTPUT_QUEUE_LEN	SERIAL_MESSAGE_POOL_POOL_LEN
#define		SERIAL_MESSAGE_POOL_INPUT_QUEUE_LEN		SERIAL_MESSAGE_POOL_POOL_LEN
#define		SERIAL_MESSAGE_POOL_MESSAGE_MAX_LEN		100




typedef struct
{
	uint8_t free;			// Indica si está libre para ser usado o no
	uint8_t command;
	uint8_t len;
	uint8_t payload[SERIAL_MESSAGE_POOL_MESSAGE_MAX_LEN];
	uint32_t payload_ptr;
}serialMessage_t;


void serialMessagePool_init (void);

serialMessage_t* serialMessagePool_getFreeSlot(void);
void serialMessagePool_releaseSlot (serialMessage_t* msg);

uint32_t serialMessagePool_pendingOutputMessage (uint8_t outputInterface);
void serialMessagePool_pushOutputQueue (serialMessage_t* msg, uint8_t outputInterface);
serialMessage_t* serialMessagePool_peekOutputQueue (uint8_t outputInterface);
void serialMessagePool_popOutputQueue (uint8_t outputInterface);
void serialMessagePool_flushOutputQueue (uint8_t outputInterface);

uint32_t serialMessagePool_pendingInputMessage (uint8_t outputInterface);
void serialMessagePool_pushInputQueue (serialMessage_t* msg, uint8_t outputInterface);
serialMessage_t* serialMessagePool_peekInputQueue (uint8_t outputInterface);
void serialMessagePool_popInputQueue (uint8_t outputInterface);


#endif /* SERIALserialMessagePoolMANAGER_H_ */