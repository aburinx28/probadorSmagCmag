#ifndef UTILITIES_H_
#define UTILITIES_H_


#include "asf.h"
#include "basicDefinitions.h"


uint8_t sacarLayer (uint8_t dato);
uint8_t toUpperCase (uint8_t ascii);
uint8_t traducirAsciiATlcd (uint8_t ascii);
uint8_t traducirTlcdAAscii (uint8_t tlcd);
uint8_t traductirHexaATeclado (uint8_t letra);


#endif 