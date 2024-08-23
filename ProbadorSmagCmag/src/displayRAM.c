#include "inc/displayRAM.h"
#include "inc/mpxh.h"
#include "inc/project.h"
#include "inc/basicDefinitions.h"
#include "inc/utilities.h"


#define             MPX9_BLANKALL                    0x41
#define             MPX9_VOLVER                      0xC8		// número cualquiera. Después es traducido
#define             MPX9_LEFT                        0xC9       // por la función displayRAM_traductor al
#define             MPX9_RIGHT                       0xCA       // comando correspondiente del TLCD.


static uint8_t hayBlankAll;
static uint8_t dr[33];
static uint8_t counterDisplay;
static uint8_t longitudDisplay;
static uint8_t layerDisplay;

#define BUFFER_CMD_LEN 10
static uint8_t bufferCmd[BUFFER_CMD_LEN];
static uint8_t ptrCmdIn = 0, ptrCmdOut = 0;


static uint8_t hexAchar (uint8_t hex);



void displayRAM_init ( void )
{
	bit_set(counterDisplay, 6);
}


void displayRAM_cargarDR ( const uint8_t* mensaje, uint8_t layer )
{
    uint8_t i;
    // for ( i = 0 ; *(mensaje + i) != '\0' ; i++ )
    for ( i = 0 ; mensaje[i] != '\0' ; i++ )
    {
	    dr[i] = mensaje[i];
    }
    longitudDisplay = i;
    counterDisplay = 0;
    
    // Se borra el buffer de comandos
    ptrCmdIn = 0;
    ptrCmdOut = 0;
    
    hayBlankAll = 1;
	
	layerDisplay = layer;
}


void displayRAM_cargarDR_sinBlank ( const uint8_t* mensaje, uint8_t layer )
{
    displayRAM_cargarDR(mensaje, layer);
    
    hayBlankAll = 0;
}


uint8_t displayRAM_sacarChar ( void )
{
    if ( counterDisplay > 32 )
	    return (1);     // 1 --> terminó
    else
    {
	    if ( !mpxh_Ocupado() )   // entro si no esta ocupado
	    {
		    if ( !counterDisplay )      // si es 0 inicializo
		    {
			    if (hayBlankAll)
			    mpxh_ArmaMensaje( MPX9_BLANKALL, 0, layerDisplay, MPXH_BITS_9 );
			    counterDisplay++;
		    }
		    else if ( counterDisplay <= longitudDisplay )      // para no recorrer hasta 32 sin sentido
		    {
			    if (dr[counterDisplay-1] != '&')
			    {
				    mpxh_ArmaMensaje( (traducirAsciiATlcd(dr[ counterDisplay-1 ])), 0, layerDisplay, MPXH_BITS_9 );
				    counterDisplay++;
			    }
		    }
		    else
		    {
			    if (ptrCmdIn != ptrCmdOut)
			    {
				    mpxh_ArmaMensaje( bufferCmd[ptrCmdOut], 0, layerDisplay, MPXH_BITS_9 );

				    ptrCmdOut++;
				    if (ptrCmdOut > BUFFER_CMD_LEN)
				    ptrCmdOut = 0;
			    }
		    }
	    }
	    return (0);     // 0 --> cargué
    }
}


void displayRAM_detenerDisplay (void)
{
    counterDisplay = 33;
    ptrCmdIn = 0;
    ptrCmdOut = 0;
}


void displayRAM_setChar (uint8_t pos, uint8_t c)
{
    dr[pos] = c;
}


void displayRAM_replaceString (uint8_t pos, uint8_t* str, uint8_t ini, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < len; i++)
    {
	    dr[pos + i] = str[ini + i];
    }
}


void displayRAM_putU8Hex (uint8_t pos, uint8_t num, bool padding)
{
    uint8_t bcd;

    bcd = displayRAM_hex2bcd(num);

	displayRAM_putU8BCD(pos, bcd, padding);
}


