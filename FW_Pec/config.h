/*
 * config.h
 *
 * Created: 3.6.2015 14:07:32
 *  Author: maticpi
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

#define F_CPU 18432000UL
//#define F_CPU 16000000UL
#define BAUD1 9600UL
#define BAUD0 115200UL

#define DISPLAY_FLIPPED
#define SHARE_SPI
#define MAXSENSORS 18
#define OW_ONE_BUS
//DS18B20 pin set in onwire.h - PB1

#endif /* CONFIG_H_ */