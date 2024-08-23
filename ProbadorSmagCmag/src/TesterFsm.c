#include "inc/TesterFsm.h"
#include "inc/SerialClientPC.h"
#include "inc/SerialClient4038.h"
#include "inc/SerialMessageCommands.h"
#include "inc/softTimers.h"
#include "inc/mpxh.h"
#include "inc/displayRAM.h"
#include "inc/project.h"

// Nombre de los dispositivos para probar
#define	PRODUCT_CMAG	1
#define PRODUCT_SMAG	2
#define PRODUCT_SOS		3
#define PRODUCT_SMP		4
#define PRODUCT_GEC		5
#define PRODUCT_SMAGE	6
// Cuenta una cantidad de veces si hubo activacion o desactivacion por presencia de imán
#define CONTADOR_EVENTOS_MIN	2
#define SHORT_CIRCUIT_COUNTER	2
// Variable utilizada para recorrer el menú de seleccion de devices
#define CANTIDAD_MAXIMA_DEVICES		4 // Para recorrer el array necesito contemplar el 0 como "cantidad_minima_devices"
#define CANTIDAD_MINIMA_DEVICES		0

typedef enum{
	FSM_INIT = 0,
	FSM_ESPERANDO_START,
	FSM_ALIMENTAR_DUT,
	FSM_ESPERANDO_PIRU, // Es a lo que llamamos BEGIN manda el 0D2 y espera respuesta, prueba de MPXH
	FSM_TERMINAR_PRUEBA,
	FSM_ERROR,
	/* Estas son nuevos estados que agrego para testeo */ 
	FSM_TESTEO_MAGNETICO,
	FSM_TESTEO_TAMPER,
	FSM_SELECCION_DEVICE,
	FSM_CORROBORO_DISPOSITIVO
}fsmState_t;

/* Estructura "efímera" para indicar la presion de las teclas					*/
/* Colocar la definición volatile _ap1 ap1 = 0;									*/
/* Colocar un clean de la estructura al finalizar el ciclo ap1.bytes = BAJO;	*/
typedef union {
	uint8_t bytes;
	struct {
		uint8_t ap_teclaModo:1;
		uint8_t ap_tecla:1;
		uint8_t ap_start:1;
		uint8_t ap_vino_6F9:1;
		uint8_t ap_vino_2A1:1;
		uint8_t ap_tecla_f:1;
		uint8_t ap_tecladown:1;
		uint8_t ap_teclaup:1;
	} bits;
} _ap1;

// Definición de la estructura para 'error_prueba_iman'
typedef struct {
	bool junto;    // Variable booleana 'junto'
	bool separado; // Variable booleana 'separado'
} ErrorPruebaIman;

// Definición de la estructura principal 'info'
typedef struct {
	ErrorPruebaIman error_prueba_iman; // Variable de tipo 'ErrorPruebaIman'
	bool prueba_exitosa_iman;  // Variable booleana 'prueba_iman'
	bool prueba_exitosa_tamper;
	bool dispo_correcto; // Variable booleana 'dispo_correcto'
	bool tiempo_expirado;
	bool corto_circuito;
} Info;

// Variable enum que indica en que dispositivo me encuentro actualmente analizando
typedef enum {
	SMAG = 0,
	CMAG = 1,
	SOS = 2,
	SMP = 3,
	GEC = 4,
	SMAGE = 5
} ActualDevice;

static fsmState_t fsmState;
static fsmState_t fsmState_previous;
static bool stateIn = true;
static bool stateOut = false;
static SoftTimer_t timerState;
static uint8_t ssid[40];
static uint8_t password[50];
static uint8_t id[8];
static uint8_t key[16];
static uint8_t versionMayor, versionMenor;
static debouncePin_t* botonSerializa;
static bool botonSerializaEstado;
static bool serializar;
static bool serializado;
static bool vinoStart;
static bool vinoPiruPiru;
static bool vinoGrave;
static bool vinoTeclaUp;
static bool vinoTeclaDown;
static bool vinoChange;
static bool vinoJunto;			// Para indicar la posicion del iman respecto al reed switch
static bool vinoSeparado;		// Para indicar la posicion del iman respecto al reed switch
static bool vinoTamper;			// Para indicar que se soltó el tamper
static bool vinoIdSmag;
static bool vinoIdSos;
static bool vinoIdGec;
static bool detected0090;
static uint8_t contador_eventos = 0;
static uint8_t* display_error;
static uint8_t producto;
static uint8_t bg96Signal = 0;
static uint8_t shortCircuitCounter = 0;
static uint8_t recorrer_devices = CANTIDAD_MINIMA_DEVICES;

