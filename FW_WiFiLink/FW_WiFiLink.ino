#include "config.h"
#include "Configuration.h"
#include "FSBrowser.h"
#include "NTP_TimeSync.h"
#include "WebServer.h"
#include "ResultManagement.h"
#include "OTAupdates.h"
#include "WiFiConnection.h"
#include "Logging.h"

//Toolchain settings:
//Board: Generic ESP8266 Module
//Flash Mode: DIO
//Flash Frequency: 40MHz
//CPU Frequency: 80MHz
//Flash Size: 1M(128k SPIFFS)
//Debug port: Disabled
//Debug Level: None
//Reset Method: dtr (aka nodemcu)
//Upload Speed: 115200 (may also work with 256000)
//Port: COM3 (for my computer only)
//Reminder: HOLD DOWN BOOT button while donloading new code

//Arduino ESP8266 core version 2.6.3: https://github.com/esp8266/Arduino
//LibraryVersions:
//Sodaq_SHT2x 1.2.0 ??? I think this is not needed. Check!
//OneWire 2.3.5
//DallasTemperature 3.7.6
//ArduinoJson 6.13.0

//!!!!!!!!!!!!!!!!!!!! CHECK config.h to see whether DEBUGGING mode is enabled !!!!!!!!!!

//For testing:
//String ArtificiallyUseRam8k = "1563415615646516556146541614561654654j465f4gh65gfj4drf5e61g6rh1r5e6g16g156aeg6eh515e6gh1re6g165eg156rh456re1gh6e1g56eh65e1gh56er1gh456reg56e1gh56e1h6e1h56re14g56er14gh56e1h56e1h561eh56er1hh61r5e61r5e6g15e6rg156er1h5e6r16re1ge65gh156err5e6gh1r5e6gh156er156erre56g1re56hg1r5e6r5e6h1r5e65re6er561t6r5e1re65t1re651tre54t6ret54re65rt546er56t4er5r5e6t46re56tkldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k kldgfjlkdfj gkldfhj gkldfhg kljdhfgkjdhfg kjndfkjnfkjdbnkjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn kkljdhfgkjdhfg kjndfkjnfkjdbnkfjfdnbgkjdf hgkjdfg kjdfhg kjdfgkj dhf gkj dhfgkj dfkjbn dfkjb nkdfjbn k";

