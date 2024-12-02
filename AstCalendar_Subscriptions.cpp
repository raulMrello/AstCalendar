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
void AstCalendar::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // si es un comando para actualizar en bloque toda la configuraci�n...
    if(MQ::MQClient::isTokenRoot(topic, "set/cfg")){
        DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

        Blob::SetRequest_t<calendar_manager>* req = NULL;
        bool json_decoded = false;
		if(_json_supported){
			req = (Blob::SetRequest_t<calendar_manager>*)Heap::memAlloc(sizeof(Blob::SetRequest_t<calendar_manager>));
			MBED_ASSERT(req);
			if(!(json_decoded = JsonParser::getSetRequestFromJson(*req, *(cJSON**)msg))){
				Heap::memFree(req);
			}
		}

        // Antes de nada, chequea que el tama�o de la zona horaria es correcto, en caso contrario, descarta el topic
        if(!json_decoded && msg_len != sizeof(Blob::SetRequest_t<calendar_manager>)){
        	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el n� de datos del mensaje, topic [%s]", topic);
			return;
        }

        // crea el mensaje para publicar en la m�quina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

        // el mensaje es un blob tipo Blob::LightCfgData_t
        if(!json_decoded){
        	req = (Blob::SetRequest_t<calendar_manager>*)Heap::memAlloc(sizeof(Blob::SetRequest_t<calendar_manager>));
        	MBED_ASSERT(req);
        	*req = *((Blob::SetRequest_t<calendar_manager>*)msg);
        }
        op->sig = RecvCfgSet;
		// apunta a los datos
		op->msg = req;

		// postea en la cola de la m�quina de estados
		if(putMessage(op) != osOK){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_PUT. al procesar el topic[%s]", topic);
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
    }

    if(MQ::MQClient::isTokenRoot(topic, "set/default")){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

		Blob::SetRequest_t<uint32_t>* req = NULL;
		bool json_decoded = false;
		if(_json_supported){
			req = (Blob::SetRequest_t<uint32_t>*)Heap::memAlloc(sizeof(Blob::SetRequest_t<uint32_t>));
			MBED_ASSERT(req);
			if(!(json_decoded = JsonParser::getSetRequestFromJson(*req, *(cJSON**)msg))){
				Heap::memFree(req);
			}
		}

		// Antes de nada, chequea que el tama�o de la zona horaria es correcto, en caso contrario, descarta el topic
		if(!json_decoded && msg_len != sizeof(Blob::SetRequest_t<uint32_t>)){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el n� de datos del mensaje, topic [%s]", topic);
			return;
		}

		// crea el mensaje para publicar en la m�quina de estados
		State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
		MBED_ASSERT(op);

		// el mensaje es un blob tipo Blob::LightCfgData_t
		if(!json_decoded){
			req = (Blob::SetRequest_t<uint32_t>*)Heap::memAlloc(sizeof(Blob::SetRequest_t<uint32_t>));
			MBED_ASSERT(req);
			*req = *((Blob::SetRequest_t<uint32_t>*)msg);
		}
		op->sig = RcvSetDefault;
		// apunta a los datos
		op->msg = req;

		// postea en la cola de la m�quina de estados
		if(putMessage(op) != osOK){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_PUT. al procesar el topic[%s]", topic);
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
		return;
	}

    // si es un comando para solicitar la configuraci�n
    if(MQ::MQClient::isTokenRoot(topic, "get/cfg")){
        DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

        Blob::GetRequest_t* req = NULL;
        bool json_decoded = false;
        if(_json_supported){
			req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
			MBED_ASSERT(req);
			if(!(json_decoded = JsonParser::getGetRequestFromJson(*req, *(cJSON**)msg))){
				Heap::memFree(req);
			}
        }

        // Antes de nada, chequea que el tama�o de la zona horaria es correcto, en caso contrario, descarta el topic
        if(!json_decoded && msg_len != sizeof(Blob::GetRequest_t)){
        	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el n� de datos del mensaje, topic [%s]", topic);
			return;
        }

        // crea el mensaje para publicar en la m�quina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

        // el mensaje es un blob tipo Blob::GetRequest_t
        if(!json_decoded){
        	req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
        	MBED_ASSERT(req);
        	*req = *((Blob::GetRequest_t*)msg);
        }
		op->sig = RecvCfgGet;
		// apunta a los datos
		op->msg = req;

		// postea en la cola de la m�quina de estados
		if(putMessage(op) != osOK){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_PUT. al procesar el topic[%s]", topic);
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
    }

    // si es un comando para solicitar la configuraci�n
    if(MQ::MQClient::isTokenRoot(topic, "get/boot")){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

        Blob::GetRequest_t* req = NULL;
        bool json_decoded = false;
        if(_json_supported){
			req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
			MBED_ASSERT(req);
			if(!(json_decoded = JsonParser::getGetRequestFromJson(*req, *(cJSON**)msg))){
				Heap::memFree(req);
			}
        }

        // Antes de nada, chequea que el tama�o de la zona horaria es correcto, en caso contrario, descarta el topic
        if(!json_decoded && msg_len != sizeof(Blob::GetRequest_t)){
        	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el n� de datos del mensaje, topic [%s]", topic);
			return;
        }

        // crea el mensaje para publicar en la m�quina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

        // el mensaje es un blob tipo Blob::GetRequest_t
        if(!json_decoded){
        	req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
        	MBED_ASSERT(req);
        	*req = *((Blob::GetRequest_t*)msg);
        }
		op->sig = RecvBootGet;
		// apunta a los datos
		op->msg = req;

		// postea en la cola de la m�quina de estados
		if(putMessage(op) != osOK){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_PUT. al procesar el topic[%s]", topic);
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
    }

	// si es un comando para solicitar la configuraci�n
    if(MQ::MQClient::isTokenRoot(topic, "set/rtc")){
        DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);
		
		Blob::SetRequest_t<time_t>* req = NULL;
        bool json_decoded = false;
		if(_json_supported){
			req = (Blob::SetRequest_t<time_t>*)Heap::memAlloc(sizeof(Blob::SetRequest_t<time_t>));
			MBED_ASSERT(req);
			if(!(json_decoded = JsonParser::getSetRequestFromJson(*req, *(cJSON**)msg))){
				Heap::memFree(req);
			}
		}

        // Antes de nada, chequea que el tama�o de la zona horaria es correcto, en caso contrario, descarta el topic
        if(!json_decoded && msg_len != sizeof(Blob::SetRequest_t<time_t>)){
        	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el n� de datos del mensaje, topic [%s]", topic);
			return;
        }

        // crea el mensaje para publicar en la m�quina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

        if(!json_decoded){
        	req = (Blob::SetRequest_t<time_t>*)Heap::memAlloc(sizeof(Blob::SetRequest_t<time_t>));
        	MBED_ASSERT(req);
        	*req = *((Blob::SetRequest_t<time_t>*)msg);
        }

		op->sig = RecvRtcSet;
		op->msg = req;
		// postea en la cola de la m�quina de estados
		if(putMessage(op) != osOK){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_PUT. al procesar el topic[%s]", topic);
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
    }
	if(MQ::MQClient::isTokenRoot(topic, "get/orto")){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

        Blob::GetRequest_t* req = NULL;
        bool json_decoded = false;
        if(_json_supported){
			req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
			MBED_ASSERT(req);
			if(!(json_decoded = JsonParser::getGetRequestFromJson(*req, *(cJSON**)msg))){
				Heap::memFree(req);
			}
        }

        // Antes de nada, chequea que el tama�o de la zona horaria es correcto, en caso contrario, descarta el topic
        if(!json_decoded && msg_len != sizeof(Blob::GetRequest_t)){
        	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el n� de datos del mensaje, topic [%s]", topic);
			return;
        }

        // crea el mensaje para publicar en la m�quina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

        // el mensaje es un blob tipo Blob::GetRequest_t
        if(!json_decoded){
        	req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
        	MBED_ASSERT(req);
        	*req = *((Blob::GetRequest_t*)msg);
        }
		op->sig = RcvOrtoGet;
		// apunta a los datos
		op->msg = req;

		// postea en la cola de la m�quina de estados
		if(putMessage(op) != osOK){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_PUT. al procesar el topic[%s]", topic);
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
	}
	if(MQ::MQClient::isTokenRoot(topic, "get/ocaso")){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

        Blob::GetRequest_t* req = NULL;
        bool json_decoded = false;
        if(_json_supported){
			req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
			MBED_ASSERT(req);
			if(!(json_decoded = JsonParser::getGetRequestFromJson(*req, *(cJSON**)msg))){
				Heap::memFree(req);
			}
        }

        // Antes de nada, chequea que el tama�o de la zona horaria es correcto, en caso contrario, descarta el topic
        if(!json_decoded && msg_len != sizeof(Blob::GetRequest_t)){
        	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el n� de datos del mensaje, topic [%s]", topic);
			return;
        }

        // crea el mensaje para publicar en la m�quina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

        // el mensaje es un blob tipo Blob::GetRequest_t
        if(!json_decoded){
        	req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
        	MBED_ASSERT(req);
        	*req = *((Blob::GetRequest_t*)msg);
        }
		op->sig = RcvOcasoGet;
		// apunta a los datos
		op->msg = req;

		// postea en la cola de la m�quina de estados
		if(putMessage(op) != osOK){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_PUT. al procesar el topic[%s]", topic);
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
	}

    DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_TOPIC. No se puede procesar el topic [%s]", topic);
}