static void gotoState (fsmState_t nextState);
static void errorPowerFeed (void);
static void display_productName (uint8_t pos);
static void showProductOK1 (void);
static void showProductOK2 (void);

/**************************************************************/
// Displays para el TLCD
/**************************************************************/
//												 0             1516             31
//												 |              ||              |
const uint8_t display_comenzando_prueba[] =     "   COMENZANDO      PRUEBA...";
const uint8_t display_error_corto[] =           "     ERROR           CORTO";
const uint8_t display_error_mpxh[] =            "     ERROR           MPXH";									
const uint8_t display_iniciando[]			=   "    INICIANDO       PROBADOR    ";
const uint8_t display_selec_device[]		=   "SELEC. DISPO.                   ";
const uint8_t display_smag[]				=	"SMAG ";
const uint8_t display_cmag[]				=	"CMAG ";
const uint8_t display_sos[]					=	"SOS  ";
const uint8_t display_smp[]					=	"SMP ";
const uint8_t display_gec[]					=	"GEC  ";
const uint8_t display_smage[]				=	"SMAGE  ";
const uint8_t display_test_magnet[]			=	"      TEST          MAGNETICO    ";
const uint8_t display_test_tamper[]			=	"      TEST            TAMPER     ";
const uint8_t display_error_magnet[]		=   "      ERROR         MAGNETICO    ";
const uint8_t display_error_device[]		=   "   DISPOSITIVO      INCORRECTO   ";
const uint8_t display_error_iman_separado[]	=   "ERROR APAGADO DE IMAN           ";
const uint8_t display_error_iman_junto[]	=   "ERROR ENCENDIDO DE IMAN         ";
const uint8_t display_error_default[]		=	"ERROR DEFAULT  ";
const uint8_t display_test_exitoso[]		=	"TEST OK! OPRIMA START O CHANGE  ";
const uint8_t display_error_recepcion_trama[] =	"ERROR RECEPCION TRAMA           ";

// Array de punteros a cadenas de caracteres. La idea es recorrerla con un valor incremental.
const uint8_t* display_messages[] = {
	display_smag,
	display_cmag,
	display_sos,
	display_smp,
	display_gec,
	display_smage
};

void testerFsm_init ( void )
{
	fsmState = FSM_INIT;
	fsmState_previous = FSM_INIT;
	
	serializar = false;
	port_pin_set_output_level(POWER, false);
}

