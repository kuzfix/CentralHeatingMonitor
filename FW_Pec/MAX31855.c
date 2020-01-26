/*
 * MAX31855.c
 *
 * Created: 01-Dec-18 13:28:05
 *  Author: maticpi
 */ 
#include "MAX31855.h"

void MAX31855_Init(uint16_t width, uint16_t height)
{
  //MAX31855_DDR_MOSI |= (1<<MAX31855_BIT_MOSI);	//MOSI set as output - NO OUTPUT NECCESSARY
  MAX31855_DDR_MISO &= ~(1<<MAX31855_BIT_MISO);	//MISO set as input
  MAX31855_PORT_MISO &= ~(1<<MAX31855_BIT_MISO);	//pull-up on MISO off
  MAX31855_DDR_SCK |= (1<<MAX31855_BIT_SCK);	//SCK set as output
  MAX31855_DDR_CS1 |= (1<<MAX31855_BIT_CS1);	//CS set as output
  MAX31855_DDR_CS2 |= (1<<MAX31855_BIT_CS2);	//CS set as output
  MAX31855_DDR_CS3 |= (1<<MAX31855_BIT_CS3);	//CS set as output
  MAX31855_CS1_DESELECT();
  MAX31855_CS2_DESELECT();
  MAX31855_CS3_DESELECT();
  
  #ifndef SHARE_SPI
  MAX31855_SPCR = MAX31855_SPCR_value;
  MAX31855_SPSR = MAX31855_SPSR_value;
  #endif
}

//ret value: 0-ERR, 1- OK
//status_flags:
// bit 0 - ID -must be 0
// bit 1 - OPEN CIRCUIT: 1 - sensor disconnected, 0 - OK
//temp: temperature in °C
int MAX31855_ReadTemperature(int SensorN, double *temp, uint8_t *status_flags)
{
  union { int32_t val; struct { uint8_t B0; uint8_t B1; uint8_t B2; uint8_t B3; }; } in, out;
  uint8_t status;
  double ThotJ;
//  double TcoldJ;
  
  if (temp == NULL) return 0; //ERR
  
  #ifdef SHARE_SPI
  uint8_t backupSPCR = MAX31855_SPCR;
  uint8_t backupSPSR = MAX31855_SPSR;
  MAX31855_SPCR = MAX31855_SPCR_value;
  MAX31855_SPSR = MAX31855_SPSR_value;
  #endif //SHARE_SPI

  in.val=0;
  
  if      (SensorN == 1)  MAX31855_CS1_SELECT();
  else if (SensorN == 2)  MAX31855_CS2_SELECT();
  else                    MAX31855_CS3_SELECT();
  
  if (!(MAX31855_SPCR & (1<<DORD)))
  {
    MAX31855_SPDR = in.B3;
    while(!(MAX31855_SPSR & (1<<SPIF)));
    out.B3 = MAX31855_SPDR;
    MAX31855_SPDR = in.B2;
    while(!(MAX31855_SPSR & (1<<SPIF)));
    out.B2 = MAX31855_SPDR;
    MAX31855_SPDR = in.B1;
    while(!(MAX31855_SPSR & (1<<SPIF)));
    out.B1 = MAX31855_SPDR;
    MAX31855_SPDR = in.B0;
    while(!(MAX31855_SPSR & (1<<SPIF)));
    out.B0 = MAX31855_SPDR;
  }
  else
  {
    MAX31855_SPDR = in.B0;
    while(!(MAX31855_SPSR & (1<<SPIF)));
    out.B0 = MAX31855_SPDR;
    MAX31855_SPDR = in.B1;
    while(!(MAX31855_SPSR & (1<<SPIF)));
    out.B1 = MAX31855_SPDR;
    MAX31855_SPDR = in.B2;
    while(!(MAX31855_SPSR & (1<<SPIF)));
    out.B2 = MAX31855_SPDR;
    MAX31855_SPDR = in.B3;
    while(!(MAX31855_SPSR & (1<<SPIF)));
    out.B3 = MAX31855_SPDR;
  }
  
  if      (SensorN == 1)  MAX31855_CS1_DESELECT();
  else if (SensorN == 2)  MAX31855_CS2_DESELECT();
  else                    MAX31855_CS3_DESELECT();
  
  #ifdef SHARE_SPI
  MAX31855_SPCR = backupSPCR;
  MAX31855_SPSR = backupSPSR;
  #endif	//SHARE_SPI

  //TcoldJ = ( (double)( ((int16_t)out.val)>>4 ) )/4.0;
  //TcoldJ isn't needed, but can be useful to measure ambient temperature. 
  ThotJ = ((double)((out.val & 0xFFFC0000)>>18))/4.0;
  *temp = ThotJ;
  status = ((out.B2 << 3) & 0x08) | ((out.B0 << 0) & 0x07);
  if (status_flags != NULL) *status_flags = status;
  return (status != 0);
}