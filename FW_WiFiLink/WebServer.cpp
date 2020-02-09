#include "WebServer.h"
using namespace std;

//TODO: make sure Name length is unified everywhere and make sure hostname is according to standards (allowed characters, length,...)

// WiFi Settings and related variables
char Host_Name[30] = "NoName";
ESP8266WebServer HTTP_SERVER(80);
uint32_t	lastServerActivity;

const char HtmlHeader[] PROGMEM = R"====(
<!DOCTYPE html>
<html><head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
)====";

const char HtmlCalPopUpStyle[] PROGMEM = R"====(
.rawvals {
	text-align: right;
	position: fixed;
	z-index:9999999;
	top: 15px;
	right: 15px;
	-webkit-box-sizing: content-box;
	-moz-box-sizing: content-box;
	box-sizing: content-box;
	padding: 20px;
	overflow: hidden;
	border: none;
	-webkit-border-radius: 24px;
	border-radius: 24px;
	font: normal 32px/1 Verdana, Geneva, sans-serif;
	color: rgba(255,255,255,1);
	text-align: center;
	-o-text-overflow: ellipsis;
	text-overflow: ellipsis;
	background: rgba(1, 153, 217, 0.5);
	-webkit-box-shadow: 4px 4px 10px 0 rgba(0, 0, 0, 0.7);
	box-shadow: 4px 4px 10px 0 rgba(0, 0, 0, 0.7);
	text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
	}
)====";

const char htmlHeadPt1[] PROGMEM = R"====(
<style> table, th, td { border: 1px solid black; border-collapse: collapse; } </style> </head>
<body><center>
<h3> Device: )====";

bool SetSoftAPStart()
{
	lastServerActivity = millis();
	return true;
}
/*
uint32_t HandleHttpRequests()
{
	static uint32_t u32_lastStatusUpdate = 0;
	static int i_lineBreak = 0;
	int numberOfConnections;
	bool connection = true;
	String symbol;

	if ((WiFi.status() != WL_CONNECTED) && (!WiFi.softAPIP().isSet()))
	{
		if (!ConnectToWiFi())
		{
			connection = CreateSoftAP();
			lastServerActivity = millis();
		}
	}


	symbol = "!";
	if (WiFi.softAPgetStationNum() > 0)	//no timeout as long as someone is connected
	{
		HTTP_SERVER.handleClient();
		lastServerActivity = millis();
		symbol = "c";
	}
	else if (connection)
	{
		HTTP_SERVER.handleClient();
		symbol = "u";
	}
	
	//Just show that we are alive in the debug console:
	if (millis() - u32_lastStatusUpdate > 1000)
	{
		if (millis() - u32_lastStatusUpdate > 2000)
		{
			u32_lastStatusUpdate = millis();
		}
		else
		{
			u32_lastStatusUpdate += 1000;
		}
		SERIAL_DBG_PORT.print(symbol);
		i_lineBreak++;
		if (i_lineBreak == 60)
		{
			SERIAL_DBG_PORT.printF("\r\n");
			i_lineBreak = 0;
		}
	}
	return lastServerActivity;
}
*/

void handleAjaxData(void) {
	String message;
	float HSPercent;
	float ChimneyT, ChimneyTmin, ChimneyTmax;
	float FireBoxT, FireBoxTmin, FireBoxTmax;

	HSPercent = HeatStorage.getPercentage();
	ChimneyTemperatures.peekT(&ChimneyT, &ChimneyTmin, &ChimneyTmax);
	FireBoxTemperatures.peekT(&FireBoxT, &FireBoxTmin, &FireBoxTmax);

	message = StringF("{");
	message += StringF("\"HSPercent\":") + String(HSPercent, 3);
	message += StringF(", \"ChimneyT\":") + String(ChimneyT, 3);
	message += StringF(", \"FireBoxT\":") + String(FireBoxT, 3);
	message += StringF("}");

	HTTP_SERVER.send(200, StringF("text/json"), message);
}

