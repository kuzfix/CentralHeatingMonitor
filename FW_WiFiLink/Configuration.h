#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "config.h"
#include "Logging.h"
#include <ArduinoJson.h>
#include <FS.h>
#include "ResultManagement.h"
#include "NTP_TimeSync.h"
#include "WiFiConnection.h"
#include "OTAupdates.h"

#define SERIAL_DBG_PORT Serial
extern HardwareSerial SERIAL_DBG_PORT;

//TODO: Think about changing these char[]s to Strings
extern char DeviceName[20];

extern String strServerAddress;
extern String SSID;
extern String pass;

void LoadConfig();
bool SaveConfig();
bool GetWiFiCredentials(int n);
bool SaveWiFiCredentials(String newSSID[3], String newpass[3]);
void LoadMiscConfig();
bool SaveMiscConfig();

#endif //CONFIGURATION_H