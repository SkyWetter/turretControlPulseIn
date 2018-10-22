
// ********   P U L S E   C O U N T E R   H E A D E R  *************
// ********   BEGIN


#ifndef _PULSECOUNTER_h
#define _PULSECOUNTER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif


// preprocessor directives
extern "C" {
#include "soc/pcnt_struct.h"
}
#include "driver/pcnt.h"

#define PCNT_TEST_UNIT PCNT_UNIT_0							// pulse counter 0 of 7
#define PCNT_H_LIM_VAL 32767								// high limit value 
#define PCNT_L_LIM_VAL -1									// low limit value

void initPcntTimers();
void IRAM_ATTR onTimer();
void initPcnt(byte pin);
void readPcntCounter_0();

// ******** E N D 
// ********   P U L S E   C O U N T E R   *************