void testerFsm_analizarMpxh (uint8_t dataH, uint8_t dataL, uint8_t layer, uint8_t nbits)
{
	switch(nbits) 
	{		
		/* Hago el analisis de los mensajes que espera recibir desde el TLCD en trama de 17 bits */
		
		/*****************************************************************************/
		//  RECEPCIÓN MPXH : 17 bits
		/*****************************************************************************/
		
		case MPXH_BITS_17:
				if (dataH == 0x00 && dataL == 0xE0) {					// 00E - Tecla pánico - Tecla Start
					vinoStart = true;
				}
				else if (dataH == 0x6C && dataL == 0x40) {				// 6C4 - Piru Piru
					vinoPiruPiru = true;
				}
				else if (dataH == 0x6F && dataL == 0xB0) {				// 6FB - Sonido grave
					vinoGrave = true;
				}
				else if (dataH == 0x61 && dataL == 0x00) {				// 610 - Tecla Up
					vinoTeclaUp = true;
				}
				else if (dataH == 0x61 && dataL == 0x10) {				// 611 - Tecla Down
					vinoTeclaDown = true;
				}
				else if (dataH == 0x00 && dataL == 0x0F) {				// 00F - Tecla Fire - Tecla otherDevice
					vinoChange = true;
				}
				else if (dataH == 0x14 && dataL == 0x10) {				//  - Iman Junto al sensor
					vinoJunto = true;
				}
				else if (dataH == 0x12 && dataL == 0x10) {				// - Iman Separado del sensor
					vinoSeparado = true;
				}
				else if (dataH == 0x21 && dataL == 0xA0) {				// - Tamper activado
					vinoTamper = true;
				}
			break;
			
		/*****************************************************************************/
		//  RECEPCIÓN MPXH : 15 bits
		/*****************************************************************************/
		
		case MPXH_BITS_15:
				/* Si detecta el mensaje de la familia de SMAG 0090 entonces empieza a escuchar la respuesta
					para asegurarnos de que device se trata.
				*/
				if (dataL == ID_SMAG_FAMILY) 
				{
					detected0090 = true;
				}else if (detected0090)
				{
					if (dataH == ID_SMAG_PGA_15BITS_MPXH) 
					{
						vinoIdSmag = true;
					} else if (dataH == ID_SOS_PGA_15BITS_MPXH) 
					{
						vinoIdSos = true;
					} else if (dataH == ID_GEC_PGA_15BITS_MPXH) 
					{
						vinoIdGec = true;
					}
					detected0090 = false; // Reseteo el estado después de procesar		
				}
			break;
			
		/* Agrego un caso de default en caso de error */
		default:
				display_error = display_error_recepcion_trama;
				gotoState(FSM_ERROR);
			break;
	}
}

