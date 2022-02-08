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

	// establezco versi�n del modelo de datos
	_astdata.uid = UID_CALENDAR_MANAGER;
	_astdata.clock.uid = UID_CALENDAR_CLOCK;

	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Recuperando datos de memoria NV...");
	bool success = true;

	bool resultRestoreManBlob = false;
	bool resultRestoreClockBlob = false;
	bool saveCfg = false;

	calendar_manager_cfg manCfgOld = {0};
	calendar_clock_cfg_old clockCfgOld = {0};

	resultRestoreManBlob = restoreParameter("CalManCfg", &manCfgOld, sizeof(calendar_manager_cfg), NVSInterface::TypeBlob);
	if(!resultRestoreManBlob)
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "ERR_NVS leyendo CalManCfg!");

	if(!restoreParameter("CalMUpdFlag", &_astdata.cfg.updFlags, sizeof(uint32_t),NVSInterface::TypeUint32)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalMUpdFlag!");
		if(!resultRestoreManBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalMUpdFlag! Establece configuracion por defecto");
			_astdata.cfg.updFlags = CalendarManagerCfgUpdNotif;
		}
		else
			_astdata.cfg.updFlags = manCfgOld.updFlags;
	}

	if(!restoreParameter("CalMEvtFlag", &_astdata.cfg.evtFlags, sizeof(uint32_t),NVSInterface::TypeUint32)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalMEvtFlag!");
		if(!resultRestoreManBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalMEvtFlag! Establece configuracion por defecto");
			_astdata.cfg.evtFlags = CalendarClockSecEvt;
		}
		else
			_astdata.cfg.evtFlags = manCfgOld.evtFlags;
	}

	if(!restoreParameter("CalManVerb", &_astdata.cfg.verbosity, sizeof(uint8_t),NVSInterface::TypeUint8)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalManVerb!");
		if(!resultRestoreManBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalManVerb! Establece configuracion por defecto");
			_astdata.cfg.verbosity = APP_ASTCALENDAR_LOG_LEVEL;
		}
		else
			_astdata.cfg.verbosity = manCfgOld.verbosity;
	}

	if(!restoreParameter("CalManNvs", &_astdata.cfg.nvs_id, sizeof(uint32_t),NVSInterface::TypeUint32)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalManNvs!");
		if(!resultRestoreManBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalManNvs! Establece configuracion por defecto");
			_astdata.cfg.nvs_id = APP_ASTCALENDAR_NVS_ID;
		}
		else
			_astdata.cfg.nvs_id = manCfgOld.nvs_id;
	}



	resultRestoreClockBlob = restoreParameter("CalClockCfg", &clockCfgOld, sizeof(calendar_clock_cfg_old), NVSInterface::TypeBlob);
	if(!resultRestoreClockBlob)
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockCfg!");

	if(!restoreParameter("CalClNumPer", &_astdata.clock.cfg._numPeriods, sizeof(uint8_t),NVSInterface::TypeUint8)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClNumPer!");
		if(!resultRestoreClockBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClNumPer! Establece configuracion por defecto");
			_astdata.clock.cfg._numPeriods = CalendarClockCfgMaxNumPeriods;
		}
		else
			_astdata.clock.cfg._numPeriods = clockCfgOld._numPeriods;
	}

	if(!restoreParameter("CalClockPer", &_astdata.clock.cfg.periods, sizeof(calendar_period[CalendarClockCfgMaxNumPeriods]),NVSInterface::TypeBlob)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockPer!");
		if(!resultRestoreClockBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockPer! Establece configuracion por defecto");
			for(int i=0;i<CalendarClockCfgMaxNumPeriods; i++){
				_astdata.clock.cfg.periods[i].since = 0;
				_astdata.clock.cfg.periods[i].until = 0;
				_astdata.clock.cfg.periods[i].enabled = false;
			}
		}
		else{
			for(int i=0;i<CalendarClockCfgMaxNumPeriods; i++){
				_astdata.clock.cfg.periods[i].since = clockCfgOld.periods[i].since;
				_astdata.clock.cfg.periods[i].until = clockCfgOld.periods[i].until;
				_astdata.clock.cfg.periods[i].enabled = clockCfgOld.periods[i].enabled;
			}
		}
	}

	if(!restoreParameter("CalClockCoo", &_astdata.clock.cfg.geoloc.coords, sizeof(double[2]),NVSInterface::TypeBlob)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockCoo!");
		if(!resultRestoreClockBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockCoo! Establece configuracion por defecto");
			_astdata.clock.cfg.geoloc.coords[0] = 40.416500;
			_astdata.clock.cfg.geoloc.coords[1] = -3.702560;
		}
		else{
			_astdata.clock.cfg.geoloc.coords[0] = clockCfgOld.geoloc.coords[0];
			_astdata.clock.cfg.geoloc.coords[1] = clockCfgOld.geoloc.coords[1];
		}
	}

	if(!restoreParameter("CalClockZon", &_astdata.clock.cfg.geoloc.timezone, SizeOfArray(_astdata.clock.cfg.geoloc.timezone),NVSInterface::TypeString)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockZon!");
		if(!resultRestoreClockBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockZon! Establece configuracion por defecto");
			strncpy(_astdata.clock.cfg.geoloc.timezone, "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", CalendarGeolocTimezoneLength);
		}
		else
			strncpy(_astdata.clock.cfg.geoloc.timezone, clockCfgOld.geoloc.timezone, CalendarGeolocTimezoneLength);
	}

	if(!restoreParameter("CalClockCod", &_astdata.clock.cfg.geoloc.timezoneCode, sizeof(uint8_t),NVSInterface::TypeUint8)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockCod! Establece configuracion por defecto");

		if(strcmp(_astdata.clock.cfg.geoloc.timezone, "WET-0WEST-1,M3.5.0/01:00:00,M10.5.0/02:00:00")==0)
			_astdata.clock.cfg.geoloc.timezoneCode = 8;
		else if(strcmp(_astdata.clock.cfg.geoloc.timezone, "EET-2EEST-3,M3.5.0/03:00:00,M10.5.0/04:00:00")==0)
			_astdata.clock.cfg.geoloc.timezoneCode = 3;
		else if(strcmp(_astdata.clock.cfg.geoloc.timezone, "NZST-12NZDT-13,M10.1.0/02:00:00,M3.3.0/03:00:00")==0)
			_astdata.clock.cfg.geoloc.timezoneCode = 32;
		else
			_astdata.clock.cfg.geoloc.timezoneCode = 1;
	}

	if(!restoreParameter("CalClockNum", &_astdata.clock.cfg.geoloc._numPeriods, sizeof(uint8_t),NVSInterface::TypeUint8)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockNum!");
		if(!resultRestoreClockBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockNum! Establece configuracion por defecto");
			_astdata.clock.cfg.geoloc._numPeriods = CalendarClockCfgMaxNumPeriods;
		}
		else
			_astdata.clock.cfg.geoloc._numPeriods = clockCfgOld.geoloc._numPeriods;
	}

	if(!restoreParameter("CalClockAst", &_astdata.clock.cfg.geoloc.astCorr, sizeof(time_t[CalendarClockCfgMaxNumPeriods][2]),NVSInterface::TypeBlob)){
		saveCfg = true;
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockAst!");
		if(!resultRestoreClockBlob){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo CalClockAst! Establece configuracion por defecto");
			for(int i=0;i<CalendarClockCfgMaxNumPeriods; i++){
				_astdata.clock.cfg.geoloc.astCorr[i][0] = 0;
				_astdata.clock.cfg.geoloc.astCorr[i][1] = 0;
			}
		}
		else{
			for(int i=0;i<CalendarClockCfgMaxNumPeriods; i++){
				_astdata.clock.cfg.geoloc.astCorr[i][0] = clockCfgOld.geoloc.astCorr[i][0];
				_astdata.clock.cfg.geoloc.astCorr[i][1] = clockCfgOld.geoloc.astCorr[i][1];
			}
		}
	}

	if(!checkIntegrity()){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_CFG. Ha fallado el check de integridad.");
		setDefaultConfig();
    }

	if(saveCfg)
		saveConfig();

	esp_log_level_set(_MODULE_, (esp_log_level_t)_astdata.cfg.verbosity);

	// Una vez establecida la configuraci�n, actualiza la hora del sistema
	// obtiene la hora actual del RTC
	if(_rtc){
		_rtc->getTime(&_now, &_pw_fail);
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

	DEBUG_TRACE_I(_EXPR_,_MODULE_,"Segundos transcurridos desde el ultimo reset: %u",(uint32_t)_pw_fail);

	setenv("TZ", _astdata.clock.cfg.geoloc.timezone, 1);
	tzset() ;
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Establece zona horaria '%s'", _astdata.clock.cfg.geoloc.timezone);
//	std::time_t lt = cpp_utils::timegm(&_now);
//	struct tm *local_field = gmtime(&lt);
//	local_field->tm_isdst = -1;
//	time_t utc = mktime(local_field);
	std::time_t utc = cpp_utils::timegm(&_now);
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC read tm_utc: %d", (int)utc);


	time_t tnow;
	timeval tv;
	tv.tv_sec = utc;
	tv.tv_usec = 0;

	settimeofday (&tv, NULL);
	tnow = time(NULL);
	localtime_r(&tnow, &_now);
	time_t t = time(NULL);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema");
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC:   %s", asctime(gmtime(&t)));
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "local: %s", asctime(localtime(&t)));

}