void displayRAM_putU8BCD (uint8_t pos, uint8_t num, bool padding)
{
	if (padding == false && (num & 0xf0) == 0)
		displayRAM_setChar(pos, hexAchar(num & 0x0F));
	else {
		displayRAM_setChar(pos, hexAchar((num & 0xF0) >> 4));
		displayRAM_setChar(pos+1, hexAchar(num & 0x0F));
	}
}


uint8_t displayRAM_hex2bcd (uint8_t value)
{
    uint8_t ret = 0;

    while (value >= 10)
    {
	    value -= 10;

	    ret += 0x10;
    }

    ret += value;

    return ret;
}


void displayRAM_put100Hex (uint8_t pos, uint8_t num)
{
    uint8_t aux1 = 0;
    uint8_t aux2 = 0;
    uint8_t aux3 = 0;
    
    while (num >= 100)
    {
	    aux1++;
	    num = num - 100;
    }
    
    if (aux1 != 0)
		displayRAM_setChar(pos, aux1 + '0');
    else
		displayRAM_setChar(pos, ' ');
    

    while (num >= 10)
    {
	    aux2++;
	    num = num - 10;
    }
    
    if (aux2 != 0 || aux1 != 0)
		displayRAM_setChar(pos+1, aux2 + '0');
    else
		displayRAM_setChar(pos+1, ' ');
    
    
    while (num > 0)
    {
	    aux3++;
	    num = num - 1;
    }
    
    displayRAM_setChar(pos+2, aux3 + '0');
}


void displayRAM_putSigned100Hex (uint8_t pos, uint8_t valor)
{
    uint8_t aux1 = 0;
    uint8_t aux2 = 0;
    uint8_t aux3 = 0;
    uint8_t esNegativo = 0;
    uint8_t num;
    
    if (num < 0) {
	    esNegativo = 1;
	    num = valor ^ 0xFF;
	    num++;
    }
    else {
	    num = valor;
    }
    
    while (num >= 100)
    {
	    aux1++;
	    num = num - 100;
    }
    
    if (esNegativo) {
	    displayRAM_setChar(pos, '-');
	    pos++;
    }
    
    
    if (aux1 != 0)
		displayRAM_setChar(pos, aux1 + '0');
    else
		displayRAM_setChar(pos, ' ');
    
    pos++;
    

    while (num >= 10)
    {
	    aux2++;
	    num = num - 10;
    }
    
    if (aux2 != 0 || aux1 != 0)
		displayRAM_setChar(pos, aux2 + '0');
    else
		displayRAM_setChar(pos, ' ');
    
    pos++;
    
    
    while (num > 0)
    {
	    aux3++;
	    num = num - 1;
    }
    
    displayRAM_setChar(pos, aux3 + '0');
}


uint8_t displayRAM_bcd2hex (uint8_t value)
{
    uint8_t ret = 0;

    while (value >= 10)
    {
	    value -= 0x10;

	    ret += 10;
    }

    ret += value;

    return ret;
}


void displayRAM_volver (void)
{
    counterDisplay = 32;

    dr[counterDisplay - 1] = MPX9_VOLVER;
}


uint8_t displayRAM_hayChar (void)
{
    uint8_t ret = 0;

    if  ( (counterDisplay <= longitudDisplay && dr[counterDisplay-1] != '&') || ptrCmdIn != ptrCmdOut)
    ret = 1;

    return ret;
}


void displayRAM_cargarComando (uint8_t cmd)
{
	bufferCmd[ptrCmdIn] = cmd;
	
	ptrCmdIn++;
	if (ptrCmdIn > BUFFER_CMD_LEN)
	ptrCmdIn = 0;
}


static uint8_t hexAchar (uint8_t hex) 
{
	u8 ret = 0;
	
	if (hex <= 9)
		ret = hex + '0';
	else
		ret = hex - 0x0a + 'A';
		
	return ret;
}