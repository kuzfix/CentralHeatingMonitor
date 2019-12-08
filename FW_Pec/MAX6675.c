/*
 * MAX6675.c
 *
 * Created: 01-Dec-18 13:28:05
 *  Author: maticpi
 */ 
#include "MAX6675.h"

void MAX6675_Init(uint16_t width, uint16_t height)
{
  //MAX6675_DDR_MOSI |= (1<<MAX6675_BIT_MOSI);	//MOSI set as output - NO OUTPUT NECCESSARY
  MAX6675_DDR_MISO &= ~(1<<MAX6675_BIT_MISO);	//MISO set as input
  MAX6675_PORT_MISO &= ~(1<<MAX6675_BIT_MISO);	//pull-up on MISO off
  MAX6675_DDR_SCK |= (1<<MAX6675_BIT_SCK);	//SCK set as output
  MAX6675_DDR_CS |= (1<<MAX6675_BIT_CS);	//CS set as output
  MAX6675_CS_DESELECT();
  
  #ifndef SHARE_SPI
  MAX6675_SPCR = MAX6675_SPCR_value;
  MAX6675_SPSR = MAX6675_SPSR_value;
  #endif
}

//ret value: 0-ERR, 1- OK
//status_flags:
// bit 0 - ID -must be 0
// bit 1 - OPEN CIRCUIT: 1 - sensor disconnected, 0 - OK
//temp: temperature in °C
int MAX6675_ReadTemperature(double *temp, uint8_t *status_flags)
{
  union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
  uint8_t status;
  
  if (temp == NULL) return 0; //ERR
  
  #ifdef SHARE_SPI
  uint8_t backupSPCR = MAX6675_SPCR;
  uint8_t backupSPSR = MAX6675_SPSR;
  MAX6675_SPCR = MAX6675_SPCR_value;
  MAX6675_SPSR = MAX6675_SPSR_value;
  #endif //SHARE_SPI

  in.val=0;
  
  MAX6675_CS_SELECT();
  
  if (!(MAX6675_SPCR & (1<<DORD)))
  {
    MAX6675_SPDR = in.msb;
    while(!(MAX6675_SPSR & (1<<SPIF)));
    out.msb = MAX6675_SPDR;
    MAX6675_SPDR = in.lsb;
    while(!(MAX6675_SPSR & (1<<SPIF)));
    out.lsb = MAX6675_SPDR;
  }
  else
  {
    MAX6675_SPDR = in.lsb;
    while(!(MAX6675_SPSR & (1<<SPIF)));
    out.lsb = MAX6675_SPDR;
    MAX6675_SPDR = in.msb;
    while(!(MAX6675_SPSR & (1<<SPIF)));
    out.msb = MAX6675_SPDR;
  }
  
  MAX6675_CS_DESELECT();
  
  #ifdef SHARE_SPI
  MAX6675_SPCR = backupSPCR;
  MAX6675_SPSR = backupSPSR;
  #endif	//SHARE_SPI

  *temp = (out.val & 0x7FF8)/(8.0*4.0);
  status = (out.lsb >> 1) & 0x03;
  if (status_flags != NULL) *status_flags = status;
  return (status != 0);
}