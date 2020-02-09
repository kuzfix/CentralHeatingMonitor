#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include "config.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include "ResultManagement.h"
#include "WiFiConnection.h"
#include "Configuration.h"

#define HTTP_SERVER httpServer
#define SERIAL_DBG_PORT Serial
  
extern char Host_Name[30];
extern HardwareSerial SERIAL_DBG_PORT;
extern ESP8266WebServer HTTP_SERVER;

void Init_WebServer(void);
//uint32_t HandleHttpRequests();
bool SetSoftAPStart();

#endif	//_WEB_SERVER_H_
