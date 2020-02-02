/*
 * Copyright (c) 2018 Matija Pirc
 *
 * Quick and dirty MAX31855 library
  * This software is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.
 */

#ifndef _MAX31855_h_
#define _MAX31855_h_

#include <avr/io.h>
#include <stddef.h>

#define SHARE_SPI

#define MAX31855_SPCR	SPCR0
#define MAX31855_SPSR	SPSR0
#define MAX31855_SPDR	SPDR0

//Tspi_clk >= 200ns => <5MHz
//F_CPU=18.432MHz/4 = 4.608MHz
#define MAX31855_SPCR_value ((0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<< CPOL) | (0<<CPHA) | (1<<SPR0))
#define MAX31855_SPSR_value (0<<SPI2X)

#define MAX31855_DDR_MISO	DDRB
#define MAX31855_PORT_MISO	PORTB
#define MAX31855_PIN_MISO	PINB
#define MAX31855_BIT_MISO	4
#define MAX31855_DDR_MOSI	DDRB
#define MAX31855_PORT_MOSI	PORTB
#define MAX31855_BIT_MOSI	3
#define MAX31855_DDR_SCK		DDRB
#define MAX31855_PORT_SCK	PORTB
#define MAX31855_BIT_SCK		5

#define MAX31855_DDR_CS1		DDRC
#define MAX31855_PORT_CS1		PORTC
#define MAX31855_BIT_CS1		2
#define MAX31855_DDR_CS2		DDRC
#define MAX31855_PORT_CS2		PORTC
#define MAX31855_BIT_CS2		3
#define MAX31855_DDR_CS3		DDRE
#define MAX31855_PORT_CS3		PORTE
#define MAX31855_BIT_CS3		3

#define MAX31855_CS1_SELECT()		MAX31855_PORT_CS1 &= ~(1<<MAX31855_BIT_CS1)
#define MAX31855_CS1_DESELECT()	MAX31855_PORT_CS1 |= (1<<MAX31855_BIT_CS1)
#define MAX31855_CS2_SELECT()		MAX31855_PORT_CS2 &= ~(1<<MAX31855_BIT_CS2)
#define MAX31855_CS2_DESELECT()	MAX31855_PORT_CS2 |= (1<<MAX31855_BIT_CS2)
#define MAX31855_CS3_SELECT()		MAX31855_PORT_CS3 &= ~(1<<MAX31855_BIT_CS3)
#define MAX31855_CS3_DESELECT()	MAX31855_PORT_CS3 |= (1<<MAX31855_BIT_CS3)

void MAX31855_Init();
int MAX31855_ReadTemperature(int SensorN, double *temp, uint8_t *status_flags);
 
#endif  // _MAX31855_h_
