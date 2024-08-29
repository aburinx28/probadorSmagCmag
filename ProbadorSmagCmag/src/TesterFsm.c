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
// Cuenta una cantidad de veces si hubo activacion o desactivacion por presencia de im�n
#define CONTADOR_EVENTOS_MIN	2
#define SHORT_CIRCUIT_COUNTER	2
// Variable utilizada para recorrer el men� de seleccion de devices
#define CANTIDAD_MAXIMA_DEVICES		5 // Para recorrer el array necesito contemplar el 0 como "cantidad_minima_devices"
#define CANTIDAD_MINIMA_DEVICES		0

typedef enum{
	FSM_INIT = 0,
	FSM_ESPERANDO_START,
	FSM_ALIMENTAR_DUT,
	FSM_ESPERANDO_090, // Es a lo que llamamos BEGIN manda el 0D2 y espera respuesta, prueba de MPXH
	FSM_TERMINAR_PRUEBA,
	FSM_ERROR,
	/* Estas son nuevos estados que agrego para testeo */ 
	FSM_TESTEO_MAGNETICO,
	FSM_TESTEO_TAMPER,
	FSM_SELECCION_DEVICE,
	FSM_CORROBORO_DISPOSITIVO
}fsmState_t;

/* Estructura "ef�mera" para indicar la presion de las teclas					*/
/* Colocar la definici�n volatile _ap1 ap1 = 0;									*/
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

// Definici�n de la estructura para 'error_prueba_iman'
typedef struct {
	bool junto;    // Variable booleana 'junto'
	bool separado; // Variable booleana 'separado'
} ErrorPruebaIman;

// Definici�n de la estructura principal 'info'
typedef struct {
	ErrorPruebaIman error_prueba_iman;		// Variable de tipo 'ErrorPruebaIman'
	bool prueba_exitosa_iman;				// Variable booleana 'prueba_iman'
	bool prueba_exitosa_tamper;
	bool dispo_correcto;					// Variable booleana 'dispo_correcto'
	bool tiempo_expirado;
	bool corto_circuito;
	bool error_prueba_tamper;
	bool error_recepcion_trama;
	bool tiempo_expirado_check_dispo;
	bool tiempo_expirado_check_tamper;
	bool tiempo_expirado_check_magnet;
	bool tiempo_expirado_check_090;
	bool no_dispo;
} Info;

// Variable enum que indica en que dispositivo me encuentro actualmente analizando
typedef enum {
	SMAG = 0,
	CMAG = 1,
	SOS = 2,
	SMP = 3,
	GEC = 4,
	SMAGE = 5,
	UNDEFINED = 6
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
static bool vinoTamper;			// Para indicar que se solt� el tamper
static bool vinoIdSmag;
static bool vinoIdSos;
static bool vinoIdGec;
static bool detected0090;
static bool finCarreraSmag = false;
static bool finCarreraCmag = false;
static uint8_t contador_eventos = 0;
static uint8_t* display_error;
static uint8_t producto;
static uint8_t bg96Signal = 0;
static uint8_t shortCircuitCounter = 0;
static uint8_t recorrer_devices = CANTIDAD_MINIMA_DEVICES;
static Info info;
static ActualDevice actualDevice = UNDEFINED; // Inicialmente actualDevice va a ser por defecto para la prueba de SMAG
static void gotoState (fsmState_t nextState);
static void errorPowerFeed (void);
static void display_productName (uint8_t pos);
static void showProductOK1 (void);
static void showProductOK2 (void);

/**************************************************************/
// Displays para el TLCD
/**************************************************************/
//															0             1516             31
//															|              ||              |
const uint8_t display_comenzando_prueba[]				=   "   COMENZANDO      PRUEBA...    ";
const uint8_t display_error_corto[]						=   "     ERROR           CORTO      ";
const uint8_t display_error_mpxh[]						=   "     ERROR           MPXH		 ";									
const uint8_t display_iniciando[]						=   "    INICIANDO       PROBADOR    ";
const uint8_t display_selec_device[]					=   "SELECCIONAR    DISPOSITIVO      ";
const uint8_t display_smag[]							=	"SELEC. DISPO.   SMAG          ud";
const uint8_t display_cmag[]							=	"SELEC. DISPO.   CMAG          ud";
const uint8_t display_sos[]								=	"SELEC. DISPO.   SOS           ud";
const uint8_t display_smp[]								=	"SELEC. DISPO.   SMP           ud";
const uint8_t display_gec[]								=	"SELEC. DISPO.   GEC           ud";
const uint8_t display_smage[]							=	"SELEC. DISPO.   SMAGE         ud";
const uint8_t display_test_magnet[]						=	"     TEST          MAGNETICO    ";
const uint8_t display_test_tamper[]						=	"     TEST            TAMPER     ";
const uint8_t display_error_magnet[]					=   "     ERROR         MAGNETICO    ";
const uint8_t display_error_device[]					=   "DISPOSITIVO     INCORRECTO      ";
const uint8_t display_error_iman_separado[]				=   "ERROR APAGADO   DE IMAN         ";
const uint8_t display_error_iman_junto[]				=   "ERROR ENCENDIDO DE IMAN         ";
const uint8_t display_error_default[]					=	"ERROR DEFAULT					 ";
const uint8_t display_test_exitoso[]					=	"TEST OK! OPRIMA START O CHANGE  ";
const uint8_t display_error_recepcion_trama[]			=	"ERROR RECEPCION TRAMA           ";
const uint8_t display_error_tamper[]					=   "ERROR TAMPER			         ";
const uint8_t display_tiempo_expirado_check_dispo[]		=   "ERROR TIMEOUT   CHECK DISPO     ";
const uint8_t display_tiempo_expirado_check_tamper[]	=	"ERROR TIMEOUT   CHEKC TAMPER    ";
const uint8_t display_tiempo_expirado_check_magnet[]	=	"ERROR TIMEOUT   CHECK MAGNETICO ";
const uint8_t display_tiempo_expirado_check_090[]		=	"ERROR TIMEOUT   CHECK 090       ";
const uint8_t display_no_dispo[]						=	"NO SE DETECTA   DISPOSITIVO     ";

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

	port_pin_set_output_level(POWER, false);
}

