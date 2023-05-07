#include "NTP_TimeSync.h"

//NTP Protocol description:
//https://www.cisco.com/c/en/us/about/press/internet-protocol-journal/back-issues/table-contents-58/154-ntp.html

//TODO: test drift correction - is it going in the right direction???
//TODO: continue retrying to sync to NTP until good enough uncertainty achieved, but no more than MAX_NUM_RETRIES.

#define NTP_PACKET_SIZE 48 // NTP time stamp is in the first 48 bytes of the message
#define NTP_REFFERENCE_UNCERTAINTY	2
//#define NTP_MIN_TIME_DIF_TO_CALC_DRIFT				120000UL
//#define NTP_TIME_TO_RESET_REFFERENCE_TIME_POINTS	600000UL	
#define NTP_REF_TIME_BUFFER_SIZE					10		//amounts to 10 minutes, if measurement interval is 1 min
#define NTP_01_01_2019_MS	3755289600000ULL

void StoreAndPrintTimeKeeppingStatistics(uint32_t u32_SyncTime_ms, uint64_t u64_NTP_TimeStamp_ms, uint32_t u32_offset);
void UpdateTimeSyncVariables(uint32_t u32_SyncTime_ms, uint64_t u64_NTP_TimeStamp_ms, uint32_t u32_offset);

IPAddress ip_NTPlocal(DEFAULT_NTP_ADDRESS);
int i_LocalPort = NTP_LOCAL_PORT;
int i_NTPServerRetries = NTP_SERVER_RETRIES;
int i_NTPTimeout = NTP_TIMEOUT;
int i_MaxUncertainty_ms = NTP_MAX_UNCERTAINTY_MS;
int i_DesiredUncertainty_ms = NTP_DESIRED_UNCERTAINTY_MS;

uint64_t u64_SysTimeOffset_ms = 0;
uint32_t u32_timeSyncUncertanty_ms = -1;	//-1 = MAX uint
uint32_t u32_LastSyncTime_ms = 0;
boolean bSysTimeSynced = false;

boolean b_HighAccuracySyncAchieved = false;
uint64_t u64_RefferenceSysTimeOffset_ms = 0;
uint32_t u32_RefferenceSyncMillis_ms = 0;
double d_MillisDrift=0.0;

#ifdef DEBUG_TIME_KEEPING_STATS
uint64_t u64_DBG_SysTimeOffset_ms[DBG_TIM_KP_ST_LENGTH];
uint32_t u32_DBG_timeSyncUncertanty_ms[DBG_TIM_KP_ST_LENGTH];
int iNTPDBG = 0;
#endif

#define MAX_NTP_POOL_NAME	36	//50% larger, just in case

const char ntpServerName[][MAX_NTP_POOL_NAME] = {"0.pool.ntp.org" , "1.pool.ntp.org" , "2.pool.ntp.org" , "3.pool.ntp.org"};
int NtpFailed[] = {0 , 0 , 0 , 0 };
int iNTPpools = sizeof(ntpServerName) / MAX_NTP_POOL_NAME; // how many NTP time pools are in array considering MAX_NTP_POOL_NAME char per pool
WiFiUDP udp;// A UDP instance to let us send and receive packets over UDP
uint8_t packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address)
{
	//  SERIAL_DBG_PORT.println("sending NTP packet...");
	memset(packetBuffer, 0, NTP_PACKET_SIZE);   // set all bytes in the buffer to 0

  // Initialize values needed to form NTP request
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}

