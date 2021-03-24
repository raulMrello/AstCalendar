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
	// actualiza variables de estado
	_astdata.clock.stat.localtime = t;
	_astdata.clock.stat.flags = flags;

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
	char strftime_buf[64];
	memset(strftime_buf, 0, 64);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &_now);
	strftime_buf[63] = 0;
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "Hora del sistema actualizada via NTP: %s", strftime_buf);

	// Actualiza hora en driver RTC
	tm* utc_tm = gmtime(&tnow);
	memset(strftime_buf, 0, 64);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", utc_tm);
	strftime_buf[63] = 0;

	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC update(time_t_utc=%d) %s", (int)tnow, strftime_buf);
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
	char strftime_buf[64];
	memset(strftime_buf, 0, 64);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &_now);
	strftime_buf[63] = 0;
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "Hora del sistema actualizada manualmente(time_t=%d): %s", (int)tnow, strftime_buf);

	// Actualiza hora en driver RTC
	tm* utc_tm = gmtime(&tnow);
	memset(strftime_buf, 0, 64);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", utc_tm);
	strftime_buf[63] = 0;
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC update(time_t_utc=%d) %s", (int)tnow, strftime_buf);
	_rtc->setTime(*utc_tm);
}
