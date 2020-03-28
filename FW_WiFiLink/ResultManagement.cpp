#include "ResultManagement.h"
#include <string.h>
  
// On ESP8266:  0 - 1023 maps to 0 - 1 volts



const char* private_key = "PEC1234";

MeasurementResultsClass MeasurementResults;
BoilerTemperatureHistoryClass ChimneyTemperatures;
BoilerTemperatureHistoryClass FireBoxTemperatures;
HeatStorageClass HeatStorage;

HeatStorageClass::HeatStorageClass()
{
	memset(HeatStorageT, 0, sizeof(HeatStorageT));
	HeatStoragePercent = -1;
}

void HeatStorageClass::updateT(int sensorNumber, float T)
{
	if (sensorNumber >= MAX_HEAT_STR_SENSORS) sensorNumber = MAX_HEAT_STR_SENSORS - 1;
	HeatStorageT[sensorNumber] = T;
}

void HeatStorageClass::getT(float Temps[MAX_HEAT_STR_SENSORS])
{
	memcpy(Temps, HeatStorageT, sizeof(HeatStorageT));
}

float HeatStorageClass::getPercentage()
{
	int i;
	float tmp, Tsuma = 0;
	for (i = 0; i < MAX_HEAT_STR_SENSORS; i++)
	{
		tmp = HeatStorageT[i] - 40;
		if (tmp < 0) tmp = 0;
		else if (tmp > 20) tmp = 20;
		//tmp = 0..20 * 5 = 0..100
		Tsuma += tmp*5;
	}
	return Tsuma/MAX_HEAT_STR_SENSORS;
}


BoilerTemperatureHistoryClass::BoilerTemperatureHistoryClass()
{
	buf_IN = 0;
	buf_OUT = 0;
	buf_N = 0;
	last_min_start = millis();
	fTsum = 0;
	fTmin = 9999;
	fTmax = -999;
	iNsum = 0;
}

boolean BoilerTemperatureHistoryClass::storeT(float Temperature)
{
	boolean result = false;
	uint32_t now = millis();

	if (now - last_min_start > BT_AVERAGING_PERIOD)
	{
		T[buf_IN] = round((fTsum/iNsum) * BT_FLOAT_INT_FACTOR);	//max 2047, min -127, max-int16_t = 32768, factor=16 
		Tmin[buf_IN] = round(fTmin * BT_FLOAT_INT_FACTOR);
		Tmax[buf_IN] = round(fTmax * BT_FLOAT_INT_FACTOR);
		buf_IN++;
		if (buf_IN >= BOILER_DATA_HISTORY_LENGTH) buf_IN = 0;
		if (buf_N < BOILER_DATA_HISTORY_LENGTH)
		{
			buf_N++;
		}
		else
		{
			buf_OUT++;
			if (buf_OUT >= BOILER_DATA_HISTORY_LENGTH) buf_OUT = 0;
		}
		result = true;
		//do not restart min,max,avg if it wasn't possible to store result
		last_min_start = now;
		fTsum = 0;
		iNsum = 0;
		fTmin = 9999;
		fTmax = -99;
		return result;
	}
	else
	{
		result = true;
	}
	fTsum += Temperature;
	iNsum++;
	if (Temperature < fTmin) fTmin = Temperature;
	if (Temperature > fTmax) fTmax = Temperature;

	return result;
}

boolean BoilerTemperatureHistoryClass::getT(float* Temperature, float* minTemperature, float* maxTemperature)
{
	boolean result = false;

	if (buf_N > 0)
	{
		*Temperature = (float)T[buf_OUT] / (float)BT_FLOAT_INT_FACTOR;
		*minTemperature = (float)Tmin[buf_OUT] / (float)BT_FLOAT_INT_FACTOR;
		*maxTemperature = (float)Tmax[buf_OUT] / (float)BT_FLOAT_INT_FACTOR;
		buf_OUT++;
		if (buf_OUT >= BOILER_DATA_HISTORY_LENGTH) buf_OUT = 0;
		buf_N--;
		result = true;
	}
	return result;
}

