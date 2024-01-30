/*
 * AstCalendar.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 */

#include "AstCalendar.h"
#include "lwip/apps/sntp.h"

//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------


static const char* _MODULE_ = "[AstCal]........";
#define _EXPR_	(!IS_ISR())

//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
AstCalendar::AstCalendar(FSManager* fs, bool defdbg) : ActiveModule("AstCal", osPriorityNormal, 3072, fs, defdbg) {

	// Establece el soporte de JSON
	_json_supported = false;
	#if ASTCAL_ENABLE_JSON_SUPPORT == 1
	_json_supported = true;
	#endif

    if(defdbg){
    	esp_log_level_set(_MODULE_, ESP_LOG_DEBUG);
    }
    else{
    	esp_log_level_set(_MODULE_, ESP_LOG_WARN);
    }

    // inicializaci�n NTP
    _ntp_enabled = false;

	// Carga callbacks est�ticas de publicaci�n/suscripci�n
    _publicationCb = callback(this, &AstCalendar::publicationCb);
    _rtc = NULL;
	_curr_dst = 0;
	_curr_sun = 1;
	_sim_tmr = new RtosTimer(callback(this, &AstCalendar::eventSimulatorCb), osTimerPeriodic, "AstCalSimTmr");
	MBED_ASSERT(_sim_tmr);
	// inicia el generador de eventos
	startSimulator();
}


//------------------------------------------------------------------------------------
void AstCalendar::startSimulator() {
	_sim_counter = 0;
	_last_rtc_time = time(NULL);
	_sim_tmr->start(1000);
}


//------------------------------------------------------------------------------------
void AstCalendar::stopSimulator() {
	_sim_tmr->stop();
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void AstCalendar::publicationCb(const char* topic, int32_t result){

}


int AstCalendar::gmtDesviation(struct tm* utc_time , struct tm* local_time ){
	bool sameDay = true;
	int gmtDiff = 0;
	int utcMins = 0;
	int localMins = 0;
	int diff_hours = 0;
	
	utcMins = utc_time->tm_hour*60 + utc_time->tm_min;
	localMins = local_time->tm_hour*60 + local_time->tm_min;
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "UTC[dia: %d]: %d:%d -> %d", utc_time->tm_mday, utc_time->tm_hour, utc_time->tm_min, utcMins);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "LCL[dia: %d]: %d:%d -> %d", local_time->tm_mday, local_time->tm_hour, local_time->tm_min, localMins);
	
	if(local_time->tm_mday != utc_time->tm_mday){
		sameDay = false;
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Diferente dia");
		
		if(localMins > utcMins)
			diff_hours = (1440+utcMins) - localMins;
		else
			diff_hours = (1440+localMins) - utcMins;
	}
	else{
		diff_hours = abs(localMins - utcMins);
	}

	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Diferencia GMT: %d", diff_hours);

	return diff_hours;
}

void AstCalendar::duskDawnCalc(){
	uint16_t sunrise = 0;
	uint16_t sunset = 0;
	uint8_t isAllDay = 0;
	uint8_t isAllNight = 0;

	int16_t corrSunrise = 0;
	int16_t corrSunset = 0;

	time_t t = time(NULL);
	tm local;
	local = *localtime(&t);
	CALENDAR_T cal = initializeCalendar(local);
	double _lat = _astdata.clock.cfg.geoloc.coords[0];
	double _lng = _astdata.clock.cfg.geoloc.coords[1];
	COORD_T lat = initializeCoord(_lat);
	COORD_T lng = initializeCoord(_lng);


	time_t tnow = time(NULL);
	localtime_r(&tnow, &_now);

	tm _gmtime = *gmtime(&t);
	tm _localtime = *localtime(&t);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC:   %s", asctime(&_gmtime));
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "local: %s", asctime(&_localtime));
	int16_t gmt = gmtDesviation(&_gmtime, &_localtime);

	zoneCalculateSuntimes(&cal, gmt, &lat, &lng, corrSunrise, corrSunset, &sunrise, &sunset,&isAllDay, &isAllNight);
	
	// Pasamos a time_t la hora de orto y ocaso
	// los minutos de orto y ocaso los dan en local
	// Si lo hacemos a las 00 es tan facil como sumarle al time_t actual los minutos calculados porque son para ese día
	time_t tnow2 = time(NULL);
	tm now, newDateSunrise, newDateSunset;
	localtime_r(&tnow2, &now);
	uint32_t currMinutes = (now.tm_hour * 60) + now.tm_min;
	newDateSunrise = now;
	newDateSunset = now;
	newDateSunrise.tm_hour = sunrise / 60;
	newDateSunrise.tm_min = sunrise % 60;
	newDateSunset.tm_hour = sunset / 60;
	newDateSunset.tm_min = sunset % 60;
	_astdata.clock.stat.dawn = mktime(&newDateSunrise);
	_astdata.clock.stat.dusk = mktime(&newDateSunset);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Sunrise: %d(%d:%d), Sunset: %d(%d:%d)", sunrise, sunrise/60, sunrise%60, sunset, sunset/60, sunset%60);
	_curr_sun = -1; //para publicar el estado actual
}

