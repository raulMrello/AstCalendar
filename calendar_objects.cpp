/*
 * ppl_energy_objects.cpp
 *
 * Implementación de los codecs JSON-OBJ
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


namespace JSON{


//------------------------------------------------------------------------------------
cJSON* getJsonFromCalendarManager(const calendar_manager& obj, ObjDataSelection type){
	cJSON* json = NULL;
	cJSON* item = NULL;
	if((json=cJSON_CreateObject()) == NULL){
		return NULL;
	}

	// uid
	cJSON_AddNumberToObject(json, JsonParser::p_uid, obj.uid);

	// cfg
	if(type != ObjSelectState){
		cJSON* cfg = NULL;
		if((cfg=cJSON_CreateObject()) == NULL){
			cJSON_Delete(json);
			return NULL;
		}
		cJSON_AddNumberToObject(cfg, JsonParser::p_updFlags, obj.cfg.updFlags);
		cJSON_AddNumberToObject(cfg, JsonParser::p_evtFlags, obj.cfg.evtFlags);
		cJSON_AddNumberToObject(cfg, JsonParser::p_verbosity, obj.cfg.verbosity);
		cJSON_AddItemToObject(json, JsonParser::p_cfg, cfg);
	}

	// stat
	/*
	if(type != ObjSelectCfg){
		cJSON* stat = NULL;
		if((stat=cJSON_CreateObject()) == NULL){
			cJSON_Delete(json);
			return NULL;
		}
		// TODO: Añadir contenido si es necesario
		cJSON_AddItemToObject(json, JsonParser::p_stat, stat);
	}
	*/

	// clock
	if((item = getJsonFromCalendarClock(obj.clock, type)) == NULL){
		cJSON_Delete(json);
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
		return NULL;
	}

	// uid
	cJSON_AddNumberToObject(json, JsonParser::p_uid, obj.uid);

	// cfg
	if(type != ObjSelectState){
		cJSON* cfg = NULL;
		if((cfg=cJSON_CreateObject()) == NULL){
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_JSON clock.cfg = null");
			return NULL;
		}

		// cfg.periods
		cJSON* array = NULL;
		if((array=cJSON_CreateArray()) == NULL){
			cJSON_Delete(cfg);
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_JSON clock.cfg.periods[] = null");
			return NULL;
		}
		for(int i=0; i<CalendarClockCfgMaxNumPeriods; i++){
			if((item = getJsonFromCalendarPeriod(obj.cfg.periods[i])) == NULL){
				cJSON_Delete(array);
				cJSON_Delete(cfg);
				cJSON_Delete(json);
				DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_JSON clock.cfg.periods[%d] = null", i);
				return NULL;
			}
			cJSON_AddItemToArray(array, item);
		}
		cJSON_AddItemToObject(cfg, JsonParser::p_periods, array);

		// cfg.geoloc
		if((item = getJsonFromCalendarGeoloc(obj.cfg.geoloc)) == NULL){
			cJSON_Delete(cfg);
			cJSON_Delete(json);
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_JSON clock.cfg.geoloc = null");
			return NULL;
		}
		cJSON_AddItemToObject(cfg, JsonParser::p_geoloc, item);
		cJSON_AddItemToObject(json, JsonParser::p_cfg, cfg);
	}

	// stat
	if(type != ObjSelectCfg){
		cJSON* stat = NULL;
		if((stat=cJSON_CreateObject()) == NULL){
			cJSON_Delete(json);
			return NULL;
		}
		cJSON_AddNumberToObject(stat, JsonParser::p_flags, obj.stat.flags);
		cJSON_AddNumberToObject(stat, JsonParser::p_period, obj.stat.period);
		cJSON_AddNumberToObject(stat, JsonParser::p_localtime, obj.stat.localtime);
		cJSON_AddNumberToObject(stat, JsonParser::p_dawn, obj.stat.dawn);
		cJSON_AddNumberToObject(stat, JsonParser::p_dusk, obj.stat.dusk);
		cJSON_AddItemToObject(json, JsonParser::p_stat, stat);
	}
	return json;
}


//------------------------------------------------------------------------------------
cJSON* getJsonFromCalendarPeriod(const calendar_period& obj){
	cJSON* json = NULL;
	if((json=cJSON_CreateObject()) == NULL){
		return NULL;
	}

	// uid
	cJSON_AddNumberToObject(json, JsonParser::p_uid, obj.uid);
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
		return NULL;
	}

	// uid
	cJSON_AddNumberToObject(json, JsonParser::p_uid, obj.uid);

	// timezone
	cJSON_AddStringToObject(json, JsonParser::p_timezone, obj.timezone);

	// coords
	cJSON* array = NULL;
	if((array=cJSON_CreateArray()) == NULL){
		cJSON_Delete(json);
		return NULL;
	}
	for(int i=0;i<2;i++){
		cJSON* item = NULL;
		if((item = cJSON_CreateNumber(obj.coords[i])) == NULL){
			cJSON_Delete(array);
			cJSON_Delete(json);
			return NULL;
		}
		cJSON_AddItemToArray(array, item);
	}
	cJSON_AddItemToObject(json, JsonParser::p_coords, array);

	// astCorr
	if((array=cJSON_CreateArray()) == NULL){
		cJSON_Delete(json);
		return NULL;
	}
	for(int i=0;i<CalendarClockCfgMaxNumPeriods;i++){
		cJSON* array2 = NULL;
		if((array2=cJSON_CreateArray()) == NULL){
			cJSON_Delete(array);
			cJSON_Delete(json);
			return NULL;
		}
		for(int j=0;j<2;j++){
			cJSON* item = NULL;
			if((item = cJSON_CreateNumber(obj.astCorr[i][j])) == NULL){
				cJSON_Delete(array2);
				cJSON_Delete(array);
				cJSON_Delete(json);
				return NULL;
			}
			cJSON_AddItemToArray(array2, item);
		}
		cJSON_AddItemToArray(array, array2);
	}
	cJSON_AddItemToObject(json, JsonParser::p_astCorr, array);
	return json;
}