void testerFsm_analizarMpxh (uint8_t dataH, uint8_t dataL, uint8_t layer, uint8_t nbits)
{
	switch(nbits) 
	{		
		/* Hago el analisis de los mensajes que espera recibir desde el TLCD en trama de 17 bits */
		
		/*****************************************************************************/
		//  RECEPCI�N MPXH : 17 bits
		/*****************************************************************************/
		
		case MPXH_BITS_17:
				if (dataH == 0x00 && dataL == 0xE0) {					// 00E - Tecla p�nico - Tecla Start
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
				else if (dataH == 0x00 && dataL == 0xF0) {				// 00F - Tecla Fire - Tecla Change
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
				else if(dataH == 0x09 && dataL == 0x00)
				{
					/* Si detecta el mensaje de la familia de SMAG 0090 entonces empieza a escuchar la respuesta
					para asegurarnos de que device se trata.
					*/
					detected0090 = true;
				}
			break;
			
		/*****************************************************************************/
		//  RECEPCI�N MPXH : 15 bits
		/*****************************************************************************/
		
		case MPXH_BITS_15:
				/* Si se detect� que es de la familia SMAG 0090 se eval�a cual fu� */
				if (detected0090)
				{
					if (dataH == ID_SMAG_PGA_15BITS_MPXH) 
					{
						vinoIdSmag = true;
						actualDevice = SMAG;
					}else if (dataH == ID_SOS_PGA_15BITS_MPXH) 
					{
						vinoIdSos = true;
						actualDevice = SOS;
					}else if (dataH == ID_GEC_PGA_15BITS_MPXH) 
					{
						vinoIdGec = true;
						actualDevice = GEC;
					}
					detected0090 = false; // Reseteo el estado despu�s de procesar		
				}
			break;
			
		/* Agrego un caso de default en caso de error */
		default:
				info.error_recepcion_trama = true; 
				gotoState(FSM_ERROR);
			break;
	}
}

void testerFsm_handler (void)
{
	uint8_t i;
	//Info info;
	
	//// Inicialmente actualDevice va a ser por defecto para la prueba de SMAG
	//ActualDevice actualDevice = SMAG;
	
	/* Se analiza la variable fsm_state de la m�quina de estados */
	switch (fsmState) 
	{
		case FSM_INIT:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
				
					softTimer_init(&timerState, 2000);
					displayRAM_cargarDR(display_iniciando, 0);
					displayRAM_cargarComando(TLCD_PERMANENT);
				}
				/**********************************************************************************************/
				if (softTimer_expired(&timerState)) 
				{				
					/* Si expir� el tiempo de presentaci�n pasa al siguiente estado */
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
					softTimer_init(&timerState, 3000);

					/* Cargo primer pantalla del men�*/
					//displayRAM_cargarDR(display_selec_device, 0); // Layer = 0
					recorrer_devices = CANTIDAD_MINIMA_DEVICES; // Vuelve al device inicial
					displayRAM_cargarDR(display_messages[recorrer_devices], 0); // Layer = 1	
					displayRAM_cargarComando(TLCD_PERMANENT);
				}
				/**********************************************************************************************/
				/* Men� navegable para la seleccion del device a probar */
				if (softTimer_expired(&timerState)) 
				{
					/*Si pasa el timeout del men� entonces se resetea y */ 
					/* sigue en el mismo estado esperando la seleccion  */
					/* del dispositivo a testear                        */
					softTimer_restart(&timerState);
				
				}else if (vinoStart && !vinoTeclaUp && !vinoTeclaDown ) 
				{ /*Evalua tecla start mientras los otros comandos est�n precionados*/
				
					vinoStart = false; // Limpio el flag que indica que se presion� la tecla Start
					gotoState(FSM_ALIMENTAR_DUT);
			
				}else if (vinoTeclaUp && !vinoTeclaDown && !vinoStart ) 
				{ /*Evalua tecla Up mientras los otros comandos est�n precionados */
				
					vinoTeclaUp = false;
					if(recorrer_devices >= CANTIDAD_MAXIMA_DEVICES ) 
					{
						recorrer_devices = CANTIDAD_MINIMA_DEVICES; // Vuelve al device inicial
					}else
					{
						recorrer_devices++;	
					}
					/* Actualizo el display segun la tecla que se haya presionado, osea el device elegido */
					displayRAM_cargarDR(display_messages[recorrer_devices], 0);
					displayRAM_cargarComando(TLCD_PERMANENT);
				}else if (vinoTeclaDown && !vinoTeclaUp && !vinoStart ) 
				{ /*Evalua tecla Down mientras los otros comandos est�n precionados */			
					vinoTeclaDown = false;			
					if(recorrer_devices <= CANTIDAD_MINIMA_DEVICES) 
					{
						recorrer_devices = CANTIDAD_MAXIMA_DEVICES; // Vuelve al device final		
					}else
					{
						recorrer_devices--;
					}
					/* Actualizo el display segun la tecla que se haya presionado, osea el device elegido */
					displayRAM_cargarDR(display_messages[recorrer_devices], 0);
					displayRAM_cargarComando(TLCD_PERMANENT);
				}						
							
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
					
					port_pin_set_output_level(POWER, true); // Alimento el dispositivo
					                
					comandos1.bits.mandar_0d2 = 1; // Env�o al device que se ponga en modo prueba
					comandos1.bits.mandar_beep = 1; // Sonido beep
					
					displayRAM_cargarDR(display_comenzando_prueba, 0);
					displayRAM_cargarComando(TLCD_PERMANENT);	
					
					softTimer_init(&timerState, 1000);
					shortCircuitCounter = 0;
				}
				/**********************************************************************************************/
				if (port_pin_get_input_level(POWER_FEED) == false) 
				{
					shortCircuitCounter++;
				
					if (shortCircuitCounter >= SHORT_CIRCUIT_COUNTER)
					{
						errorPowerFeed(); // La funcion errorPowerFeed() arma el error para mostrar en display la falla.
					}
				}
				else if (softTimer_expired(&timerState)) 
				{
					// Se limpia el buffer donde se recibe el 4038 porque hasta este momento estuvo
					// desalimentado y probablemente se recibi� basura.
					gotoState(FSM_ESPERANDO_090);
				}
				/**********************************************************************************************/
				if (stateOut)
				{
					stateIn = true;
					stateOut = false;
				}
			break;
			
		case FSM_ESPERANDO_090:
				if (stateIn)
				{
					stateIn = false;
					stateOut = false;
				
					//comandos1.bits.mandar_0d2 = 1; // Env�o al device que se ponga en modo prueba
					//vinoPiruPiru = false;
					//detected0090 = false;
					shortCircuitCounter = 0;
				
					softTimer_init(&timerState, 1000);
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
					//display_error = display_error_mpxh;
					//gotoState(FSM_ERROR);
					info.tiempo_expirado_check_090 = true;
					gotoState(FSM_ERROR);
				}
				else if ( actualDevice != UNDEFINED ) // Espero que se analice el mensaje mpxh
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
					//display_error = display_error_mpxh;
					info.tiempo_expirado_check_dispo = true;
					gotoState(FSM_ERROR);
					
				}else
				{
					/* Harcodeo flags */
					finCarreraCmag = true;
					finCarreraSmag = false;
					
					switch (recorrer_devices) 
					{
						case SMAG:
								/* Corroboro el Id del Smag */
								if ( vinoIdSmag == true && finCarreraCmag == false && finCarreraSmag == true && actualDevice == recorrer_devices )
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case CMAG:
								/* Corroboro el Id del Cmag */
								if ( vinoIdSmag == true && finCarreraCmag == true && finCarreraSmag == false && actualDevice == SMAG )
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case SOS:
								/* Corroboro el Id del Sos */
								if ( vinoIdSos && actualDevice == recorrer_devices )
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case SMP:
								/* Corroboro el Id del Smp */
								if ( vinoIdSmag == true && finCarreraCmag == false && finCarreraSmag == false && actualDevice == SMAG ) 
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case SMAGE:
								/* Corroboro el Id del Smage */
								if ( vinoIdSmag == true && finCarreraCmag == false && finCarreraSmag == false && actualDevice == SMAG )
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO);
								}
							break;
					
						case GEC:
								/* Corroboro el Id del Gec */
								if ( vinoIdGec && actualDevice == recorrer_devices )
								{
									info.dispo_correcto = true;
									gotoState(FSM_TESTEO_MAGNETICO); /*TODO: El GEC manda un codigo en un evento especial*/
								}
							break;
						
						/* Caso default no es el dispositivo correcto */
						default:
								//display_error = display_error_device;
								if( actualDevice == UNDEFINED)
								{
									info.no_dispo = true;	
								}
								else
								{
									info.dispo_correcto = false;	
								}
								
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
				
					/* Cargo pantalla del estado testeo de magnetico */
					displayRAM_cargarDR(display_test_magnet, 0);
					displayRAM_cargarComando(TLCD_PERMANENT);
					comandos1.bits.mandar_beep = 1; // Sonido beep
					
					softTimer_init(&timerState, 5000);
					port_pin_set_output_level(IMAN_ENT, true);			// Enciendo el electroim�n				
				}
				/**********************************************************************************************/
				if (port_pin_get_input_level(POWER_FEED) == false) 
				{
					shortCircuitCounter ++;				
					if (shortCircuitCounter >= SHORT_CIRCUIT_COUNTER)
					errorPowerFeed();	
				}else if (softTimer_expired(&timerState)) 
				{
					//display_error = display_error_mpxh;
					//info.tiempo_expirado = true;
					info.tiempo_expirado_check_magnet = true;						
					gotoState(FSM_ERROR);	
				}
				else
				{
					/* Detecta Magn�tico? se recibe codigo 141 ( se junt� ) o 121( se separ� )? */
					port_pin_set_output_level(IMAN_ENT, true);			// Enciendo el electroim�n - repito la acci�n
					if (vinoJunto && !vinoSeparado)	/* Recibo el valor 0141 - Corroboro la recepci�n */
					{
						vinoJunto = false;
						comandos1.bits.mandar_beep = 1; // Sonido beep
						port_pin_set_output_level(IMAN_ENT, false);		// Apago el electroim�n	
					}
					else if( !vinoJunto && vinoSeparado) /* Recibo el valor 0121, y si estaba el 0141 significa que di� ok */
					{
						//vinoJunto = false;
						vinoSeparado = false;
						contador_eventos++;
						comandos1.bits.mandar_beep = 1; // Sonido beep
						if(contador_eventos >= CONTADOR_EVENTOS_MIN) // Con que sea mayor a 2 eventos se toma como v�lido
						{
							info.prueba_exitosa_iman = true;
							contador_eventos = 0; // Reseteo el contador de eventos de deteccion de iman
							/* TODO: verificar segun device a que estado salta, if actual device etc */
							if( actualDevice == CMAG )
							{
								gotoState( FSM_TESTEO_TAMPER );	
							}
							else
							{
								gotoState( FSM_TERMINAR_PRUEBA );
							}													
						}						
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
					//display_error = display_error_mpxh;
					info.tiempo_expirado_check_tamper = true;
					gotoState(FSM_ERROR);
				}else
				{ 
					/* Corroboro que tenga sentido evaluar el tamper segun el dispositivo */
					if(actualDevice == CMAG) // Se har� la OR || con los otros devices que lleven tamper
					{
						/* Detecta Tamper? se recibe codigo 021A( se solt� )? - si se precion� no entrega nada el device */
						if (vinoTamper)	/* Recibo el valor 021A - Corroboro la recepci�n */
						{
							info.prueba_exitosa_tamper = true;
							gotoState(FSM_TERMINAR_PRUEBA);
						
						}else // No hubo recepci�n de mensaje por lo que se considera falla de magnetico
						{
							info.error_prueba_tamper = true;
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
					/* Quiero cambiar de dispositivo asique vuelvo al men� */
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
			if ( stateIn )
            {
                stateIn = false;
                stateOut = false;	
				port_pin_set_output_level(POWER, false); // Al entrar en error desenergizo el dispositivo
				shortCircuitCounter = 0;
				comandos1.bits.mandar_grave = 1; // Mando sonido grave de error
				
				/* Selecci�n del error segun flags info */
				if( info.corto_circuito == true )
				{
					display_error = display_error_corto;
				}
				else if( info.error_prueba_iman.junto == true )
				{
					display_error = display_error_iman_junto;
				}
				else if( info.error_prueba_iman.separado == true )
				{
					display_error = display_error_iman_separado;
				}
				else if( info.error_prueba_tamper )
				{
					display_error = display_error_tamper;
				}
				else if( info.dispo_correcto == false )
				{
					display_error = display_error_device;
				}
				else if( info.error_recepcion_trama == true )
				{
					display_error = display_error_recepcion_trama;
				}
				else if( info.tiempo_expirado_check_dispo == true )
				{
					display_error = display_tiempo_expirado_check_dispo;
				}
				else if( info.tiempo_expirado_check_tamper == true )
				{
					display_error = display_tiempo_expirado_check_tamper;
				}
				else if( info.tiempo_expirado_check_magnet == true )
				{
					display_error = display_tiempo_expirado_check_magnet;
				}
				else if( info.tiempo_expirado_check_090 == true)
				{
					display_error = display_tiempo_expirado_check_090;
				}
				else if( info.no_dispo == true )
				{
					display_error = display_no_dispo;
				}
				else if( info.tiempo_expirado == true )
				{
					display_error = display_error_mpxh;
				}
				else
				{
					display_error = display_error_default;
				}
				
				/* Limpio los flags de informe */
				info.corto_circuito = false;
				info.dispo_correcto = false;
				info.error_prueba_iman.junto = false;
				info.error_prueba_iman.separado = false;
				info.prueba_exitosa_iman = false;
				info.prueba_exitosa_tamper = false;
				info.error_prueba_tamper = false;
				info.error_recepcion_trama = false;
				info.tiempo_expirado_check_dispo = false;
				info.no_dispo = false;
				
				/* Cargo en pantalla el error ocurrido */
				displayRAM_cargarDR(display_error, 0);
				displayRAM_cargarComando(TLCD_PERMANENT);	
				softTimer_init(&timerState, 1000);		
			}
            /**********************************************************************************************/
			if ( softTimer_expired( &timerState ) )
			{
				softTimer_restart( &timerState ); // Hasta no recibir tecla START o CHANGE no sale del estado
			}else if ( vinoStart )
			{
				/* Vuelvo a empezar un analisis del mismo tipo de dispositivo */
				vinoStart = false;
				actualDevice = UNDEFINED; // Para que vuelva a validar por las dudas que sea el mismo dispositivo colocado
				gotoState( FSM_ALIMENTAR_DUT );		
			}else if( vinoChange )
			{
				/* Quiero cambiar de dispositivo asique vuelvo al men� */
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
	//display_error = display_error_corto;
	info.corto_circuito = true;
	gotoState(FSM_ERROR);
}

void getFinCarrera ( debouncePin_t* botonCmag, debouncePin_t* botonSmag )
{
	if( debouncePin_getEstado(&botonSmag) )
	{
		finCarreraSmag = true;
	}
	else if( debouncePin_getEstado(&botonCmag) )
	{
		finCarreraCmag = true;
	}else
	{
		finCarreraCmag = false;
		finCarreraSmag = false;
	}
}