bool GetNTPTime(IPAddress timeServerIP)
{
	uint32_t u32_NTPStartTime, u32_NTPStopTime, u32_SyncTime_ms, u32_offset, u32_sec, u32_frac, u32_ms;
	uint64_t u64_NTP_TimeStamp_ms;
	int packetSize;
	int i;
	bool result = false;

	delay(100);
	for (i = 0;i < i_NTPServerRetries; i++)
	{
		SERIAL_DBG_PORT.printF("TimeSync: Requesting NTP packet (");
		SERIAL_DBG_PORT.print(timeServerIP);
		SERIAL_DBG_PORT.print(StringF(") try ")+String(i)+StringF("/")+String(i_NTPServerRetries)+StringF(" ..."));
		udp.begin(i_LocalPort);
		u32_NTPStartTime = millis();
		sendNTPpacket(timeServerIP); // send an NTP packet to a time server
		while ((millis() - u32_NTPStartTime) < i_NTPTimeout)
		{
			packetSize = udp.parsePacket();
			if (packetSize > 0)	break;
		}
		u32_NTPStopTime = millis();
		if (packetSize == NTP_PACKET_SIZE)
		{
			u32_offset = ceil((u32_NTPStopTime - u32_NTPStartTime) / 2.0);
			u32_SyncTime_ms = u32_NTPStartTime + u32_offset;
			SERIAL_DBG_PORT.println(StringF("received! Length = ") + String(packetSize) + StringF("Start systick:") + String(u32_NTPStartTime) +
				StringF("ms, Duration:") + String(u32_NTPStopTime - u32_NTPStartTime) +
				StringF("ms, MaxErr:") + String(u32_offset) + StringF("ms"));
			udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

			// "(Server)Transmit timestamp" is located on locations 40-47 (40-43 seconds, 44-47 fraction) (40=MSB)
			u32_sec = (packetBuffer[40] << 24) |
				(packetBuffer[41] << 16) |
				(packetBuffer[42] << 8) |
				(packetBuffer[43] << 0);
			u32_frac = (packetBuffer[44] << 24) |
				(packetBuffer[45] << 16) |
				(packetBuffer[46] << 8) |
				(packetBuffer[47] << 0);
			u32_ms = ((float)u32_frac / (1ULL << 32)) * 1000;	//convert fraction of a second to ms
			u64_NTP_TimeStamp_ms = u32_sec * 1000ULL + u32_ms;
			//Save TimeSync data if valid and accurate
			UpdateTimeSyncVariables(u32_SyncTime_ms, u64_NTP_TimeStamp_ms, u32_offset);
			result = true;
			udp.stop();
			break;
		}
		else
		{
			SERIAL_DBG_PORT.printlnF("failed!");
		}
		udp.stop();
	}
	return result;
}

uint32_t CalculateExpandedTimeUncertainty(uint32_t u32_time_ms)
{
	uint32_t u32_Uncertainty;

	u32_Uncertainty = u32_timeSyncUncertanty_ms + ceil((abs((int64_t)u32_time_ms - u32_LastSyncTime_ms)) / 60000.0 * NTP_DEFAULT_MILLIS_DRIFT);
	return u32_Uncertainty;
}

#ifdef DEBUG_TIME_KEEPING_STATS
void StoreAndPrintTimeKeeppingStatistics(uint32_t u32_SyncTime_ms, uint64_t u64_NTP_TimeStamp_ms, uint32_t u32_offset)
{
	char txt[100];
	int i;

	if (u32_offset <= NTP_MAX_UNCERTAINTY_MS)
	{
		if (iNTPDBG < DBG_TIM_KP_ST_LENGTH)
		{
			u64_DBG_SysTimeOffset_ms[iNTPDBG] = u64_NTP_TimeStamp_ms - u32_SyncTime_ms;
			u32_DBG_timeSyncUncertanty_ms[iNTPDBG++] = u32_offset;
		}
		SERIAL_DBG_PORT.printF("\n\nConsecutive TimeSync Values:\r\n");
		i = 0;
		SERIAL_DBG_PORT.print(String(i + 1) + StringF("/") + String(iNTPDBG) + StringF(" Uncert: ") + String(u32_DBG_timeSyncUncertanty_ms[i]) + StringF(" SysTickOffset: "));
		sprintf(txt, StringF("%llu diff: -----").c_str(), u64_DBG_SysTimeOffset_ms[i]);
		SERIAL_DBG_PORT.println(txt);
		for (i = 1; i < iNTPDBG; i++)
		{
			SERIAL_DBG_PORT.print(String(i + 1) + StringF("/") + String(iNTPDBG) + StringF(" Uncert: ") + String(u32_DBG_timeSyncUncertanty_ms[i]) + StringF(" SysTickOffset: "));
			sprintf(txt, StringF("%llu diff: %lld").c_str(), u64_DBG_SysTimeOffset_ms[i], (int64_t)u64_DBG_SysTimeOffset_ms[i] - (int64_t)u64_DBG_SysTimeOffset_ms[0]);
			SERIAL_DBG_PORT.println(txt);
		}
	}
	SERIAL_DBG_PORT.printlnF("Refference sync findings:");
	sprintf(txt, StringF("Ref millis: %lu, Ref NTP: %llu").c_str(), u32_RefferenceSyncMillis_ms, u64_RefferenceSysTimeOffset_ms);
	SERIAL_DBG_PORT.println(txt);
	sprintf(txt, StringF("Cur millis: %lu, Cur NTP: %llu").c_str(), u32_SyncTime_ms, u64_SysTimeOffset_ms);
	SERIAL_DBG_PORT.println(txt);
	sprintf(txt, StringF("Delta millis: %lu, Delta NTP: %lld, Drift %.2f ms/min").c_str(),
		(int32_t)u32_SyncTime_ms - (int32_t)u32_RefferenceSyncMillis_ms,
		(int64_t)u64_SysTimeOffset_ms - (int64_t)u64_RefferenceSysTimeOffset_ms, d_MillisDrift);
	SERIAL_DBG_PORT.println(txt);
}
#endif