const char javaScriptMainPage[] PROGMEM = R"====(
<script type="text/javascript" src="graphs.js"></script>
<script type="text/javascript">
function getData() {
	var xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			var measData = JSON.parse(this.responseText);
			if (measData.hasOwnProperty('HSPercent') && 
				measData.hasOwnProperty('FireBoxT') && 
				measData.hasOwnProperty('ChimneyT')) {
				if (document.getElementById("measurements") !== null ) {
					document.getElementById("measurements").innerHTML = measData.HSPercent + " &percnt;<br>" + 
						measData.FireBoxT + " &deg;C<br>" +
						measData.ChimneyT + " &deg;C";
				}
				HeatStorePercent.add(measData.HSPercent);
				ChimneyT.add(measData.ChimneyT);
			}
			if (measData.hasOwnProperty('HSPercent')) {
				HeatStorePercent.add(measData.HSPercent);
			}
			if (measData.hasOwnProperty('FireBoxT')) {
				FireBoxT.add(measData.FireBoxT);
			}
			if (measData.hasOwnProperty('ChimneyT')) {
				ChimneyT.add(measData.ChimneyT);
			}
			setTimeout(getData, 1000);
		}
	};
	xhttp.open("GET", "ajax_data", true);
	xhttp.send();
}

function onBodyLoad(){
    HeatStorePercent = createGraph(document.getElementById("HSPercent"), "Temperatura zalogovnika [&percnt;]", 100, 200, 0, 100, true, "green");
    ChimneyT = createGraph(document.getElementById("CHTemperature"), "Temperatura v dimniku", 100, 125, 0, 500, false, "orange");
    FireBoxT = createGraph(document.getElementById("FBTemperature"), "Temperatura v peci", 100, 125, 0, 1400, false, "red");
	setTimeout(getData, 10);
}
</script>
)====";

const char dataDisplayMainPage[] PROGMEM = R"====(
	<div id='CHTemperature'></div>
	<div id='HSPercent'></div>
	<div id='FBTemperature'></div>
	<div id='measurements'>
		Waiting for data...</br>
		Waiting for data...
	</div>
)====";
extern String LastUARTmsg;
extern int msgCnt;
void handleRoot(void)
{
	Log(StringF("Index accessed."), LL_WEB_COMMANDS);
	String message = String(FPSTR(HtmlHeader)) + StringF("\n<title>");
	message += Host_Name;
	message += StringF("</title> ");
	message += String(FPSTR(javaScriptMainPage));
	message += StringF("</head>");
	message += StringF("<body onload=\"onBodyLoad()\"><center>");
	message += StringF("<h3>");
	message += Host_Name;
	message += StringF("</h3>");
	message += StringF("<p> Server address: ");
	message += strServerAddress;
	message += StringF("</p>");

	message += String(FPSTR(dataDisplayMainPage));

	message += StringF("<p>DEBUG INFO:<br>");
	message += StringF("<br>millis():") + String(millis());
	message += StringF(" last_min_start:") + String(ChimneyTemperatures.last_min_start);
	message += StringF(" dt:") + String(millis()-ChimneyTemperatures.last_min_start);
	message += StringF("<br>iNsum:") + String(ChimneyTemperatures.iNsum);
	message += StringF("<br>fTsum:") + String(ChimneyTemperatures.fTsum);
	message += StringF("   fTsum/iNsum:") + String(ChimneyTemperatures.fTsum/ ChimneyTemperatures.iNsum);
	message += StringF("<br>fTmax:") + String(ChimneyTemperatures.fTmax);
	message += StringF("<br>fTmin:") + String(ChimneyTemperatures.fTmin);
	message += StringF("<br>buf_IN:") + String(ChimneyTemperatures.buf_IN);
	message += StringF("<br>buf_OUT:") + String(ChimneyTemperatures.buf_OUT);
	message += StringF("<br>buf_N:") + String(ChimneyTemperatures.buf_N);
	message += StringF("<br>T[0]:") + String(ChimneyTemperatures.T[0]);
	message += StringF("<br>T[1]:") + String(ChimneyTemperatures.T[1]);
	message += StringF("<br>T[buf_IN-1]:") + String(ChimneyTemperatures.T[ChimneyTemperatures.buf_IN - 1]);
	message += StringF(" T[buf_IN-1]/fact:") + String((double)ChimneyTemperatures.T[ChimneyTemperatures.buf_IN - 1]/ (float)BT_FLOAT_INT_FACTOR);
	message += StringF("<br>Last UART msg(") + String(msgCnt) + StringF("):") + LastUARTmsg;

	LastUARTmsg = "";
	message += StringF("</p>");

	message += StringF("</br>");
	message += StringF("<a href='chg_name'>Change Name, ID, Position and Log level</a>");
	message += StringF("</br>");
	message += StringF("<a href='confWiFi'>Configure WiFi connection</a>");
	message += StringF("</br>");
	message += StringF("<a href='confConfigMisc'>Advanced Configuration</a>");
	message += StringF("</br></br><small><i>");
	message += StringF("FWtime: ");
	message += StringF(" " __DATE__ " at " __TIME__);
	message += StringF("</br>");

	message += StringF("LogLevel=");
	message += LogLevel;
	message += StringF("</br>");
	message += StringF("</i></small></center></body>");

	HTTP_SERVER.send(200, StringF("text/html"), message);
}