boolean BoilerTemperatureHistoryClass::peekT(int i, float* Temperature, float* minTemperature, float* maxTemperature)
{
	boolean result = false;
	int out_idx;

	if (buf_N > i)
	{
		out_idx = buf_OUT + i;
		if (out_idx >= BOILER_DATA_HISTORY_LENGTH) out_idx -= BOILER_DATA_HISTORY_LENGTH;
		*Temperature = (float)T[out_idx] / (float)BT_FLOAT_INT_FACTOR;
		*minTemperature = (float)Tmin[out_idx] / (float)BT_FLOAT_INT_FACTOR;
		*maxTemperature = (float)Tmax[out_idx] / (float)BT_FLOAT_INT_FACTOR;
		result = true;
	}
	return result;
}

int BoilerTemperatureHistoryClass::getNumberOfStoredT()
{
	return buf_N;
}

boolean BoilerTemperatureHistoryClass::removeLastT()
{
	boolean result = false;

	if (buf_N > 0)
	{
		buf_OUT++;
		if (buf_OUT >= BOILER_DATA_HISTORY_LENGTH) buf_OUT = 0;
		buf_N--;
		result = true;
	}
	return result;
}


MeasurementResultsClass::MeasurementResultsClass()
{
	int i;

	for (i = 0; i < MEASUREMENT_RESULT_BUFFER_SIZE; i++)
	{
		results[i].ID = 0;
		results[i].value = -111;
	}
	buf_IN = 0;
	buf_OUT = 0;
	buf_N = 0;

	i_MaxSendRetries = MAX_SERVER_SEND_RETRIES;
}

boolean MeasurementResultsClass::storeResult(uint32_t u32_time, uint16_t type, uint16_t ID, float value, float Vdd, uint8_t retries)
{
	boolean result = false;

	if (buf_N < MEASUREMENT_RESULT_BUFFER_SIZE)
	{
		results[buf_IN].local_timestamp = u32_time;
		results[buf_IN].type = type;
		results[buf_IN].ID = ID;
		results[buf_IN].value = value;
		results[buf_IN].Vdd = Vdd;
		results[buf_IN].retries = retries;
		buf_IN++;
		if (buf_IN >= MEASUREMENT_RESULT_BUFFER_SIZE) buf_IN = 0;
		buf_N++;
		result = true;
	}
	return result;
}

boolean MeasurementResultsClass::getResult(uint32_t* u32_time, uint16_t* type, uint16_t* ID, float* value, float* Vdd, uint8_t* retries)
{
	boolean result = false;

	if (buf_N > 0)
	{
		*u32_time = results[buf_OUT].local_timestamp;
		*type = results[buf_OUT].type;
		*ID = results[buf_OUT].ID;
		*value = results[buf_OUT].value;
		*Vdd = results[buf_OUT].Vdd;
		*retries = results[buf_OUT].retries;
		results[buf_OUT].ID = 0;
		results[buf_OUT].value = -555;
		buf_OUT++;
		if (buf_OUT >= MEASUREMENT_RESULT_BUFFER_SIZE) buf_OUT = 0;
		buf_N--;
		result = true;
	}
	return result;
}

boolean MeasurementResultsClass::peekResult(uint32_t* u32_time, uint16_t* type, uint16_t* ID, float* value, float* Vdd, uint8_t* retries)
{
	boolean result = false;

	if (buf_N > 0)
	{
		*u32_time = results[buf_OUT].local_timestamp;
		*type = results[buf_OUT].type;
		*ID = results[buf_OUT].ID;
		*value = results[buf_OUT].value;
		*Vdd = results[buf_OUT].Vdd;
		*retries = results[buf_OUT].retries;
		result = true;
	}
	return result;
}

