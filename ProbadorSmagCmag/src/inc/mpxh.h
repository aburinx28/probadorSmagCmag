/*
 * mpxh.h
 *
 * Created: 11/11/2016 16:35:06
 *  Author: mmondani
 */ 


#ifndef MPXH_H_
#define MPXH_H_

#include <asf.h>


// Configurar estos defines de acuerdo al intervalo de tiempo con que se llama a mpxh_Analizar.
// Se lo llama cada 250 usegs.
#define MPXH_MSEG						 4           // unidad de tiempo de t0
#define MPXH_MI_PRIORIDAD                10*MPXH_MSEG     // unidad de tiempo de t0
#define MPXH_T                           5           // unidad de tiempo de tbSend
#define MPXH_T9                          3           // unidad de tiempo de tbSend


typedef enum {
	MPXH_BITS_8 = 6,
	MPXH_BITS_4  = 5,
	MPXH_BITS_17 = 4,
	MPXH_BITS_16 = 3,
	MPXH_BITS_15 = 2,
	MPXH_BITS_12 = 1,
	MPXH_BITS_9 = 0
}MPXH_Bits_t;

/******************************************************************************/
// Módulo PGA
/******************************************************************************/
#define ID_SMAG_PGA_15BITS_MPXH		0b00000000   // G_XX - El tema que hay otros que entran con éste código CMAG y SMP
#define ID_SOS_PGA_15BITS_MPXH		0b00000001   // H_XX
#define ID_GEC_PGA_15BITS_MPXH		0b00000101   // L_XX
#define	ID_SMAG_FAMILY				0b10010000	 // Código de familia dataL = 0090


void mpxh_init (void);
void mpxh_Analizar ( void );
uint32_t mpxh_Ocupado ( void );
void mpxh_ArmaMensaje ( uint8_t dataH, uint8_t dataL, uint8_t layer, uint8_t nbits );
uint8_t mpxh_recibio ( uint8_t flag );
void mpxh_getRecibido ( uint8_t *dataH, uint8_t *dataL, uint8_t* layer );
uint8_t mpxh_tiempoIdle ( uint8_t tiempo);
uint8_t mpxh_tiempoLow ( uint8_t tiempo);
void mpxh_clearRecFlag ( MPXH_Bits_t flag );
void mpxh_forceMPXHLow ( void );
void mpxh_releaseMPXH ( void );
void mpxh_abortTx ( void );
void mpxh_clearSendFlag ( uint8_t flag );


#endif /* MPXH_H_ */