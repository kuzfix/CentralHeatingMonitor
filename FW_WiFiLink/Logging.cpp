#include "Logging.h"

#define MAX_LOG_FILE_SIZE	(200*1024)
uint8_t LogLevel = DEFAULT_LOG_LEVEL;

bool BeginLog(void) {

	File logFile;
	String logString;

	logFile = SPIFFS.open(StringF("/LOG/log.htm") , "r");	//test if Log file can be opened
	if (!logFile)
	{
		logFile = SPIFFS.open(StringF("/LOG/log.htm") , "w");
		if (!logFile) {
			SERIAL_DBG_PORT.printlnF("Failed to open logFile file for writing");
			return false;
		}
		logString = StringF("Log of: ") + String(DeviceName);
		logString += StringF("\r\n\r\nLog levels (LL): 0 = NONE, 1 = SYSTEM_ERRORS , 2 = CONFIG_ERRORS , 3 = CONNECTION_ERRORS, 4 = NTP_ERRORS, 5 = DATA_UPLOAD_ERRORS, 7 = WEB_COMMANDS, 10 = NTP_STATS, 11 = DATA_UPLOAD_STATS,");
		logString += StringF("\r\n20 = SYSTEM_WARNINGS, 21 = SYSTEM_MESSAGES, 25 = CONFIG_MESSAGES, 30 = CONNECTION_MESSAGES, 35 = NTP_MESSAGES, 40 = DATA_UPLOAD_MESSAGES, 50 = MEMORY STATISTICS");
		logString += StringF("\r\n\r\n    timestamp        , LL,  Description");
		logFile.write((uint8_t*)logString.c_str(), logString.length());
	}
	logFile.close();
	return true;
}

#define MAX_NUMBER_OF_OLD_FILES	13	//(3MB/200kB - 2)
#define MAX_FILE_NUMBER 99999

bool prepareLogFile() // checks log size, and if it is bigger than 200kB moves it to_old and create a new empty one
{
	File logFile;
	size_t logFileSize;
	String fileName;
	Dir dir;
	int i_maxFileNum = -1, i_minFileNum = MAX_FILE_NUMBER, i_fnum, i_oldFiles = 0;

	logFile = SPIFFS.open(StringF("/LOG/log.htm"), "a");
	if (!logFile)
	{
		SERIAL_DBG_PORT.printlnF("Failed to open logFile file for writing!");
		return false; //Should be created by now. If not, abort.
	}

	logFileSize = logFile.size();
	if (logFileSize > MAX_LOG_FILE_SIZE)
	{
		logFile.write(StringF("/////////// LOG END - continued in next file ///////////").c_str(), 56);
		logFile.close();
		SERIAL_DBG_PORT.println(StringF("Max Log file size exceeded (") + String(logFileSize) + StringF("/")
			+ String(MAX_LOG_FILE_SIZE) + StringF("B). Attempting to create a new one..."));

		dir = SPIFFS.openDir(StringF("/LOG"));
		while (dir.next()) {
			fileName = dir.fileName();
			if (fileName.startsWith(StringF("/LOG/old")))
			{
				fileName.remove(0, 8);
				if (fileName[0] >= '0' && fileName[0] <= '9')
				{
					i_fnum = fileName.toInt();
					i_oldFiles++;
					if (i_fnum > i_maxFileNum) i_maxFileNum = i_fnum;
					if (i_fnum < i_minFileNum) i_minFileNum = i_fnum;
				}
			}
		}
		if (i_maxFileNum >= MAX_FILE_NUMBER)	//overflow of file numbers: delete all old files
		{
			dir.rewind();
			while (dir.next()) {
				fileName = dir.fileName();
				if (fileName.startsWith(StringF("/LOG/old")))
				{
					SPIFFS.remove(fileName.c_str());
					SERIAL_DBG_PORT.println(StringF("\nDeleting ") + String(fileName));
				}
			}
			i_maxFileNum = -1;
		}
		else if (i_oldFiles > MAX_NUMBER_OF_OLD_FILES)
		{
			fileName = StringF("/LOG/old") + String(i_minFileNum) + StringF(".htm");
			SPIFFS.remove(fileName.c_str());
			SERIAL_DBG_PORT.println(StringF("\nDeleting ") + String(fileName));
		}
		fileName = StringF("/LOG/old") + String(i_maxFileNum + 1) + StringF(".htm");
		SPIFFS.rename(StringF("/LOG/log.htm"), fileName.c_str());
		SERIAL_DBG_PORT.println(StringF("\nRenaming /LOG/log.htm to ") + String(fileName));
	}
	BeginLog(); // cerates new log.htm file with headers
	return true;
}

void Log(String LogEntry, int Level)
{
	File logFile;
	String logString;
	time_t now;
	struct tm* ptm_now;
	char txt[50];
	int length;
	WiFiClient client;
	HTTPClient httpClient;
	String serverMsg;

	if (Level <= LogLevel)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			serverMsg = StringF(DEBUG_MESSAGE_SERVER) + StringF("?dbgMsg=") + LogEntry;
			serverMsg.replace("%", "%25"); //ta mora bit vedno prvi, ker drugace spremeni procente od ostalih
			serverMsg.replace("#", "%23");
			serverMsg.replace(" ", "%20");
			serverMsg.replace("\n", "");	//any line feed characters break HTTP communication (ctime adds one)
			serverMsg.replace("\r", "");	//not sure about these, but probably better if there are none
			httpClient.begin(client, serverMsg);
			httpClient.GET();
		}
		if (prepareLogFile())
		{
			logFile = SPIFFS.open(StringF("/LOG/log.htm"), "a");
			if (logFile)
			{
				if (bSysTimeSynced)
				{
					now = millis2UNIXtime(millis());
					ptm_now = gmtime(&now);
					length = strftime(txt, 30, StringF("\r\nUTC %Y/%m/%d %H:%M:%S,  ").c_str(), ptm_now);
				}
				else
				{
					now = millis();
					sprintf(txt, StringF("\r\nUnsynced - %10u,  ").c_str(), now);
				}
				logString = String(txt) + String(Level) + StringF(",  ") + LogEntry;

				logFile.write((uint8_t*)logString.c_str(), logString.length());
				logFile.close();
				SERIAL_DBG_PORT.println(logString);
			}
		}
	}
}
