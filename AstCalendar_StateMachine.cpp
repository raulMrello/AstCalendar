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
        	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando recuperación de datos...");
        	// recupera los datos de memoria NV
        	restoreConfig();

        	// realiza la suscripción local ej: "[get,set]/+/$module"
        	char* sub_topic_local = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
        	MBED_ASSERT(sub_topic_local);
        	sprintf(sub_topic_local, "set/+/%s", _sub_topic_base);
        	if(MQ::MQClient::subscribe(sub_topic_local, new MQ::SubscribeCallback(this, &AstCalendar::subscriptionCb)) == MQ::SUCCESS){
        		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Sucripción LOCAL hecha a %s", sub_topic_local);
        	}
        	else{
        		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_SUBSC en la suscripción LOCAL a %s", sub_topic_local);
        	}
        	sprintf(sub_topic_local, "get/+/%s", _sub_topic_base);
        	if(MQ::MQClient::subscribe(sub_topic_local, new MQ::SubscribeCallback(this, &AstCalendar::subscriptionCb)) == MQ::SUCCESS){
        		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Sucripción LOCAL hecha a %s", sub_topic_local);
        	}
        	else{
        		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_SUBSC en la suscripción LOCAL a %s", sub_topic_local);
        	}
        	Heap::memFree(sub_topic_local);

        	// marca como componente iniciado
        	_ready = true;
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en cmd/$BASE/cfg/set
        case RecvCfgSet:{
        	Blob::SetRequest_t<calendar_manager>* req = (Blob::SetRequest_t<calendar_manager>*)st_msg->msg;
        	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Recibida nueva configuración");
			// si no hay errores, actualiza la configuración
			if(req->_error.code == Blob::ErrOK){
				_updateConfig(req->data, req->_error);
			}
        	// si hay errores en el mensaje o en la actualización, devuelve resultado sin hacer nada
        	if(req->_error.code != Blob::ErrOK){
        		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_UPD al actualizar code=%d", req->_error.code);
        		char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "stat/cfg/%s", _pub_topic_base);

				Blob::Response_t<calendar_manager>* resp = new Blob::Response_t<calendar_manager>(req->idTrans, req->_error, _astdata);

				if(_json_supported){
					cJSON* jresp = JsonParser::getJsonFromResponse(*resp, ObjSelectCfg);
					if(jresp){
						char* jmsg = cJSON_PrintUnformatted(jresp);
						cJSON_Delete(jresp);
						MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
						Heap::memFree(jmsg);
						delete(resp);
						Heap::memFree(pub_topic);
						return State::HANDLED;
					}
					else{
						DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR al formar Blob::Response_t<calendar_manager>");
						Heap::memFree(pub_topic);
						return State::HANDLED;
					}
				}

				MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<calendar_manager>), &_publicationCb);
				delete(resp);
				Heap::memFree(pub_topic);
				return State::HANDLED;
        	}

        	// almacena en el sistema de ficheros
        	saveConfig();
        	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Config actualizada");

        	// si está habilitada la notificación de actualización, lo notifica
        	if((_astdata.cfg.updFlags & CalendarManagerCfgUpdNotif) != 0){
        		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Notificando actualización");
				char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "stat/cfg/%s", _pub_topic_base);

				Blob::Response_t<calendar_manager>* resp = new Blob::Response_t<calendar_manager>(req->idTrans, req->_error, _astdata);
				MBED_ASSERT(resp);
				if(_json_supported){
					cJSON* jresp = JsonParser::getJsonFromResponse(*resp, ObjSelectCfg);
					if(jresp){
						char* jmsg = cJSON_PrintUnformatted(jresp);
						cJSON_Delete(jresp);
						MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
						Heap::memFree(jmsg);
						delete(resp);
						Heap::memFree(pub_topic);
						return State::HANDLED;
					}
					else{
						DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error on getJsonFromResponse <%s>", resp->error.descr);
						delete(resp);
						Heap::memFree(pub_topic);
						return State::HANDLED;
					}
				}
				else{
					MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<calendar_manager>), &_publicationCb);
					delete(resp);
				}
				Heap::memFree(pub_topic);
        	}

            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en cmd/$BASE/cfg/get
        case RecvCfgGet:{
        	Blob::GetRequest_t* req = (Blob::GetRequest_t*)st_msg->msg;
        	// prepara el topic al que responder
        	char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(pub_topic);
			sprintf(pub_topic, "stat/cfg/%s", _pub_topic_base);

			// responde con los datos solicitados y con los errores (si hubiera) de la decodificación de la solicitud
			Blob::Response_t<calendar_manager>* resp = new Blob::Response_t<calendar_manager>(req->idTrans, req->_error, _astdata);
			MBED_ASSERT(resp);
			if(_json_supported){
				cJSON* jresp = JsonParser::getJsonFromResponse(*resp, ObjSelectCfg);
				if(jresp){
					char* jmsg = cJSON_PrintUnformatted(jresp);
					cJSON_Delete(jresp);
					MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
					Heap::memFree(jmsg);
					delete(resp);
					Heap::memFree(pub_topic);
					return State::HANDLED;
				}
				else{
					DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR al formar Blob::NotificationData_t<calendar_manager>");
					delete(resp);
					Heap::memFree(pub_topic);
					return State::HANDLED;
				}
			}

			MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<calendar_manager>), &_publicationCb);
			delete(resp);

        	// libera la memoria asignada al topic de publicación
			Heap::memFree(pub_topic);

			DEBUG_TRACE_I(_EXPR_, _MODULE_, "Enviada respuesta con la configuracion solicitada");
            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en get/boot
        case RecvBootGet:{
			char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(pub_topic);
			sprintf(pub_topic, "stat/boot/%s", _pub_topic_base);
			Blob::NotificationData_t<calendar_manager> *notif = new Blob::NotificationData_t<calendar_manager>(_astdata);
			MBED_ASSERT(notif);
			if(_json_supported){
				cJSON* jboot = JsonParser::getJsonFromNotification<calendar_manager>(*notif, ObjSelectAll);
				if(jboot){
					char* jmsg = cJSON_PrintUnformatted(jboot);
					cJSON_Delete(jboot);
					MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
					Heap::memFree(jmsg);
					Heap::memFree(pub_topic);
					delete(notif);
					return State::HANDLED;
				}
				else{
					DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR al formar Blob::NotificationData_t<calendar_manager>");
					Heap::memFree(pub_topic);
					delete(notif);
					return State::HANDLED;
				}
			}

			MQ::MQClient::publish(pub_topic, notif, sizeof(Blob::NotificationData_t<calendar_manager>), &_publicationCb);
			Heap::memFree(pub_topic);
			delete(notif);
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


