#include "inc/SerialMessagePoolManager.h"


serialMessage_t messagePool[SERIAL_MESSAGE_POOL_POOL_LEN];
serialMessage_t* outputQueue[SERIAL_MESSAGE_POOL_POOL_LEN][2];
serialMessage_t* inputQueues[SERIAL_MESSAGE_POOL_POOL_LEN][2];



void serialMessagePool_init (void)
{
	for (int i = 0; i < SERIAL_MESSAGE_POOL_POOL_LEN; i++) {
		messagePool[i].free = 1;
		messagePool[i].payload_ptr = 0;
	}
	
	for (int j = 0 ; j < 2; j ++) {
		for (int i = 0; i < SERIAL_MESSAGE_POOL_OUTPUT_QUEUE_LEN; i++) {
			outputQueue[i][j] = NULL;
		}
	}
	
	for (int j = 0 ; j < 2; j ++) {
		for (int i = 0; i < SERIAL_MESSAGE_POOL_INPUT_QUEUE_LEN; i++) {
			inputQueues[i][j] = NULL;
		}
	}
}


serialMessage_t* serialMessagePool_getFreeSlot(void)
{
	serialMessage_t* ret = NULL;
	
	for (int i = 0; i < SERIAL_MESSAGE_POOL_OUTPUT_QUEUE_LEN; i++) {
		if (messagePool[i].free == 1) {
			messagePool[i].free = 0;
			
			ret = &(messagePool[i]);
			
			break;
		}
	}
	
	return ret;
}


void serialMessagePool_releaseSlot (serialMessage_t* msg)
{
	if (msg != NULL) {
		msg->payload_ptr = 0;
		msg->free = 1;
	}
}


uint32_t serialMessagePool_pendingOutputMessage (uint8_t outputInterface)
{
	uint32_t ret = 0;
	
	if (outputInterface >= 2)
		return 0;
	
	for (int i = 0; i < SERIAL_MESSAGE_POOL_OUTPUT_QUEUE_LEN; i++) {
		if (outputQueue[i][outputInterface] != NULL) {
			ret = 1;
			break;
		}
	}
	
	return ret;
}


void serialMessagePool_pushOutputQueue (serialMessage_t* msg, uint8_t outputInterface)
{
	if (outputInterface >= 2)
		return 0;
	
	for (int i = 0; i < SERIAL_MESSAGE_POOL_OUTPUT_QUEUE_LEN; i++) {
		if (outputQueue[i][outputInterface] == NULL) {
			outputQueue[i][outputInterface] = msg;
			break;
		}
	}
}


serialMessage_t* serialMessagePool_peekOutputQueue (uint8_t outputInterface)
{
	return (outputQueue[0][outputInterface]);
}


void serialMessagePool_popOutputQueue (uint8_t outputInterface)
{
	serialMessagePool_releaseSlot(outputQueue[0][outputInterface]);
	
	
	for (int i = 0; i < SERIAL_MESSAGE_POOL_OUTPUT_QUEUE_LEN - 1; i++) {
		outputQueue[i][outputInterface] = outputQueue[i+1][outputInterface];
	}
	
	outputQueue[SERIAL_MESSAGE_POOL_OUTPUT_QUEUE_LEN - 1][outputInterface] = NULL;
}


void serialMessagePool_flushOutputQueue (uint8_t outputInterface)
{
	for (int i = 0; i < SERIAL_MESSAGE_POOL_OUTPUT_QUEUE_LEN; i++) {
		outputQueue[i][outputInterface] = NULL;
	}
}



uint32_t serialMessagePool_pendingInputMessage (uint8_t outputInterface)
{
	uint32_t ret = 0;
	
	for (int i = 0; i < SERIAL_MESSAGE_POOL_INPUT_QUEUE_LEN; i++) {
		if (inputQueues[i][outputInterface] != NULL) {
			ret = 1;
			break;
		}
	}

	
	return ret;
}


void serialMessagePool_pushInputQueue (serialMessage_t* msg, uint8_t outputInterface)
{
	uint32_t ret = 0;
	
	for (int i = 0; i < SERIAL_MESSAGE_POOL_INPUT_QUEUE_LEN; i++) {
		if (inputQueues[i][outputInterface] == NULL) {
			inputQueues[i][outputInterface] = msg;
			break;
		}
	}
}


serialMessage_t* serialMessagePool_peekInputQueue (uint8_t outputInterface)
{
	return (inputQueues[0][outputInterface]);
}


void serialMessagePool_popInputQueue (uint8_t outputInterface)
{
	serialMessagePool_releaseSlot(inputQueues[0][outputInterface]);
	
	for (int i = 0; i < SERIAL_MESSAGE_POOL_INPUT_QUEUE_LEN - 1; i++) {
		inputQueues[i][outputInterface] = inputQueues[i+1][outputInterface];
	}
	
	inputQueues[SERIAL_MESSAGE_POOL_INPUT_QUEUE_LEN - 1][outputInterface] = NULL;
}