#include "../inc/uartRB.h"

static bool inTransmittingIsr = false;
static uint8_t dataToSend;


void uartRB_init (uartRB_t* uartRB, struct usart_module* instance, ringBuffer_t* rbTx, ringBuffer_t* rbRx, usart_callback_t callbackTx, usart_callback_t callbackRx)
{
	uartRB->usart_instance = instance;
	uartRB->rbTx = rbTx;
	uartRB->rbRx = rbRx;
	
	// Se configuran los callbacks de TX y RX
	usart_register_callback(instance, callbackTx, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(instance, callbackRx, USART_CALLBACK_BUFFER_RECEIVED);
	
	usart_enable_callback(instance, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(instance, USART_CALLBACK_BUFFER_RECEIVED);
	
	usart_read_buffer_job(instance,(uint8_t *)&(uartRB->rxChar), 1);
}


void uartRB_writeByte (uartRB_t* uartRB, uint8_t data)
{
	if (ringBuffer_getFreeSpace(uartRB->rbTx) > 0)
	{
		ringBuffer_put(uartRB->rbTx, data);
		
		// Se fuerza el envío.
		if (!inTransmittingIsr)
		uartRB_txHandler(uartRB);
	}
}


uint32_t uartRB_writeBytes (uartRB_t* uartRB, uint8_t* data, uint32_t len)
{
	uint32_t freeSpace, writtenBytes;
	uint32_t i;
	
	freeSpace = ringBuffer_getFreeSpace(uartRB->rbTx);

	if (len <= freeSpace)
	{
		// Hay espacio en el ringbuffer
		for (i = 0; i < len; i ++)
		ringBuffer_put(uartRB->rbTx, *(data+i));

		writtenBytes = len;
	}
	else
	{
		// No hay suficiente espacio por lo que se trunca data
		for (i = 0; i < freeSpace; i ++)
		ringBuffer_put(uartRB->rbTx, *(data+i));

		writtenBytes = freeSpace;
	}

	// Se envía el primer caracter
	if (!inTransmittingIsr)
	uartRB_txHandler(uartRB);
	
	return writtenBytes;
}


uint32_t uartRB_writeString (uartRB_t* uartRB, uint8_t* data)
{
	uint32_t freeSpace, writtenBytes;
	uint32_t i;
	uint32_t len;
	
	// Se determina el largo del string
	for (len = 0; data[len] != '\0'; len++);
	
	freeSpace = ringBuffer_getFreeSpace(uartRB->rbTx);

	if (len <= freeSpace)
	{
		// Hay espacio en el ringbuffer
		for (i = 0; i < len; i ++)
		ringBuffer_put(uartRB->rbTx, *(data+i));

		writtenBytes = len;
	}
	else
	{
		// No hay suficiente espacio por lo que se trunca data
		for (i = 0; i < freeSpace; i ++)
		ringBuffer_put(uartRB->rbTx, *(data+i));

		writtenBytes = freeSpace;
	}

	// Se envía el primer caracter
	if (!inTransmittingIsr)
	uartRB_txHandler(uartRB);
	
	return writtenBytes;
}


uint8_t uartRB_readByte (uartRB_t* uartRB)
{
	return ringBuffer_remove(uartRB->rbRx);
}


uint32_t uartRB_readBytes (uartRB_t* uartRB, uint8_t* data, uint32_t len)
{
	uint32_t bytesRead = 0;
	uint32_t bytesToRead = ringBuffer_getPending(uartRB->rbRx);
	

	if (ringBuffer_getPending(uartRB->rbRx) > 0)
	{
		for (bytesRead = 0; bytesRead < bytesToRead && bytesRead < len; bytesRead++) {
			*(data+bytesRead) = uartRB_readByte(uartRB);
		}
	}

	return bytesRead;
}


uint32_t uartRB_getFreeSpace (uartRB_t* uartRB)
{
	return ringBuffer_getFreeSpace(uartRB->rbTx);
}


uint32_t uartRB_getPendingBytes (uartRB_t* uartRB)
{
	return ringBuffer_getPending(uartRB->rbRx);
}

uint32_t uartRB_getTxPendingBytes (uartRB_t* uartRB)
{
	return ringBuffer_getPending(uartRB->rbTx);
}


void uartRB_txHandler (uartRB_t* uartRB)
{
	enum status_code res;
	
	inTransmittingIsr = true;

	if (ringBuffer_getPending(uartRB->rbTx) > 0)
	{
		if (usart_get_job_status(uartRB->usart_instance, USART_TRANSCEIVER_TX) == STATUS_OK)
		{
			dataToSend = ringBuffer_remove(uartRB->rbTx);

			do {
				res = usart_write_job(uartRB->usart_instance, (uint8_t *)&dataToSend);
			} while(res != STATUS_OK);

			//ringBuffer_remove(uartRB->rbTx);
		}
	}
	else {
		inTransmittingIsr = false;
	}
	
	
}


void uartRB_rxHandler (uartRB_t* uartRB)
{
	ringBuffer_put(uartRB->rbRx, uartRB->rxChar);
	usart_read_buffer_job(uartRB->usart_instance, (uint8_t *)&(uartRB->rxChar), 1);
}


void uartRB_txFlush (uartRB_t* uartRB)
{
	ringBuffer_flush(uartRB->rbTx);
}


void uartRB_rxFlush (uartRB_t* uartRB)
{
	ringBuffer_flush(uartRB->rbRx);
}