void UpdateTimeSyncVariables(uint32_t u32_SyncTime_ms, uint64_t u64_NTP_TimeStamp_ms, uint32_t u32_offset)
{
	static uint64_t u64_RefTimeBuffer[NTP_REF_TIME_BUFFER_SIZE];
	static uint32_t u32_RefMillBuffer[NTP_REF_TIME_BUFFER_SIZE];
	static int RefTptr = 0;
	static int refValuesAge = 0;
	uint32_t u32_LastSyncUncertainty;

	u32_LastSyncUncertainty = CalculateExpandedTimeUncertainty(u32_SyncTime_ms);
	SERIAL_DBG_PORT.print(StringF("   PreviousSyncUncertainty:") + String(u32_timeSyncUncertanty_ms) + StringF("\n   Expandede previous sync uncertainty:") + String(u32_LastSyncUncertainty));
	if ((u64_NTP_TimeStamp_ms > NTP_01_01_2019_MS) && (u32_offset < u32_LastSyncUncertainty))	//only if more acurate than previous sync
	{
		u64_SysTimeOffset_ms = u64_NTP_TimeStamp_ms - u32_SyncTime_ms;
		u32_timeSyncUncertanty_ms = u32_offset;
		u32_LastSyncTime_ms = u32_SyncTime_ms;
		SERIAL_DBG_PORT.println(StringF(" ...   Using new sync data (uncertainty ") + String(u32_timeSyncUncertanty_ms)+StringF("ms)."));
		if (u32_offset <= i_MaxUncertainty_ms)
		{
			bSysTimeSynced = true;
			//only if CURRENT and REF time sync are both accurate, recalculate time drift
			if ((u32_offset <= i_DesiredUncertainty_ms) && b_HighAccuracySyncAchieved)
			{
				//Do not calculate drift, if there is not enough of a time difference for reasonable accuracy
//				if ((u32_SyncTime_ms - u32_RefferenceSyncMillis_ms) > NTP_MIN_TIME_DIF_TO_CALC_DRIFT)
//				{
					d_MillisDrift = 60000.0 *
						(float)((int64_t)u64_SysTimeOffset_ms - (int64_t)u64_RefferenceSysTimeOffset_ms) /
						(float)((int64_t)u32_SyncTime_ms - (int64_t)u32_RefferenceSyncMillis_ms);
//				}
			}
		}
	}
	else SERIAL_DBG_PORT.println(StringF(" ...   sticking with old sync data (new uncertainty too large: ") + String(u32_offset) + StringF("ms)."));

	//Save Reference time data for calculating clock drift
	if (u64_RefTimeBuffer[RefTptr] != 0)	//if valid value in the last place of circular buffer, use it
	{
		u64_RefferenceSysTimeOffset_ms = u64_RefTimeBuffer[RefTptr];
		u32_RefferenceSyncMillis_ms = u32_RefMillBuffer[RefTptr];
		b_HighAccuracySyncAchieved = true;
		refValuesAge = 0;
	}
	else
	{
		refValuesAge++;
		if (refValuesAge > (2 * NTP_REF_TIME_BUFFER_SIZE)) b_HighAccuracySyncAchieved = false;
	}
	//Store Refference time data in a circular buffer, or 0 if insufficient accuracy
	if (u32_offset <= i_DesiredUncertainty_ms)
	{
		u64_RefTimeBuffer[RefTptr] = u64_SysTimeOffset_ms;
		u32_RefMillBuffer[RefTptr] = u32_SyncTime_ms;
	}
	else
	{
		u64_RefTimeBuffer[RefTptr] = 0;
		u32_RefMillBuffer[RefTptr] = 0;
	}
	RefTptr++;
	if (RefTptr >= NTP_REF_TIME_BUFFER_SIZE) RefTptr = 0;

/*	if (!b_HighAccuracySyncAchieved || ((u32_SyncTime_ms - u32_RefferenceSyncMillis_ms) > NTP_TIME_TO_RESET_REFFERENCE_TIME_POINTS))
	{
		if (u32_offset <= NTP_REFFERENCE_UNCERTAINTY)
		{
			u64_RefferenceSysTimeOffset_ms = u64_SysTimeOffset_ms;
			u32_RefferenceSyncMillis_ms = u32_SyncTime_ms;
			b_HighAccuracySyncAchieved = true;
		}
	}*/
#ifdef DEBUG_TIME_KEEPING_STATS
	StoreAndPrintTimeKeeppingStatistics(u32_SyncTime_ms, u64_NTP_TimeStamp_ms, u32_offset);
#endif
}

