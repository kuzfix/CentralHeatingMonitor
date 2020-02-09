/*
 * DS18B20management.h
 *
 * Created: 26/01/2020 14:50:36
 *  Author: maticpi
 */ 

#ifndef DS18B20MANAGEMENT_H_
#define DS18B20MANAGEMENT_H_

#include <avr/io.h>
#include <avr/eeprom.h>
#include "systime.h"
#include "kbd.h"
#include "config.h"
#include "onewire.h"
#include "ds18x20.h"
#include "crc8.h"
#include "Graphics.h"

void Read_DS18x20_Temperature();
void Read_DS18x20_Temperature_OneByOne();
void EraseSensorOrderEEPROM();
void SaveSensorOrderToEEPROM();
void ChangSensorOrder();
int ReadSensorOrderFromEEPROM();
//void WriteDefaultSensorOrderToEEPROM();

#endif /* DS18B20MANAGEMENT_H_ */