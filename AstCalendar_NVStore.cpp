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
	// verifico periodos activos sin rangos establecidos
	for(int i=0;i<Blob::AstCalMaxPeriodCount; i++){
		if((_astdata.cfg.periods[i].since == 0 || _astdata.cfg.periods[i].until == 0) && _astdata.cfg.periods[i].enabled){
			return false;
		}
	}
	return true;
}


//------------------------------------------------------------------------------------
void AstCalendar::setDefaultConfig(){
	// inicializa notificación de flags
	_astdata.cfg.updFlagMask = (Blob::AstCalUpdFlags)(Blob::EnableAstCalCfgUpdNotif);
	_astdata.cfg.evtFlagMask = (Blob::AstCalEvtFlags)(Blob::AstCalIVEvt | Blob::AstCalVIEvt | Blob::AstCalDayEvt | Blob::AstCalDawnEvt | Blob::AstCalDuskEvt | Blob::AstCalHourEvt | Blob::AstCalMinEvt | Blob::AstCalSecEvt);
	// desactiva toda la configuración astronómica y establece Madrid como localización por defecto
	_astdata.cfg.astCfg = {40.416500, -3.702560, 0, 0, 0, 0, 0, 0};
	// establece cambio horario por defecto de la zona UE
	strcpy(_astdata.cfg.seasonCfg.envText, "GMT-1GMT-2,M3.5.0/2,M10.5.0");
	// desactiva todos los periodos
	for(int i=0;i<Blob::AstCalMaxPeriodCount; i++){
		_astdata.cfg.periods[i].since = 0;
		_astdata.cfg.periods[i].until = 0;
		_astdata.cfg.periods[i].enabled = false;
	}
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
	for(int i=0;i<Blob::AstCalMaxPeriodCount; i++){
		char periodname[16];
		sprintf(periodname, "AstCalPeriod_%d", i);
		if(!restoreParameter(periodname, &_astdata.cfg.periods[i], sizeof(Blob::AstCalPeriod_t), NVSInterface::TypeBlob)){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS leyendo Period_%d!", i);
		}
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
	for(int i=0;i<Blob::AstCalMaxPeriodCount; i++){
		char periodname[16];
		sprintf(periodname, "AstCalPeriod_%d", i);
		if(!saveParameter(periodname, &_astdata.cfg.periods[i], sizeof(Blob::AstCalPeriod_t), NVSInterface::TypeBlob)){
			DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando Period_%d!", i);
		}
	}
	uint32_t crc = Blob::getCRC32(&_astdata.cfg, sizeof(Blob::AstCalCfgData_t));
	if(!saveParameter("AstCalChecksum", &crc, sizeof(uint32_t), NVSInterface::TypeUint32)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS grabando Checksum!");
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
	if(keys & Blob::AstCalKeyCfgPeriods){
		for(int i=0;i<Blob::AstCalMaxPeriodCount; i++){
			_astdata.cfg.periods[i] = cfg.periods[i];
		}
	}

_updateConfigExit:
	strcpy(err.descr, Blob::errList[err.code]);
}


