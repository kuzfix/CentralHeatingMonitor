#include "Configuration.h"
#include <string.h>

#define MAX_CONFIG_FILE_SIZE	1024

//TODO: Think about changing these char[]s to Strings
char DeviceName[20] = "No_Name";

String strServerAddress = SERVER_ADDRESS_STRING;
String SSID = DEFAULT_WIFI_SSID;
String pass = DEFAULT_WIFI_PASSWORD;

void LoadConfig() 
{
	File configFile;
	int16_t i16_CALoffset_V[3];
	double d_CALgain_V[3];
	int16_t i16_CALoffset_T;
	double d_CALgain_T;
	size_t size;
	const char* tmp;
	StaticJsonDocument<ARDUINOJSON_DOC_OBJ_SIZE> jsonDoc;

	configFile = SPIFFS.open(StringF("/config.htm"), "r");
	if (!configFile) {
		SERIAL_DBG_PORT.printlnF("Failed to open config file");
		SaveConfig();
		return;
	}

	size = configFile.size();
	SERIAL_DBG_PORT.println(StringF("Config file size: ")+ String(size) + StringF("B."));
	if (size > MAX_CONFIG_FILE_SIZE) {
		SERIAL_DBG_PORT.println(StringF("Max config file size is ") + String(MAX_CONFIG_FILE_SIZE) + StringF("B."));
		configFile.close();
		return;
	}

	auto error = deserializeJson(jsonDoc,configFile);
	configFile.close();
	if (error) {
		SERIAL_DBG_PORT.printF("deserializeJson() failed with code ");
		SERIAL_DBG_PORT.println(error.c_str());
		return;
	}

	tmp = (const char *)jsonDoc["ServerString"];
	if (tmp != NULL) strServerAddress = tmp;
	
	tmp = jsonDoc["DeviceName"];
	if (tmp != NULL) memcpy(DeviceName,tmp,20);

	LogLevel = jsonDoc["LogLevel"] | DEFAULT_LOG_LEVEL;

	SERIAL_DBG_PORT.println(StringF("ID:") + String(DeviceName));
	SERIAL_DBG_PORT.println(StringF("Server address:") + strServerAddress);
	SERIAL_DBG_PORT.println(StringF("Log Level: ") + String(LogLevel));
}

bool SaveConfig() {
    StaticJsonDocument<ARDUINOJSON_DOC_OBJ_SIZE> jsonDoc;
	File configFile;
	int16_t i16_CALoffset_V[3];
	double d_CALgain_V[3];
	int16_t i16_CALoffset_T;
	double d_CALgain_T;
	
	jsonDoc["ServerString"] = strServerAddress;
	jsonDoc["DeviceName"] = DeviceName;
	jsonDoc["LogLevel"] = LogLevel;
	SERIAL_DBG_PORT.printlnF("\nTo bi moralo biti napisano:\n");
	SERIAL_DBG_PORT.println(StringF("ID:") + String(DeviceName));
	SERIAL_DBG_PORT.println(StringF("Server address:") + strServerAddress);
	SERIAL_DBG_PORT.println(StringF("Log Level: ") + String(LogLevel));
	SERIAL_DBG_PORT.println(StringF("Json object mem usage:") + String(jsonDoc.memoryUsage()));

    configFile = SPIFFS.open(StringF("/config.htm"), "w");
    if (!configFile) {
        SERIAL_DBG_PORT.printlnF("Failed to open config file for writing");
        return false;
    }
  
	serializeJsonPretty(jsonDoc, configFile);
	configFile.close();
    return true;
}

