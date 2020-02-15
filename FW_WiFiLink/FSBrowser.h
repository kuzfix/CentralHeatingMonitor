#ifndef _FS_BROWSER_H_
#define _FS_BROWSER_H_

#include "config.h"
#include <ESP8266WebServer.h>
#include "FS.h"

#define HTTP_SERVER httpServer
#define SERIAL_DBG_PORT Serial
  
extern HardwareSerial SERIAL_DBG_PORT;
extern ESP8266WebServer HTTP_SERVER;
  
void Init_FSBrowser(); //Requires that http server is started afterwards
String formatBytes(size_t bytes);
String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();
void handleBasicUpload();
void handleFavicon();

extern File fsUploadFile;


#endif	//_FS_BROWSER_H_
