#include "inc/SerialClient4038.h"
#include "inc/softTimers.h"


typedef enum{
	FSM_INIT = 0,
	FSM_IDLE,
	FSM_MSG_SEND
}fsmState_t;

static fsmState_t fsmState;
static fsmState_t fsmState_previous;

static bool stateIn = true;
static bool stateOut = false;

static SoftTimer_t timerState;

static struct usart_module* uart_module;
static uartRB_t uartRB;
static uint8_t rx_buffer[100];
static uint8_t tx_buffer[100];
static ringBuffer_t rbTx;
static ringBuffer_t rbRx;


static uint8_t dataIn[100];
static uint8_t dataIn_ptr;

static uint8_t payloadLength = 0;

static uint8_t dataOut[100];
static uint8_t dataOut_ptr;


static void generateFrameToSend (serialMessage_t* msg);
static void parseMessageIn (uint8_t* data);
static uint8_t getChecksum (uint8_t* msg, uint8_t len);
static void gotoState (fsmState_t nextState);
static void received (void);
static void sent (void);

static void usart_read_callback(struct usart_module *const usart_module);
static void usart_write_callback(struct usart_module *const usart_module);






void serialClient4038_init (struct usart_module * uart)
{
	uart_module = uart;
	
	dataIn_ptr = 0;
	fsmState = FSM_INIT;
	fsmState_previous = FSM_INIT;
	
	serialMessagePool_init();
}


void serialClient4038_handler (void)
{
	serialMessage_t* msg;
	
	
	switch (fsmState) {
		case FSM_INIT:
			if (stateIn)
            {
                stateIn = false;
                stateOut = false;

				ringBuffer_init(&rbTx, tx_buffer, 100);
				ringBuffer_init(&rbRx, rx_buffer, 100);
				uartRB_init(&uartRB, uart_module, &rbTx, &rbRx, usart_write_callback, usart_read_callback);
            }

            //**********************************************************************************************
			gotoState(FSM_IDLE);
			
            //**********************************************************************************************
            if (stateOut)
            {
                stateIn = true;
                stateOut = false;
            }
			
			break;
			
			
		case FSM_IDLE:
			if (stateIn)
            {
                stateIn = false;
                stateOut = false;
				
            }

            //**********************************************************************************************
			if (serialMessagePool_pendingOutputMessage(OUTPUT_INTERFACE_4038) == 1)
				gotoState(FSM_MSG_SEND);
				
            //**********************************************************************************************
            if (stateOut)
            {
                stateIn = true;
                stateOut = false;
            }
			
			break;
			
			
		case FSM_MSG_SEND:
			if (stateIn)
            {
                stateIn = false;
                stateOut = false;
				
				msg = serialMessagePool_peekOutputQueue(OUTPUT_INTERFACE_4038);
				generateFrameToSend (msg);
            }

            //**********************************************************************************************
			// Cuando termina de enviarse el mensaje, vuelvo a IDLE
			if (uartRB_getTxPendingBytes(&uartRB) == 0) {
				serialMessagePool_popOutputQueue(OUTPUT_INTERFACE_4038);
				gotoState(FSM_IDLE);
			}
            //**********************************************************************************************
            if (stateOut)
            {
                stateIn = true;
                stateOut = false;
            }
			
			break;
	}
}



serialMessage_t* serialClient4038_getFreeMessageSlot (void)
{
	return serialMessagePool_getFreeSlot();
}


void serialClient4038_releaseMessageSlot (serialMessage_t* msg)
{
	serialMessagePool_releaseSlot(msg);
}


uint32_t serialClient4038_putPayloadByte (serialMessage_t* msg, uint8_t data)
{
	uint32_t i;
	
	if (msg->payload_ptr > SERIAL_MESSAGE_POOL_MESSAGE_MAX_LEN) {
		return 0;
	}
	
	msg->payload[msg->payload_ptr] = data;
	msg->payload_ptr++;
	
	return 1;
}


uint32_t serialClient4038_putPayloadBytes (serialMessage_t* msg, uint8_t* data, uint32_t len)
{
	uint32_t i;

	for(i = 0; i < len; i++) {
		if (msg->payload_ptr > SERIAL_MESSAGE_POOL_MESSAGE_MAX_LEN) {
			break;
		}
		
		msg->payload[msg->payload_ptr] = data[i];
		msg->payload_ptr++;
	}
	
	return i;
}