// *********** setup ***********
void setup()
{
	rst_info* resetInfo;
	char txt[100];	//25 should be enough, but better save than sorry
	String Str_ResetReason;
	uint8_t mac[6];// = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };

	resetInfo = ESP.getResetInfoPtr();
	Str_ResetReason = ESP.getResetReason();
	WiFi.mode(WIFI_OFF);
	if (WiFi.getAutoConnect()) WiFi.setAutoConnect(false);	//disable autoconnect if set (efect on reboot)
	//ESP.eraseConfig(); //Figure out how to do this only if realy needed in order not to wear out flash.
	digitalWrite(BlueLED, LOW); // LED ON
	pinMode(CS, OUTPUT);		//Chip Select for SPI and LED
	pinMode(BlueLED, OUTPUT);
	pinMode(BTN, INPUT);
	SERIAL_DBG_PORT.begin(115200);
	SERIAL_DBG_PORT.printlnF("\nInitalising...");
	SPIFFS.begin();
	BeginLog();
	if (resetInfo->reason == REASON_DEFAULT_RST) Log(StringF("RESET REASON: Normal power up."), LL_SYSTEM);
	else if (resetInfo->reason == REASON_EXT_SYS_RST) Log(StringF("RESET REASON: Normal power up (external reset)."), LL_SYSTEM);
	else Log(StringF("RESET REASON: ") + Str_ResetReason, LL_SYSTEM_ERROR);
	//TODO: make use of ESP.checkFlashCRC(), to detect flash corruption - NOT YET IMPLEMENTED IN CURRENT VERSION OF ARDUINO LIBRARY
	//if (ESP.checkFlashCRC()) Log(StringF("FLASH check OK."), LL_SYSTEM);
	//else Log(StringF("FLASH CRC ERROR!!!"), LL_SYSTEM);
	LoadConfig();
	LoadMiscConfig();
	GetWiFiCredentials(0);

	SERIAL_DBG_PORT.printlnF("Firmware version: " FW_VER " (" __DATE__ " at " __TIME__ ")");
	sprintf(Host_Name, StringF("%s").c_str(), DeviceName);
	WiFi.hostname(Host_Name);
	SERIAL_DBG_PORT.println(StringF("Host name: ") + String(Host_Name));

	WiFi.setPhyMode(WIFI_PHY_MODE_11G);
	WiFi.setOutputPower(17); // max. power 802.11n 14dBm ; 802.11g 17dBm ; 802.11b 20dBm
	WiFi.macAddress(mac);
	sprintf(txt, StringF("MAC: %02x:%02x:%02x:%02x:%02x:%02x").c_str(), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	SERIAL_DBG_PORT.println(txt);

	Init_WebServer();
	Init_FSBrowser();  //sets up all the necessary server paths
	Init_HTTPupdater();

	SERIAL_DBG_PORT.printlnF("Init Done!\n");               // to monitor
	digitalWrite(BlueLED, HIGH); // LED OFF
}
// END setup()

/*int lastButton;
bool ReadButton()
{
	static int oldState;
	int newState;
	bool eventHappened = false;

	newState = digitalRead(BTN);

	if ((newState == LOW) && (oldState == HIGH))
	{
		lastButton = 1;
		eventHappened = true;
	}
	oldState = newState;
	return eventHappened;
}*/

void CheckForAnyHttpRequests()
{
//	for (int i = 0; i < 1000; i++)
//	{
		httpServer.handleClient();
//		delay(1);
//	}
}


void ProcessServerReply(String reply)
{
	//if (reply == StringF("update")) GetFWUpdate();
}
/*
void PrintHeapStatistics()
{
	// we could use getFreeHeap() getMaxFreeBlockSize() and getHeapFragmentation()
	// or all at once:
	uint32_t free;
	uint16_t max;
	uint8_t frag;
	char txt[100];
	ESP.getHeapStats(&free, &max, &frag);
	sprintf(txt,StringF("HEAP stats: free: %5d - max: %5d - frag: %3d%%").c_str(), free, max, frag);
	Log(String(txt), LL_MEM_STATS);
}
*/
#define THIS_STATION_ID  0xFE
#define STYPE_Tds18b20		0x01
#define STYPE_Tsht21			0x02
#define STYPE_Hsht21			0x03
#define STYPE_Tbmp280			0x04
#define STYPE_Pbmp280			0x05
#define STYPE_TankLevel		0x06
//Expanded types (that NRF link doesnt know, but WiFi link does)
#define STYPE_FurnaceTemperatures  0x11
#define STYPE_HeatStorageLevels   0x12

String LastUARTmsg;
int msgCnt;

void DecodeMessage(String msg)
{
	uint32_t local_timestamp;
	uint16_t type=0;
	boolean valueParsed = false;
	boolean dataMissing = false;
	boolean nonessentialDataMissing = false;
	uint16_t ID=0;
	float value=-999.99;
	float Vdd=-1;
	uint8_t retries=-1;
	uint8_t SensorNumber = 0;
	int i;

	local_timestamp = millis();

	if (msg.indexOf("SoftAP") >= 0)
	{
		//CreateSoftAP();
		return;
	}

	if (msg.indexOf(StringF("Type=")) >= 0) type = msg.substring(msg.indexOf("Type=") + 5).toInt();

	if (type == STYPE_HeatStorageLevels)
	{
		if (msg.indexOf(StringF("Tlist=")) >= 0)
		{
			String Tlist = msg.substring(msg.indexOf(StringF("Tlist=")) + 6);
			for (i = 0; i < MAX_HEAT_STR_SENSORS; i++)
			{
				value = Tlist.toDouble();
				//SERIAL_DBG_PORT.print(String(i)+": "+Tlist+"   val=");
				//SERIAL_DBG_PORT.println(value);
				HeatStorage.updateT(i, value);
				if (Tlist.indexOf(StringF(";")) < 0) break;
				Tlist = Tlist.substring(msg.indexOf(StringF(";")) + 1);
			}
			
			if (i< MAX_HEAT_STR_SENSORS)
			{
				//Something's not right. Debug msg?
				msgCnt++;
				LastUARTmsg = msg;
				Log(StringF("DecodeHSL(")+String(msgCnt)+StringF("):")+ LastUARTmsg, LL_DEBUG_MSG);
			}
		}
	}
	else if (type == STYPE_FurnaceTemperatures)
	{
		if (msg.indexOf(StringF("Tch=")) >= 0)
		{
			ChimneyTemperatures.storeT(msg.substring(msg.indexOf(StringF("Tch=")) + 4).toFloat());
		}
		else dataMissing = true;
		if (msg.indexOf(StringF("Tfb=")) >= 0)
		{
			FireBoxTemperatures.storeT(msg.substring(msg.indexOf(StringF("Tfb=")) + 4).toFloat());
		}
		else dataMissing = true;

		if (dataMissing)
		{
			//Something's not right. Debug msg?
			msgCnt++;
			LastUARTmsg = msg;
			Log(StringF("DecodeFT(") + String(msgCnt) + StringF("):") + LastUARTmsg, LL_DEBUG_MSG);
		}
	}
	else if ( (type == STYPE_Tds18b20) ||
			  (type == STYPE_Tsht21) ||
			  (type == STYPE_Hsht21) ||
			  (type == STYPE_Tbmp280) ||
			  (type == STYPE_Pbmp280) ||
			  (type == STYPE_TankLevel) )
	{
		if (msg.indexOf(StringF("ID=")) >= 0)
		{
			ID = msg.substring(msg.indexOf("ID=") + 3).toInt();
			if (ID == 0) dataMissing = true;
		}
		else dataMissing = true;
		if (msg.indexOf(StringF("val=")) >= 0)
		{
			value = msg.substring(msg.indexOf("val=") + 4).toFloat();
			valueParsed = true;
		}
		else dataMissing = true;
		if (msg.indexOf(StringF("Vdd=")) >= 0)
		{
			Vdd = msg.substring(msg.indexOf("Vdd=") + 4).toFloat();
			if (Vdd == 0.0) nonessentialDataMissing = true;
		}
		else nonessentialDataMissing = true;
		if (msg.indexOf(StringF("rtr=")) >= 0)
		{
			retries = msg.substring(msg.indexOf("rtr=") + 4).toInt();
		}
		else nonessentialDataMissing = true;
		
		if (!dataMissing) MeasurementResults.storeResult(local_timestamp, type, ID, value, Vdd, retries);
		else
		{
			//Data Missing. Debug?
			msgCnt++;
			LastUARTmsg = msg;
			Log(StringF("DecodeSensorDataMissing(") + String(msgCnt) + StringF("):") + LastUARTmsg, LL_DEBUG_MSG);
		}
		
		if (nonessentialDataMissing)
		{
			//Nonessential Data Missing. Debug?
			msgCnt++;
			LastUARTmsg = msg;
			Log(StringF("DecodeNonessentialDM(") + String(msgCnt) + StringF("):") + LastUARTmsg, LL_DEBUG_MSG);
		}
	}
	else
	{
		//Unknown type. Debug?
		msgCnt++;
		LastUARTmsg = msg;
		Log(StringF("DecodeUnknown(") + String(msgCnt) + StringF("):") + LastUARTmsg, LL_DEBUG_MSG);
	}
}



void ProcessSerialData()
{
	static String SerialMsg;
	String part1;
	char SerialBuffer[100];
	int msgEnd;
	int numRead;
	int NumAvailableBytes = Serial.available();

	if (NumAvailableBytes > 0)
	{
		if (NumAvailableBytes > 99) NumAvailableBytes = 99;	//limit to the size of buffer
		numRead=Serial.readBytes(SerialBuffer, NumAvailableBytes);
		SerialBuffer[numRead] = 0;
		SerialMsg += SerialBuffer;

		while ((msgEnd = SerialMsg.indexOf("\n")) >= 0)
		{
			part1 = SerialMsg.substring(0, msgEnd);
			SerialMsg = SerialMsg.substring(msgEnd+1);
			DecodeMessage(part1);
		}
	}
}

// *********** main ***********
void loop()
{
	String reply;
	uint32_t lastConnectionAttempt = -DEFAULT_WIFI_CONNECT_INTERVAL;	//Works-out fine due to circular nature of binary numbers

	ProcessSerialData();
	if (WiFi.status() == WL_CONNECTED)
	{
		if (!bSysTimeSynced || ( (millis()-u32_LastSyncTime_ms) > NTP_SYNC_INTERVAL) )	GetTime();
		if (bSysTimeSynced)
		{
			reply = PostData();
			ProcessServerReply(reply);
		}
		CheckForAnyHttpRequests();
	}
	else if (WiFi.softAPIP().isSet())	//if SoftAP mode
	{
		CheckForAnyHttpRequests();
	}
	else 
	{
		//if connection fails, do not try to recconnect for a while - do not pollute during the night 
		if (millis() - lastConnectionAttempt > DEFAULT_WIFI_CONNECT_INTERVAL)
		{
			lastConnectionAttempt = millis();
			ConnectToWiFi();
		}
	}
}
