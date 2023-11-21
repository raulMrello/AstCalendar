/*
 * ppl_energy_objects.cpp
 *
 * Implementaci�n de los codecs JSON-OBJ
 *
 *  Created on: Feb 2019
 *      Author: raulMrello
 */

#include "JsonParserBlob.h"

//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------

static const char* _MODULE_ = "[calendar:].....";
#define _EXPR_	(!IS_ISR())

char* calendarClockStatFlags2String(calendar_clock_stat_flags flags){
	switch(flags){
		case CalendarClockNoEvents:			return (char*)"CalendarClockNoEvents";
		case CalendarClockYearEvt:			return (char*)"CalendarClockYearEvt";
		case CalendarClockIVEvt:			return (char*)"CalendarClockIVEvt";
		case CalendarClockVIEvt:			return (char*)"CalendarClockVIEvt";
		case CalendarClockMonthEvt:			return (char*)"CalendarClockMonthEvt";
		case CalendarClockWeekEvt:			return (char*)"CalendarClockWeekEvt";
		case CalendarClockDayEvt:			return (char*)"CalendarClockDayEvt";
		case CalendarClockMiddayEvt:		return (char*)"CalendarClockMiddayEvt";
		case CalendarClockPreDuskEvt:		return (char*)"CalendarClockPreDuskEvt";
		case CalendarClockDuskEvt:			return (char*)"CalendarClockDuskEvt";
		case CalendarClockPostDuskEvt:		return (char*)"CalendarClockPostDuskEvt";
		case CalendarClockReducStartEvt:	return (char*)"CalendarClockReducStartEvt";
		case CalendarClockReducStopEvt:		return (char*)"CalendarClockReducStopEvt";
		case CalendarClockPreDawnEvt:		return (char*)"CalendarClockPreDawnEvt";
		case CalendarClockDawnEvt:			return (char*)"CalendarClockDawnEvt";
		case CalendarClockPostDawnEvt:		return (char*)"CalendarClockPostDawnEvt";
		case CalendarClockHourEvt:			return (char*)"CalendarClockHourEvt";
		case CalendarClockMinEvt:			return (char*)"CalendarClockMinEvt";
		case CalendarClockSecEvt:			return (char*)"CalendarClockSecEvt";
		case CalendarClockDawnDuskUpdEvt:	return (char*)"CalendarClockDawnDuskUpdEvt";
		case CalendarClockPeriodEvt:		return (char*)"CalendarClockPeriodEvt";
		case CalendarClockNTPEvt:			return (char*)"CalendarClockNTPEvt";
		case CalendarClockEvtINVALID:		return (char*)"CalendarClockEvtINVALID";
		default:							return (char*)"CalendarClockEvtUNKNOWN";
	}
}

COORD_T initializeCoord(double coord){
	COORD_T coord_t;
	coord_t.Grados = (int16_t)coord;
	coord_t.Minutos = abs((int8_t)((coord - coord_t.Grados)*60));
	coord_t.Segundos = abs((int8_t)((((coord - coord_t.Grados)*60) - coord_t.Minutos)*60));
	coord_t.Signo = (coord < 0)?-1:1;
	return coord_t;
}

CALENDAR_T initializeCalendar(tm _tm){
	CALENDAR_T cal;
	cal.hour = _tm.tm_hour;
	cal.minute = _tm.tm_min;
	cal.second = _tm.tm_sec;
	cal.weekday = _tm.tm_wday;
	cal.month = _tm.tm_mon + 1;
	cal.date = _tm.tm_mday;
	cal.year = _tm.tm_year - 100;
	cal._NAN = 0;
	return cal;
}

