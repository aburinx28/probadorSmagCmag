#ifndef PROJECT_H_
#define PROJECT_H_


#include "asf.h"

#define PROJECT_FIRMWARE_VERSION_MAYOR		1
#define PROJECT_FIRMWARE_VERSION_MENOR		3


union Comnados1{
	uint8_t byte;
	struct {
		uint8_t mandar_0d2:1;
		uint8_t mandar_grave:1;
		uint8_t mandar_beep:1;
		uint8_t mandar_pirupiru:1;
		uint8_t bit4:1;
		uint8_t bit5:1;
		uint8_t bit6:1;
		uint8_t bit7:1;
	} bits;
} comandos1;


#endif /* PROJECT_H_ */