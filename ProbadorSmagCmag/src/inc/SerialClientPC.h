#ifndef serialClientPC_H_
#define serialClientPC_H_

#include "asf.h"
#include "uartRB.h"
#include "SerialMessagePoolManager.h"


void serialClientPC_init (struct usart_module * uart);
void serialClientPC_handler (void);
serialMessage_t* serialClientPC_getFreeMessageSlot (void);
void serialClientPC_releaseMessageSlot (serialMessage_t* msg);
uint32_t serialClientPC_putPayloadByte (serialMessage_t* msg, uint8_t data);
uint32_t serialClientPC_putPayloadBytes (serialMessage_t* msg, uint8_t* data, uint32_t len);
void serialClientPC_send (serialMessage_t* msg, uint8_t cmd);
uint32_t serialClientPC_messagePendingToRead (void);
serialMessage_t* serialClientPC_getMessageToRead (void);
void serialClientPC_removeMessageToRead (void);

#endif 