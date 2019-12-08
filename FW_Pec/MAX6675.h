/*
 * Copyright (c) 2018 Matija Pirc
 *
 * Quick and dirty MAX6675 library
  * This software is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.
 */

#ifndef _MAX6675_h_
#define _MAX6675_h_

#include <avr/io.h>
#include <stddef.h>

#define SHARE_SPI

#define MAX6675_SPCR	SPCR0
#define MAX6675_SPSR	SPSR0
#define MAX6675_SPDR	SPDR0

//Tspi_clk >= 400ns => <2.5MHz
//F_CPU=16MHz/8 = 2MHz
#define 	MAX6675_SPCR_value ((0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<< CPOL) | (0<<CPHA) | (1<<SPR0))
#define 	MAX6675_SPSR_value (1<<SPI2X)

#define MAX6675_DDR_MISO	DDRB
#define MAX6675_PORT_MISO	PORTB
#define MAX6675_PIN_MISO	PINB
#define MAX6675_BIT_MISO	4
#define MAX6675_DDR_MOSI	DDRB
#define MAX6675_PORT_MOSI	PORTB
#define MAX6675_BIT_MOSI	3
#define MAX6675_DDR_SCK		DDRB
#define MAX6675_PORT_SCK	PORTB
#define MAX6675_BIT_SCK		5

#define MAX6675_DDR_CS		DDRE
#define MAX6675_PORT_CS		PORTE
#define MAX6675_BIT_CS		0

#define MAX6675_CS_SELECT()		MAX6675_PORT_CS &= ~(1<<MAX6675_BIT_CS)
#define MAX6675_CS_DESELECT()	MAX6675_PORT_CS |= (1<<MAX6675_BIT_CS)

void MAX6675_Init();
int MAX6675_ReadTemperature(double *temp, uint8_t *status_flags);
 

#endif  // _MAX6675_h_