//------------------------------------------------------------------------------------
bool AstCalendar::checkIntegrity(){
	if(_astdata.cfg.nvs_id != APP_ASTCALENDAR_NVS_ID){
		return false;
	}
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

	// Verifico n�mero de periodos discordante
	if((_astdata.clock.cfg._numPeriods != _astdata.clock.cfg.geoloc._numPeriods) || (_astdata.clock.cfg._numPeriods != CalendarClockCfgMaxNumPeriods) || (_astdata.clock.cfg.geoloc._numPeriods != CalendarClockCfgMaxNumPeriods)){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_INTEGRITY clock_cfg_periods=%d, geoloc_periods=%d", _astdata.clock.cfg._numPeriods, _astdata.clock.cfg.geoloc._numPeriods);
		return false;
	}

	#warning TODO: otras verificaciones m�s exhaustivas
	// ...

	return true;
}


//------------------------------------------------------------------------------------
void AstCalendar::setDefaultConfig(){

	// borro la configuraci�n y el estado
	_astdata = {0};

	// establezco versi�n del modelo de datos
	_astdata.uid = UID_CALENDAR_MANAGER;
	_astdata.clock.uid = UID_CALENDAR_CLOCK;

	// establezco configuraci�n por defecto del manager
	_astdata.cfg.updFlags = CalendarManagerCfgUpdNotif;
	_astdata.cfg.evtFlags = CalendarClockSecEvt;
	_astdata.cfg.verbosity = APP_ASTCALENDAR_LOG_LEVEL;

	// establezco configuraci�n por defecto del reloj integrado (para Madrid)
	strncpy(_astdata.clock.cfg.geoloc.timezone, "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", CalendarGeolocTimezoneLength);
	_astdata.clock.cfg.geoloc.timezoneCode = 1;
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
	_astdata.cfg.nvs_id = APP_ASTCALENDAR_NVS_ID;

	saveConfig();
}



