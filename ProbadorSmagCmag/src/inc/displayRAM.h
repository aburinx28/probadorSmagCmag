#ifndef DISPLAYRAM_H_
#define DISPLAYRAM_H_

#include "asf.h"


/*****************************************************************************/
//  COMANDOS
/*****************************************************************************/
#define             TLCD_VOLVER                     0x47
#define             TLCD_LEFT                       0x42
#define             TLCD_RIGHT                      0x43
#define             TLCD_PERMANENT                  0x46
#define             TLCD_HOME                       0x40



void displayRAM_init ( void );
void displayRAM_cargarDR ( const uint8_t* mensaje, uint8_t layer );
void displayRAM_cargarDR_sinBlank ( const uint8_t* mensaje, uint8_t layer );
void displayRAM_cargarComando (uint8_t cmd);
uint8_t displayRAM_sacarChar ( void );
void displayRAM_detenerDisplay (void);
void displayRAM_setChar (uint8_t pos, uint8_t c);
void displayRAM_replaceString (uint8_t pos, uint8_t* str, uint8_t ini, uint8_t len);
void displayRAM_putU8Hex (uint8_t pos, uint8_t num, bool padding);
void displayRAM_putU8BCD (uint8_t pos, uint8_t num, bool padding);
uint8_t displayRAM_hex2bcd (uint8_t value);
void displayRAM_put100Hex (uint8_t pos, uint8_t num);
void displayRAM_putSigned100Hex (uint8_t pos, uint8_t valor);
uint8_t displayRAM_bcd2hex (uint8_t value);
void displayRAM_volver (void);
uint8_t displayRAM_hayChar (void);


#endif