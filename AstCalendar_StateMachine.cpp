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
State::StateResult AstCalendar::Init_EventHandler(State::StateEvent* se){
	State::Msg* st_msg = (State::Msg*)se->oe->value.p;
    switch((int)se->evt){
        case State::EV_ENTRY:{
        	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando recuperaci�n de datos...");
        	// recupera los datos de memoria NV
        	restoreConfig();

        	// realiza la suscripci�n local ej: "[get,set]/+/$module"
        	char* sub_topic_local = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
        	MBED_ASSERT(sub_topic_local);
        	sprintf(sub_topic_local, "set/+/%s", _sub_topic_base);
        	if(MQ::MQClient::subscribe(sub_topic_local, new MQ::SubscribeCallback(this, &AstCalendar::subscriptionCb)) == MQ::SUCCESS){
        		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Sucripci�n LOCAL hecha a %s", sub_topic_local);
        	}
        	else{
        		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_SUBSC en la suscripci�n LOCAL a %s", sub_topic_local);
        	}
        	sprintf(sub_topic_local, "get/+/%s", _sub_topic_base);
        	if(MQ::MQClient::subscribe(sub_topic_local, new MQ::SubscribeCallback(this, &AstCalendar::subscriptionCb)) == MQ::SUCCESS){
        		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Sucripci�n LOCAL hecha a %s", sub_topic_local);
        	}
        	else{
        		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_SUBSC en la suscripci�n LOCAL a %s", sub_topic_local);
        	}
        	Heap::memFree(sub_topic_local);

			duskDawnCalc();
        	// marca como componente iniciado
        	_ready = true;
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicaci�n en cmd/$BASE/cfg/set
        case RecvCfgSet:{
        	Blob::SetRequest_t<calendar_manager>* req = (Blob::SetRequest_t<calendar_manager>*)st_msg->msg;
        	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Recibida nueva configuraci�n");
			// si no hay errores, actualiza la configuraci�n
			if(req->_error.code == Blob::ErrOK){
				_updateConfig(req->data, req->_error);
				// si cambia la configuración de zona, recarga la hora
				if((req->data.clock.cfg.geoloc._keys & (1 << 2)) || (req->data.clock.cfg.geoloc._keys & (1 << 4))){
					_updateRtcFromCfg();
				}
			}
        	// si hay errores en el mensaje o en la actualizaci�n, devuelve resultado sin hacer nada
        	if(req->_error.code != Blob::ErrOK){
        		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_UPD al actualizar code=%d", req->_error.code);
        		char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "stat/cfg/%s", _pub_topic_base);

				Blob::Response_t<calendar_manager>* resp = new Blob::Response_t<calendar_manager>(req->idTrans, req->_error, _astdata);
				MBED_ASSERT(resp);
				if(_json_supported){
					cJSON* jresp = JsonParser::getJsonFromResponse(*resp, ObjSelectCfg);
					MBED_ASSERT(jresp);
					MQ::MQClient::publish(pub_topic, &jresp, sizeof(cJSON**), &_publicationCb);
					cJSON_Delete(jresp);
				}
				else{
					MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<calendar_manager>), &_publicationCb);
				}
				delete(resp);
				Heap::memFree(pub_topic);
				return State::HANDLED;
        	}
			//recalculamos horas de orto y ocaso
			duskDawnCalc();
        	// almacena en el sistema de ficheros
        	saveConfig();
        	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Config actualizada");

        	// si est� habilitada la notificaci�n de actualizaci�n, lo notifica
        	if((_astdata.cfg.updFlags & CalendarManagerCfgUpdNotif) != 0){
        		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Notificando actualizaci�n");
				char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "stat/cfg/%s", _pub_topic_base);

				Blob::Response_t<calendar_manager>* resp = new Blob::Response_t<calendar_manager>(req->idTrans, req->_error, _astdata);
				MBED_ASSERT(resp);
				if(_json_supported){
					cJSON* jresp = JsonParser::getJsonFromResponse(*resp, ObjSelectCfg);
					MBED_ASSERT(jresp);
					MQ::MQClient::publish(pub_topic, &jresp, sizeof(cJSON**), &_publicationCb);
					cJSON_Delete(jresp);
				}
				else{
					MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<calendar_manager>), &_publicationCb);
				}
				delete(resp);
				Heap::memFree(pub_topic);
				return State::HANDLED;
        	}

            return State::HANDLED;
        }
        case RcvSetDefault:{
        	DEBUG_TRACE_E(_EXPR_,_MODULE_,"Factory Reset!!!");
        	setDefaultConfig();
        	return State::HANDLED;
        }
        // Procesa datos recibidos de la publicaci�n en cmd/$BASE/cfg/get
        case RecvCfgGet:{
        	Blob::GetRequest_t* req = (Blob::GetRequest_t*)st_msg->msg;
        	// prepara el topic al que responder
        	char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(pub_topic);
			sprintf(pub_topic, "stat/cfg/%s", _pub_topic_base);

			// responde con los datos solicitados y con los errores (si hubiera) de la decodificaci�n de la solicitud
			Blob::Response_t<calendar_manager>* resp = new Blob::Response_t<calendar_manager>(req->idTrans, req->_error, _astdata);
			MBED_ASSERT(resp);
			if(_json_supported){
				cJSON* jresp = JsonParser::getJsonFromResponse(*resp, ObjSelectCfg);
				MBED_ASSERT(jresp);
				MQ::MQClient::publish(pub_topic, &jresp, sizeof(cJSON**), &_publicationCb);
				cJSON_Delete(jresp);
			}
			else{
				MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<calendar_manager>), &_publicationCb);
			}
			delete(resp);
			Heap::memFree(pub_topic);
			return State::HANDLED;
        }

        // Procesa datos recibidos de la publicaci�n en get/boot
        case RecvBootGet:{
			char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(pub_topic);
			sprintf(pub_topic, "stat/boot/%s", _pub_topic_base);
			Blob::NotificationData_t<calendar_manager> *notif = new Blob::NotificationData_t<calendar_manager>(_astdata);
			MBED_ASSERT(notif);
			if(_json_supported){
				cJSON* jboot = JsonParser::getJsonFromNotification<calendar_manager>(*notif, ObjSelectAll);
				MBED_ASSERT(jboot);
				MQ::MQClient::publish(pub_topic, &jboot, sizeof(cJSON**), &_publicationCb);
				cJSON_Delete(jboot);
			}
			else {
				MQ::MQClient::publish(pub_topic, notif, sizeof(Blob::NotificationData_t<calendar_manager>), &_publicationCb);
			}
			delete(notif);
			Heap::memFree(pub_topic);
			return State::HANDLED;
        }

		case RecvRtcSet:{
			Blob::SetRequest_t<time_t>* req = (Blob::SetRequest_t<time_t>*)st_msg->msg;
        	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Recibida nuevo datetime");
			// si no hay errores, actualiza la configuraci�n
			if(req->_error.code == Blob::ErrOK){
				setRtcTime(req->data);
			}
			time_t sysTime;
			time(&sysTime);
			//recalculamos horas de orto y ocaso
			duskDawnCalc();

			DEBUG_TRACE_I(_EXPR_, _MODULE_, "Notificando localtime");
			char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(pub_topic);
			sprintf(pub_topic, "stat/rtc/%s", _pub_topic_base);

			Blob::Response_t<time_t>* resp = new Blob::Response_t<time_t>(req->idTrans, req->_error, sysTime);
			MBED_ASSERT(resp);
			if(_json_supported){
				cJSON* jresp = JsonParser::getJsonFromResponse(*resp, ObjSelectAll);
				MBED_ASSERT(jresp);
				MQ::MQClient::publish(pub_topic, &jresp, sizeof(cJSON**), &_publicationCb);
				cJSON_Delete(jresp);
			}
			else{
				MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<time_t>), &_publicationCb);
			}
			delete(resp);
			Heap::memFree(pub_topic);
			return State::HANDLED;
        }

        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }

        default:{
        	return State::IGNORED;
        }

     }
}


