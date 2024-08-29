#include <asf.h>
#include "inc/project.h"
#include "inc/mpxh.h"
#include "inc/mainTimer.h"
#include "inc/debounce.h"
#include "inc/TesterFsm.h"
#include "inc/displayRAM.h"
#include "inc/SerialClientPC.h"
#include "inc/SerialClient4038.h"
#include "inc/softTimers.h"

static SoftTimer_t timerInit;

static void init_pins (void);

int main (void)
{
	uint8_t dataH, dataL, dataLayer;
	debouncePin_t botonSmag;
	debouncePin_t botonCmag;
	
	system_init();
	init_pins();
	mpxh_init();
	displayRAM_init();
	mainTimer_init();
	debouncePin_init(&botonSmag, DEBOUNCE_PIN_BAJO_ACTIVO, SMAG_ENT);
	debouncePin_init(&botonCmag, DEBOUNCE_PIN_BAJO_ACTIVO, CMAG_ENT);
	testerFsm_init();
	
	softTimer_init(&timerInit, 4000);
	while(!softTimer_expired(&timerInit)){
		

	}
	while (1) 
	{				
		if (mainTimer_expired(TIMER_4MSEG)) 
		{
			maintTimer_clearExpired(TIMER_4MSEG);
			
			/* Se pone un '1' en el pin DUTY_PAL para indicar que comienza la rutina de 4 segundos de código */
			port_pin_set_output_level(DUTY_PAL, true);
						
			/*****************************************************************************/
			//  RECEPCIÓN MPXH : 17 bits
			/*****************************************************************************/
			
			if ( mpxh_recibio ( MPXH_BITS_17 ) ) 
			{
				mpxh_clearRecFlag(MPXH_BITS_17);
				mpxh_getRecibido( &dataH, &dataL, &dataLayer );
				testerFsm_analizarMpxh(dataH, dataL, dataLayer, MPXH_BITS_17);
			}
			
			/*****************************************************************************/
			//  RECEPCIÓN MPXH : 15 bits
			/*****************************************************************************/
			if ( mpxh_recibio ( MPXH_BITS_15 ) ) 
			{	
				mpxh_clearRecFlag( MPXH_BITS_15 );
				mpxh_getRecibido( &dataH, &dataL, &dataLayer );
				testerFsm_analizarMpxh(dataH, dataL, dataLayer, MPXH_BITS_15);
			}
						
			/*****************************************************************************/
			//  HANDLERS
			/*****************************************************************************/
			debouncePin_handler (&botonSmag);
			debouncePin_handler (&botonCmag);
			getFinCarrera(&botonCmag, &botonSmag); // Se realiza el debounce de las teclas fisicas de los fines de carrera
			testerFsm_handler();
						
			/*****************************************************************************/
			//  ACCIONES
			/*****************************************************************************/
			
			/*****************************************************************************/
			//  SACARBUF MPXH - Se encarga de sacar el buffer mpxh cuando éste no está ocupado
			/*****************************************************************************/
			
			if (!mpxh_Ocupado()) 
			{
				/* Se envía el comando 0D20 Begin Provador */
				if (comandos1.bits.mandar_0d2) 
				{
					comandos1.bits.mandar_0d2 = 0;
					mpxh_ArmaMensaje(0x0d, 0x20, 0, MPXH_BITS_17);
				}
				else if (comandos1.bits.mandar_grave) 
				{
					comandos1.bits.mandar_grave = 0;
					mpxh_ArmaMensaje(0x6f, 0xb0, 0, MPXH_BITS_17);
				}
				else if (comandos1.bits.mandar_beep) 
				{
					comandos1.bits.mandar_beep = 0;
					mpxh_ArmaMensaje(0x6d, 0xa0, 0, MPXH_BITS_17);
				}
				else if (comandos1.bits.mandar_pirupiru) 
				{
					comandos1.bits.mandar_pirupiru = 0;
					mpxh_ArmaMensaje(0x6c, 0x40, 0, MPXH_BITS_17);
				}
				//else if (displayRAM_hayChar()) 
				//{
					//displayRAM_sacarChar();
				//}
				else if ( mpxh_tiempoIdle(4*MPXH_MSEG) )
				{
					if( displayRAM_hayChar() )
					{
						displayRAM_sacarChar();
					}
				}
			}
		}
		/* Se pone un '0' en el pin DUTY_PAL para indicar que terminó la rutina de 4 segundos de código */
		/* Para tener un testigo para testear de manera externa con otro dispositivo					*/		
		port_pin_set_output_level(DUTY_PAL, false);
	}
}


static void init_pins (void)
{
	struct port_config pin_conf;
	
	port_get_config_defaults(&pin_conf);
	
	/* Configuración de pines como entrada */
	
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	pin_conf.input_pull = SYSTEM_PINMUX_PIN_PULL_NONE;
	
	port_pin_set_config(POWER_FEED, &pin_conf);
	// port_pin_set_config(BOTON_SERIALIZAR, &pin_conf);
	port_pin_set_config(IMAN_ENT,&pin_conf);
	port_pin_set_config(SMAG_ENT,&pin_conf);
	port_pin_set_config(CMAG_ENT,&pin_conf);
	
	/* Configuración de pines como salida */
	
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	pin_conf.input_pull = SYSTEM_PINMUX_PIN_PULL_NONE;
	
	port_pin_set_config(POWER, &pin_conf);
	port_pin_set_config(DUTY_INT, &pin_conf);
	port_pin_set_output_level(DUTY_INT, false);
	port_pin_set_config(DUTY_PAL, &pin_conf);
	port_pin_set_output_level(DUTY_PAL, false);
	port_pin_set_config(IMAN_SAL,&pin_conf);
	port_pin_set_output_level(IMAN_SAL, false);
}