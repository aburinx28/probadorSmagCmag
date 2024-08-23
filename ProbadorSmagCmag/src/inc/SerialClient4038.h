#ifndef serialClient4038_H_
#define serialClient4038_H_

#include "asf.h"
#include "uartRB.h"
#include "SerialMessagePoolManager.h"


void serialClient4038_init (struct usart_module * uart);
void serialClient4038_handler (void);
serialMessage_t* serialClient4038_getFreeMessageSlot (void);
void serialClient4038_releaseMessageSlot (serialMessage_t* msg);
uint32_t serialClient4038_putPayloadByte (serialMessage_t* msg, uint8_t data);
uint32_t serialClient4038_putPayloadBytes (serialMessage_t* msg, uint8_t* data, uint32_t len);
void serialClient4038_send (serialMessage_t* msg, uint8_t cmd);
void serialClient4038_flushRxBuffer (void);
uint32_t serialClient4038_messagePendingToRead (void);
serialMessage_t* serialClient4038_getMessageToRead (void);
void serialClient4038_removeMessageToRead (void);

#endif 