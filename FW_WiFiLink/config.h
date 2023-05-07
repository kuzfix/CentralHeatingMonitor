#ifndef _CONFIG_H
#define _CONFIG_H

//#define DEBUGING

#include "HelperMacros.h"

#define FW_VER	"0.1.0"


#define BlueLED 2
#define BTN 0

#define DEFAULT_WIFI_CONNECT_TIMEOUT	100
#define DEFAULT_WIFI_CONNECT_INTERVAL	(60*1000UL)
#define DEFAULT_WIFI_SSID	"EIM"			//hard coded
#define DEFAULT_WIFI_PASSWORD	"pisukapoldi"	//hard coded
#define DEFAULT_UPDATE_SERVER	"192.168.0.11"
#define DEFAULT_UPDATE_FILE		"/update/ESPUpdate.php"

#define DEBUG_MESSAGE_SERVER	"http://192.168.0.11/submit/debugMsg.php"

//Result Management
#define MAX_SERVER_SEND_RETRIES	3
#define MAX_HEAT_STR_SENSORS	17
//#define BOILER_DATA_HISTORY_LENGTH	10
#define BOILER_DATA_HISTORY_LENGTH	100
#define MEASUREMENT_RESULT_BUFFER_SIZE	(32*12)
//#define MEASUREMENT_RESULT_BUFFER_SIZE	(32)
//#define BT_AVERAGING_PERIOD	5000
#define BT_AVERAGING_PERIOD	60000

//Time Sync config
#define DEFAULT_NTP_ADDRESS	192,168,0,11
#define NTP_SERVER_RETRIES	3
#define NTP_TIMEOUT	500
#define NTP_MAX_UNCERTAINTY_MS	20
#define NTP_DESIRED_UNCERTAINTY_MS	5
#define NTP_LOCAL_PORT	2390
#define MAX_VALID_DRIFT	10
#define NTP_SYNC_INTERVAL	(3600UL*1000UL*24UL)
//Komentar za mikrotik router: v6.41.? je imela probleme
//Zdaj je nalo�ena verzija 6.44.3 in zaenkrat zgleda da deluje precej bolje

//SoftAP
#define DEFAULT_SOFTAP_SSID		"PEC_SoftAP"
#define DEFAULT_SOFTAP_PASS		""
#define DEFAULT_SOFTAP_IP		192,168,0,1
#define DEFAULT_SOFTAP_GATEWAY	192,168,0,1
#define DEFAULT_SOFTAP_SUBNET	255,255,255,0

//Logging defaults
#define DEFAULT_LOG_LEVEL	LL_CONFIG							//hard coded

//SPI
#define CS      16							
//these are already defined in common.h
//#define MISO    12							
//#define MOSI    13							
//#define SCK     14							
//I2C
//#define SDA     4							
//#define SCL     5							

#define SERVER_ADDRESS_STRING	"http://192.168.0.11/submit/submitPEC.php"

#define ARDUINOJSON_DOC_OBJ_SIZE	512		//hard coded
//increase number of decimal points from 6 to 9
#define ARDUINOJSON_USE_DOUBLE	1			
#define ARDUINOJSON_POSITIVE_EXPONENTIATION_THRESHOLD 1e1	//Makes it switch to scientific notation very quickly (default 1e7)	//hard coded
#define ARDUINOJSON_NEGATIVE_EXPONENTIATION_THRESHOLD 1e-1	//(default 1e-5)	//hard coded

//*************************************************************************
//********************** DEFAULTS FOR DEBUGGING PURPOSES ******************
#ifdef DEBUGING

#define DEBUG_TIME_KEEPING_STATS
#define DBG_TIM_KP_ST_LENGTH	15
//serverMsg = String("http://212.235.187.34/Jubopotdata.aspx?private_key=JB00B000&jubopotid=0&Vpot=-1&Istr=-1&timestamp=1562056666&MeasurmentNr=0&RSSi[dBm]=-1&synced[s]=0&Name=No_Name-PID-%2300-x0.00&Comment=test3");

#endif // DEBUGING


#endif //_CONFIG_H
