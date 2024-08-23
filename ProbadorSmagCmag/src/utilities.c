#include "inc/utilities.h"


uint8_t sacarLayer (uint8_t dato)
{
	uint8_t i;
	
	for (i = 0; i < 8; i++) {
		if (bit_test(dato, i))
		break;
	}
	
	return i;
}


uint8_t toUpperCase (uint8_t ascii)
{
	uint8_t ret = ascii;
	
	if (ascii >= 'a' && ascii <= 'z')
	ret = ascii - 'a' + 'A';
	
	return ret;
}


uint8_t traducirAsciiATlcd (uint8_t ascii)
{
	uint8_t ret = 0x2b;
	
	if (ascii >= '0' && ascii <= '9')
	ret = (ascii - '0');

	else if (ascii >= 'A' && ascii <= 'Z')
	ret =  (ascii - 'A' + 0x0A);

	else if (ascii == ';')
	ret =  0x24;

	else if (ascii == '_')
	ret =  0x26;

	else if (ascii == 'c')      // cuadrado
	ret =  0x24;

	else if (ascii == ':')
	ret =  0x25;

	else if (ascii == 'Ñ')
	ret =  0x28;

	else if (ascii == '-')
	ret =  0x29;

	else if (ascii == '/')
	ret =  0x2A;

	else if (ascii == ' ')
	ret =  0x2B;

	else if (ascii == '*')
	ret =  0x2D;

	else if (ascii == ',')
	ret =  0x2E;

	else if (ascii == '.')
	ret =  0x2F;

	else if (ascii == '(')
	ret =  0x31;

	else if (ascii == ')')
	ret =  0x32;

	else if (ascii == 'u')      // Flecha arriba
	ret =  0x33;

	else if (ascii == 'd')      // Flecha abajo
	ret =  0x34;

	else if (ascii == '¿')
	ret =  0x35;

	else if (ascii == '?')
	ret =  0x36;

	else if (ascii == '¡')
	ret =  0x37;

	else if (ascii == '!')
	ret =  0x38;

	else if (ascii == 'a')          // A superindice
	ret =  0x39;

	else if (ascii == 'o')          // O superindice
	ret =  0x3A;

	else if (ascii == '"')
	ret =  0x3B;

	else if (ascii == '#')
	ret =  0x3C;

	else if (ascii == '+')
	ret =  0x3D;

	else if (ascii == '<')
	ret =  0x3E;

	else if (ascii == '>')
	ret =  0x3F;

	return ret;
}


uint8_t traducirTlcdAAscii (uint8_t tlcd)
{
	uint8_t ret = ' ';
	
	if (tlcd >= 0 && tlcd <= 9)
	ret = (tlcd + '0');

	else if (tlcd >= 0x0a && tlcd <= 0x23)
	ret =  (tlcd - 0x0A + 'A');

	else if (tlcd == 0x24)
	ret =  ';';

	else if (tlcd == 0x26)
	ret =  '_';

	else if (tlcd == 0x24)      // cuadrado
	ret =  'c';

	else if (tlcd == 0x25)
	ret =  ':';
	
	else if (tlcd == 0x26)
	ret =  '_';

	else if (tlcd == 0x28)
	ret =  'Ñ';

	else if (tlcd == 0x29)
	ret =  '-';

	else if (tlcd == 0x2A)
	ret =  '/';

	else if (tlcd == 0x2B)
	ret =  ' ';

	else if (tlcd == 0x2D)
	ret =  '*';

	else if (tlcd == 0x2E)
	ret =  ',';

	else if (tlcd == 0x2F)
	ret =  '.';

	else if (tlcd == 0x31)
	ret =  '(';

	else if (tlcd == 0x32)
	ret =  ')';

	else if (tlcd == 0x33)      // Flecha arriba
	ret =  'u';

	else if (tlcd == 0x34)      // Flecha abajo
	ret =  'd';

	else if (tlcd == 0x35)
	ret =  '?';

	else if (tlcd == 0x36)
	ret =  '¿';

	else if (tlcd == 0x37)
	ret =  '¡';

	else if (tlcd == 0x38)
	ret =  '!';

	else if (tlcd == 0x39)          // A superindice
	ret =  'a';

	else if (tlcd == 0x3A)          // O superindice
	ret =  'o';

	else if (tlcd == 0x3B)
	ret =  '"';

	else if (tlcd == 0x3C)
	ret =  '#';

	else if (tlcd == 0x3D)
	ret =  '+';

	else if (tlcd == 0x3E)
	ret =  '<';

	else if (tlcd == 0x3F)
	ret =  '>';

	return ret;
}


uint8_t traductirHexaATeclado (uint8_t hexa)
{
	uint8_t ret;
	
		
	if (hexa >= 0 && hexa <= 9)
		ret = hexa;
	else if (hexa == 0x0a || hexa == 0x0b || hexa == 0x0c)
		ret = 2;
	else if (hexa == 0x0d || hexa == 0x0e || hexa == 0x0f)
		ret = 3;
	
	
	return ret;
}