void serialClient4038_send (serialMessage_t* msg, uint8_t cmd)
{
	msg->command = cmd;
	
	serialMessagePool_pushOutputQueue(msg, OUTPUT_INTERFACE_4038);
}


void serialClient4038_flushRxBuffer (void)
{
	uartRB_rxFlush(&uartRB);
	dataIn_ptr = 0;
}


uint32_t serialClient4038_messagePendingToRead (void)
{
	return serialMessagePool_pendingInputMessage(OUTPUT_INTERFACE_4038);
}


serialMessage_t* serialClient4038_getMessageToRead (void)
{
	return serialMessagePool_peekInputQueue(OUTPUT_INTERFACE_4038);
}


void serialClient4038_removeMessageToRead (void)
{
	serialMessagePool_popInputQueue(OUTPUT_INTERFACE_4038);	
}





static void received (void)
{
	uint32_t sum = 0;
	
	
	for (uint32_t i = 0; i < uartRB_getPendingBytes(&uartRB); i ++) {
		dataIn[dataIn_ptr] = uartRB_readByte(&uartRB);
		
		// Si es el segundo byte de la trama, es el largo el payload
		if (dataIn_ptr == 1)
			payloadLength = dataIn[dataIn_ptr];
			
		dataIn_ptr ++;
	}
	
	// Se chequea si se terminó de recibir una trama
	if ((dataIn_ptr >= (payloadLength + 4)) && dataIn[dataIn_ptr - 1] == 0x85) {
		
		for (uint32_t i = 0; i < (dataIn_ptr - 1); i++) {
			sum += dataIn[i];
		}
		
		dataIn_ptr = 0;
		payloadLength = 0;
		
		if ((sum & 0x000000ff) == 0xff) {
			// Está bien el checksum
			parseMessageIn(dataIn);
		}
		else {
			// Hay un error en el checksum
			
		}
	}
}


static void sent (void)
{
	
}


static void generateFrameToSend (serialMessage_t* msg)
{
	dataOut_ptr = 0;
	
	uint8_t len = msg->payload_ptr;
	
	dataOut[dataOut_ptr] = msg->command;
	dataOut_ptr ++;
	len++;
	
	dataOut[dataOut_ptr] = msg->payload_ptr;
	dataOut_ptr ++;
	len++;
	
	for (uint8_t i = 0; i < msg->payload_ptr; i++) {
		dataOut[dataOut_ptr] = msg->payload[i];
		dataOut_ptr ++;
	}
	
	dataOut[dataOut_ptr] = getChecksum (dataOut, len);
	dataOut_ptr ++;
	
	dataOut[dataOut_ptr] = 0x85;
	dataOut_ptr ++;
	
	
	
	uartRB_writeBytes(&uartRB, dataOut, dataOut_ptr);
}


static void parseMessageIn (uint8_t* data)
{
	serialMessage_t* msg;
	uint32_t index = 0;
	
	
	
	msg = serialMessagePool_getFreeSlot();
	

	if (msg != NULL) {

		msg->command = data[index];
		index ++;
		
		msg->len = data[index];
		index++;
		
		for (uint32_t i = 0; i < msg->len; i++) {
			msg->payload[i] = data[i + 2];
			index ++;
		}
		
		
		serialMessagePool_pushInputQueue(msg, OUTPUT_INTERFACE_4038);
	}
}


static uint8_t getChecksum (uint8_t* msg, uint8_t len)
{
	uint32_t sum = 0;
	uint8_t checksum = 0;
	
	
	for (int i = 0; i < len; i++) {
		sum += msg[i];
	}
	
	checksum = sum & 0x000000ff;
	
	return (0xff - checksum);
}


static void gotoState (fsmState_t nextState)
{
	fsmState_previous = fsmState;
	fsmState = nextState;
	stateIn = false;
	stateOut = true;
}



static void usart_read_callback(struct usart_module *const usart_module)
{
	uartRB_rxHandler(&uartRB);
	
	received();
}

static void usart_write_callback(struct usart_module *const usart_module)
{
	uartRB_txHandler(&uartRB);
	
	sent();
}