bool GetWiFiCredentials(int n)
{
	File configFile;
	size_t size;
	const char* tmp;
	String newSSID[3], newpass[3];
	StaticJsonDocument<ARDUINOJSON_DOC_OBJ_SIZE> jsonDoc;

	configFile = SPIFFS.open(StringF("/configWiFi.htm"), "r");
	if (!configFile) {
		SERIAL_DBG_PORT.printlnF("Failed to open WiFi config file");
		newSSID[0] = SSID;
		newpass[0] = pass;
		SaveWiFiCredentials(newSSID, newpass);
		return false;
	}

	size = configFile.size();
	SERIAL_DBG_PORT.println(StringF("Config file size: ") + String(size) + StringF("B."));
	if (size > MAX_CONFIG_FILE_SIZE) {
		SERIAL_DBG_PORT.println(StringF("Max config file size is ") + String(MAX_CONFIG_FILE_SIZE) + StringF("B."));
		configFile.close();
		return false;
	}

	auto error = deserializeJson(jsonDoc, configFile);
	configFile.close();
	if (error) {
		SERIAL_DBG_PORT.printF("deserializeJson() failed with code ");
		SERIAL_DBG_PORT.println(error.c_str());
		return false;
	}
	
	if (n > 2) n = 0;
	tmp = (const char*)jsonDoc["SSID" + String(n)];
	if (tmp != NULL) 
	{
		SSID = tmp;
		tmp = (const char*)jsonDoc["pass" + String(n)];
		if (tmp != NULL) pass = tmp;
		else pass = "";
	}
	else
	{
		if (n > 0)
		{
			GetWiFiCredentials(0);
		}
		else
		{
			SSID = StringF(DEFAULT_WIFI_SSID);
			pass = StringF(DEFAULT_WIFI_PASSWORD);
		}
		return false;
	}
	SERIAL_DBG_PORT.println(StringF("Cred ")+String(n)+ StringF(": ") + SSID + StringF(", ") + pass);
	return true;
}

bool SaveWiFiCredentials(String newSSID[3], String newpass[3])
{
	StaticJsonDocument<ARDUINOJSON_DOC_OBJ_SIZE> jsonDoc;
	File configFile;
	size_t size;
	const char* tmp;
	int i, j;
	String SSID[3], pass[3];

	configFile = SPIFFS.open(StringF("/configWiFi.htm"), "r");
	if (!configFile) {
		SERIAL_DBG_PORT.printlnF("Failed to open wifi config file");
	}
	else
	{
		size = configFile.size();
		SERIAL_DBG_PORT.println(StringF("Config file size: ") + String(size) + StringF("B."));
		if (size > MAX_CONFIG_FILE_SIZE) {
			SERIAL_DBG_PORT.println(StringF("Max config file size is ") + String(MAX_CONFIG_FILE_SIZE) + StringF("B."));
			return false;
		}
			
		auto error = deserializeJson(jsonDoc, configFile);
		if (error) {
			SERIAL_DBG_PORT.printF("deserializeJson() failed with code ");
			SERIAL_DBG_PORT.println(error.c_str());
			return false;
		}
		configFile.close();
	}

	//remove any holes in SSID order (e.g. SSID0 and SSID2 exist, but not SSID1)
	for (i = 0; i < 2; i++)
	{
		for (j = 1; j >= 0; j--)
		{
			if (newSSID[j].isEmpty())
			{
				newSSID[j] = newSSID[j + 1];
				newpass[j] = newpass[j + 1];
				newSSID[j + 1].clear();
				newpass[j + 1].clear();
			}
		}
	}

	//Get current SSIDs and passwords(if password is "**********", we have to keep it)
	for (i = 0; i < 3; i++)
	{
		tmp = (const char*)jsonDoc["SSID"+String(i)];
		if (tmp != NULL)
		{
			SSID[i] = tmp;
			tmp = (const char*)jsonDoc["pass"+String(i)];
			if (tmp != NULL) pass[i] = tmp;
			else pass[i].clear();
		}
	}

	//check if we already have data for any of the SSIDs, for which we don't have a password
	for (i = 0; i < 3; i++)
	{
		if (newpass[i] == StringF("**********"))
		{
			for (j = 0; j < 3; j++)
			{
				if (newSSID[i] == SSID[j])
				{
					newpass[i] = pass[j];
				}
			}
		}
	}

	//if no SSIDs and passwords available, save defaults.
	if (newSSID[0].isEmpty())
	{
		newSSID[0] = StringF(DEFAULT_WIFI_SSID);
		newpass[0] = StringF(DEFAULT_WIFI_PASSWORD);
	}

	//create new json
	jsonDoc.clear();
	for (i = 0; i < 3; i++)
	{
		if (!newSSID[i].isEmpty())
		{
			jsonDoc["SSID" + String(i)] = newSSID[i];
			jsonDoc["pass" + String(i)] = newpass[i];
		}
	}

	SERIAL_DBG_PORT.printlnF("\nTo bi moralo biti napisano:\n");
	JsonObject obj = jsonDoc.as<JsonObject>();
	for (JsonObject::iterator it = obj.begin(); it != obj.end(); ++it)
	{
		SERIAL_DBG_PORT.print(it->key().c_str());
		if (it->value().is<char*>()) {
			const char* s = it->value();
			SERIAL_DBG_PORT.println(":" + String(s));
		}
		else
		{
			double val = it->value();
			SERIAL_DBG_PORT.println(":" + String(val));
		}
	}

	configFile = SPIFFS.open(StringF("/configWiFi.htm"), "w");
	if (!configFile) {
		SERIAL_DBG_PORT.printlnF("Failed to open config file for writing");
		return false;
	}

	serializeJsonPretty(jsonDoc, configFile);
	configFile.close();
	return true;
}