//------------------------------------------------------------------------------------
void AstCalendar::eventSimulatorCb() {
	uint32_t flags = CalendarClockNoEvents;
	// obtiene la hora actual y genera los eventos correspondientes
	time_t t = time(NULL);
	localtime_r(&t, &_now);

	// chequea si ha habido actualizaci�n NTP

	if(_ntp_enabled){
		_last_rtc_time++;
		if(_last_rtc_time < (t - NtpDifSecUpdate) || _last_rtc_time > (t + NtpDifSecUpdate)){
			_ntpUpdateCb();
			flags |= CalendarClockNTPEvt;
		}
	}
	else{
		_last_rtc_time = t;
	}

	flags |= CalendarClockSecEvt;
	if(_now.tm_sec == 0){
		flags |= CalendarClockMinEvt;
		if(_now.tm_min == 0){
			flags |= CalendarClockHourEvt;
			if(_now.tm_hour == 12){
				flags |= CalendarClockMiddayEvt;
			}
			if(_now.tm_hour == 0){
				flags |= CalendarClockDayEvt;
				// Calculamos el amanecer y atardecer
				duskDawnCalc();
				if(_now.tm_wday == 1){
					flags |= CalendarClockWeekEvt;
				}
				if(_now.tm_mday == 1){
					flags |= CalendarClockMonthEvt;
					if(_now.tm_mon == 1){
						flags |= CalendarClockYearEvt;
					}
				}
			}
		}
	}
	if(_curr_dst == 0 && _now.tm_isdst > 0){
		flags |= CalendarClockIVEvt;
		_curr_dst = _now.tm_isdst;
	}
	else if(_curr_dst !=0 && _now.tm_isdst == 0){
		flags |= CalendarClockVIEvt;
		_curr_dst = _now.tm_isdst;
	}
	//Orto y ocaso
	uint32_t dawnCorr = (uint32_t)_astdata.clock.cfg.geoloc.astCorr[0][0]*60;//a segundos
	uint32_t duskCorr = (uint32_t)_astdata.clock.cfg.geoloc.astCorr[0][1]*60;
	uint32_t dawnTrigger = (uint32_t)_astdata.clock.stat.dawn + dawnCorr;
	uint32_t duskTrigger = (uint32_t)_astdata.clock.stat.dusk + duskCorr;
	if((_curr_sun == 0 || _curr_sun == -1) && (t > dawnTrigger && t < duskTrigger)){
		flags |= CalendarClockDawnEvt;
		_curr_sun = 1;
	}
	else if((_curr_sun == 1 || _curr_sun == -1) && (t > duskTrigger)){
		flags |= CalendarClockDuskEvt;
		_curr_sun = 0;
	}



	// actualiza variables de estado
	_astdata.clock.stat.localtime = t;
	_astdata.clock.stat.flags = flags;
	_curr_sun == 1 ?_astdata.clock.stat.dayTime = true : _astdata.clock.stat.dayTime = false;

	// si hay flags que notificar...
	if((flags & _astdata.cfg.evtFlags)!=0){
		// crea el objeto a notificar
		Blob::NotificationData_t<calendar_manager> *notif = new Blob::NotificationData_t<calendar_manager>(_astdata);
		MBED_ASSERT(notif);

		char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
		MBED_ASSERT(pub_topic);
		sprintf(pub_topic, "stat/value/%s", _pub_topic_base);

		if(_json_supported){
			cJSON* jstat = JsonParser::getJsonFromNotification(*notif, ObjSelectAll);
			MBED_ASSERT(jstat);
			MQ::MQClient::publish(pub_topic, &jstat, sizeof(cJSON**), &_publicationCb);
			cJSON_Delete(jstat);
		}
		else {
			MQ::MQClient::publish(pub_topic, notif, sizeof(Blob::NotificationData_t<calendar_manager>), &_publicationCb);
		}
		Heap::memFree(pub_topic);
		delete(notif);

		if(_astdata.clock.stat.flags & CalendarClockDawnEvt || _astdata.clock.stat.flags & CalendarClockDuskEvt){
			// crea el objeto a notificar
			notif = new Blob::NotificationData_t<calendar_manager>(_astdata);
			MBED_ASSERT(notif);

			pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(pub_topic);
			if(_astdata.clock.stat.flags & CalendarClockDawnEvt)
				sprintf(pub_topic, "stat/orto/%s", _pub_topic_base);
			else
				sprintf(pub_topic, "stat/ocaso/%s", _pub_topic_base);

			if(_json_supported){
				cJSON* jstat = JsonParser::getJsonFromNotification(*notif, ObjSelectAll);
				MBED_ASSERT(jstat);
				MQ::MQClient::publish(pub_topic, &jstat, sizeof(cJSON**), &_publicationCb);
				cJSON_Delete(jstat);
			}
			else {
				MQ::MQClient::publish(pub_topic, notif, sizeof(Blob::NotificationData_t<calendar_manager>), &_publicationCb);
			}
			Heap::memFree(pub_topic);
			delete(notif);
		}
	}
}