namespace JSON{


//------------------------------------------------------------------------------------
cJSON* getJsonFromCalendarManager(const calendar_manager& obj, ObjDataSelection type){
	cJSON* json = NULL;
	cJSON* item = NULL;
	if((json=cJSON_CreateObject()) == NULL){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarManager cJSON_CreateObject json = null");
		return NULL;
	}

	// uid
	cJSON_AddNumberToObject(json, JsonParser::p_uid, obj.uid);

	// cfg
	if(type != ObjSelectState){
		cJSON* cfg = NULL;
		if((cfg=cJSON_CreateObject()) == NULL){
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarManager cJSON_CreateObject cfg = null");
			return NULL;
		}
		cJSON_AddNumberToObject(cfg, JsonParser::p_updFlags, obj.cfg.updFlags);
		cJSON_AddNumberToObject(cfg, JsonParser::p_evtFlags, obj.cfg.evtFlags);
		cJSON_AddNumberToObject(cfg, JsonParser::p_verbosity, obj.cfg.verbosity);
		cJSON_AddItemToObject(json, JsonParser::p_cfg, cfg);
	}

	// clock
	if((item = getJsonFromCalendarClock(obj.clock, type)) == NULL){
		cJSON_Delete(json);
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarManager getJsonFromCalendarClock");
		return NULL;
	}
	cJSON_AddItemToObject(json, JsonParser::p_clock, item);
	return json;
}


//------------------------------------------------------------------------------------
cJSON* getJsonFromCalendarClock(const calendar_clock& obj, ObjDataSelection type){
	cJSON* json = NULL;
	cJSON* item = NULL;
	if((json=cJSON_CreateObject()) == NULL){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarClock cJSON_CreateObject json = null");
		return NULL;
	}

	// uid
	cJSON_AddNumberToObject(json, JsonParser::p_uid, obj.uid);

	// cfg
	if(type != ObjSelectState){
		if((item = getJsonFromCalendarClockCfg(obj.cfg)) == NULL){
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarClock getJsonFromCalendarClockCfg");
			return NULL;
		}
		cJSON_AddItemToObject(json, JsonParser::p_cfg, item);
	}

	// stat
	if(type != ObjSelectCfg){
		cJSON* stat = NULL;
		if((stat=cJSON_CreateObject()) == NULL){
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarClock cJSON_CreateObject stat = null");
			return NULL;
		}
		cJSON_AddNumberToObject(stat, JsonParser::p_flags, obj.stat.flags);
		// s�lo incluye <period> si tiene un valor v�lido >=0
		if(obj.stat.period >= 0){
			cJSON_AddNumberToObject(stat, JsonParser::p_period, obj.stat.period);
		}
		cJSON_AddNumberToObject(stat, JsonParser::p_localtime, obj.stat.localtime);
		cJSON_AddNumberToObject(stat, JsonParser::p_dawn, obj.stat.dawn);
		cJSON_AddNumberToObject(stat, JsonParser::p_dusk, obj.stat.dusk);
		cJSON_AddItemToObject(json, JsonParser::p_stat, stat);
	}
	return json;
}


//------------------------------------------------------------------------------------
cJSON* getJsonFromCalendarClockCfg(const calendar_clock_cfg& obj){
	cJSON* json = NULL;
	cJSON* item = NULL;
	if((json=cJSON_CreateObject()) == NULL){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarClockCfg cJSON_CreateObject json = null");
		return NULL;
	}

	// cfg.periods
	cJSON* array = NULL;
	if((array=cJSON_CreateArray()) == NULL){
		cJSON_Delete(json);
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_JSON clock_cfg.periods[] = null");
		return NULL;
	}
	for(int i=0; i<obj._numPeriods; i++){
		if((item = getJsonFromCalendarPeriod(obj.periods[i])) == NULL){
			cJSON_Delete(array);
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_JSON clock_cfg.periods[%d] = null", i);
			return NULL;
		}
		cJSON_AddItemToArray(array, item);
	}
	cJSON_AddItemToObject(json, JsonParser::p_periods, array);

	// cfg.geoloc
	if((item = getJsonFromCalendarGeoloc(obj.geoloc)) == NULL){
		cJSON_Delete(json);
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_JSON clock_cfg.geoloc = null");
		return NULL;
	}
	cJSON_AddItemToObject(json, JsonParser::p_geoloc, item);

	return json;
}


//------------------------------------------------------------------------------------
cJSON* getJsonFromCalendarPeriod(const calendar_period& obj){
	cJSON* json = NULL;
	if((json=cJSON_CreateObject()) == NULL){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarPeriod cJSON_CreateObject json = null");
		return NULL;
	}

	// since
	cJSON_AddNumberToObject(json, JsonParser::p_since, obj.since);
	// until
	cJSON_AddNumberToObject(json, JsonParser::p_until, obj.until);
	// enabled
	cJSON_AddNumberToObject(json, JsonParser::p_enabled, obj.enabled);

	return json;
}


//------------------------------------------------------------------------------------
cJSON* getJsonFromCalendarGeoloc(const calendar_geoloc& obj){
	cJSON* json = NULL;
	if((json=cJSON_CreateObject()) == NULL){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarGeoloc cJSON_CreateObject json = null");
		return NULL;
	}

	// timezone
	cJSON_AddStringToObject(json, JsonParser::p_timezone, obj.timezone);

	// coords
	cJSON* array = NULL;
	if((array=cJSON_CreateArray()) == NULL){
		cJSON_Delete(json);
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarGeoloc cJSON_CreateObject array coords = null");
		return NULL;
	}
	for(int i=0;i<2;i++){
		cJSON* item = NULL;
		if((item = cJSON_CreateNumber(obj.coords[i])) == NULL){
			cJSON_Delete(array);
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarGeoloc cJSON_CreateObject item = null");
			return NULL;
		}
		cJSON_AddItemToArray(array, item);
	}
	cJSON_AddItemToObject(json, JsonParser::p_coords, array);

	// astCorr
	if((array=cJSON_CreateArray()) == NULL){
		cJSON_Delete(json);
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarGeoloc cJSON_CreateObject array astCorr = null");
		return NULL;
	}
	for(int i=0;i<CalendarClockCfgMaxNumPeriods;i++){
		cJSON* array2 = NULL;
		if((array2=cJSON_CreateArray()) == NULL){
			cJSON_Delete(array);
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarGeoloc cJSON_CreateObject array periods = null");
			return NULL;
		}
		for(int j=0;j<2;j++){
			cJSON* item = NULL;
			if((item = cJSON_CreateNumber(obj.astCorr[i][j])) == NULL){
				cJSON_Delete(array2);
				cJSON_Delete(array);
				cJSON_Delete(json);
				DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR getJsonFromCalendarGeoloc cJSON_CreateObject item periods = null");
				return NULL;
			}
			cJSON_AddItemToArray(array2, item);
		}
		cJSON_AddItemToArray(array, array2);
	}
	cJSON_AddItemToObject(json, JsonParser::p_astCorr, array);
	
	// timezoneCode
	cJSON_AddNumberToObject(json, JsonParser::p_timezoneCode, obj.timezoneCode);
	return json;
}


//------------------------------------------------------------------------------------
uint32_t getCalendarManagerFromJson(calendar_manager &obj, cJSON* json){
	obj._keys = 0;
	cJSON* value = NULL;
	if(json == NULL){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR calendar_manager json is Null");
		return 0;
	}

	// uid
	if((value = cJSON_GetObjectItem(json,JsonParser::p_uid)) != NULL){
		obj.uid = value->valueint;
		obj._keys |= (1 << 0);
	}

	// cfg
	obj.cfg._keys = 0;
	if((value = cJSON_GetObjectItem(json, JsonParser::p_cfg)) != NULL){
		if((value = cJSON_GetObjectItem(json,JsonParser::p_updFlags)) != NULL){
			obj.cfg.updFlags = value->valueint;
			obj.cfg._keys |= (1 << 0);
		}
		if((value = cJSON_GetObjectItem(json,JsonParser::p_evtFlags)) != NULL){
			obj.cfg.evtFlags = value->valueint;
			obj.cfg._keys |= (1 << 1);
		}
		if((value = cJSON_GetObjectItem(json,JsonParser::p_verbosity)) != NULL){
			obj.cfg.verbosity = value->valueint;
			obj.cfg._keys |= (1 << 2);
		}
		if(obj.cfg._keys){
			obj._keys |= (1 << 1);
		}
	}

	// clock
	if((value = cJSON_GetObjectItem(json,JsonParser::p_clock)) != NULL){
		uint32_t subkey = getCalendarClockFromJson(obj.clock, value)? (1 << 2) : 0;
		obj._keys |= subkey;
	}
	return obj._keys;
}


//------------------------------------------------------------------------------------
uint32_t getCalendarClockFromJson(calendar_clock &obj, cJSON* json){
	uint32_t subkey = 0;
	cJSON* value = NULL;
	obj._keys = 0;
	if(json == NULL){
		return 0;
	}

	// uid
	if((value = cJSON_GetObjectItem(json,JsonParser::p_uid)) != NULL){
		obj.uid = value->valueint;
		obj._keys |= (1 << 0);
	}

	// cfg
	if((value = cJSON_GetObjectItem(json, JsonParser::p_cfg)) != NULL){
		subkey = getCalendarClockCfgFromJson(obj.cfg, value)? (1 << 1) : 0;
		obj._keys |= subkey;
	}

	// stat
	subkey = 0;
	if((value = cJSON_GetObjectItem(json, JsonParser::p_stat)) != NULL){
		if((value = cJSON_GetObjectItem(json,JsonParser::p_flags)) != NULL){
			obj.stat.flags = value->valueint;
		}
		// si el periodo no est� presente, lo marca como inv�lido (=-1)
		if((value = cJSON_GetObjectItem(json,JsonParser::p_period)) != NULL){
			obj.stat.period = value->valueint;
		}
		else{
			obj.stat.period = -1;
		}
		if((value = cJSON_GetObjectItem(json,JsonParser::p_localtime)) != NULL){
			obj.stat.localtime = (time_t)value->valuedouble;
		}
		if((value = cJSON_GetObjectItem(json,JsonParser::p_dawn)) != NULL){
			obj.stat.dawn = (time_t)value->valuedouble;
		}
		if((value = cJSON_GetObjectItem(json,JsonParser::p_dusk)) != NULL){
			obj.stat.dusk = (time_t)value->valuedouble;
		}
		subkey = (1 << 2);
	}

	return (obj._keys | subkey);
}


//------------------------------------------------------------------------------------
uint32_t getCalendarClockCfgFromJson(calendar_clock_cfg &obj, cJSON* json){
	uint32_t keys = 0;
	uint32_t subkey = 0;
	cJSON* value = NULL;
	cJSON* array = NULL;
	obj._keys = 0;

	if(json == NULL){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR json calendar.clock.cfg is Null");
		return obj._keys;
	}

	// cfg.periods
	if((array = cJSON_GetObjectItem(json, JsonParser::p_periods)) != NULL){
		if(cJSON_GetArraySize(array) <= CalendarClockCfgMaxNumPeriods){
			obj._numPeriods = cJSON_GetArraySize(array);
			subkey = obj._numPeriods;
			for(int i=0;i<cJSON_GetArraySize(array);i++){
				cJSON* item = cJSON_GetArrayItem(array, i);
				subkey = getCalendarPeriodFromJson(obj.periods[i], item)? (subkey-1) : subkey;
			}
			if(subkey == 0){
				obj._keys |= (1 << 0);
			}
		}
	}

	// cfg.geoloc
	if((value = cJSON_GetObjectItem(json,JsonParser::p_geoloc)) != NULL){
		subkey = getCalendarGeolocFromJson(obj.geoloc, value)? (1 << 1) : 0;
		obj._keys |= subkey;
	}

	return obj._keys;
}


//------------------------------------------------------------------------------------
uint32_t getCalendarPeriodFromJson(calendar_period &obj, cJSON* json){
	uint32_t keys = 0;
	cJSON* value = NULL;
	if(json == NULL){
		return 0;
	}

	// since
	if((value = cJSON_GetObjectItem(json,JsonParser::p_since)) != NULL){
		obj.since = (time_t)value->valuedouble;
		keys |= (1 << 1);
	}

	// until
	if((value = cJSON_GetObjectItem(json,JsonParser::p_until)) != NULL){
		obj.until = (time_t)value->valuedouble;
		keys |= (1 << 2);
	}

	// enabled
	if((value = cJSON_GetObjectItem(json,JsonParser::p_enabled)) != NULL){
		obj.enabled = (bool)value->valueint;
		keys |= (1 << 3);
	}
	return keys;
}


//------------------------------------------------------------------------------------
uint32_t getCalendarGeolocFromJson(calendar_geoloc &obj, cJSON* json){
	uint32_t subkey = 0;
	cJSON* value = NULL;
	obj._keys = 0;

	if(json == NULL){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR json calendar.geoloc is Null");
		return 0;
	}

	// coords
	cJSON* array = NULL;
	if((array = cJSON_GetObjectItem(json, JsonParser::p_coords)) != NULL){
		if(cJSON_GetArraySize(array) <= 2){
			subkey = cJSON_GetArraySize(array);
			for(int i=0;i<cJSON_GetArraySize(array);i++){
				if((value = cJSON_GetArrayItem(array, i)) != NULL){
					obj.coords[i] = value->valuedouble;
					subkey--;
				}
			}
			if(subkey == 0){
				obj._keys |= (1 << 1);
			}
		}
	}

	// timezone
	if((value = cJSON_GetObjectItem(json,JsonParser::p_timezone)) != NULL){
		strncpy(obj.timezone, value->valuestring, CalendarGeolocTimezoneLength);
		obj._keys |= (1 << 2);
	}

	// astCorr
	if((array = cJSON_GetObjectItem(json, JsonParser::p_astCorr)) != NULL){
		if(cJSON_GetArraySize(array) <= CalendarClockCfgMaxNumPeriods){
			obj._numPeriods = cJSON_GetArraySize(array);
			subkey = obj._numPeriods;
			for(int i=0;i<cJSON_GetArraySize(array);i++){
				cJSON* array2 = NULL;
				if((array2 = cJSON_GetArrayItem(array, i)) != NULL){
					if(cJSON_GetArraySize(array2) <= 2){
						uint32_t subsubkey = cJSON_GetArraySize(array2);
						for(int j=0;j<cJSON_GetArraySize(array2);j++){
							if((value = cJSON_GetArrayItem(array2, j)) != NULL){
								obj.astCorr[i][j] = (time_t)value->valuedouble;
								subsubkey--;
							}
						}
						if(subsubkey == 0){
							subkey--;
						}
					}
				}
			}
			if(subkey == 0){
				obj._keys |= (1 << 3);
			}
		}
	}
	// timezoneCode
	if((value = cJSON_GetObjectItem(json,JsonParser::p_timezoneCode)) != NULL){
		obj.timezoneCode = value->valueint;
		obj._keys |= (1 << 4);
	}
	return obj._keys;
}



}	// end namespace JSON