void LoadMiscConfig()
{
	File configFile;
	size_t size;
	const char* tmp;
	int retries;
	StaticJsonDocument<ARDUINOJSON_DOC_OBJ_SIZE> jsonDoc;

	configFile = SPIFFS.open(StringF("/configMisc.htm"), "r");
	if (!configFile) {
		SERIAL_DBG_PORT.printlnF("Failed to open WiFi config file");
		SaveMiscConfig();
		return;
	}

	size = configFile.size();
	SERIAL_DBG_PORT.println(StringF("Config file size: ") + String(size) + StringF("B."));
	if (size > MAX_CONFIG_FILE_SIZE) {
		SERIAL_DBG_PORT.println(StringF("Max config file size is ") + String(MAX_CONFIG_FILE_SIZE) + StringF("B."));
		configFile.close();
		return;
	}

	auto error = deserializeJson(jsonDoc, configFile);
	configFile.close();
	if (error) {
		SERIAL_DBG_PORT.printF("deserializeJson() failed with code ");
		SERIAL_DBG_PORT.println(error.c_str());
		return;
	}

	i_WiFiConnectTimeout = jsonDoc["WifiConTout"] | DEFAULT_WIFI_CONNECT_TIMEOUT;
	retries = jsonDoc["NSendRtry"] | MAX_SERVER_SEND_RETRIES;
	MeasurementResults.SetMaxSendRetries(retries);
	i_NTPServerRetries = jsonDoc["NTPRtry"] | NTP_SERVER_RETRIES;
	i_NTPTimeout = jsonDoc["NTPTout"] |	NTP_TIMEOUT;
	i_MaxUncertainty_ms = jsonDoc["NTPMuncert"] | NTP_MAX_UNCERTAINTY_MS;
	i_DesiredUncertainty_ms = jsonDoc["NTPDuncert"] | NTP_DESIRED_UNCERTAINTY_MS;
	i_LocalPort = jsonDoc["NTPPort"] | NTP_LOCAL_PORT;
	
	tmp = (const char*)jsonDoc["UpdateIP"];
	if (tmp != NULL) str_updateServer = tmp;
	else str_updateServer = DEFAULT_UPDATE_SERVER;

	tmp = (const char*)jsonDoc["UpdateFile"];
	if (tmp != NULL) str_updateFile = tmp;
	else str_updateFile = DEFAULT_UPDATE_FILE;

	tmp = (const char*)jsonDoc["NTPadr"];
	if (tmp != NULL) ip_NTPlocal.fromString(tmp);
	else ip_NTPlocal = IPAddress(DEFAULT_NTP_ADDRESS);

	SERIAL_DBG_PORT.printlnF("\nPrebrane nastavitve iz misc:");
	SERIAL_DBG_PORT.println(StringF("UpdateIP+file: ") + str_updateServer + str_updateFile);
	SERIAL_DBG_PORT.printF("NTP_IP: ");
	SERIAL_DBG_PORT.println(ip_NTPlocal);
	SERIAL_DBG_PORT.println(StringF("NTP port: ") + String(i_LocalPort));
	SERIAL_DBG_PORT.println(StringF("Wifi connection timeout: ") + String(i_WiFiConnectTimeout));
	SERIAL_DBG_PORT.println(StringF("Upload data retries: ") + String(retries));
	SERIAL_DBG_PORT.println(StringF("NTP retries: ") + String(i_NTPServerRetries));
	SERIAL_DBG_PORT.println(StringF("NTP timeout: ") + String(i_NTPTimeout));
	SERIAL_DBG_PORT.println(StringF("Max NTP Uncertainty: ") + String(i_MaxUncertainty_ms));
	SERIAL_DBG_PORT.println(StringF("Desired NTP Uncertainty: ") + String(i_DesiredUncertainty_ms));
}

