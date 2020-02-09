// *********** Definitions related to DS18S20 sensor
#ifndef _DS18S20_H
#define _DS18S20_H

#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#ifndef ONE_WIRE_BUS
#define ONE_WIRE_BUS 0  // DS18B20 pin
#warning "ONE_WIRE_BUS pin not defined. Using default value (0)."
#endif

#endif	//_DS18S20_H