void testerFsm_handler (void)
{
	uint8_t i;
	Info info;
	
	// Inicialmente actualDevice va a ser por defecto para la prueba de SMAG
	ActualDevice actualDevice = SMAG;
	
	/* Se analiza la variable fsm_state de la máquina de estados */
	switch (fsmState) 
	{
		case FSM_INIT:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
				
					softTimer_init(&timerState, 1000);
					displayRAM_cargarDR(display_iniciando, 0);
					displayRAM_cargarComando(TLCD_PERMANENT);
				}
				/**********************************************************************************************/
				if (softTimer_expired(&timerState)) 
				{				
					/* Si expiró el tiempo de presentación pasa al siguiente estado */
					gotoState(FSM_SELECCION_DEVICE); 			
				}
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
			
		case FSM_SELECCION_DEVICE:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
					softTimer_init(&timerState, 1000);

					/* Cargo primer pantalla del menú*/
					displayRAM_cargarDR(display_selec_device, 0); // Layer = 0 - Renglon 1
					displayRAM_cargarDR(display_messages[recorrer_devices], 1); // Layer = 1 - Renglon 2	
					displayRAM_cargarComando(TLCD_PERMANENT);
				}
				/**********************************************************************************************/
				/* Menú navegable para la seleccion del device a probar */
				if (softTimer_expired(&timerState)) 
				{
					/*Si pasa el timeout del menú entonces se resetea y */ 
					/* sigue en el mismo estado esperando la seleccion  */
					/* del dispositivo a testear                        */
					softTimer_restart(&timerState);
				
				}else if (vinoStart && !vinoTeclaUp && !vinoTeclaDown ) 
				{ /*Evalua tecla start mientras los otros comandos estén precionados*/
				
					vinoStart = false; // Limpio el flag que indica que se presionó la tecla Start
					gotoState(FSM_ALIMENTAR_DUT);
			
				}else if (vinoTeclaUp && !vinoTeclaDown && !vinoStart ) 
				{ /*Evalua tecla Up mientras los otros comandos estén precionados */
				
					vinoTeclaUp = false;
					if(recorrer_devices >= CANTIDAD_MAXIMA_DEVICES ) 
					{
						recorrer_devices = CANTIDAD_MINIMA_DEVICES; // Vuelve al device inicial
					}else
					{
						recorrer_devices++;	
					}
				}else if (vinoTeclaDown && !vinoTeclaUp && !vinoStart ) 
				{ /*Evalua tecla Down mientras los otros comandos estén precionados */			
					vinoTeclaDown = false;			
					if(recorrer_devices <= CANTIDAD_MINIMA_DEVICES) 
					{
						recorrer_devices = CANTIDAD_MAXIMA_DEVICES; // Vuelve al device final		
					}else
					{
						recorrer_devices--;
					}
				}						
				/* Actualizo el display segun la tecla que se haya presionado, osea el device elegido */
				displayRAM_cargarDR(display_messages[recorrer_devices], 1);
				displayRAM_cargarComando(TLCD_PERMANENT);			
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
					
		case FSM_ALIMENTAR_DUT:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
                
					displayRAM_cargarDR(display_comenzando_prueba, 0);
					displayRAM_cargarComando(TLCD_PERMANENT);
					port_pin_set_output_level(POWER, true);
					softTimer_init(&timerState, 2000);
					shortCircuitCounter = 0;
				}
				/**********************************************************************************************/
				if (port_pin_get_input_level(POWER_FEED) == false) 
				{
					shortCircuitCounter ++;
				
					if (shortCircuitCounter >= SHORT_CIRCUIT_COUNTER)
						errorPowerFeed(); // La funcion errorPowerFeed() arma el error para mostrar en display la falla.
				}
				else if (softTimer_expired(&timerState)) 
				{
					// Se limpia el buffer donde se recibe el 4038 porque hasta este momento estuvo
					// desalimentado y probablemente se recibió basura.
					//serialClient4038_flushRxBuffer();
					gotoState(FSM_ESPERANDO_PIRU);
				}
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
			
		case FSM_ESPERANDO_PIRU:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
				
					comandos1.bits.mandar_0d2 = 1; // Envío al device que se ponga en modo prueba
					vinoPiruPiru = false;
					shortCircuitCounter = 0;
				
					softTimer_init(&timerState, 2000);
				}
				/**********************************************************************************************/
				if (port_pin_get_input_level(POWER_FEED) == false) 
				{
					shortCircuitCounter ++;
				
					if (shortCircuitCounter >= SHORT_CIRCUIT_COUNTER)
						errorPowerFeed();
				}
				else if (softTimer_expired(&timerState)) 
				{
					display_error = display_error_mpxh;
					gotoState(FSM_ERROR);
				}
				else if (vinoPiruPiru) 
				{
				
					/* Si recibo una respuesta entonces es que funciona el cable mpxh */
					/* Accion que tenga que hacer */
					gotoState(FSM_CORROBORO_DISPOSITIVO);
				}
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
		
		case FSM_CORROBORO_DISPOSITIVO:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
							
					softTimer_init(&timerState, 2000);
				}
				/**********************************************************************************************/
				if (port_pin_get_input_level(POWER_FEED) == false) 
				{
					shortCircuitCounter ++;
				
					if (shortCircuitCounter >= SHORT_CIRCUIT_COUNTER)
						errorPowerFeed();
				}else if (softTimer_expired(&timerState)) 
				{
					display_error = display_error_mpxh;
					gotoState(FSM_ERROR);
				}else
				{
					switch (actualDevice) 
					{
						case SMAG:
								/* Corroboro el Id del Smag */
								if (vinoIdSmag)
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case CMAG:
								/* Corroboro el Id del Cmag */
								if (1/*TODO: Condicion*/)
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case SOS:
								/* Corroboro el Id del Sos */
								if (vinoIdSos)
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case SMP:
								/* Corroboro el Id del Smp */
								if (1/*TODO:Condicion*/)
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case SMAGE:
								/* Corroboro el Id del Smage */
								if (1/*TODO:Condicion*/)
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case GEC:
								/* Corroboro el Id del Gec */
								if (1/*TODO:Condicion*/)
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
						
						/* Caso default no es el dispositivo correcto */
						default:
								display_error = display_error_device;
								info.dispo_correcto = false;
								gotoState(FSM_ERROR);
							break;
					}							
				}
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
				
		case FSM_TESTEO_MAGNETICO:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
				
					softTimer_init(&timerState, 2000);
				
					/* Cargo pantalla del estado testeo de magnetico */
					displayRAM_cargarDR(display_test_magnet, 0);
			
					displayRAM_cargarComando(TLCD_PERMANENT);
					port_pin_set_output_level(IMAN_ENT, true);			// Enciendo el electroimán				
				}
				/**********************************************************************************************/
				if (port_pin_get_input_level(POWER_FEED) == false) 
				{
					shortCircuitCounter ++;				
					if (shortCircuitCounter >= SHORT_CIRCUIT_COUNTER)
					errorPowerFeed();	
				}else if (softTimer_expired(&timerState)) 
				{
					display_error = display_error_mpxh;
					gotoState(FSM_ERROR);
				}else{
					/* Detecta Magnético? se recibe codigo 141 ( se juntó ) o 121( se separó )? */
					port_pin_set_output_level(IMAN_ENT, true);			// Enciendo el electroimán - repito la acción
					if (vinoJunto)	/* Recibo el valor 0141 - Corroboro la recepción */
					{
						port_pin_set_output_level(IMAN_ENT, false);		// Apago el electroimán	
					}else if(vinoSeparado && vinoJunto) /* Recibo el valor 0121, y si estaba el 0141 significa que dió ok */
					{
						vinoJunto = false;
						vinoSeparado = false;
						contador_eventos++;
						if(contador_eventos >= CONTADOR_EVENTOS_MIN) // Con que sea mayor a 2 eventos se toma como válido
						{
							info.prueba_exitosa_iman = true;
							contador_eventos = 0; // Reseteo el contador de eventos de deteccion de iman
							gotoState(FSM_TESTEO_TAMPER);												
						}						
					}else // No hubo recepción de mensaje por lo que se considera falla de magnetico
					{
						info.prueba_exitosa_iman = false;
						if(!vinoSeparado)
						{
							info.error_prueba_iman.separado = true;
							display_error = display_error_iman_separado;
						}else if(!vinoJunto)
						{
							info.error_prueba_iman.junto = true;
							display_error = display_error_iman_junto;
						}
						gotoState(FSM_ERROR);
					}
				}	
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
		
		case FSM_TESTEO_TAMPER:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
				
					softTimer_init(&timerState, 2000);
				
					/* Cargo pantalla del estado testeo de magnetico */
					displayRAM_cargarDR(display_test_tamper, 0);
					displayRAM_cargarComando(TLCD_PERMANENT);	
				}
				/**********************************************************************************************/
				if (port_pin_get_input_level(POWER_FEED) == false) 
				{			
					shortCircuitCounter ++;	
					if (shortCircuitCounter >= SHORT_CIRCUIT_COUNTER)
					errorPowerFeed();
				}else if (softTimer_expired(&timerState)) 
				{			
					display_error = display_error_mpxh;
					gotoState(FSM_ERROR);
				}else
				{ 
					/* Corroboro que tenga sentido evaluar el tamper segun el dispositivo */
					if(actualDevice == CMAG) // Se hará la OR || con los otros devices que lleven tamper
					{
						/* Detecta Tamper? se recibe codigo 021A( se soltó )? - si se precionó no entrega nada el device */
						if (vinoTamper)	/* Recibo el valor 021A - Corroboro la recepción */
						{
							info.prueba_exitosa_tamper = true;
							gotoState(FSM_TERMINAR_PRUEBA);
						
						}else // No hubo recepción de mensaje por lo que se considera falla de magnetico
						{
							info.prueba_exitosa_tamper = false;
							gotoState(FSM_ERROR);
						}			
					}else
					{
						gotoState(FSM_TERMINAR_PRUEBA);
					}
				}
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
		
		case FSM_TERMINAR_PRUEBA:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
				
					port_pin_set_output_level(POWER, false); // Desenergizo el dispositivo
					displayRAM_cargarDR(display_test_exitoso, 0); // Presione START O CHANGE para continuar
					displayRAM_cargarComando(TLCD_PERMANENT);
				
					/* Limpio los flags de respuesta de dispositivo */
					vinoIdSmag = false;
					vinoIdSos = false;
					vinoIdGec = false;
				
					/* Limpio los flags de informe */
					info.corto_circuito = false;
					info.dispo_correcto = false;
					info.error_prueba_iman.junto = false;
					info.error_prueba_iman.separado = false;
					info.prueba_exitosa_iman = false;
					info.prueba_exitosa_tamper = false;
					comandos1.bits.mandar_pirupiru = 1; // Mando comando para generar sonido de aprobacion		
				
					softTimer_init(&timerState, 2000);
				}
				/**********************************************************************************************/
				if (softTimer_expired(&timerState)) 
				{				
					softTimer_restart(&timerState); // Hasta no recibir tecla START o CHANGE no sale del estado	
				}else if (vinoStart) 
				{				
					/* Vuelvo a empezar un analisis del mismo dispositivo */
					vinoStart = false;
					gotoState(FSM_ALIMENTAR_DUT);	
				}else if(vinoChange)
				{
					/* Quiero cambiar de dispositivo asique vuelvo al menú */
					vinoChange = false;
					gotoState(FSM_SELECCION_DEVICE);
				}
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
			
		case FSM_ERROR:
			if (stateIn)
            {
                stateIn = false;
                stateOut = false;	
				port_pin_set_output_level(POWER, false); // Al entrar en error desenergizo el dispositivo
	
				/* Cargo en pantalla el error ocurrido */
				displayRAM_cargarDR(display_error, 0);
				displayRAM_cargarComando(TLCD_PERMANENT);		
				softTimer_init(&timerState, 2000);		
				comandos1.bits.mandar_grave = 1; // Mando sonido grave de error
			}
            /**********************************************************************************************/
			if (softTimer_expired(&timerState))
			{
				softTimer_restart(&timerState); // Hasta no recibir tecla START o CHANGE no sale del estado
			}else if (vinoStart)
			{
				/* Vuelvo a empezar un analisis del mismo tipo de dispositivo */
				vinoStart = false;
				gotoState(FSM_ALIMENTAR_DUT);		
			}else if(vinoChange)
			{
				/* Quiero cambiar de dispositivo asique vuelvo al menú */
				vinoChange = false;
				gotoState(FSM_SELECCION_DEVICE);
			}
            /**********************************************************************************************/
            if (stateOut)
            {
                stateIn = true;
                stateOut = false;
            }
			break;
	
		default:
				display_error = display_error_default;
				gotoState(FSM_ERROR);	
			break;
	}
}


