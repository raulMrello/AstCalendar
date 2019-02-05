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
void AstCalendar::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){

    // si es un comando para actualizar en bloque toda la configuración...
    if(MQ::MQClient::isTokenRoot(topic, "set/cfg")){
        DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

        Blob::SetRequest_t<Blob::AstCalCfgData_t>* req = NULL;
        bool json_decoded = false;
		if(_json_supported){
			req = (Blob::SetRequest_t<Blob::AstCalCfgData_t>*)Heap::memAlloc(sizeof(Blob::SetRequest_t<Blob::AstCalCfgData_t>));
			MBED_ASSERT(req);
			if(!(json_decoded = decodeSetRequest(*req, (char*)msg))){
				Heap::memFree(req);
			}
		}

        // Antes de nada, chequea que el tamaño de la zona horaria es correcto, en caso contrario, descarta el topic
        if(!json_decoded && msg_len != sizeof(Blob::SetRequest_t<Blob::AstCalCfgData_t>)){
        	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el nº de datos del mensaje, topic [%s]", topic);
			return;
        }

        // crea el mensaje para publicar en la máquina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

        // el mensaje es un blob tipo Blob::LightCfgData_t
        if(!json_decoded){
        	req = (Blob::SetRequest_t<Blob::AstCalCfgData_t>*)Heap::memAlloc(sizeof(Blob::SetRequest_t<Blob::AstCalCfgData_t>));
        	MBED_ASSERT(req);
        	*req = *((Blob::SetRequest_t<Blob::AstCalCfgData_t>*)msg);
        }
        op->sig = RecvCfgSet;
		// apunta a los datos
		op->msg = req;

		// postea en la cola de la máquina de estados
		if(putMessage(op) != osOK){
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
    }

    // si es un comando para solicitar la configuración
    if(MQ::MQClient::isTokenRoot(topic, "get/cfg")){
        DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

        Blob::GetRequest_t* req = NULL;
        bool json_decoded = false;
        if(_json_supported){
			req = (Blob::GetRequest_t*)Heap::memAlloc(sizeof(Blob::GetRequest_t));
			MBED_ASSERT(req);
			if(!(json_decoded = JSON::decodeGetRequest(*req, (char*)msg))){
				Heap::memFree(req);
			}
        }

        // Antes de nada, chequea que el tamaño de la zona horaria es correcto, en caso contrario, descarta el topic
        if(!json_decoded && msg_len != sizeof(Blob::GetRequest_t)){
        	DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_MSG. Error en el nº de datos del mensaje, topic [%s]", topic);
			return;
        }

        // crea el mensaje para publicar en la máquina de estados
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

		// postea en la cola de la máquina de estados
		if(putMessage(op) != osOK){
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
    }

    // si es un comando para solicitar la configuración
    if(MQ::MQClient::isTokenRoot(topic, "get/boot")){
        DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s", topic);

        // crea el mensaje para publicar en la máquina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

        op->sig = RecvBootGet;
		// apunta a los datos
		op->msg = NULL;
		// postea en la cola de la máquina de estados
		if(putMessage(op) != osOK){
			if(op->msg){
				Heap::memFree(op->msg);
			}
			Heap::memFree(op);
		}
        return;
    }

    DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_TOPIC. No se puede procesar el topic [%s]", topic);
}

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
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en cmd/$BASE/cfg/set
        case RecvCfgSet:{
        	Blob::SetRequest_t<Blob::AstCalCfgData_t>* req = (Blob::SetRequest_t<Blob::AstCalCfgData_t>*)st_msg->msg;
        	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Recibida nueva configuración keys=%x", req->keys);
			// si no hay errores, actualiza la configuración
			if(req->_error.code == Blob::ErrOK){
				_updateConfig(req->data, req->keys, req->_error);
			}
        	// si hay errores en el mensaje o en la actualización, devuelve resultado sin hacer nada
        	if(req->_error.code != Blob::ErrOK){
        		DEBUG_TRACE_I(_EXPR_, _MODULE_, "ERR_UPD al actualizar code=%d", req->_error.code);
        		char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "stat/cfg/%s", _pub_topic_base);

				Blob::Response_t<Blob::AstCalCfgData_t>* resp = new Blob::Response_t<Blob::AstCalCfgData_t>(req->idTrans, req->_error, _astdata.cfg);

				if(_json_supported){
					cJSON* jresp = encodeCfgResponse(*resp);
					if(jresp){
						char* jmsg = cJSON_Print(jresp);
						cJSON_Delete(jresp);
						MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
						Heap::memFree(jmsg);
						delete(resp);
						Heap::memFree(pub_topic);
						return State::HANDLED;
					}
				}

				MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<Blob::AstCalCfgData_t>), &_publicationCb);
				delete(resp);
				Heap::memFree(pub_topic);
				return State::HANDLED;
        	}

        	// almacena en el sistema de ficheros
        	saveConfig();
        	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Config actualizada");

        	// si está habilitada la notificación de actualización, lo notifica
        	if((_astdata.cfg.updFlagMask & Blob::EnableAstCalCfgUpdNotif) != 0){
				char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "stat/cfg/%s", _pub_topic_base);

				Blob::Response_t<Blob::AstCalCfgData_t>* resp = new Blob::Response_t<Blob::AstCalCfgData_t>(req->idTrans, req->_error, _astdata.cfg);

				if(_json_supported){
					cJSON* jresp = encodeCfgResponse(*resp);
					if(jresp){
						char* jmsg = cJSON_Print(jresp);
						cJSON_Delete(jresp);
						MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
						Heap::memFree(jmsg);
						delete(resp);
						Heap::memFree(pub_topic);
						return State::HANDLED;
					}
				}

				MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<Blob::AstCalCfgData_t>), &_publicationCb);
				delete(resp);
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
			Blob::Response_t<Blob::AstCalCfgData_t>* resp = new Blob::Response_t<Blob::AstCalCfgData_t>(req->idTrans, req->_error, _astdata.cfg);

			if(_json_supported){
				cJSON* jresp = encodeCfgResponse(*resp);
				if(jresp){
					char* jmsg = cJSON_Print(jresp);
					cJSON_Delete(jresp);
					MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
					Heap::memFree(jmsg);
					delete(resp);
					Heap::memFree(pub_topic);
					return State::HANDLED;
				}
			}

			MQ::MQClient::publish(pub_topic, resp, sizeof(Blob::Response_t<Blob::AstCalCfgData_t>), &_publicationCb);
			delete(resp);

        	// libera la memoria asignada al topic de publicación
			Heap::memFree(pub_topic);
            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en get/boot
        case RecvBootGet:{
			char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(pub_topic);
			sprintf(pub_topic, "stat/boot/%s", _pub_topic_base);

			if(_json_supported){
				cJSON* jboot = _encodeBoot();
				if(jboot){
					char* jmsg = cJSON_Print(jboot);
					cJSON_Delete(jboot);
					MQ::MQClient::publish(pub_topic, jmsg, strlen(jmsg)+1, &_publicationCb);
					Heap::memFree(jmsg);
					Heap::memFree(pub_topic);
					return State::HANDLED;
				}
			}

			MQ::MQClient::publish(pub_topic, &_astdata, sizeof(Blob::AstCalBootData_t), &_publicationCb);
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


//------------------------------------------------------------------------------------
osEvent AstCalendar::getOsEvent(){
	return _queue.get();
}



//------------------------------------------------------------------------------------
void AstCalendar::publicationCb(const char* topic, int32_t result){

}


//------------------------------------------------------------------------------------
bool AstCalendar::checkIntegrity(){
	// verifico zona horaria
	if(strlen(_astdata.cfg.seasonCfg.envText)<= 1 || strlen(_astdata.cfg.seasonCfg.envText) > strlen("GMT+XXGMT+XX,Mmm.s.w/hh:mm,Mmm.5.0/hh:mm")+1){
		return false;
	}

	// verifico rangos astCfg
	if(_astdata.cfg.astCfg.reductionStart > Blob::TimestampMinutesDayLimit || _astdata.cfg.astCfg.reductionStop > Blob::TimestampMinutesDayLimit ||
	   _astdata.cfg.astCfg.wdowDawnStart > Blob::TimestampMinutesDayLimit || _astdata.cfg.astCfg.wdowDawnStop > Blob::TimestampMinutesDayLimit ||
	   _astdata.cfg.astCfg.wdowDuskStart > Blob::TimestampMinutesDayLimit || _astdata.cfg.astCfg.wdowDuskStop > Blob::TimestampMinutesDayLimit){
		return false;
	}
	return true;
}


//------------------------------------------------------------------------------------
void AstCalendar::setDefaultConfig(){
	// inicializa notificación de flags
	_astdata.cfg.updFlagMask = (Blob::AstCalUpdFlags)(Blob::EnableAstCalCfgUpdNotif);
	_astdata.cfg.evtFlagMask = (Blob::AstCalEvtFlags)(Blob::AstCalIVEvt | Blob::AstCalVIEvt | Blob::AstCalDayEvt | Blob::AstCalDawnEvt | Blob::AstCalDuskEvt | Blob::AstCalHourEvt | Blob::AstCalMinEvt | Blob::AstCalSecEvt);
	// desactiva toda la configuración astronómica y establece Madrid como localización por defecto
	_astdata.cfg.astCfg = {40416500, -3702560, 0, 0, 0, 0, 0, 0};
	// establece cambio horario por defecto de la zona UE
	strcpy(_astdata.cfg.seasonCfg.envText, "GMT-1GMT-2,M3.5.0/2,M10.5.0");
	saveConfig();
}


//------------------------------------------------------------------------------------
void AstCalendar::restoreConfig(){
	uint32_t crc = 0;
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recuperando datos de memoria NV...");
	bool success = true;
	if(!restoreParameter("AstCalUpdFlags", &_astdata.cfg.updFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo UpdFlags!");
		success = false;
	}
	if(!restoreParameter("AstCalEvtFlags", &_astdata.cfg.evtFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo EvtFlags!");
		success = false;
	}
	if(!restoreParameter("AstCalIVData", _astdata.cfg.seasonCfg.envText, Blob::LengthOfSeasonEnvText, NVSInterface::TypeString)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo IVData!");
		success = false;
	}
	if(!restoreParameter("AstCalAstData", &_astdata.cfg.astCfg, sizeof(Blob::AstCalAstData_t), NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo AstData!");
		success = false;
	}
	if(!restoreParameter("AstCalChecksum", &crc, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo Checksum!");
		success = false;
	}

	if(success){
		// chequea el checksum crc32 y después la integridad de los datos
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Datos recuperados. Chequeando integridad...");
		if(Blob::getCRC32(&_astdata.cfg, sizeof(Blob::AstCalCfgData_t)) != crc){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_CFG. Ha fallado el checksum");
		}
    	else if(!checkIntegrity()){
    		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_CFG. Ha fallado el check de integridad. Establece configuración por defecto.");
    	}
    	else{
    		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Check de integridad OK!");
    	}
	}
	else{
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_FS. Error en la recuperación de datos. Establece configuración por defecto");
		setDefaultConfig();
	}

	// Una vez establecida la configuración, actualiza la hora del sistema
	// obtiene la hora actual del RTC
	if(_rtc){
		_rtc->getTime(&_now);
		if (_now.tm_year < (2018 - 1900)) {
			DEBUG_TRACE_I(_EXPR_, _MODULE_, "ERR_RTC_READ datos incorrectos: %d:%d:%d, %d-%d-%d diasem=%d",
					_now.tm_hour,
					_now.tm_min,
					_now.tm_sec,
					_now.tm_mday,
					_now.tm_mon+1,
					_now.tm_year,
					_now.tm_wday);

			// establece la hora por defecto lunes, 1 Ene 2018 a las 00:00
			// set January 1st,2018 0am as a default
			_now.tm_sec  = 0;
			_now.tm_min  = 0;
			_now.tm_hour = 0;
			_now.tm_mday = 1;
			_now.tm_wday = 1;
			_now.tm_mon  = 0;
			_now.tm_year = 2018- 1900;
			_now.tm_yday = 0;
			_now.tm_isdst = 0;
			_rtc->setTime(_now);
		}
	}

	setenv("TZ", _astdata.cfg.seasonCfg.envText, 1);
	tzset() ;
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Establece zona horaria '%s'", _astdata.cfg.seasonCfg.envText);
	time_t tnow = mktime(&_now);
	timeval tv;
	tv.tv_sec = tnow;
	tv.tv_usec = 0;

	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Establece hora del sistema...");
	settimeofday (&tv, NULL);
	tnow = time(NULL);
	localtime_r(&tnow, &_now);
	char strftime_buf[64];
	memset(strftime_buf, 0, 64);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &_now);
	strftime_buf[63] = 0;
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema %s", strftime_buf);
}


//------------------------------------------------------------------------------------
void AstCalendar::saveConfig(){
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Guardando datos en memoria NV...");

	// almacena en el sistema de ficheros
	if(!saveParameter("AstCalUpdFlags", &_astdata.cfg.updFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando UpdFlags!");
	}
	if(!saveParameter("AstCalEvtFlags", &_astdata.cfg.evtFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando EvtFlags!");
	}
	if(!saveParameter("AstCalAstData", &_astdata.cfg.astCfg, sizeof(Blob::AstCalAstData_t), NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando AstData!");
	}
	if(!saveParameter("AstCalIVData", _astdata.cfg.seasonCfg.envText, Blob::LengthOfSeasonEnvText, NVSInterface::TypeString)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando IVData!");
	}
	uint32_t crc = Blob::getCRC32(&_astdata.cfg, sizeof(Blob::AstCalCfgData_t));
	if(!saveParameter("AstCalChecksum", &crc, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando Checksum!");
	}
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
			cJSON* jstat = _encodeStat();
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


//------------------------------------------------------------------------------------
void AstCalendar::_updateConfig(const Blob::AstCalCfgData_t& cfg, uint32_t keys, Blob::ErrorData_t& err){
	if(keys == Blob::AstCalKeyNone){
		err.code = Blob::ErrEmptyContent;
		goto _updateConfigExit;
	}
	if( ((keys & Blob::AstCalKeyCfgLat) && (cfg.astCfg.latitude < Blob::AstCalMinLatitude || cfg.astCfg.latitude > Blob::AstCalMaxLatitude)) ||
	    ((keys & Blob::AstCalKeyCfgLon) && (cfg.astCfg.longitude < Blob::AstCalMinLongitude || cfg.astCfg.longitude > Blob::AstCalMaxLongitude)) ||
		((keys & Blob::AstCalKeyCfgWdaSta) && (cfg.astCfg.wdowDawnStart < Blob::AstCalMinAstWdow || cfg.astCfg.wdowDawnStart > Blob::AstCalMaxAstWdow)) ||
		((keys & Blob::AstCalKeyCfgWdaStp) && (cfg.astCfg.wdowDawnStop < Blob::AstCalMinAstWdow || cfg.astCfg.wdowDawnStop > Blob::AstCalMaxAstWdow)) ||
		((keys & Blob::AstCalKeyCfgWduSta) && (cfg.astCfg.wdowDuskStart < Blob::AstCalMinAstWdow || cfg.astCfg.wdowDuskStart > Blob::AstCalMaxAstWdow)) ||
		((keys & Blob::AstCalKeyCfgWduStp) && (cfg.astCfg.wdowDuskStop < Blob::AstCalMinAstWdow || cfg.astCfg.wdowDuskStop > Blob::AstCalMaxAstWdow)) ||
		((keys & Blob::AstCalKeyCfgRedSta) && cfg.astCfg.reductionStart > Blob::AstCalMaxAstRedct) ||
		((keys & Blob::AstCalKeyCfgRedStp) && cfg.astCfg.reductionStop > Blob::AstCalMaxAstRedct) ||
		((keys & Blob::AstCalKeyCfgSeason) && (strlen(cfg.seasonCfg.envText) == 0 ||  strlen(cfg.seasonCfg.envText) >= Blob::LengthOfSeasonEnvText))){
		err.code = Blob::ErrRangeValue;
		goto _updateConfigExit;
	}

	if(keys & Blob::AstCalKeyCfgUpd){
		_astdata.cfg.updFlagMask = cfg.updFlagMask;
	}
	if(keys & Blob::AstCalKeyCfgEvt){
		_astdata.cfg.evtFlagMask = cfg.evtFlagMask;
	}
	if(keys & Blob::AstCalKeyCfgLat){
		_astdata.cfg.astCfg.latitude = cfg.astCfg.latitude;
	}
	if(keys & Blob::AstCalKeyCfgLon){
		_astdata.cfg.astCfg.longitude = cfg.astCfg.longitude;
	}
	if(keys & Blob::AstCalKeyCfgWdaSta){
		_astdata.cfg.astCfg.wdowDawnStart = cfg.astCfg.wdowDawnStart;
	}
	if(keys & Blob::AstCalKeyCfgWdaStp){
		_astdata.cfg.astCfg.wdowDawnStop = cfg.astCfg.wdowDawnStop;
	}
	if(keys & Blob::AstCalKeyCfgWduSta){
		_astdata.cfg.astCfg.wdowDuskStart = cfg.astCfg.wdowDuskStart;
	}
	if(keys & Blob::AstCalKeyCfgWduStp){
		_astdata.cfg.astCfg.wdowDuskStop = cfg.astCfg.wdowDuskStop;
	}
	if(keys & Blob::AstCalKeyCfgRedSta){
		_astdata.cfg.astCfg.reductionStart = cfg.astCfg.reductionStart;
	}
	if(keys & Blob::AstCalKeyCfgRedStp){
		_astdata.cfg.astCfg.reductionStop = cfg.astCfg.reductionStop;
	}
	if(keys & Blob::AstCalKeyCfgSeason){
		strcpy(_astdata.cfg.seasonCfg.envText, cfg.seasonCfg.envText);
	}
_updateConfigExit:
	strcpy(err.descr, Blob::errList[err.code]);
}




//------------------------------------------------------------------------------------
//-- JSON SUPPORT --------------------------------------------------------------------
//------------------------------------------------------------------------------------


/** JSON keys */
static const char* p_astcal 		= "astcal";
static const char* p_astCfg 		= "astCfg";
static const char* p_astData 		= "astData";
static const char* p_code	 		= "code";
static const char* p_descr	 		= "descr";
static const char* p_error	 		= "error";
static const char* p_evtFlags 		= "evtFlags";
static const char* p_flags			= "flags";
static const char* p_header			= "header";
static const char* p_idTrans 		= "idTrans";
static const char* p_keys 			= "keys";
static const char* p_latitude 		= "latitude";
static const char* p_longitude 		= "longitude";
static const char* p_now	 		= "now";
static const char* p_period 		= "period";
static const char* p_reductionStart = "reductionStart";
static const char* p_reductionStop	= "reductionStop";
static const char* p_seasonCfg 		= "seasonCfg";
static const char* p_timestamp		= "timestamp";
static const char* p_updFlags 		= "updFlags";
static const char* p_wdowDawnStart 	= "wdowDawnStart";
static const char* p_wdowDawnStop 	= "wdowDawnStop";
static const char* p_wdowDuskStart 	= "wdowDuskStart";
static const char* p_wdowDuskStop 	= "wdowDuskStop";



//------------------------------------------------------------------------------------
cJSON* AstCalendar::encodeCfg(const Blob::AstCalCfgData_t& cfg){
	cJSON* astcal = NULL;
	cJSON* value = NULL;

	if((astcal=cJSON_CreateObject()) == NULL){
		return NULL;
	}
	// key: astcal.updFlags
	cJSON_AddNumberToObject(astcal, p_updFlags, cfg.updFlagMask);

	// key: astcal.evtFlags
	cJSON_AddNumberToObject(astcal, p_evtFlags, cfg.evtFlagMask);
	// key: astCfg
	if((value=cJSON_CreateObject()) == NULL){
		cJSON_Delete(astcal);
		return NULL;
	}
	cJSON_AddNumberToObject(value, p_latitude, cfg.astCfg.latitude);
	cJSON_AddNumberToObject(value, p_longitude, cfg.astCfg.longitude);
	cJSON_AddNumberToObject(value, p_wdowDawnStart, cfg.astCfg.wdowDawnStart);
	cJSON_AddNumberToObject(value, p_wdowDawnStop, cfg.astCfg.wdowDawnStop);
	cJSON_AddNumberToObject(value, p_wdowDuskStart, cfg.astCfg.wdowDuskStart);
	cJSON_AddNumberToObject(value, p_wdowDuskStop, cfg.astCfg.wdowDuskStop);
	cJSON_AddNumberToObject(value, p_reductionStart, cfg.astCfg.reductionStart);
	cJSON_AddNumberToObject(value, p_reductionStop, cfg.astCfg.reductionStop);
	cJSON_AddItemToObject(astcal, p_astCfg, value);
	// key: seasonCfg
	if((value=cJSON_CreateString(cfg.seasonCfg.envText)) == NULL){
		cJSON_Delete(astcal);
		return NULL;
	}
	cJSON_AddItemToObject(astcal, p_seasonCfg, value);
	return astcal;
}


//------------------------------------------------------------------------------------
cJSON* AstCalendar::encodeStat(const Blob::AstCalStatData_t& stat){
	cJSON* astcal = NULL;

	if((astcal=cJSON_CreateObject()) == NULL){
		return NULL;
	}

	cJSON_AddNumberToObject(astcal, p_flags, stat.flags);
	cJSON_AddNumberToObject(astcal, p_period, stat.period);
	cJSON_AddNumberToObject(astcal, p_now, stat.now);
	cJSON* astData = NULL;
	if((astData=cJSON_CreateObject()) == NULL){
		cJSON_Delete(astcal);
		return NULL;
	}
	cJSON_AddNumberToObject(astData, p_latitude, stat.astData.latitude);
	cJSON_AddNumberToObject(astData, p_longitude, stat.astData.longitude);
	cJSON_AddNumberToObject(astData, p_wdowDawnStart, stat.astData.wdowDawnStart);
	cJSON_AddNumberToObject(astData, p_wdowDawnStop, stat.astData.wdowDawnStop);
	cJSON_AddNumberToObject(astData, p_wdowDuskStart, stat.astData.wdowDuskStart);
	cJSON_AddNumberToObject(astData, p_wdowDuskStop, stat.astData.wdowDuskStop);
	cJSON_AddNumberToObject(astData, p_reductionStart, stat.astData.reductionStart);
	cJSON_AddNumberToObject(astData, p_reductionStop, stat.astData.reductionStop);
	cJSON_AddItemToObject(astcal, p_astData, astData);
	return astcal;
}


//------------------------------------------------------------------------------------
bool AstCalendar::decodeSetRequest(Blob::SetRequest_t<Blob::AstCalCfgData_t>&req, char* json_data){
	cJSON *obj = NULL;
	cJSON *astcal = NULL;
	cJSON *astcfg = NULL;
	cJSON *root = NULL;
	req.keys = Blob::AstCalKeyNone;
	req._error.code = Blob::ErrOK;

	if((root = cJSON_Parse(json_data)) == NULL){
		req._error.code = Blob::ErrJsonMalformed;
		req.idTrans = Blob::UnusedIdTrans;
		goto __decode_null_exitAstCalCfg;
	}

	// key: idTrans
	if((obj = cJSON_GetObjectItem(root, p_idTrans)) == NULL){
		req._error.code = Blob::ErrIdTransInvalid;
		req.idTrans = Blob::UnusedIdTrans;
		goto __decode_null_exitAstCalCfg;
	}
	req.idTrans = obj->valueint;

	// key:astcal
	if((astcal = cJSON_GetObjectItem(root, p_astcal)) == NULL){
		req._error.code = Blob::ErrAstCalMissing;
		goto __decode_null_exitAstCalCfg;
	}

	if((obj = cJSON_GetObjectItem(astcal,p_updFlags)) != NULL){
		req.data.updFlagMask = (Blob::AstCalUpdFlags)obj->valueint;
		req.keys |= Blob::AstCalKeyCfgUpd;
	}
	if((obj = cJSON_GetObjectItem(astcal,p_evtFlags)) != NULL){
		req.data.evtFlagMask = (Blob::AstCalEvtFlags)obj->valueint;
		req.keys |= Blob::AstCalKeyCfgEvt;
	}
	if((obj = cJSON_GetObjectItem(astcal, p_seasonCfg)) != NULL){
		char* seas_cfg = obj->valuestring;
		if(!seas_cfg || strlen(seas_cfg) >= Blob::LengthOfSeasonEnvText){
			req._error.code = Blob::ErrSeasonCfgFormat;
			goto __decode_null_exitAstCalCfg;
		}
		strcpy(req.data.seasonCfg.envText, seas_cfg);
		req.keys |= Blob::AstCalKeyCfgSeason;
	}

	if((astcfg = cJSON_GetObjectItem(astcal, p_astCfg)) != NULL){
		if((obj = cJSON_GetObjectItem(astcfg, p_latitude)) != NULL){
			req.data.astCfg.latitude = obj->valueint;
			req.keys |= Blob::AstCalKeyCfgLat;
		}
		if((obj = cJSON_GetObjectItem(astcfg, p_longitude)) != NULL){
			req.data.astCfg.longitude = obj->valueint;
			req.keys |= Blob::AstCalKeyCfgLon;
		}
		if((obj = cJSON_GetObjectItem(astcfg, p_wdowDawnStart)) != NULL){
			req.data.astCfg.wdowDawnStart = obj->valueint;
			req.keys |= Blob::AstCalKeyCfgWdaSta;
		}
		if((obj = cJSON_GetObjectItem(astcfg, p_wdowDawnStop)) != NULL){
			req.data.astCfg.wdowDawnStop = obj->valueint;
			req.keys |= Blob::AstCalKeyCfgWdaStp;
		}
		if((obj = cJSON_GetObjectItem(astcfg, p_wdowDuskStart)) != NULL){
			req.data.astCfg.wdowDuskStart = obj->valueint;
			req.keys |= Blob::AstCalKeyCfgWduSta;
		}
		if((obj = cJSON_GetObjectItem(astcfg, p_wdowDuskStop)) != NULL){
			req.data.astCfg.wdowDuskStop = obj->valueint;
			req.keys |= Blob::AstCalKeyCfgWduStp;
		}
		if((obj = cJSON_GetObjectItem(astcfg, p_reductionStart)) != NULL){
			req.data.astCfg.reductionStart = obj->valueint;
			req.keys |= Blob::AstCalKeyCfgRedSta;
		}
		if((obj = cJSON_GetObjectItem(astcfg, p_reductionStop)) != NULL){
			req.data.astCfg.reductionStop = obj->valueint;
			req.keys |= Blob::AstCalKeyCfgRedStp;
		}
	}

__decode_null_exitAstCalCfg:
	strcpy(req._error.descr, Blob::errList[req._error.code]);
	if(root){
		cJSON_Delete(root);
	}
	if(req._error.code == Blob::ErrOK){
		return true;
	}
	return false;
}


//------------------------------------------------------------------------------------
cJSON* AstCalendar::encodeCfgResponse(const Blob::Response_t<Blob::AstCalCfgData_t> &resp){
	// keys: root, idtrans, header, error, astcal
	cJSON *header = NULL;
	cJSON *error = NULL;
	cJSON *astcal = NULL;
	cJSON *value = NULL;
	cJSON *root = cJSON_CreateObject();

	if(!root){
		return NULL;
	}

	// key: idTrans sólo se envía si está en uso
	if(resp.idTrans != Blob::UnusedIdTrans){
		cJSON_AddNumberToObject(root, p_idTrans, resp.idTrans);
	}

	// key: header
	if((header=cJSON_CreateObject()) == NULL){
		goto _parseAstCalCfg_Exit;
	}
	cJSON_AddNumberToObject(header, p_timestamp, resp.header.timestamp);
	cJSON_AddItemToObject(root, p_header, header);

	// key: error sólo se envía si el error es distinto de Blob::ErrOK
	if(resp.error.code != Blob::ErrOK){
		if((error=cJSON_CreateObject()) == NULL){
			goto _parseAstCalCfg_Exit;
		}
		cJSON_AddNumberToObject(error, p_code, resp.error.code);
		if((value=cJSON_CreateString(resp.error.descr)) == NULL){
			goto _parseAstCalCfg_Exit;
		}
		cJSON_AddItemToObject(error, p_descr, value);
		// añade objeto
		cJSON_AddItemToObject(root, p_error, error);
	}

	// key: astcal sólo se envía si el error es Blob::ErrOK
	if(resp.error.code == Blob::ErrOK){
		if((astcal = encodeCfg(resp.data)) == NULL){
			goto _parseAstCalCfg_Exit;
		}
		// añade objeto
		cJSON_AddItemToObject(root, p_astcal, astcal);
	}
	return root;

_parseAstCalCfg_Exit:
	cJSON_Delete(root);
	return NULL;
}


//------------------------------------------------------------------------------------
cJSON* AstCalendar::encodeBoot(const Blob::AstCalBootData_t& boot){
	cJSON* astcal = NULL;
	cJSON* item = NULL;
	if((astcal=cJSON_CreateObject()) == NULL){
		return NULL;
	}

	if((item = encodeCfg(boot.cfg)) == NULL){
		goto __encodeBoot_Err;
	}
	cJSON_AddItemToObject(astcal, p_astCfg, item);

	if((item = encodeStat(boot.stat)) == NULL){
		goto __encodeBoot_Err;
	}
	cJSON_AddItemToObject(astcal, p_astData, item);
	return astcal;

__encodeBoot_Err:
	cJSON_Delete(astcal);
	return NULL;
}


