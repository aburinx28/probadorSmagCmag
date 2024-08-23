#ifndef UARTRB_H_
#define UARTRB_H_

#include <asf.h>
#include "ringBuffer.h"

typedef struct  
{
	struct usart_module* usart_instance;
	ringBuffer_t* rbTx;
	ringBuffer_t* rbRx;
	uint8_t rxChar;
}uartRB_t;


void uartRB_init (uartRB_t* uartRB, struct usart_module* instance, ringBuffer_t* rbTx, ringBuffer_t* rbRx, usart_callback_t callbackTx, usart_callback_t callbackRx);
void uartRB_writeByte (uartRB_t* uartRB, uint8_t data);
uint32_t uartRB_writeBytes (uartRB_t* uartRB, uint8_t* data, uint32_t len);
uint8_t uartRB_readByte (uartRB_t* uartRB);
uint32_t uartRB_readBytes (uartRB_t* uartRB, uint8_t* data, uint32_t len);
uint32_t uartRB_getFreeSpace (uartRB_t* uartRB);
uint32_t uartRB_getPendingBytes (uartRB_t* uartRB);
uint32_t uartRB_getTxPendingBytes (uartRB_t* uartRB);
void uartRB_txHandler (uartRB_t* uartRB);
void uartRB_rxHandler (uartRB_t* uartRB);
void uartRB_txFlush (uartRB_t* uartRB);
void uartRB_rxFlush (uartRB_t* uartRB);


#endif /* UARTRB_H_ */