//TODO: Add javascript to encrypt passwords before sending and decrypt them here.
void handleWiFiPasswords(void)
{
	String SSIDtmp, passtmp;
	String message = String(FPSTR(HtmlHeader)) + StringF("\n<title>") + String(Host_Name) + StringF("</title>");
	message += String(FPSTR(htmlHeadPt1));
	message += Host_Name;
	message += StringF("</h3><form action='chgWiFi'><TABLE>");
	
	SSIDtmp.clear();
	passtmp.clear();
	if (GetWiFiCredentials(0))
	{
		SSIDtmp = SSID;
		passtmp = StringF("**********");
	}
	message += StringF("<tr align = 'center'><td>SSID0</td><td>");
	message += StringF("<input type='text' name='SSID0' title='WiFi network name' value='"); // Patern restricts host name to letters and numbers from 3 to 9 charatcters
	message += SSIDtmp + StringF("'> </td></tr>");
	message += StringF("<tr align='center'><td>pass0</td><td>");
	message += StringF("<input type='text' name='pass0' title='WiFi password' value='");
	message += passtmp + StringF("'> </td></tr>");

	SSIDtmp.clear();
	passtmp.clear();
	if (GetWiFiCredentials(1))
	{
		SSIDtmp = SSID;
		passtmp = StringF("**********");
	}
	message += StringF("<tr align = 'center'><td>SSID1</td><td>");
	message += StringF("<input type='text' name='SSID1' title='WiFi network name - alternative 1' value='"); 
	message += SSIDtmp + StringF("'> </td></tr>");
	message += StringF("<tr align='center'><td>pass1</td><td>");
	message += StringF("<input type='text' name='pass1' title='WiFi password' value='");
	message += passtmp + StringF("'> </td></tr>");

	SSIDtmp.clear();
	passtmp.clear();
	if (GetWiFiCredentials(2))
	{
		SSIDtmp = SSID;
		passtmp = StringF("**********");
	}
	message += StringF("<tr align = 'center'><td>SSID2</td><td>");
	message += StringF("<input type='text' name='SSID2' title='WiFi network name - alternative 2' value='"); 
	message += SSIDtmp + StringF("'> </td></tr>");
	message += StringF("<tr align='center'><td>pass2</td><td>");
	message += StringF("<input type='text' name='pass2' title='WiFi password' value='");
	message += passtmp + StringF("'> </td></tr>");

	message += StringF("<tr align='center'>");
	message += StringF("<td colspan='2'>");
	message += StringF("<input type='submit' value='Change WiFi settings'>");
	message += StringF("<input type='button' onclick=\"window.location.href = '/'\" value='Cancel'>");
	message += StringF("</td>");
	message += StringF("</tr>");
	message += StringF("</TABLE></br>");
	message += StringF("</form>");

	message += StringF("<p>Warning! Passwords are sent over the network in plain text. Use this form within a safe local network only!</p>");
	message += StringF("<p>Delete all SSIDs to restore default SSID and password.</p>");

	message += StringF("<a href='/'>Back to main page</a>");
	message += StringF("</center></body>");

	HTTP_SERVER.send(200, StringF("text/html"), message);
}