void GetTime(void)
{
	int i;
	IPAddress timeServerIP;

	if (!GetNTPTime(IPAddress(ip_NTPlocal)))
	{
		//Fallback on worldwide NTP servers
		for (i = 0; i < iNTPpools; i++)
		{
			SERIAL_DBG_PORT.println(StringF("Get NTP IP in pool nr. ") + String(i));  //get a random server from the pool
			WiFi.hostByName(ntpServerName[i], timeServerIP);
			SERIAL_DBG_PORT.print(ntpServerName[i]);
			SERIAL_DBG_PORT.printF(" gave IP: ");
			SERIAL_DBG_PORT.println(timeServerIP);
			if (timeServerIP[3] == 0) {	//if invalid server address
				NtpFailed[i]++; // count servers fail times
			}
			else
			{	//found valid server
				if (GetNTPTime(timeServerIP)) break;	//if Timesync successful, continue, else try next IP
			}
		}
	}
}

/* Convert millis (systick) to absolute time with NTP epoch.
   Warning: NOT COMPATIBLE with NTP format: this applicatiun uses milliseconds even for NTP time*/
uint64_t millis2NTPtime(uint32_t ms)
{
	//TODO: improve accuracy using d_MillisDrift
	//char txt[200];
	uint64_t result = 0;
	double drift_correction = 0;
	if (u64_SysTimeOffset_ms != 0)
	{
		//TODO: test if correct
		if (!isnan(d_MillisDrift))
		{
			if (abs(d_MillisDrift) < MAX_VALID_DRIFT)
			{
				drift_correction = round(d_MillisDrift * ((int64_t)ms - u32_LastSyncTime_ms) / 60000.0);
				Log(StringF("Drift: ")+String(d_MillisDrift)+StringF(" ms/minute, Drift correction: ")
					+String(drift_correction)+StringF(" ms."),LL_MEASUREMENT_SYNC_STATS);
			}
		}
		result = u64_SysTimeOffset_ms + ms + round(drift_correction);
/*		SERIAL_DBG_PORT.println("DRIFT TEST:");
		sprintf(txt, "result  = %lld,\nresult2 = %lld", result- (uint64_t)round(drift_correction), result);
		SERIAL_DBG_PORT.println(txt);
		sprintf(txt, "drift=%f, ms=%d, Lsync=%d, Delta=%d corr = %f", d_MillisDrift, ms, u32_LastSyncTime_ms, (ms - u32_LastSyncTime_ms), d_MillisDrift * ((int64_t)ms - u32_LastSyncTime_ms) / 60000.0);
		SERIAL_DBG_PORT.println(txt);*/
	}
	return result;
}

