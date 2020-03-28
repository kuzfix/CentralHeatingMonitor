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

const char htmlHeadPt1[] PROGMEM = R"====(
<style> table, th, td { border: 1px solid black; border-collapse: collapse; } </style> </head>
<body><center>
<h3> Device: )====";

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
	int i;
	float HSPercent;
	float ChimneyT, ChimneyTmin, ChimneyTmax;
	float FireBoxT, FireBoxTmin, FireBoxTmax;
	float HeatStorageT[MAX_HEAT_STR_SENSORS];

	HeatStorage.getT(HeatStorageT);
	HSPercent = HeatStorage.getPercentage();

	message = StringF("{");
	message += StringF("\"HSPercent\":") + String(HSPercent, 3);
	message += StringF(", \"HSTemps\": [");
	for (i = MAX_HEAT_STR_SENSORS - 1; i > 0; i--)
	{
		message += String(HeatStorageT[i], 3)+ StringF(", ");
	}
	message += String(HeatStorageT[i], 3) + StringF("]");
	
	message += StringF(", \"ChimneyT\": [");
	for (i = 0; i < ChimneyTemperatures.getNumberOfStoredT()-1 ; i++)
	{
		ChimneyTemperatures.peekT(i, &ChimneyT, &ChimneyTmin, &ChimneyTmax);
		message += String(ChimneyT, 1) + StringF(",");
	}
	ChimneyTemperatures.peekT(i, &ChimneyT, &ChimneyTmin, &ChimneyTmax);
	message += String(ChimneyT, 1) + StringF("]");
	
	message += StringF(", \"FireBoxT\": [");
	for (i = 0; i < FireBoxTemperatures.getNumberOfStoredT() - 1; i++)
	{
		FireBoxTemperatures.peekT(i, &FireBoxT, &FireBoxTmin, &FireBoxTmax);
		message += String(FireBoxT, 1) + StringF(",");
	}
	FireBoxTemperatures.peekT(i, &FireBoxT, &FireBoxTmin, &FireBoxTmax);
	message += String(FireBoxT, 1) + StringF("]");
	//DEBUG data
/*	message += StringF(", \"buf_IN\":") + String(MeasurementResults.buf_IN);
	message += StringF(", \"buf_OUT\":") + String(MeasurementResults.buf_OUT);
	message += StringF(", \"buf_N\":") + String(MeasurementResults.buf_N);
	message += StringF(", \"buffer\": [");
	for (i = 0; i < MEASUREMENT_RESULT_BUFFER_SIZE - 1; i++)
	{
		message += StringF("[")+String(MeasurementResults.results[i].ID) 
			+ StringF(",") + String(MeasurementResults.results[i].value,1)+ StringF("],");
	}
	message += StringF("[") + String(MeasurementResults.results[i].ID)
		+ StringF(",") + String(MeasurementResults.results[i].value, 1) + StringF("]]");
		*/
	message += StringF("}");

	HTTP_SERVER.send(200, StringF("text/json"), message);
}

#if 0
const char javaScriptMainPage[] PROGMEM = R"====(
<link rel="stylesheet" type="text/css" href="chartist.css">
        <style type="text/css">
.wrapper { 
  overflow:hidden;
}
.wrapper div {
   background-color: white;
   min-height: 100px;
   padding: 10px;
 }

.flt-div {
  float:left; 
  max-width:300px;
  border-style: solid 1px;
}
</style>
<script type="text/javascript" src="chartist.js"></script>
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
			}
			if (measData.hasOwnProperty('HSPercent')) {
				AddHSPercent(measData.HSPercent);
			}
			if (measData.hasOwnProperty('FireBoxT')) {
			    FireTdata.series[0]=[];
				measData.FireBoxT.forEach(function(element){
					FireTdata.series[0].push(element);
				});
				ChartFireBoxT.update(FireTdata);
			}
			if (measData.hasOwnProperty('ChimneyT')) {
			    ChimTdata.series[0]=[];
				measData.ChimneyT.forEach(function(element){
					ChimTdata.series[0].push(element);
				});
				ChartChimneyT.update(ChimTdata);
			}
			if (measData.hasOwnProperty('HSTemps')) {
				let sensorCnt=0;
				measData.HSTemps.forEach(function(element){
					UpdateHSTemp(sensorCnt, element);
					sensorCnt++;
				});
			}

			if (measData.hasOwnProperty('buf_IN') &&
				measData.hasOwnProperty('buf_OUT') &&
				measData.hasOwnProperty('buf_N') ) 
			{
				if (document.getElementById("dbg1") !== null ) {
					document.getElementById("dbg1").innerHTML = "IN=" + measData.buf_IN + "<br>" +
						"OUT=" + measData.buf_OUT + "<br>" +
						"N=" + measData.buf_N;
				}
			}
			if (measData.hasOwnProperty('buffer'))
			{
				if (document.getElementById("dbg2") !== null ) {
					let div_txt="";
					let buf_idx=0;
					measData.buffer.forEach(function(element){
						div_txt += buf_idx + ": ID=" + measData.buffer[buf_idx][0] + ", val=" +measData.buffer[buf_idx][1] + "<br>";
						buf_idx++;
					});
					document.getElementById("dbg2").innerHTML = div_txt;
				}
			}
			setTimeout(getData, 100);
		}
	};
	xhttp.open("GET", "ajax_data", true);
	xhttp.send();
}

