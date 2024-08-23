#include "../inc/mpxh.h"
#include "asf.h"
#include "../inc/mainTimer.h"
#include "../inc/softTimers.h"


void tc_callback (struct tc_module *const module_inst);


typedef union {
	uint8_t byte;
	struct {
		uint8_t useg250:1;
		uint8_t mseg1:1;
		uint8_t mseg4:1;
		uint8_t seg1:1;
		uint8_t min1:1;
		uint8_t hora1:1;
		uint8_t bit6:1;
		uint8_t bit7:1;
	} bits;
} _timers1;

volatile _timers1 timers1;
volatile unsigned char t0;
volatile unsigned char t01;
volatile unsigned char t1;
volatile unsigned char t2;
volatile unsigned char t3;
volatile unsigned char t4;
volatile unsigned char t_4ms; 

volatile uint32_t random1, random2;


struct tc_module tc_instance;


void mainTimer_init(void)
{
	// Se configura algún TC para que interrumpa cada 25us
	struct tc_config config;

	tc_get_config_defaults(&config);

	config.counter_size = TC_COUNTER_SIZE_8BIT;
	config.clock_source = GCLK_GENERATOR_0;
	config.clock_prescaler = TC_CLOCK_PRESCALER_DIV8;
	config.counter_8_bit.period = system_gclk_gen_get_hz(GCLK_GENERATOR_0) / (40000 * 8);
	
	tc_init(&tc_instance, TC5, &config);
	tc_enable(&tc_instance);
	
	tc_register_callback(&tc_instance,tc_callback, TC_CALLBACK_OVERFLOW);
	tc_enable_callback(&tc_instance, TC_CALLBACK_OVERFLOW);
	
	timers1.byte = 0;
}


uint32_t mainTimer_expired (mainTimer_timer_t timer)
{
	if (timers1.byte & timer)
		return 1;
	else
		return 0;
}


void maintTimer_clearExpired (mainTimer_timer_t timer)
{
	timers1.byte = timers1.byte & (~timer);
}


uint32_t maintTimer_getRandom1 (void)
{
	return random1;
}


uint32_t maintTimer_getRandom2 (void)
{
	return random2;
}



void tc_callback (struct tc_module *const module_inst)
{
	/*****************************************************************************/
	//  Cada 25 useg
	/*****************************************************************************/
	t0++;

	random1 ++;
	random2 --;

	if ( t0 == 10 )
	{
		t0 = 0;
		
		/*****************************************************************************/
		//  Cada 250 useg
		/*****************************************************************************/
		timers1.bits.useg250 = 1;
		
		
		port_pin_set_output_level(DUTY_INT, true);
		    
		mpxh_Analizar ();

		t01++;
		    
		if ( t01 == 4 )
		{
			t01 = 0;
			
			/*****************************************************************************/
			//  Cada 1 mseg
			/*****************************************************************************/
			timers1.bits.mseg1 = 1;
			

			softTimer_handler();    // base de tiempo de 1ms

			t1++;
			
			if ( t1 == 4 )
			{
				t1 = 0;
				
				/*****************************************************************************/
				//  Cada 4 mseg
				/*****************************************************************************/
				timers1.bits.mseg4 = 1;
				
				t2++;

				if ( t2 >= 250 )
				{
					t2 = 0;
					
					/*****************************************************************************/
					//  Cada 1 seg
					/*****************************************************************************/
					timers1.bits.seg1 = 1;
					
					t3++;
					
					if (t3 >= 60) {
						t3 = 0;
						
						/*****************************************************************************/
						//  Cada 1 minuto
						/*****************************************************************************/
						timers1.bits.min1 = 1;
						
						t4++;
						
						if (t4 >= 60) {
							t4 = 0;
							
							/*****************************************************************************/
							//  Cada 1 hora
							/*****************************************************************************/
							timers1.bits.hora1 = 1;
						}
						
					}
				}
			}

			// fin_incre
			    
		}
	}
	
	port_pin_set_output_level(DUTY_INT, false);
}