//------------------------------------------------------------------------------------
void AstCalendar::enableNTPClient() {
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Activando servicio NTP");
	_ntp_enabled = true;
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_init();
	for(int i=0;i<3;i++){
		if(!sntp_enabled()){
			DEBUG_TRACE_I(_EXPR_, _MODULE_, "Reiniciando servicio NTP %d de 3", i);
			sntp_stop();
			sntp_init();
		}
		else{
			DEBUG_TRACE_I(_EXPR_, _MODULE_, "NTP activo!");
			return;
		}
	}
	sntp_stop();
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERROR iniciando servicio NTP");
}


//------------------------------------------------------------------------------------
void AstCalendar::_ntpUpdateCb(){
	time_t tnow = time(NULL);
	_last_rtc_time = tnow;
	localtime_r(&tnow, &_now);
	_curr_dst = _now.tm_isdst;
	time_t t = time(NULL);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema actualizada via NTP");
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC:   %s", asctime(gmtime(&t)));
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "local: %s", asctime(localtime(&t)));

	// Actualiza hora en driver RTC
	tm* utc_tm = gmtime(&tnow);
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC update(time_t_utc=%d) %s", (int)tnow, asctime(gmtime(&tnow)));
	_rtc->setTime(*utc_tm);
}

void AstCalendar::setRtcTime(time_t tnow){
	_last_rtc_time = tnow;

	timeval tv;
	tv.tv_sec = tnow;
	tv.tv_usec = 0;
	settimeofday (&tv, NULL);

	tnow = time(NULL);
	localtime_r(&tnow, &_now);
	_curr_dst = _now.tm_isdst;
	time_t t = time(NULL);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema actualizada manualmente");
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC:   %s", asctime(gmtime(&t)));
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "local: %s", asctime(localtime(&t)));

	// Actualiza hora en driver RTC
	tm* utc_tm = gmtime(&tnow);
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC update(time_t_utc=%d) %s", (int)tnow, asctime(gmtime(&tnow)));
	_rtc->setTime(*utc_tm);
}

//------------------------------------------------------------------------------------
time_t AstCalendar::GetSecondFromReset(){
	return _pw_fail;
}

