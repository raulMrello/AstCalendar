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
#define _EXPR_	(_defdbg && !IS_ISR())
 
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
        DEBUG_TRACE_I(_EXPR_, _MODULE_, "QUEUE_PUT_ERROR %d", ost);
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
	Blob::AstCalEvtFlags flags = Blob::AstCalNoEvents;

	// obtiene la hora actual y genera los eventos correspondientes
	time_t t = time(NULL);
	localtime_r(&t, &_now);
	flags = (Blob::AstCalEvtFlags)(flags | Blob::AstCalSecEvt);
	if(_now.tm_sec == 0){
		flags = (Blob::AstCalEvtFlags)(flags | Blob::AstCalMinEvt);
		if(_now.tm_min == 0){
			flags = (Blob::AstCalEvtFlags)(flags | Blob::AstCalHourEvt);
			if(_now.tm_hour == 12){
				flags = (Blob::AstCalEvtFlags)(flags | Blob::AstCalMiddayEvt);
			}
			if(_now.tm_hour == 0){
				flags = (Blob::AstCalEvtFlags)(flags | Blob::AstCalDayEvt);
				if(_now.tm_wday == 1){
					flags = (Blob::AstCalEvtFlags)(flags | Blob::AstCalWeekEvt);
				}
				if(_now.tm_mday == 1){
					flags = (Blob::AstCalEvtFlags)(flags | Blob::AstCalMonthEvt);
					if(_now.tm_mon == 1){
						flags = (Blob::AstCalEvtFlags)(flags | Blob::AstCalYearEvt);
					}
				}
			}
		}
	}
	// si hay flags que notificar...
	if((flags & _astdata.cfg.evtFlagMask)!=0){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "Notificando evento=%x", flags);
		// crea el objeto a notificar
		Blob::AstCalStatData_t stat;
		stat.flags = flags;
		stat.now = t;
		stat.period=0;
		char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
		MBED_ASSERT(pub_topic);
		sprintf(pub_topic, "stat/value/%s", _pub_topic_base);

		if(_json_supported){
			cJSON* jstat = JsonParser::getJsonFromObj(stat);
			if(jstat){
				char* jmsg = cJSON_Print(jstat);
				cJSON_Delete(jstat);
				MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
				Heap::memFree(jmsg);
				Heap::memFree(pub_topic);
				return;
			}
		}

		// publica estado
		MQ::MQClient::publish(pub_topic, &stat, sizeof(Blob::AstCalStatData_t), &_publicationCb);
		Heap::memFree(pub_topic);
	}
}



