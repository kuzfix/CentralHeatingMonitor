#ifndef _OTA_UPDATES_H_
#define _OTA_UPDATES_H_

#include "config.h"
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
#include <bearssl/bearssl.h>

#define HTTP_SERVER httpServer
#define SERIAL_DBG_PORT Serial

extern HardwareSerial SERIAL_DBG_PORT;
extern ESP8266WebServer HTTP_SERVER;
extern String str_updateServer;
extern String str_updateFile;

void Init_HTTPupdater();
void GetFWUpdate();

#endif //_OTA_UPDATES_H_