//------------------------------------------------------------------------------------
unsigned char AstCalendar::IsNaN(double hh, double mm, signed short int gmt){
	double2longs_t v64;
	v64.d = hh;
	if(v64.s.lo == 0 && (v64.s.hi == 0x7ff00000 || v64.s.hi == 0xfff00000)){
		return 1;
	}
	v64.d = mm;
	if(v64.s.lo == 0 && (v64.s.hi == 0x7ff00000 || v64.s.hi == 0xfff00000)){
		return 1;
	}
	if(((hh * 60) + gmt + mm) < 0){
		return 1;
	}
	return 0;
}

//------------------------------------------------------------------------------------
double AstCalendar::own_abs(double x){
	if( x >= 0 )
	{
		return x;
	}
	return( x - x - x);
}

//------------------------------------------------------------------------------------
int8_t AstCalendar::zoneCalculateSuntimes(CALENDAR_T *cal, int16_t gmt, COORD_T *lat, COORD_T *lng, int16_t corrSunrise, int16_t corrSunset, uint16_t * sunrise, uint16_t *sunset, uint8_t *isAllDay, uint8_t *isAllNight){
	double Lon, Lat;
	double JD, J1, P, AM, V, L, O_B, DC;
	double AR, ET, H0, VD, DCOR, HORTO;
	double HOC, MOC, HOR, MOR;
	double TUC, VDOC, DCOC, VHOC, TUOC, ACOC;
	double ACOR, GACOC, MACOC, GACOR, MACOR;
	char GGG, A;
	
	*isAllDay = 0;
	*isAllNight = 0;
	//MBED_ASSERT((!cal || !lat || !lng || !sunrise || !sunset), "Zone_CalculateSuntimes", -1);

	// bugfix cálculos para diferentes años. Utiliza siempre el año 2016 para bisiestos y 2017 para no bisiestos
	uint8_t year = cal->year;;
	if(year % 4 == 0)
		year = 16;
	else
		year = 17;
	
	if (lng->Grados < 0 || lng->Signo == -1){
	    Lon =	lng->Grados -	(double)(lng->Minutos)/60 - (double)(lng->Segundos)/3600;	    
	}
	else{
	    Lon =	lng->Grados + (double)(lng->Minutos)/60 + (double)(lng->Segundos)/3600;	    
	}
	

	if (lat->Grados < 0 || lat->Signo == -1){
	    Lat =	lat->Grados - (double)(lat->Minutos)/60 -	(double)(lat->Segundos)/3600;
	}
	else{
	    Lat =	lat->Grados + (double)(lat->Minutos)/60 + (double)(lat->Segundos)/3600;		    
	}
	
	GGG = 1;	
	/*nunca menor de 2009
	if (pFecha->Anyo <= 1585) GGG = 0;
	*/

    JD = -1 * floor(7 * (floor((double)(cal->month + 9) / 12) + (2000+year)) / 4);
	
	P = 1;	
	if ((cal->month - 9)<0){
	    P=-1;    
	}
	
	A = (char)own_abs(cal->month - 9);	
	J1 = floor((2000+year) + P * floor((double)(A) / 7));	
	J1 = -1 * floor((floor(J1 / 100) + 1) * 3 / 4);	
	JD = JD + floor(275 * (double)(cal->month) / 9) +cal->date + (GGG * J1);	
	JD = JD + 1721027 + 2 * GGG + 367 * (2000+year) - 0.5;	
			
			  
	P = 4.93204;
	ET = 0.016718;

	P = P + (JD-T0)*VP/100;
	AM = M0 + MN*(JD-T0);
	AM = AM - 2*PI*floor(AM/(2 * PI));
	V = AM + 2*ET*sin(AM) + 1.25*ET*ET*sin(2*AM);
		
	if (V<0){
	    V = 2*PI + V;    
	}
		
	L = P + V;
	L = L - 2*PI*floor(L/(2*PI));
	DC = (JD-2415020.5)/365.2422;

	O_B = 23.452294-(0.46845*DC+0.00000059*DC*DC)/3600;
	O_B = O_B/RAD;

	DC = asin(sin(O_B)*sin(L));

	AR = acos(cos(L)/cos(DC));
		
	if (L>PI){
	    AR = 2*PI - AR;    
	}
		
	O_B = O_B*RAD;		
	L = L*RAD;
	AR = AR * 12 / PI;	
	DC = DC*RAD; 
		
	AR = 2*ET*FF*sin(AM) + 1.25*ET*ET*FF*sin(2*AM);		
	H0 = -MR*FF*sin(2*(P+AM))+MR*MR*FF*sin(4*(P+AM))/2;		
	ET = AR + H0;
		
	H0 = acos(-tan(Lat/RAD)*tan(DC/RAD));
	H0 = H0*RAD;
		
	VD = 0.9856*sin(O_B/RAD)*cos(L/RAD)/cos(DC/RAD);

	DCOR = VD*(-H0+180)/360;
	DCOR += DC;
	HORTO = -acos(-tan(Lat/RAD)*tan(DCOR/RAD));
	HOR = 5/(6*cos(Lat/RAD)*cos(DCOR/RAD)*sin(HORTO));
	HORTO = (HORTO*RAD+HOR)/15;
	TUC = HORTO+ET/3600-Lon/15+12;		
		
	HOR = floor(TUC);	
	MOR = floor((TUC - HOR)*60 + 0.5);
		
	if (MOR>59)
	{	
		MOR = MOR - 60;	
		HOR = HOR + 1;	
	}
	
	TUC = 12 + ET/3600 - Lon/15;
		
	VDOC = floor(TUC);		
		
	VDOC = VD*(H0+180)/360;
	DCOC = DC+VDOC;
	HOC = acos(-tan(Lat/RAD)*tan(DCOC/RAD));
	VHOC =5/(6*cos(Lat/RAD)*cos(DCOC/RAD)*sin(HOC));
	HOC = (HOC*RAD+VHOC)/15;
	TUOC = HOC+ET/3600-Lon/15+12;
	HOC=floor(TUOC);	
	MOC=floor((TUOC - HOC) * 60+0.5);
	if (MOC>59)
	{		
		MOC=MOC-60;
		HOC=HOC+1;
	}

//Para calcular culminaci�n (sin uso)				
//			   HCUL=90-Lat+(DCOR+DCOC)/2
//			   GCUL=floor(HCUL);
//			   MCUL=floor((HCUL - GCUL) * 60+0.5)
//			   if (MCUL>59) {
//				MCUL=MCUL-60;
//				HCUL=HCUL+1;
//				}
			
	ACOC = acos(-sin(DCOC/RAD)/cos(Lat/RAD))*RAD;
	ACOR = 360 - acos(-sin(DCOR/RAD)/cos(Lat/RAD))*RAD;
	GACOC = floor(ACOC);
	MACOC = floor((ACOC - GACOC) * 60+0.5);
	if(MACOC>59)
	{
		MACOC = MACOC - 60;
		GACOC = GACOC + 1;
	}
		
	GACOR = floor(ACOR);
	MACOR = floor((ACOR - GACOR) * 60 + 0.5);
	if(MACOR>59)
	{
		MACOR = MACOR-60;
		GACOR = GACOR+1;
	}

	if(IsNaN(HOR, MOR, gmt)){
		*sunrise = Zone_SET_SUNTIME_OOB;
		if(cal->month < Rtc_MON_MAR || cal->month > Rtc_MON_OCT){
			*isAllNight = 1;
		}
		else{
			*isAllDay = 1;
			*sunset = Zone_SET_SUNTIME_OOB;
			return 0;
		}
	}
	else{
		*sunrise = (uint16_t)((HOR * 60) + gmt);
		*sunrise += (uint16_t)MOR;					
		if(*sunrise > Zone_MAX_SUNTIME_LIMIT)	{
			*sunrise = Zone_SET_SUNTIME_OOB;
		}
	}

	if(IsNaN(HOC, MOC, gmt)){
		if(isAllNight){
			*sunset = 0;
		}
		else{
			*sunset = Zone_SET_SUNTIME_OOB;
		}
	}
	else{
		*sunset = (uint16_t)((HOC * 60)	+ gmt);
		*sunset += (uint16_t)MOC;
		if(*sunset > Zone_MAX_SUNTIME_LIMIT && (*sunset - Zone_MAX_SUNTIME_LIMIT) >= *sunrise)	{
			*isAllDay = 1;
			*sunset = (uint16_t)Zone_SET_SUNTIME_OOB;
		}
	}
	return 0;
}