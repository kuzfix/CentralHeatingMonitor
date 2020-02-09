// Measure.h

#ifndef _MEASURE_h
#define _MEASURE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define SERIAL_DBG_PORT Serial
extern HardwareSerial SERIAL_DBG_PORT;

void Measure(void);

#endif