bool SaveMiscConfig() {
	StaticJsonDocument<ARDUINOJSON_DOC_OBJ_SIZE> jsonDoc;
	File configFile;
	
	jsonDoc["WifiConTout"] = i_WiFiConnectTimeout;	//DEFAULT_WIFI_CONNECT_TIMEOUT
	jsonDoc["UpdateIP"] = str_updateServer;	//DEFAULT_UPDATE_SERVER
	jsonDoc["UpdateFile"] = str_updateFile;	//DEFAULT_UPDATE_FILE
	jsonDoc["NSendRtry"] = MeasurementResults.GetMaxSendRetries();	//MAX_SERVER_SEND_RETRIES
	jsonDoc["NTPadr"] = ip_NTPlocal.toString();		//DEFAULT_NTP_ADDRESS
	jsonDoc["NTPPort"] = i_LocalPort;	//NTP_LOCAL_PORT
	jsonDoc["NTPRtry"] = i_NTPServerRetries;	//NTP_SERVER_RETRIES
	jsonDoc["NTPTout"] = i_NTPTimeout;	//NTP_TIMEOUT
	jsonDoc["NTPMuncert"] = i_MaxUncertainty_ms;	//NTP_MAX_UNCERTAINTY_MS
	jsonDoc["NTPDuncert"] = i_DesiredUncertainty_ms;	//NTP_DESIRED_UNCERTAINTY_MS

	configFile = SPIFFS.open(StringF("/configMisc.htm"), "w");
	if (!configFile) {
		SERIAL_DBG_PORT.printlnF("Failed to open config file for writing");
		return false;
	}

	serializeJsonPretty(jsonDoc, configFile);
	configFile.close();

	SERIAL_DBG_PORT.printlnF("\nTo bi moralo biti shranjeno v misc:");
	SERIAL_DBG_PORT.println(StringF("UpdateIP+file: ") + str_updateServer + str_updateFile);
	SERIAL_DBG_PORT.printF("NTP_IP: ");
	SERIAL_DBG_PORT.println(ip_NTPlocal);
	SERIAL_DBG_PORT.println(StringF("NTP port: ") + String(i_LocalPort));
	SERIAL_DBG_PORT.println(StringF("Wifi connection timeout: ") + String(i_WiFiConnectTimeout));
	SERIAL_DBG_PORT.println(StringF("Upload data retries: ") + String(MeasurementResults.GetMaxSendRetries()));
	SERIAL_DBG_PORT.println(StringF("NTP retries: ") + String(i_NTPServerRetries));
	SERIAL_DBG_PORT.println(StringF("NTP timeout: ") + String(i_NTPTimeout));
	SERIAL_DBG_PORT.println(StringF("Max NTP Uncertainty: ") + String(i_MaxUncertainty_ms));
	SERIAL_DBG_PORT.println(StringF("Desired NTP Uncertainty: ") + String(i_DesiredUncertainty_ms));
	return true;
}

/*SPIFFS.open("filename.ext", "a")
  "r"   read              Open a file for reading   read from start   failure to open
  "w"   write             Create a file for writing   destroy contents  create new
  "a"   append            Append to a file  write to end  create new
  "r+"  read extended     Open a file for read/write  read from start   error
  "w+"  write extended    Create a file for read/write  destroy contents  create new
  "a+"  append extended   Open a file for read/write  write to end  create new

  - File access mode flag "b" can optionally be specified to open a file in binary mode. This flag has no effect on POSIX systems, but on Windows, for example, it disables special handling of '\n' and '\x1A'.
  - On the append file access modes, data is written to the end of the file regardless of the current position of the file position indicator.
  - File access mode flag "x" can optionally be appended to "w" or "w+" specifiers. This flag forces the function to fail if the file exists, instead of overwriting it. (C++17)
  - The behavior is undefined if the mode is not one of the strings listed above. Some implementations define additional supported modes (e.g. Windows).

  other comands for SPIFS.
  bool remove(const char* path);
  bool rename(const char* pathFrom, const char* pathTo);
  bool exists(const char* path);
*/
