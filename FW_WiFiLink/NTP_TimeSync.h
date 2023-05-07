#ifndef _NTP_TIMESYNC_H
#define _NTP_TIMESYNC_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "config.h"
#include "Logging.h"
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

extern HardwareSerial SERIAL_DBG_PORT;

#ifndef NTP_MAX_UNCERTAINTY_MS
#define NTP_MAX_UNCERTAINTY_MS	30
#warning "Undefined macro. Using default NTP_MAX_UNCERTAINTY_MS	30!"
#endif
#ifndef NTP_DESIRED_UNCERTAINTY_MS
#define NTP_DESIRED_UNCERTAINTY_MS	5
#warning "Undefined macro. Using default NTP_DESIRED_UNCERTAINTY_MS	5!"
#endif
#ifndef DEFAULT_NTP_ADDRESS
#define DEFAULT_NTP_ADDRESS	192,168,3,1
#warning "Undefined macro. Using default DEFAULT_NTP_ADDRESS	192,168,3,1!"
#endif

#ifndef NTP_SERVER_RETRIES
#define NTP_SERVER_RETRIES	5
#warning "Undefined macro. Using default NTP_SERVER_RETRIES	5!"
#endif
#ifndef NTP_TIMEOUT
#define NTP_TIMEOUT	500
#warning "Undefined macro. Using default NTP_TIMEOUT	500!"
#endif
#ifndef NTP_LOCAL_PORT
#define NTP_LOCAL_PORT	2390
#warning "Undefined macro. Using default NTP_LOCAL_PORT	2390!"
#endif

#define NTP_DEFAULT_MILLIS_DRIFT	5	//Measured drift on 1 ESP at cca. 30�C: 3.4ms/min

/*
EPOCH: thursday 1.1.1970
sunday=0, thursday=4
EUROPEAN UNION Daylight savings since year 2002 until 2020??? 
DST starts 1:00am GMT (UTC) last sunday of march,
DST ends 1:00am GMT (UTC) last sunday of october
*/
#define SECSPERDAY		86400UL
#define SECSPERHOUR		3600
#define SECSPERMINUTE	60
#define UNIX_VS_NTP_OFFSET_S 2208988800ULL

#ifndef __isleap
/* Nonzero if YEAR is a leap year (every 4 years,
except every 100th isn't, and every 400th is).  */
# define __isleap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
#endif

extern IPAddress ip_NTPlocal;
extern int i_LocalPort;
extern int i_NTPServerRetries;
extern int i_NTPTimeout;
extern int i_MaxUncertainty_ms;
extern int i_DesiredUncertainty_ms;

extern uint64_t u64_SysTimeOffset_ms;
extern uint32_t u32_timeSyncUncertanty_ms;
extern boolean bSysTimeSynced;
extern uint32_t u32_LastSyncTime_ms;

void GetTime(void);
uint32_t CalculateExpandedTimeUncertainty(uint32_t u32_time_ms);
uint64_t millis2NTPtime(uint32_t ms);
uint32_t NTPtime2millis(uint64_t NTPtime);
uint32_t millis2UNIXtime(uint32_t ms);

#endif //_NTP_TIMESYNC_H