var HSPdata = {
    series: [
      []
    ]
};

var ChimTdata = {
      series: [[]]
};

var FireTdata = {
      series: [[]]
};
var HSTdata = {
    labels: ['Storage'],
    series: [[{x:0, y:10}],[{x:0, y:10}],[{x:0, y:10}],[{x:0, y:10}],
             [{x:0, y:10}],[{x:0, y:10}],[{x:0, y:10}],[{x:0, y:10}],
             [{x:0, y:10}],[{x:0, y:10}],[{x:0, y:10}],[{x:0, y:10}],
             [{x:0, y:10}],[{x:0, y:10}],[{x:0, y:10}],[{x:0, y:10}],
             [{x:0, y:10}] ]
};

function AddHSPercent(value)
{
    if (value<0) value = 0;
    if (value>100) value = 100;
    HSPdata.series[0].push(value);
    ChartHSP.update(HSPdata);
}

function UpdateHSTemp(sensorN, value)
{
    if (value<0) value = 0;
    if (value>60) value = 60;
    if (sensorN < 18)
    {
        HSTdata.series[sensorN][0].x=value;
    }
    else
	{
		HSTdata.series[0][0].x=value;
	}
    ChartHST.update(HSTdata);
}
function onBodyLoad(){
    ChartChimneyT = new Chartist.Line('#CHTemperature', ChimTdata, {lineSmooth: false, axisX: { showGrid: false, showLabel: false}});
    ChartHSP = new Chartist.Line('#HSPercent', HSPdata, {showPoint: false, axisX: { showGrid: false, showLabel: false}});
    ChartHST = new Chartist.Bar('#HSTemp', HSTdata, {stackBars: true});
    ChartFireBoxT = new Chartist.Line('#FBTemperature', FireTdata,{axisX: { showGrid: false, showLabel: false}});
    
    ChartHST.on('draw', function(context) {
        if(context.type === 'bar') {
          var clr_val=Chartist.getMultiValue(context.value.x);
          if (clr_val > 60) clr_val=60;
          context.element.attr({
            style: 'stroke-width: 100px; stroke: hsl(' + Math.floor((60-clr_val) / 60 * 240) + ', 50%, 50%);'
          });
        }
    });
	setTimeout(getData, 10);
}

</script>
)====";
#else
const char javaScriptMainPage[] PROGMEM = R"====(
<link rel="stylesheet" type="text/css" href="chartist.css">
        <style type="text/css">
.wrapper { 
  overflow:hidden;
}
.wrapper div {
   background-color: white;
   min-height: 100px;
   padding: 10px;
 }

.flt-div {
  float:left; 
  max-width:300px;
  border-style: solid 1px;
}
</style>
<script type="text/javascript" src="chartist.js"></script>
<script type="text/javascript" src="ajaxScript.js">></script>
)====";

#endif
const char dataDisplayMainPage[] PROGMEM = R"====(
	<div class="wrapper">
        <div class="flt-div" id="dbg1"></div>
        <div class="flt-div" id="dbg2"></div>
    </div>
	<div id='measurements'>
		Waiting for data...
	</div>
    <div class="wrapper">
        <div class="ct-chart ct-minor-seventh flt-div" id="CHTemperature"></div>
        <div class="ct-chart ct-minor-seventh flt-div" id="HSPercent"></div>
        <div class="ct-chart ct-minor-seventh flt-div" id="HSTemp"></div>
        <div class="ct-chart ct-minor-seventh flt-div" id="FBTemperature"></div>
    </div>
	
)====";
/*<div id = 'HeatStrTemps'>
Waiting...
< / div>*/
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
	message += StringF("<br>Last UART msg(") + String(msgCnt) + StringF("):") + LastUARTmsg;
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
