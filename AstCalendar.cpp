/*
 * AstCalendar.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 */

#include "AstCalendar.h"

//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------


static const char* _MODULE_ = "[AstCal]........";
#define _EXPR_	(!IS_ISR())
 
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
AstCalendar::AstCalendar(FSManager* fs, bool defdbg) : ActiveModule("AstCal", osPriorityNormal, 3096, fs, defdbg) {

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

	// Carga callbacks estáticas de publicación/suscripción
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
	_sim_tmr->start(1000);
}


//------------------------------------------------------------------------------------
void AstCalendar::stopSimulator() {
	_sim_tmr->stop();
}


//------------------------------------------------------------------------------------
osStatus AstCalendar::putMessage(State::Msg *msg){
    osStatus ost = _queue.put(msg, ActiveModule::DefaultPutTimeout);
    if(ost != osOK){
        DEBUG_TRACE_E(_EXPR_, _MODULE_, "QUEUE_PUT_ERROR %d", ost);
    }
    return ost;
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
osEvent AstCalendar::getOsEvent(){
	return _queue.get();
}


//------------------------------------------------------------------------------------
void AstCalendar::publicationCb(const char* topic, int32_t result){

}


//------------------------------------------------------------------------------------
void AstCalendar::eventSimulatorCb() {
	uint32_t flags = CalendarClockNoEvents;

	// obtiene la hora actual y genera los eventos correspondientes
	time_t t = time(NULL);
	localtime_r(&t, &_now);
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
			MQ::MQClient::publish(pub_topic, jstat, sizeof(cJSON*), &_publicationCb);
			cJSON_Delete(jstat);
		}
		else {
			MQ::MQClient::publish(pub_topic, notif, sizeof(Blob::NotificationData_t<calendar_manager>), &_publicationCb);
		}
		Heap::memFree(pub_topic);
		delete(notif);
	}
}



