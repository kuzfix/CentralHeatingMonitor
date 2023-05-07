#include "WiFiConnection.h"

int i_WiFiConnectTimeout = DEFAULT_WIFI_CONNECT_TIMEOUT;
//int FindBestChannel();

boolean ConnectToWiFi()
{
	int i,j,w;
	bool result = true;

	if (WiFi.status() == WL_CONNECTED)
	{
		SERIAL_DBG_PORT.printlnF("Connecting... already connected!");
		return true;
	}
	for (i=0; i<3; i++)	//Start with default SSID, lookup others if this fails
	{
		for (j=0; j<3; j++)	//Try to connect to each SSID 3 times TODO: Make it configurable
		{
			w = 0;
			SERIAL_DBG_PORT.print(StringF("Connecting to ") + SSID + StringF("..."));
			if (WiFi.status() != WL_CONNECTED) 
				WiFi.begin(SSID, pass);
			while (WiFi.status() != WL_CONNECTED) 
			{
				SERIAL_DBG_PORT.print(String(w) + StringF(" "));
				delay(200);
				if (w > i_WiFiConnectTimeout) {
					w = -1;
					SERIAL_DBG_PORT.printF(" - failed!");
					Log(StringF("Connect to ")+SSID+StringF(" failed."), LL_CONNECTION);
					break;
				}
				w++;
			}
			if (WiFi.status() == WL_CONNECTED)
			{
				SERIAL_DBG_PORT.printlnF(" - connected!");
				SERIAL_DBG_PORT.printF("My IP:");
				SERIAL_DBG_PORT.println(WiFi.localIP());
				break;
			}
		}
		if (WiFi.status() == WL_CONNECTED) break;
		else if (!GetWiFiCredentials(i + 1)) break;	//if new credentials aren't available, quit
	}

	if (WiFi.status() != WL_CONNECTED)
	{
		result = false;
		Log(StringF("Connection to any provided network failed."), LL_CONNECTION);
	}

	return result;
}

void DisconnectFromWiFi()
{
		WiFi.mode(WIFI_OFF);		//Seams OK, no current spikes, TODO: test if reconnection works OK (Could break WiFi state machines)
		//WiFi.disconnect(true);	//Should be the same as WIFI_OFF, no current spikes after this.
									//disconnect(true) seams to cause trouble with the watchdog.
}
/*
//Combining STA mode and Soft-AP mode does not work well!
//The reason is hardware limitations of the module (only one radio).
//Whenever a Soft-AP mode is used, reset the module to return the module to station mode.
//Also disable autoconnect mode, erase config (ESP.eraseConfig();),... Anything else?
bool CreateSoftAP()
{
	uint8_t mac[6];
	int bestChannel;
	char txt[5];
	IPAddress local_IP(DEFAULT_SOFTAP_IP);
	IPAddress gateway(DEFAULT_SOFTAP_GATEWAY);
	IPAddress subnet(DEFAULT_SOFTAP_SUBNET);
	bool status;
	bool result = false;

	WiFi.disconnect(true);	//Carefull! Seems to cause instability!
	WiFi.macAddress(mac);
	sprintf(txt, StringF("%02X%02X").c_str(), mac[4], mac[5]);
	bestChannel = FindBestChannel();
	if (WiFi.softAPConfig(local_IP, gateway, subnet))
	{
		if (WiFi.softAP(String(DEFAULT_SOFTAP_SSID) + String(txt), DEFAULT_SOFTAP_PASS, bestChannel))
		{
			Log(StringF("Created Soft-AccessPoint."), LL_CONNECTION);
			SERIAL_DBG_PORT.printF("Soft-AP IP: ");
			SERIAL_DBG_PORT.println(WiFi.softAPIP());
			result = true;
		}
		else
		{
			Log(StringF("Creating Soft-AccessPoint failed."), LL_CONNECTION_ERROR);
		}
	}
	return result;
}

int FindBestChannel()
{
	int bestChannel = 3;
	//TODO:
	return bestChannel;
}*/
