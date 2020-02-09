// WiFiConnection.h

#ifndef _WIFICONNECTION_h
#define _WIFICONNECTION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "config.h"
#include "Configuration.h"

#define SERIAL_DBG_PORT Serial
extern HardwareSerial SERIAL_DBG_PORT;

extern char Host_Name[30];
extern int i_WiFiConnectTimeout;

boolean ConnectToWiFi();
void DisconnectFromWiFi();
bool CreateSoftAP();

#endif