//------------------------------------------------------------------------------------
void AstCalendar::saveConfig(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Guardando datos en memoria NV...");

	if(!saveParameter("CalMUpdFlag", &_astdata.cfg.updFlags, sizeof(uint32_t),NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalMUpdFlag!");
	}
	if(!saveParameter("CalMEvtFlag", &_astdata.cfg.evtFlags, sizeof(uint32_t),NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalMEvtFlag!");
	}
	if(!saveParameter("CalManVerb", &_astdata.cfg.verbosity, sizeof(uint8_t),NVSInterface::TypeUint8)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalManVerb!");
	}
	if(!saveParameter("CalManNvs", &_astdata.cfg.nvs_id, sizeof(uint32_t),NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalManNvs!");
	}
	if(!saveParameter("CalClNumPer", &_astdata.clock.cfg._numPeriods, sizeof(uint8_t),NVSInterface::TypeUint8)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalClNumPer!");
	}
	if(!saveParameter("CalClockPer", &_astdata.clock.cfg.periods, sizeof(calendar_period[CalendarClockCfgMaxNumPeriods]),NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalClockPer!");
	}
	if(!saveParameter("CalClockCoo", &_astdata.clock.cfg.geoloc.coords, sizeof(double[2]),NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalClockCoo!");
	}
	if(!saveParameter("CalClockZon", &_astdata.clock.cfg.geoloc.timezone, SizeOfArray(_astdata.clock.cfg.geoloc.timezone),NVSInterface::TypeString)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalClockZon!");
	}
	if(!saveParameter("CalClockCod", &_astdata.clock.cfg.geoloc.timezoneCode, sizeof(uint8_t),NVSInterface::TypeUint8)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalClockCod!");
	}
	if(!saveParameter("CalClockNum", &_astdata.clock.cfg.geoloc._numPeriods, sizeof(uint8_t),NVSInterface::TypeUint8)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalClockNum!");
	}
	if(!saveParameter("CalClockAst", &_astdata.clock.cfg.geoloc.astCorr, sizeof(time_t[CalendarClockCfgMaxNumPeriods][2]),NVSInterface::TypeBlob)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando CalClockAst!");
	}

	// aplica el nivel de verbosidad configurado
	esp_log_level_set(_MODULE_, (esp_log_level_t)_astdata.cfg.verbosity);
}


//------------------------------------------------------------------------------------
void AstCalendar::_updateConfig(const calendar_manager& data, Blob::ErrorData_t& err){
	err.code = Blob::ErrOK;

	// calendadar_manager.uid
	if((data._keys & (1 << 0)) && data.uid != _astdata.uid){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_UPDATE uid inv�lido");
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
			DEBUG_TRACE_I(_EXPR_, _MODULE_, "Actualizando verbosity=(de %d a %d)", _astdata.cfg.verbosity, data.cfg.verbosity);
			_astdata.cfg.verbosity = data.cfg.verbosity;
			esp_log_level_set(_MODULE_, _astdata.cfg.verbosity);
		}
	}
	// calendar:clock:cfg
	if((data._keys & (1 << 2)) && (data.clock._keys & (1 << 1))){
		// clock.cfg.periods
		if((data.clock.cfg._keys & (1 << 0))){
			DEBUG_TRACE_I(_EXPR_, _MODULE_, "Actualizando clock.cfg.periods");
			for(int i=0;i<data.clock.cfg._numPeriods;i++){
				_astdata.clock.cfg.periods[i] = data.clock.cfg.periods[i];
			}
		}
		// calendar:clock:cfg:geoloc
		if((data.clock.cfg._keys & (1 << 1))){
			// coords
			if((data.clock.cfg.geoloc._keys & (1 << 1))){
				DEBUG_TRACE_I(_EXPR_, _MODULE_, "Actualizando clock.cfg.geoloc.coords = %.03f, %.03f", data.clock.cfg.geoloc.coords[0], data.clock.cfg.geoloc.coords[1]);
				_astdata.clock.cfg.geoloc.coords[0] = data.clock.cfg.geoloc.coords[0];
				_astdata.clock.cfg.geoloc.coords[1] = data.clock.cfg.geoloc.coords[1];
			}
			// timezone
			if((data.clock.cfg.geoloc._keys & (1 << 2))){
				DEBUG_TRACE_I(_EXPR_, _MODULE_, "Actualizando clock.cfg.geoloc.timezone = %s", data.clock.cfg.geoloc.timezone);
				strncpy(_astdata.clock.cfg.geoloc.timezone, data.clock.cfg.geoloc.timezone, CalendarGeolocTimezoneLength);
			}
			// astCorr
			if((data.clock.cfg.geoloc._keys & (1 << 3))){
				DEBUG_TRACE_I(_EXPR_, _MODULE_, "Actualizando clock.cfg.geoloc.astCorr");
				for(int i=0;i<data.clock.cfg.geoloc._numPeriods;i++){
					_astdata.clock.cfg.geoloc.astCorr[i][0] = data.clock.cfg.geoloc.astCorr[i][0];
					_astdata.clock.cfg.geoloc.astCorr[i][1] = data.clock.cfg.geoloc.astCorr[i][1];
				}
			}
			// timezoneCode
			if((data.clock.cfg.geoloc._keys & (1 << 4))){
				DEBUG_TRACE_I(_EXPR_, _MODULE_, "Actualizando clock.cfg.geoloc.timezoneCode = %d", data.clock.cfg.geoloc.timezoneCode);
				_astdata.clock.cfg.geoloc.timezoneCode = data.clock.cfg.geoloc.timezoneCode;
			}
		}
	}

_updateConfigExit:
	strcpy(err.descr, Blob::errList[err.code]);
}


