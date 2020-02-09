#ifndef _SHT21_H
#define _SHT21_H

#include "config.h"
#include <Wire.h>
//#include "font.h"

#ifndef SDA
#define SDA     4
#warning "SDA pin not defined. Using default value (4)."
#endif 
#ifndef SCL
#define SCL     5
#warning "SCL pin not defined. Using default value (5)."
#endif 

#define SHT21_ADDRESS 0x40  //I2C address for the sensor
  
#define TRIGGER_TEMP_MEASURE_NOHOLD  0xF3
#define TRIGGER_HUMD_MEASURE_NOHOLD  0xF5
  
class SHT21Class {     
public:
    SHT21Class();   
	void begin();
	void begin(int sda, int scl);
	float getHumidity(void);
    float getTemperature(void);
	void readAll();
	float fBox_Temp, fBox_Humidity = 5555;

private:   
    uint16_t readSHT21(uint8_t command);
};

extern SHT21Class SHT21;

#endif
