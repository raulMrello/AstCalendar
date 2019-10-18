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
void AstCalendar::restoreConfig(){
	uint32_t crc = 0;

	// establezco versión del modelo de datos
	_astdata.uid = UID_CALENDAR_MANAGER;
	_astdata.clock.uid = UID_CALENDAR_CLOCK;

	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Recuperando datos de memoria NV...");
	bool success = true;
	if(!restoreParameter("CalManCfg", &_astdata.cfg, sizeof(calendar_manager_cfg), NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo calendar_manager_cfg!");
		success = false;
	}
	if(!restoreParameter("CalClockCfg", &_astdata.clock.cfg, sizeof(calendar_clock_cfg), NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo calendar_clock_cfg!");
		success = false;
	}
	if(!restoreParameter("CalManCrc", &crc, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo Checksum!");
		success = false;
	}

	if(success){

		// chequea el crc
		uint8_t* crc_buf = (char*)Heap::memAlloc(sizeof(calendar_manager_cfg) + sizeof(calendar_clock_cfg));
		MBED_ASSERT(crc_buf);
		memcpy(crc_buf, &_astdata.cfg, sizeof(calendar_manager_cfg));
		memcpy(&crc_buf[sizeof(calendar_manager_cfg)], &_astdata.clock.cfg, sizeof(calendar_clock_cfg));
		uint32_t calc_crc = Blob::getCRC32(crc_buf, sizeof(calendar_manager_cfg) + sizeof(calendar_clock_cfg));
		free(crc_buf);

		if(calc_crc != crc){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_CFG. Ha fallado el checksum");
			success = false;
		}
    	else if(!checkIntegrity()){
    		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_CFG. Ha fallado el check de integridad.");
			success = false;
    	}
    	else{
    		DEBUG_TRACE_W(_EXPR_, _MODULE_, "Check de integridad OK!");
    		esp_log_level_set(_MODULE_, (esp_log_level_t)_astdata.cfg.verbosity);
    	}
	}

	if(!success){
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
	else {
		// establece la hora por defecto martes, 1 Ene 2019 a las 00:00
		// set January 1st,2018 0am as a default
		_now.tm_sec  = 0;
		_now.tm_min  = 0;
		_now.tm_hour = 0;
		_now.tm_mday = 1;
		_now.tm_wday = 2;
		_now.tm_mon  = 0;
		_now.tm_year = 2019 - 1900;
		_now.tm_yday = 0;
		_now.tm_isdst = 0;
	}

	setenv("TZ", _astdata.clock.cfg.geoloc.timezone, 1);
	tzset() ;
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Establece zona horaria '%s'", _astdata.clock.cfg.geoloc.timezone);
	time_t tnow = mktime(&_now);
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
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema %s", strftime_buf);

}


//------------------------------------------------------------------------------------
bool AstCalendar::checkIntegrity(){
	// verifico zona horaria
	if(strlen(_astdata.clock.cfg.geoloc.timezone)==0){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_INTEGRITY timezone=%s", _astdata.clock.cfg.geoloc.timezone);
		return false;
	}

	// verifico periodos activos sin rangos establecidos
	for(int i=0;i<CalendarClockCfgMaxNumPeriods; i++){
		if((_astdata.clock.cfg.periods[i].since == 0 || _astdata.clock.cfg.periods[i].until == 0) && _astdata.clock.cfg.periods[i].enabled){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_INTEGRITY period=%d, since=%ld, until=%ld,enabled=%d", i,_astdata.clock.cfg.periods[i].since, _astdata.clock.cfg.periods[i].until, _astdata.clock.cfg.periods[i].enabled);
			return false;
		}
	}

	// Verifico número de periodos discordante
	if((_astdata.clock.cfg._numPeriods != _astdata.clock.cfg.geoloc._numPeriods) || (_astdata.clock.cfg._numPeriods != CalendarClockCfgMaxNumPeriods) || (_astdata.clock.cfg.geoloc._numPeriods != CalendarClockCfgMaxNumPeriods)){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_INTEGRITY clock_cfg_periods=%d, geoloc_periods=%d", _astdata.clock.cfg._numPeriods, _astdata.clock.cfg.geoloc._numPeriods);
		return false;
	}

	#warning TODO: otras verificaciones más exhaustivas
	// ...

	return true;
}


//------------------------------------------------------------------------------------
void AstCalendar::setDefaultConfig(){

	// borro la configuración y el estado
	_astdata = {0};

	// establezco versión del modelo de datos
	_astdata.uid = UID_CALENDAR_MANAGER;
	_astdata.clock.uid = UID_CALENDAR_CLOCK;

	// establezco configuración por defecto del manager
	_astdata.cfg.updFlags = CalendarManagerCfgUpdNotif;
	_astdata.cfg.evtFlags = CalendarClockNoEvents;
	_astdata.cfg.verbosity = ESP_LOG_DEBUG;

	// establezco configuración por defecto del reloj integrado (para Madrid)
	strncpy(_astdata.clock.cfg.geoloc.timezone, "GMT-1GMT-2,M3.5.0/2,M10.5.0", CalendarGeolocTimezoneLength);
	_astdata.clock.cfg.geoloc.coords[0] = 40.416500;
	_astdata.clock.cfg.geoloc.coords[1] = -3.702560;
	for(int i=0;i<CalendarClockCfgMaxNumPeriods; i++){
		_astdata.clock.cfg.periods[i].since = 0;
		_astdata.clock.cfg.periods[i].until = 0;
		_astdata.clock.cfg.periods[i].enabled = false;
		_astdata.clock.cfg.geoloc.astCorr[i][0] = 0;
		_astdata.clock.cfg.geoloc.astCorr[i][1] = 0;
	}
	_astdata.clock.cfg._numPeriods = CalendarClockCfgMaxNumPeriods;
	_astdata.clock.cfg.geoloc._numPeriods = CalendarClockCfgMaxNumPeriods;

	saveConfig();
}



//------------------------------------------------------------------------------------
void AstCalendar::saveConfig(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Guardando datos en memoria NV...");

	// almacena en el sistema de ficheros
	if(!saveParameter("CalManCfg", &_astdata.cfg, sizeof(calendar_manager_cfg), NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando calendar_manager_cfg!");
	}
	if(!saveParameter("CalClockCfg", &_astdata.clock.cfg, sizeof(calendar_clock_cfg), NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando calendar_clock_cfg!");
	}

	// genera el crc
	uint8_t* crc_buf = (char*)Heap::memAlloc(sizeof(calendar_manager_cfg) + sizeof(calendar_clock_cfg));
	MBED_ASSERT(crc_buf);
	memcpy(crc_buf, &_astdata.cfg, sizeof(calendar_manager_cfg));
	memcpy(&crc_buf[sizeof(calendar_manager_cfg)], &_astdata.clock.cfg, sizeof(calendar_clock_cfg));
	uint32_t crc = Blob::getCRC32(crc_buf, sizeof(calendar_manager_cfg) + sizeof(calendar_clock_cfg));
	free(crc_buf);

	// graba el crc
	if(!saveParameter("CalManCrc", &crc, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando Checksum!");
	}

	// aplica el nivel de verbosidad configurado
	esp_log_level_set(_MODULE_, (esp_log_level_t)_astdata.cfg.verbosity);
}


//------------------------------------------------------------------------------------
void AstCalendar::_updateConfig(const calendar_manager& data, Blob::ErrorData_t& err){
	err.code = Blob::ErrOK;

	// calendadar_manager.uid
	if((data._keys & (1 << 0)) && data.uid != _astdata.uid){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_UPDATE uid inválido");
		err.code = Blob::ErrUidInvalid;
		goto _updateConfigExit;
	}

	// calendar_manager.cfg
	if((data._keys & (1 << 1))){
		if((data.cfg._keys & (1 << 0))){
			_astdata.cfg.updFlags = data.cfg.updFlags;
		}
		if((data.cfg._keys & (1 << 1))){
			_astdata.cfg.evtFlags = data.cfg.evtFlags;
		}
		if((data.cfg._keys & (1 << 2))){
			_astdata.cfg.verbosity = data.cfg.verbosity;
		}
	}
	// calendar:clock:cfg
	if((data._keys & (1 << 2)) && (data.clock._keys & (1 << 1))){
		// clock.cfg.periods
		if((data.clock.cfg._keys & (1 << 0))){
			for(int i=0;i<data.clock.cfg._numPeriods;i++){
				_astdata.clock.cfg.periods[i] = data.clock.cfg.periods[i];
			}
		}
		// calendar:clock:cfg:geoloc
		if((data.clock.cfg._keys & (1 << 1))){
			// coords
			if((data.clock.cfg.geoloc._keys & (1 << 1))){
				_astdata.clock.cfg.geoloc.coords[0] = data.clock.cfg.geoloc.coords[0];
				_astdata.clock.cfg.geoloc.coords[1] = data.clock.cfg.geoloc.coords[1];
			}
			// timezone
			if((data.clock.cfg.geoloc._keys & (1 << 2))){
				strncpy(_astdata.clock.cfg.geoloc.timezone, data.clock.cfg.geoloc.timezone, CalendarGeolocTimezoneLength);
			}
			// astCorr
			if((data.clock.cfg.geoloc._keys & (1 << 3))){
				for(int i=0;i<data.clock.cfg.geoloc._numPeriods;i++){
					_astdata.clock.cfg.geoloc.astCorr[i][0] = data.clock.cfg.geoloc.astCorr[i][0];
					_astdata.clock.cfg.geoloc.astCorr[i][1] = data.clock.cfg.geoloc.astCorr[i][1];
				}
			}
		}
	}

_updateConfigExit:
	strcpy(err.descr, Blob::errList[err.code]);
}


