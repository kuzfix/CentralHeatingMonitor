#ifndef _ADS1118_ADC_H_
#define _ADS1118_ADC_H_

#include "config.h"
#include <SPI.h>

#ifndef CS  
#define CS      16
#warning "CS pin not defined. Using default value (16)."
#endif
#ifndef MISO
#define MISO    12    
#warning "MISO pin not defined. Using default value (12)."
#endif
#ifndef MOSI
#define MOSI    13 
#warning "MOSI pin not defined. Using default value (13)."
#endif
#ifndef SCK
#define SCK     14
#warning "SCK pin not defined. Using default value (14)."
#endif

/* ALTERNATIVNA KONFIGURACIJA PINOV NA ESP8266
#define CS      0
#define MISO    7    
#define MOSI    8 
#define SCK     6
*/

  //Used ADS1118 ADC chip - http://www.ti.com/lit/ds/symlink/ads1118.pdf
  #define ESP_SPI_settings SPISettings(1000000, MSBFIRST, SPI_MODE1) //Max SPI clock 4Mhz, MSB first(Set to 1MHz to be on the safe side), CLK idle = LOW, Data change at falling edge
  
  #define ADS_Config_default 0x858B    //default cettings of the config register
  
#define ADS_DATARATE	7
  //iADS_Config_regsiter  = (0<<SS) | (1<<MUX) | (4<<PGA) | (0<<MODE) | (ADS_DATARATE<<DR) | (0<<TS_MODE) | (1<<PULL_UP_EN) | (1<<NOP) | (1<<Reeserved)
  #define ADS_Config_ADC0 ((1<<SS) | (1<<MUX) | (3<<PGA) | (0<<MODE) | (ADS_DATARATE<<DR) | (0<<TS_MODE) | (1<<PULL_UP_EN) | (1<<NOP) | (1<<Reeserved))
  #define ADS_Config_ADC1 ((1<<SS) | (2<<MUX) | (3<<PGA) | (0<<MODE) | (ADS_DATARATE<<DR) | (0<<TS_MODE) | (1<<PULL_UP_EN) | (1<<NOP) | (1<<Reeserved))
  #define ADS_Config_ADC2 ((1<<SS) | (3<<MUX) | (3<<PGA) | (0<<MODE) | (ADS_DATARATE<<DR) | (0<<TS_MODE) | (1<<PULL_UP_EN) | (1<<NOP) | (1<<Reeserved))
  //#define ADS_Config_ADC3 ((1<<SS) | (7<<MUX) | (2<<PGA) | (0<<MODE) | (ADS_DATARATE<<DR) | (0<<TS_MODE) | (1<<PULL_UP_EN) | (1<<NOP) | (1<<Reeserved))
  #define ADS_Config_Temp ((1<<SS) | (0<<MUX) | (2<<PGA) | (0<<MODE) | (ADS_DATARATE<<DR) | (1<<TS_MODE) | (1<<PULL_UP_EN) | (1<<NOP) | (1<<Reeserved))
  
  // ADS1118 config register values
  #define SS 15         /*  0 = No effect
                            1 = Start a single conversion (when in power-down state)  */
  #define MUX 12        /*  000 = AIN P is AIN0 and AIN N is AIN1 (default)
                            001 = AIN P is AIN0 and AIN N is AIN3 
                            010 = AIN P is AIN1 and AIN N is AIN3
                            011 = AIN P is AIN2 and AIN N is AIN3
                            100 = AIN P is AIN0 and AIN N is GND
                            101 = AIN P is AIN1 and AIN N is GND 
                            110 = AIN P is AIN2 and AIN N is GND
                            111 = AIN P is AIN3 and AIN N is GND  */      
  #define PGA 9         /* Programmable gain amplifier - Full-Scale Range
                            000 = FSR is Â±6.144 V max VDD+0.3V
                            001 = FSR is Â±4.096 V max VDD+0.3V
                            010 = FSR is Â±2.048 V (default)
                            011 = FSR is Â±1.024 V
                            100 = FSR is Â±0.512 V
                            101 = FSR is Â±0.256 V
                            110 = FSR is Â±0.256 V
                            111 = FSR is Â±0.256 V */
  #define MODE 8        /*  0 = Continuous conversion mode
                            1 = Power-down and single-shot mode(default) */
  #define DR 5          /*  Data rate setting:
                            000 = 8 SPS
                            001 = 16 SPS
                            010 = 32 SPS
                            011 = 64 SPS
                            100 = 128 SPS (default)
                            101 = 250 SPS
                            110 = 475 SPS
                            111 = 860 SPS  */
  #define TS_MODE 4     /*  0 = ADC mode (default)
                            1 = Temperature sensor mode */
  #define PULL_UP_EN 3  /*  Enable 400k pullup resistor on MISO pin when CS is HIGH else high-Z
                            0 = Pullup resistor disabled on DOUT/DRDY pin
                            1 = Pullup resistor enabled on DOUT/DRDY pin (default)  */
  #define NOP 1         /*  DIN can be held high or low during SCLK pulses without data being written to the Config register
                            00 = Invalid data, do not update the contents of the Config register
                            01 = Valid data, update the Config register (default)
                            10 = Invalid data, do not update the contents of the Config register
                            11 = Invalid data, do not update the contents of the Config register  */
  #define Reeserved 0   /*  1 = Always write 1 */


int16_t ADS1118_ReadAndSetNewConfig(uint16_t config);
void ADS1118_WaitForDataReady();


#endif	//_ADS1118_ADC_H_