uint32_t NTPtime2millis(uint64_t NTPtime)
{
	if (NTPtime < u64_SysTimeOffset_ms)
		return -1;	//MAX positive value - Error
	else 
		return NTPtime - u64_SysTimeOffset_ms;
}

uint32_t millis2UNIXtime(uint32_t ms)
{
	return (u64_SysTimeOffset_ms + ms) / 1000 - UNIX_VS_NTP_OFFSET_S;
}

/****************************************************************************/
//time utilities from time.h modified to work for NTP epoch (NO DAYLIGHT SAVINGS!)
/****************************************************************************/
struct tm _tm_store;	//temporary time storage, for returning results in a form of a pointer

const unsigned short int __mon_yday[2][13] =
{
	/* Normal years.  */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	/* Leap years.  */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

uint32_t mkNTPtime(struct tm* tim)
{
	uint32_t t;
	int year = tim->tm_year;
	int month = tim->tm_mon;
	int day = tim->tm_mday;
	int hour = tim->tm_hour;
	int min = tim->tm_min;
	int s = tim->tm_sec;

	//year
	if (year > 1900)
		t = ((year - 1900) * 365
			+ /* Compute the number of leapdays between 1900 and YEAR
				(exclusive).  There is a leapday every 4th year ...  */
			+((year - 1) / 4 - 1900 / 4)
			/* ... except every 100th year ... */
			- ((year - 1) / 100 - 1900 / 100)
			/* ... but still every 400th year.  */
			+ ((year - 1) / 400 - 1900 / 400)) * SECSPERDAY;
	else
		t = 0;

	//month
	if (month > 12) month = 12;
	if (month < 1) month = 1;
	t += __mon_yday[__isleap(year)][month - 1] * SECSPERDAY;

	//day
	t += (day - 1) * SECSPERDAY;

	//daytime
	t += hour * SECSPERHOUR;
	t += min * SECSPERMINUTE;
	t += s;

	return t;
}

struct tm* NTPtimestamp2tm(const uint32_t* _timer)
{
	uint32_t t = *_timer - UNIX_VS_NTP_OFFSET_S;
	uint32_t days;
	uint32_t time_of_day;
	int year, month, mday, yday, wday, leap_years, dst = 0;
	int h, m, s;


	days = t / SECSPERDAY;
	wday = (days + 1) % 7;	//FOR NTP epoch. For UNIX EPOCH: wday = (days + 4) % 7;
	time_of_day = t % SECSPERDAY;

	year = days / 365 + 1900;
	yday = days % 365;
	leap_years = (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400
		- (1900 / 4 - 1900 / 100 + 1900 / 400);
	yday -= leap_years;

	while (yday < 0)
	{
		year--;
		if (__isleap(year)) yday += 366;
		else yday += 365;
	}

	month = 0;
	while (__mon_yday[__isleap(year)][month] <= yday) month++;

	mday = 1 + yday - __mon_yday[__isleap(year)][month - 1];

	h = time_of_day / SECSPERHOUR;
	m = (time_of_day % SECSPERHOUR) / SECSPERMINUTE;
	s = time_of_day % SECSPERMINUTE;

	_tm_store.tm_year = year;
	_tm_store.tm_yday = yday;
	_tm_store.tm_mon = month;
	_tm_store.tm_mday = mday;
	_tm_store.tm_wday = wday;
	_tm_store.tm_isdst = dst;
	_tm_store.tm_hour = h;
	_tm_store.tm_min = m;
	_tm_store.tm_sec = s;

	return &_tm_store;
}