void handleChangePass() {
	String SSID[3], pass[3];
	int i;

	//LIST all GET/POST parameters on SERIAL_DBG_PORT port
//	SERIAL_DBG_PORT.println();
	for (i = 0; i < HTTP_SERVER.args(); i++)
	{
//		SERIAL_DBG_PORT.print(HTTP_SERVER.argName(i));
//		SERIAL_DBG_PORT.printF(" = ");
//		SERIAL_DBG_PORT.println(HTTP_SERVER.arg(i));
		if (HTTP_SERVER.argName(i) == StringF("SSID0"))	SSID[0] = HTTP_SERVER.arg(i);
		if (HTTP_SERVER.argName(i) == StringF("SSID1"))	SSID[1] = HTTP_SERVER.arg(i);
		if (HTTP_SERVER.argName(i) == StringF("SSID2"))	SSID[2] = HTTP_SERVER.arg(i);
		if (HTTP_SERVER.argName(i) == StringF("pass0"))	pass[0] = HTTP_SERVER.arg(i);
		if (HTTP_SERVER.argName(i) == StringF("pass1"))	pass[1] = HTTP_SERVER.arg(i);
		if (HTTP_SERVER.argName(i) == StringF("pass2"))	pass[2] = HTTP_SERVER.arg(i);
	}

	SaveWiFiCredentials(SSID, pass);
	handleRoot();
}

void handleConfigMisc(void)
{
	String message = String(FPSTR(HtmlHeader)) + StringF("\n<title>") + String(Host_Name) + StringF("</title>");
	message += String(FPSTR(htmlHeadPt1));
	message += Host_Name;
	message += StringF("</h3><form action='chgConfigMisc'><TABLE>");
	
	message += StringF("<tr align='center'><td>WiFi Connection Timeout (200ms multiple)</td><td>");
	message += StringF("<input type='number' name='WifiConTout' title='WiFi Connection Timeout' value='");
	message += String(i_WiFiConnectTimeout) + StringF("'> </td></tr>");

	message += StringF("<tr align = 'center'><td>Update server IP</td><td>");
	message += StringF("<input type='text' name='UpdateIP' title='Update server IP' value='");
	message += str_updateServer + StringF("'> </td></tr>");

	message += StringF("<tr align='center'><td>Update script path</td><td>");
	message += StringF("<input type='text' name='UpdateFile' title='Update script path' value='");
	message += str_updateFile + StringF("'> </td></tr>");

	message += StringF("<tr align = 'center'><td>Maximum number of upload retries</td><td>");
	message += StringF("<input type='number' name='NSendRtry' title='Maximum number of upload retries' value='");
	message += String(MeasurementResults.GetMaxSendRetries()) + StringF("'> </td></tr>");
	
	message += StringF("<tr align='center'><td>Local NTP server IP address</td><td>");
	message += StringF("<input type='text' name='NTPadr' title='Local NTP server IP address' value='");
	message += ip_NTPlocal.toString() + StringF("'> </td></tr>");

	message += StringF("<tr align = 'center'><td>NTP Port</td><td>");
	message += StringF("<input type='number' name='NTPPort' title='NTP Port' value='");
	message += String(i_LocalPort) + StringF("'> </td></tr>");

	message += StringF("<tr align = 'center'><td>Maximum number of NTP sync retries</td><td>");
	message += StringF("<input type='number' name='NTPRtry' title='Maximum number of NTP sync retries' value='");
	message += String(i_NTPServerRetries) + StringF("'> </td></tr>");

	message += StringF("<tr align = 'center'><td>NTP Server Timeout</td><td>");
	message += StringF("<input type='number' name='NTPTout' title='NTP Server Timeout' value='");
	message += String(i_NTPTimeout) + StringF("'> </td></tr>");

	message += StringF("<tr align = 'center'><td>Maximum NTP Sync Uncertainty</td><td>");
	message += StringF("<input type='number' name='NTPMuncert' title='Maximum NTP Sync Uncertainty' value='");
	message += String(i_MaxUncertainty_ms) + StringF("'> </td></tr>");

	message += StringF("<tr align = 'center'><td>Desired NTP Sync Uncertainty</td><td>");
	message += StringF("<input type='number' name='NTPDuncert' title='Desired NTP Sync Uncertainty' value='");
	message += String(i_DesiredUncertainty_ms) + StringF("'> </td></tr>");

	message += StringF("<tr align='center'>");
	message += StringF("<td colspan='2'>");
	message += StringF("<input type='submit' value='Change Misc settings'>");
	message += StringF("<input type='button' onclick=\"window.location.href = '/'\" value='Cancel'>");
	message += StringF("</td>");
	message += StringF("</tr>");
	message += StringF("</TABLE></br>");
	message += StringF("</form>");

	message += StringF("<p>Measurement interval has to be a divisor of 60000 and probably shouldn't be smaller than approximately 20000. Other values must be tested.</p>");

	message += StringF("<a href='/'>Back to main page</a>");
	message += StringF("</center></body>");
	
	HTTP_SERVER.send(200, StringF("text/html"), message);
}