//------------------------------------------------------------------------------------
uint32_t getCalendarManagerFromJson(calendar_manager &obj, cJSON* json){
	uint32_t keys = 0;
	uint32_t subkey = 0;
	cJSON* value = NULL;
	if(json == NULL){
		return 0;
	}

	// uid
	if((value = cJSON_GetObjectItem(json,JsonParser::p_uid)) != NULL){
		obj.uid = value->valueint;
		keys |= (1 << 0);
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
		subkey = (obj.cfg._keys!=0)? (1 << 1) : 0;
		keys |= subkey;
	}

	// stat
	/*
	if((value = cJSON_GetObjectItem(json, JsonParser::p_stat)) != NULL){
		subkey = 0;
		//TODO: añadir objetos a procesar
		subkey = (subkey!=0)? (1 << ??) : 0;
		keys |= subkey;
	}
	*/

	// clock
	if((value = cJSON_GetObjectItem(json,JsonParser::p_clock)) != NULL){
		subkey = getCalendarClockFromJson(obj.clock, value)? (1 << 2) : 0;
		keys |= subkey;
	}

	return keys;
}


//------------------------------------------------------------------------------------
uint32_t getCalendarClockFromJson(calendar_clock &obj, cJSON* json){
	uint32_t keys = 0;
	uint32_t subkey = 0;
	cJSON* value = NULL;
	if(json == NULL){
		return 0;
	}

	// uid
	if((value = cJSON_GetObjectItem(json,JsonParser::p_uid)) != NULL){
		obj.uid = value->valueint;
		keys |= (1 << 0);
	}

	// cfg
	obj.cfg._keys = 0;
	if((value = cJSON_GetObjectItem(json, JsonParser::p_cfg)) != NULL){
		cJSON* array = NULL;

		// cfg.periods
		if((array = cJSON_GetObjectItem(value, JsonParser::p_periods)) != NULL){
			if(cJSON_GetArraySize(array) <= CalendarClockCfgMaxNumPeriods){
				subkey = cJSON_GetArraySize(array);
				for(int i=0;i<cJSON_GetArraySize(array);i++){
					cJSON* item = cJSON_GetArrayItem(array, i);
					subkey = getCalendarPeriodFromJson(obj.cfg.periods[i], item)? (subkey-1) : subkey;
				}
				if(subkey == 0){
					obj.cfg._keys |= (1 << 0);
				}
			}
		}

		// cfg.geoloc
		if((value = cJSON_GetObjectItem(json,JsonParser::p_geoloc)) != NULL){
			obj.cfg._keys = getCalendarGeolocFromJson(obj.cfg.geoloc, value)? (1 << 1) : 0;
		}
		subkey = (obj.cfg._keys!=0)? (1 << 1) : 0;
		keys |= subkey;
	}

	// stat
	if((value = cJSON_GetObjectItem(json, JsonParser::p_stat)) != NULL){
		if((value = cJSON_GetObjectItem(json,JsonParser::p_flags)) != NULL){
			obj.stat.flags = value->valueint;
		}
		if((value = cJSON_GetObjectItem(json,JsonParser::p_period)) != NULL){
			obj.stat.period = value->valueint;
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
		keys |= (1 << 2);
	}

	return keys;
}


//------------------------------------------------------------------------------------
uint32_t getCalendarPeriodFromJson(calendar_period &obj, cJSON* json){
	uint32_t keys = 0;
	cJSON* value = NULL;
	if(json == NULL){
		return 0;
	}

	// uid
	if((value = cJSON_GetObjectItem(json,JsonParser::p_uid)) != NULL){
		obj.uid = value->valueint;
		keys |= (1 << 0);
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
	uint32_t keys = 0;
	uint32_t subkey = 0;
	cJSON* value = NULL;
	if(json == NULL){
		return 0;
	}

	// uid
	if((value = cJSON_GetObjectItem(json,JsonParser::p_uid)) != NULL){
		obj.uid = value->valueint;
		keys |= (1 << 0);
	}

	// timezone
	if((value = cJSON_GetObjectItem(json,JsonParser::p_timezone)) != NULL){
		strncpy(obj.timezone, value->valuestring, CalendarGeolocTimezoneLength);
		keys |= (1 << 1);
	}

	// coords
	cJSON* array = NULL;
	if((array = cJSON_GetObjectItem(value, JsonParser::p_coords)) != NULL){
		if(cJSON_GetArraySize(array) <= 2){
			subkey = cJSON_GetArraySize(array);
			for(int i=0;i<cJSON_GetArraySize(array);i++){
				if((value = cJSON_GetArrayItem(array, i)) != NULL){
					obj.coords[i] = value->valuedouble;
					subkey--;
				}
			}
			if(subkey == 0){
				keys |= (1 << 2);
			}
		}
	}

	// astCorr
	if((array = cJSON_GetObjectItem(value, JsonParser::p_astCorr)) != NULL){
		if(cJSON_GetArraySize(array) <= CalendarClockCfgMaxNumPeriods){
			subkey = cJSON_GetArraySize(array);
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
				keys |= (1 << 3);
			}
		}
	}

	return keys;
}



}	// end namespace JSON

