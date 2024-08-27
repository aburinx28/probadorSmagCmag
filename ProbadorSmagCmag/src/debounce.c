#include "../inc/debounce.h"



void debouncePin_init ( debouncePin_t* gpio, uint8_t nivelActivo, uint8_t pin )
{
    gpio->pin = pin;
    gpio->cont = 0;
    gpio->estado = 0;
    gpio->nivelActivo = nivelActivo;
    gpio->flanco = DEBOUNCE_PIN_SIN_FLANCO;
}


void debouncePin_handler (debouncePin_t* gpio)
{ 
    if ( port_pin_get_input_level(gpio->pin) == 1)
    {
		if (gpio->cont <= 4)
			gpio->cont ++;
    }
    else
    {
		if (gpio->cont > 0)
			gpio->cont --;
    }

    if ((gpio->cont) >= 4)
    {
        if ( gpio->nivelActivo == DEBOUNCE_PIN_ALTO_ACTIVO )
        {
            if (gpio->estado == 0)
                gpio->flanco = DEBOUNCE_PIN_FLANCO_A_ACTIVO;

            gpio->estado = 1;
        }
        else
        {
            if (gpio->estado == 1)
                gpio->flanco = DEBOUNCE_PIN_FLANCO_A_PASIVO;

            gpio->estado = 0;
        }
    }
    else if ((gpio->cont) == 0)
    {
        if ( gpio->nivelActivo == DEBOUNCE_PIN_ALTO_ACTIVO )
        {
            if (gpio->estado == 1)
                gpio->flanco = DEBOUNCE_PIN_FLANCO_A_PASIVO;

            gpio->estado = 0;
        }
        else
        {
            if (gpio->estado == 0)
                gpio->flanco = DEBOUNCE_PIN_FLANCO_A_ACTIVO;

            gpio->estado = 1;
        }
    }

}

uint32_t debouncePin_getEstado ( debouncePin_t* gpio)
{
    return ( gpio->estado );
}

uint32_t debouncePin_getFlanco (debouncePin_t* gpio)
{
    uint8_t aux_flanco;
    aux_flanco = gpio->flanco;

    return (aux_flanco);
}


void debouncePin_clearFlanco (debouncePin_t* gpio)
{
	gpio->flanco = DEBOUNCE_PIN_SIN_FLANCO;
}