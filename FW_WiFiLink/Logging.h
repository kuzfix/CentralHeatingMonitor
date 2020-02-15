// Logging.h

#ifndef _LOGGING_h
#define _LOGGING_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "config.h"
#include <FS.h>
#include "NTP_TimeSync.h"
#include "Configuration.h"

#define SERIAL_DBG_PORT Serial
extern HardwareSerial SERIAL_DBG_PORT;

#ifndef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL LL_CONFIG
#endif // !DEFAULT_LOG_LEVEL


// Log levels
#define LL_NONE						0
#define LL_SYSTEM_ERROR				1
#define LL_DEBUG_MSG				2
#define LL_CONNECTION_ERROR			3
#define LL_NTP_ERROR				4
#define LL_DATA_UPLOAD_ERROR		5
#define LL_WEB_COMMANDS				7
#define LL_NTP_STATS				10
#define LL_DATA_UPLOAD_STATS		11
#define LL_SYSTEM					20
#define LL_SYSTEM_INFO				21
#define LL_CONFIG					25
#define LL_CONNECTION				30
#define LL_NTP						35
#define LL_MEASUREMENT_SYNC_STATS	40
#define LL_DATA_UPLOAD				45
#define LL_MEM_STATS				50

extern uint8_t LogLevel;	//lower number = higher priority

bool BeginLog();
void Log(String LogEntry, int Level);


#endif