int MeasurementResultsClass::getNumberOfStoredResults()
{
	return buf_N;
}

boolean MeasurementResultsClass::removeLastResult()
{
	boolean result = false;

	if (buf_N > 0)
	{
		results[buf_OUT].ID=0;
		results[buf_OUT].value=-333;
		buf_OUT++;
		if (buf_OUT >= MEASUREMENT_RESULT_BUFFER_SIZE) buf_OUT = 0;
		buf_N--;
		result = true;
	}
	return result;
}

String MeasurementResultsClass::getServerString()
{
	String serverMsg,str1,str2;
	time_t tt_UNIXtimestamp;
	static int i = 0;

	i32_wifiRSSI = -abs(WiFi.RSSI());
	if (getNumberOfStoredResults() > 0)
	{
		tt_UNIXtimestamp = millis2UNIXtime(results[buf_OUT].local_timestamp);
		serverMsg = strServerAddress;
		serverMsg += StringF("?sensID=") + String(results[buf_OUT].ID*256 + results[buf_OUT].type);
		serverMsg += StringF("&time=") + String(tt_UNIXtimestamp);
		serverMsg += StringF("&temp=") + String(results[buf_OUT].value, 3);
		serverMsg += StringF("&Vdd=") + String(results[buf_OUT].Vdd, 3);
		serverMsg += StringF("&retries=") + String(results[buf_OUT].retries);

		serverMsg.replace("%", "%25"); //ta mora bit vedno prvi, ker drugace spremeni procente od ostalih
		serverMsg.replace("#", "%23");
		serverMsg.replace(" ", "%20");
		serverMsg.replace("\n", "");	//any line feed characters break HTTP communication (ctime adds one)
		serverMsg.replace("\r", "");	//not sure about these, but probably better if there are none
	}

	return serverMsg;
}

void MeasurementResultsClass::SetMaxSendRetries(int n)
{
	i_MaxSendRetries = n;
}

int MeasurementResultsClass::GetMaxSendRetries()
{
	return i_MaxSendRetries;
}

String PostData()
{
	boolean result;
	int i_httpResult,i_ResultIndex,i;
	WiFiClient client;
	HTTPClient httpClient;
	String ServerMessage;
	String ServerReply;
	int maxRetries = MeasurementResults.GetMaxSendRetries();
	int NumberOfResults = MeasurementResults.getNumberOfStoredResults();

	if (bSysTimeSynced)
	{
		for (i_ResultIndex = 0; i_ResultIndex < NumberOfResults; i_ResultIndex++)
		{
			result = false;
			ServerMessage = MeasurementResults.getServerString();
			SERIAL_DBG_PORT.println(StringF("Uploading data:\r\n") + ServerMessage);

			httpClient.begin(client, ServerMessage);
//			httpClient.begin(ServerMessage);	deprecated
			i_httpResult = httpClient.GET();
			SERIAL_DBG_PORT.print(StringF("Result ") + String(i_httpResult) + StringF(". "));
			if (i_httpResult == HTTP_CODE_OK)
			{
				MeasurementResults.removeLastResult();
				result = true;
				ServerReply = httpClient.getString();
			}
			else
			{	
				SERIAL_DBG_PORT.println(StringF("Retrying (max ") + String(maxRetries) + StringF(" times)..."));
				for (i = 0; i < maxRetries; i++)
				{
					SERIAL_DBG_PORT.print(String(i+1) + StringF("..."));
					i_httpResult = httpClient.GET();

					if (i_httpResult == HTTP_CODE_OK)
					{
						MeasurementResults.removeLastResult();
						result = true;
						ServerReply = httpClient.getString();
						break;
					}
				}
			}
			if (!result) break;	//if any attempt fails for more than MAX times, abort (and wait for next connection)
		}
	}
	else
	{
		SERIAL_DBG_PORT.printlnF("No timesync. Can't send data until it can be timestamped.");
	}
	return ServerReply;
}

// End
