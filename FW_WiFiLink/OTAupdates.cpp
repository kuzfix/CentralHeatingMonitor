#include "OTAupdates.h"

//#define USE_WEB_UPDATES

String str_updateServer = DEFAULT_UPDATE_SERVER;
String str_updateFile = DEFAULT_UPDATE_FILE;

//OTA - path, user, pass
const char update_path [] PROGMEM = "/fw__update";
const char update_username [] PROGMEM = "jubomaster";
const char update_password [] PROGMEM = "zajebanOgeslo3";

//Update security

#ifdef USE_WEB_UPDATES
const char pubkey[] PROGMEM = R"====(
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA+65eoFSFpAwirfKnV+kg
aLhtnK1n7y3v3wd4p0irRElAVS72sAwnKPa6g0L7RVpjhfBxiqPFkN2WDLeh+jsT
tvYNwxSzp3ZVObLKpy/i0H+GrAw8Li4S4AgFJ3tuKkL42J41U0nV9OsLKHwgIHiB
OVax9G0FtiHaY8iVb7ASMqF7Z/xUseG4Qop51xE+lybcHBHJ+se+sSQKnzoplbOR
4LZxuYFQM4z+Xbu/EGx8p4K6yhQf6QrMd2OZHnT2M69ADyO/23LqVxw47nZkIVdX
MyjUIntUXjrs3v8CXPTCscGB+8Kg/5pwYB7EfgPfJsw/6M1wNfBajYG1FdF6Cng6
1QIDAQAB
-----END PUBLIC KEY-----
)====";

BearSSL::PublicKey* signPubKey = nullptr;
BearSSL::HashSHA256* hash;
BearSSL::SigningVerifier* sign;
#endif

ESP8266HTTPUpdateServer httpUpdater;

void Init_HTTPupdater()
{
	httpUpdater.setup(&HTTP_SERVER, FPSTR(update_path), FPSTR(update_username), FPSTR(update_password));
}
#ifdef USE_WEB_UPDATES
void UPD_OnStart()
{
	SERIAL_DBG_PORT.printF("Starting update ");
}

void UPD_OnProgress(int a, int b)
{
	SERIAL_DBG_PORT.print(String((double)a/(double)b*100,1) + StringF("% "));
}

void UPD_OnError(int err)
{
	SERIAL_DBG_PORT.print(ESPhttpUpdate.getLastErrorString());
}

void UPD_OnEnd()
{
	SERIAL_DBG_PORT.printF("HTTP_UPDATE_OK");
}

void GetFWUpdate()
{
	WiFiClient client;
	bool updateSuccessfull = false;

	signPubKey = new BearSSL::PublicKey(pubkey);
	hash = new BearSSL::HashSHA256();
	sign = new BearSSL::SigningVerifier(signPubKey);

		ESPhttpUpdate.setLedPin(BlueLED, LOW);
	ESPhttpUpdate.onStart(UPD_OnStart);
	ESPhttpUpdate.onProgress(UPD_OnProgress);
	ESPhttpUpdate.onEnd(UPD_OnEnd);
	ESPhttpUpdate.onError(UPD_OnError);
	Update.installSignature(hash, sign);			//install signature
	t_httpUpdate_return ret = ESPhttpUpdate.update(client, str_updateServer ,80, str_updateFile, FW_VER);
	Update.installSignature(nullptr, nullptr);		//uninstall signature
	delete sign;
	delete hash;
	delete signPubKey;

	switch (ret) {
	//Other cases are missing because callbacks report every other eventuality
	case HTTP_UPDATE_NO_UPDATES:
		SERIAL_DBG_PORT.printlnF("HTTP_UPDATE_NO_UPDATES");
		break;
	case HTTP_UPDATE_OK:
	case HTTP_UPDATE_FAILED:	
		ESP.restart();	//this actually isn't necessary as the update function restarts on success anyway
						//restart is actually only necessary on error, because update process screws up NTP syncing
	}
}
#endif
