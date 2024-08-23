#ifndef TESTERFSM_H_
#define TESTERFSM_H_

#include <asf.h>
#include "debounce.h"



void testerFsm_init (debouncePin_t* _botonSerializa);
void testerFsm_handler (void);
void testerFsm_analizarMpxh (uint8_t dataH, uint8_t dataL, uint8_t layer, uint8_t nbits);



#endif /* TESTERFSM_H_ */