void gotoState (fsmState_t nextState)
{
	fsmState_previous = fsmState;
	fsmState = nextState;
	
	stateIn = false;
	stateOut = true;
}

void errorPowerFeed (void)
{
	port_pin_set_output_level(POWER, false);
	display_error = display_error_corto;
	gotoState(FSM_ERROR);
}

void display_productName (uint8_t pos)
{
	if(producto == PRODUCT_CMAG) 
	{
		for(uint8_t i = 0; i < sizeof(display_cmag) - 1; i++) 
		{
			displayRAM_setChar(pos + i, display_cmag[i]);
		}
	}
	else if(producto == PRODUCT_SMAG) 
	{
		for(uint8_t i = 0; i < sizeof(display_smag) - 1; i++) 
		{
			displayRAM_setChar(pos + i, display_smag[i]);
		}
	}
	else if(producto == PRODUCT_SOS) 
	{
		for(uint8_t i = 0; i < sizeof(display_sos) - 1; i++) 
		{
			displayRAM_setChar(pos + i, display_sos[i]);
		}
	}
	else if(producto == PRODUCT_SMP) 
	{
		for(uint8_t i = 0; i < sizeof(display_smp) - 1; i++) 
		{
			displayRAM_setChar(pos + i, display_smp[i]);
		}
	}
	else if(producto == PRODUCT_GEC) 
	{
		for(uint8_t i = 0; i < sizeof(display_gec) - 1; i++) 
		{
			displayRAM_setChar(pos + i, display_gec[i]);
		}
	}
	else if(producto == PRODUCT_SMAGE)
	{
		for(uint8_t i = 0; i < sizeof(display_smage) - 1; i++) 
		{
			displayRAM_setChar(pos + i, display_smage[i]);
		}
	}
}

