#ifndef DEBOUNCE_H
#define	DEBOUNCE_H

#include "asf.h"

#define         DEBOUNCE_PIN_ALTO_ACTIVO            (uint8_t)1
#define         DEBOUNCE_PIN_BAJO_ACTIVO            (uint8_t)0

#define         DEBOUNCE_PIN_SIN_FLANCO             (uint8_t)0
#define         DEBOUNCE_PIN_FLANCO_A_ACTIVO        (uint8_t)1
#define         DEBOUNCE_PIN_FLANCO_A_PASIVO        (uint8_t)2

typedef struct
{
   uint8_t pin;
   uint8_t nivelActivo;     // GPIO_ALTO_ACTIVO, GPIO_BAJO_ACTIVO
   uint8_t estado;          // BAJO = 0 y ALTO = 1
   uint8_t cont;            // acumulador
   uint8_t flanco;          // Cuando se produce un flanco
} debouncePin_t;

// Prototipos


void		 debouncePin_init       ( debouncePin_t* gpio, uint8_t nivelActivo, uint8_t pin );
void		 debouncePin_handler    ( debouncePin_t* gpio);
uint32_t     debouncePin_getEstado  ( debouncePin_t* gpio);
uint32_t     debouncePin_getFlanco  ( debouncePin_t* gpio);
void		 debouncePin_clearFlanco (debouncePin_t* gpio);

#endif	/* DEBOUNCE_H */