void handleChangeConfigMisc() {
	String SSID0, pass0;
	String SSID1, pass1;
	String SSID2, pass2;
	bool allowSoftAP = false;
	int i;

	//LIST all GET/POST parameters on SERIAL_DBG_PORT port
	SERIAL_DBG_PORT.println();
	for (i = 0; i < HTTP_SERVER.args(); i++)
	{
		SERIAL_DBG_PORT.print(HTTP_SERVER.argName(i));
		SERIAL_DBG_PORT.printF(" = ");
		SERIAL_DBG_PORT.println(HTTP_SERVER.arg(i));
		if (HTTP_SERVER.argName(i) == StringF("WifiConTout"))	i_WiFiConnectTimeout = HTTP_SERVER.arg(i).toInt();
		if (HTTP_SERVER.argName(i) == StringF("UpdateIP"))		str_updateServer = HTTP_SERVER.arg(i);
		if (HTTP_SERVER.argName(i) == StringF("UpdateFile"))	str_updateFile = HTTP_SERVER.arg(i);
		if (HTTP_SERVER.argName(i) == StringF("NSendRtry"))		MeasurementResults.SetMaxSendRetries(HTTP_SERVER.arg(i).toInt());
		if (HTTP_SERVER.argName(i) == StringF("NTPPort"))		i_LocalPort = HTTP_SERVER.arg(i).toInt();
		if (HTTP_SERVER.argName(i) == StringF("NTPRtry"))		i_NTPServerRetries = HTTP_SERVER.arg(i).toInt();
		if (HTTP_SERVER.argName(i) == StringF("NTPTout"))		i_NTPTimeout = HTTP_SERVER.arg(i).toInt();
		if (HTTP_SERVER.argName(i) == StringF("NTPMuncert"))	i_MaxUncertainty_ms = HTTP_SERVER.arg(i).toInt();
		if (HTTP_SERVER.argName(i) == StringF("NTPDuncert"))	i_DesiredUncertainty_ms = HTTP_SERVER.arg(i).toInt();
		if (HTTP_SERVER.argName(i) == StringF("NTPadr"))
		{
			if (!ip_NTPlocal.fromString(HTTP_SERVER.arg(i))) ip_NTPlocal = IPAddress(DEFAULT_NTP_ADDRESS);
		}
	}

	SaveMiscConfig();
	handleRoot();
}


void handleESP_restart(void) 
{
    SERIAL_DBG_PORT.printlnF("manual remote reset");
    HTTP_SERVER.send(200, "text/plain", "Forced Reset");
    Log(F("Manual remote reset"), LL_WEB_COMMANDS);
    delay(100);
    ESP.restart();
}

