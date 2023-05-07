#ifndef _RESULT_POSTING_H
#define _RESULT_POSTING_H

#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
//#include <ESP8266WebServer.h>
#include "NTP_TimeSync.h"
#include "WebServer.h"
#include "Configuration.h"
#include "Logging.h"

#ifndef SERVER_ADDRESS_STRING
#define SERVER_ADDRESS_STRING	"http://212.235.187.34/jubodata.php"
#warning "SERVER_ADDRESS_STRING undefined. Using default (http://212.235.187.34/jubodata.php)"
#endif

#ifndef MAX_SERVER_SEND_RETRIES
#define MAX_SERVER_SEND_RETRIES	3
#warning "MAX_SERVER_SEND_RETRIES undefined. Using default (3)"
#endif

#ifndef MEASUREMENT_RESULT_BUFFER_SIZE
#define MEASUREMENT_RESULT_BUFFER_SIZE	32
#warning "MEASUREMENT_RESULT_BUFFER_SIZE undefined. Using default (32)"
#endif

#ifndef BT_AVERAGING_PERIOD
#define BT_AVERAGING_PERIOD	60000
#warning "BT_AVERAGING_PERIOD undefined. Using default (60000)"
#endif

#define SERIAL_DBG_PORT Serial
extern HardwareSerial SERIAL_DBG_PORT;

class HeatStorageClass
{
public:
	HeatStorageClass();
	void updateT(int sensorNumber, float T);
	void getT(float Temps[MAX_HEAT_STR_SENSORS]);
	float getPercentage();

protected:
	float HeatStorageT[MAX_HEAT_STR_SENSORS];
	float HeatStoragePercent;
};

extern HeatStorageClass HeatStorage;

#define	BT_FLOAT_INT_FACTOR	16	//max 2047, min -127, max-int16_t = 32768, factor=16

class BoilerTemperatureHistoryClass
{
public:
	BoilerTemperatureHistoryClass();
	boolean storeT(float Temperature);
	boolean getT(float* Temperature, float* minTemperature, float* maxTemperature);
	boolean peekT(int i, float* Temperature, float* minTemperature, float* maxTemperature);
	int getNumberOfStoredT();
	boolean removeLastT();

protected:
	uint32_t last_min_start;
	int	iNsum;
	float	fTsum,fTmax,fTmin;
	int16_t	T[BOILER_DATA_HISTORY_LENGTH];
	int16_t	Tmin[BOILER_DATA_HISTORY_LENGTH];
	int16_t	Tmax[BOILER_DATA_HISTORY_LENGTH];
	int buf_IN;
	int buf_OUT;
	int buf_N;
};

extern BoilerTemperatureHistoryClass ChimneyTemperatures;
extern BoilerTemperatureHistoryClass FireBoxTemperatures;

struct MeasurementResult_strT
{
	uint32_t local_timestamp;
	uint16_t type;
	uint16_t ID;
	float value;
	float Vdd;
	uint8_t retries;
};

class MeasurementResultsClass
{
public:
	MeasurementResultsClass();
	boolean storeResult(uint32_t u32_time, uint16_t type, uint16_t ID, float value, float Vdd, uint8_t retries);
	boolean getResult(uint32_t* u32_time, uint16_t* type, uint16_t* ID, float* value, float* Vdd, uint8_t* retries);
	boolean peekResult(uint32_t* u32_time, uint16_t* type, uint16_t* ID, float* value, float* Vdd, uint8_t* retries);
	int getNumberOfStoredResults();
	boolean removeLastResult();
	String getServerString();
	void SetMaxSendRetries(int n);
	int GetMaxSendRetries();

//protected:
	MeasurementResult_strT results[MEASUREMENT_RESULT_BUFFER_SIZE];
	int buf_IN;
	int buf_OUT;
	int buf_N;

	int32_t i32_wifiRSSI;
	int i_MaxSendRetries;
};

extern MeasurementResultsClass MeasurementResults;

String PostData();

#endif	//_RESULT_POSTING_H