void handleChangeName(void) 
{
	String message = String(FPSTR(HtmlHeader)) + StringF("\n<title>") + String(Host_Name) + StringF("</title>");
	message += String(FPSTR(htmlHeadPt1));
	message += Host_Name;
	message += StringF("</h3><form action='changeID'><TABLE><tr align='center'><td>Name</td><td>");
	message += DeviceName;
	message += StringF("</td><td>");
	message += StringF("<input type='text' name='NewName' pattern='[_A-Za-z0-9]{3,12}' title='Name of the device' value='"); // Patern restricts host name to letters and numbers from 3 to 9 charatcters
	message += String(DeviceName) + StringF("'> </td></tr>");

	message += StringF("<tr align='center'>");
	message += StringF("<td>");
	message += StringF("min. Log level");
	message += StringF("</td>");
	message += StringF("<td>");
	message += LogLevel;
	message += StringF("</td>");
	message += StringF("<td>");
	message += StringF("<input type='number' name='NewMinLogLevel' min='0' max='99' title='minimum Log level must be from 0 - 15' value='");
	message += LogLevel;
	message += StringF("'> </td>");
	message += StringF("</tr>");

	message += StringF("<tr align='center'>");
	message += StringF("<td>");
	message += StringF("Server address");
	message += StringF("</td>");
	message += StringF("<td>");
	message += strServerAddress;
	message += StringF("</td>");
	message += StringF("<td>");
	message += StringF("<input type='text' name='NewServerAddress' title='URL, e.g.: http://my.server.com/jubodata.php' value='");
	message += strServerAddress;
	message += StringF("'> </td>");
	message += StringF("</tr>");

	message += StringF("<tr align='center'>");
	message += StringF("<td colspan='3'>");
	message += StringF("<input type='submit' value='Change ID'>");
	message += StringF("<input type='reset' value='Reset to defaults'>");
	message += StringF("<input type='button' onclick=\"window.location.href = '/'\" value='Cancel'>");
	message += StringF("</td>");
	message += StringF("</tr>");
	message += StringF("</TABLE></br>");
	message += StringF("</form>");

	message += StringF("<a href='/'>Back to main page</a>");
	message += StringF("</center></body>");

	HTTP_SERVER.send(200, StringF("text/html"), message);
}

void handleChangeID() {
	String NewName,NewPos;
	int i;

	Log("Change name and ID", LL_WEB_COMMANDS);
	//LIST all GET/POST parameters on SERIAL_DBG_PORT port
	SERIAL_DBG_PORT.println();
	for (i = 0; i < HTTP_SERVER.args(); i++)
	{
		SERIAL_DBG_PORT.print(HTTP_SERVER.argName(i));
		SERIAL_DBG_PORT.print(" = ");
		SERIAL_DBG_PORT.println(HTTP_SERVER.arg(i));
		if (HTTP_SERVER.argName(i) == "NewName")        NewName  = HTTP_SERVER.arg(i);
		if (HTTP_SERVER.argName(i) == "NewMinLogLevel") LogLevel = HTTP_SERVER.arg(i).toInt();
		if (HTTP_SERVER.argName(i) == "NewServerAddress") strServerAddress = HTTP_SERVER.arg(i);
	}

	NewName.toCharArray(DeviceName, 10);
	sprintf(Host_Name, StringF("%s").c_str(), DeviceName);

	SaveConfig();
	LoadConfig();	//make sure nothing went wrong during the SAVE operation - show what was actually saved
	handleRoot();
}
  
void  handleWrite_Config() {    
   
    SaveConfig();
	Log(StringF("Write config"), LL_WEB_COMMANDS);
      
    String message = StringF("<center>");
    message += StringF("<p> Calibration data updated ");
    message += StringF("</br>");
    message += StringF("<a href='calibrate'>Calibrate</a>");
    message += StringF("</br></br>");
    message += StringF("<a href='update_off'>Disable update</a>");
    message += StringF("</center>");
    HTTP_SERVER.send(200, StringF("text/html"), message);    // Send same page so they can send another msg
  
    LoadConfig();   
}

void Init_WebServer(void)
{
	
	HTTP_SERVER.on(StringF("/confConfigMisc"), handleConfigMisc);
	HTTP_SERVER.on(StringF("/chgConfigMisc"), handleChangeConfigMisc);
	HTTP_SERVER.on(StringF("/confWiFi"), handleWiFiPasswords);
	HTTP_SERVER.on(StringF("/chgWiFi"), handleChangePass);
	HTTP_SERVER.on(StringF("/"), handleRoot);
	HTTP_SERVER.on(StringF("/chg_name"), handleChangeName);
	HTTP_SERVER.on(StringF("/ajax_data"), handleAjaxData);
	HTTP_SERVER.on(StringF("/changeID"), handleChangeID);
	HTTP_SERVER.on(StringF("/write_config"), handleWrite_Config);
	HTTP_SERVER.on(StringF("/esp_restart"), handleESP_restart);

	HTTP_SERVER.begin();
}
