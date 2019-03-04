/*
 * AstCalendar_JSONParser.cpp
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


static const char* _MODULE_ = "[AstCal]........";
#define _EXPR_	(!IS_ISR())


namespace JSON{

//------------------------------------------------------------------------------------
cJSON* getJsonFromAstCalCfg(const Blob::AstCalCfgData_t& cfg){
	cJSON* astcal = NULL;
	cJSON* value = NULL;
	cJSON* array = NULL;
	cJSON* item = NULL;

	if((astcal=cJSON_CreateObject()) == NULL){
		return NULL;
	}
	// key: astcal.updFlags
	cJSON_AddNumberToObject(astcal, JsonParser::p_updFlags, cfg.updFlagMask);

	// key: astcal.evtFlags
	cJSON_AddNumberToObject(astcal, JsonParser::p_evtFlags, cfg.evtFlagMask);
	// key: astCfg
	if((value=cJSON_CreateObject()) == NULL){
		cJSON_Delete(astcal);
		return NULL;
	}
	cJSON_AddNumberToObject(value, JsonParser::p_latitude, cfg.astCfg.latitude);
	cJSON_AddNumberToObject(value, JsonParser::p_longitude, cfg.astCfg.longitude);
	cJSON_AddNumberToObject(value, JsonParser::p_wdowDawnStart, cfg.astCfg.wdowDawnStart);
	cJSON_AddNumberToObject(value, JsonParser::p_wdowDawnStop, cfg.astCfg.wdowDawnStop);
	cJSON_AddNumberToObject(value, JsonParser::p_wdowDuskStart, cfg.astCfg.wdowDuskStart);
	cJSON_AddNumberToObject(value, JsonParser::p_wdowDuskStop, cfg.astCfg.wdowDuskStop);
	cJSON_AddNumberToObject(value, JsonParser::p_reductionStart, cfg.astCfg.reductionStart);
	cJSON_AddNumberToObject(value, JsonParser::p_reductionStop, cfg.astCfg.reductionStop);
	cJSON_AddItemToObject(astcal, JsonParser::p_ast, value);
	// key: seasonCfg
	if((value=cJSON_CreateString(cfg.seasonCfg.envText)) == NULL){
		cJSON_Delete(astcal);
		return NULL;
	}
	cJSON_AddItemToObject(astcal, JsonParser::p_seasonCfg, value);

	// key: periods
	if((array=cJSON_CreateArray()) == NULL){
		cJSON_Delete(astcal);
		return NULL;
	}
	for(int i=0; i < Blob::AstCalMaxPeriodCount; i++){
		if((item=cJSON_CreateObject()) == NULL){
			cJSON_Delete(array);
			cJSON_Delete(astcal);
			return NULL;
		}
		cJSON_AddNumberToObject(item, JsonParser::p_since, cfg.periods[i].since);
		cJSON_AddNumberToObject(item, JsonParser::p_until, cfg.periods[i].until);
		cJSON_AddBoolToObject(item, JsonParser::p_enabled, cfg.periods[i].enabled);
		cJSON_AddItemToArray(array, item);
	}
	cJSON_AddItemToObject(astcal, JsonParser::p_periods, array);
	// key: verbosity
	cJSON_AddNumberToObject(astcal, JsonParser::p_verbosity, cfg.verbosity);

	return astcal;
}


//------------------------------------------------------------------------------------
cJSON* getJsonFromAstCalStat(const Blob::AstCalStatData_t& stat){
	cJSON* astcal = NULL;

	if((astcal=cJSON_CreateObject()) == NULL){
		return NULL;
	}

	cJSON_AddNumberToObject(astcal, JsonParser::p_flags, stat.flags);
	cJSON_AddNumberToObject(astcal, JsonParser::p_period, stat.period);
	cJSON_AddNumberToObject(astcal, JsonParser::p_now, stat.now);
	cJSON* astData = NULL;
	if((astData=cJSON_CreateObject()) == NULL){
		cJSON_Delete(astcal);
		return NULL;
	}
	cJSON_AddNumberToObject(astData, JsonParser::p_latitude, stat.astData.latitude);
	cJSON_AddNumberToObject(astData, JsonParser::p_longitude, stat.astData.longitude);
	cJSON_AddNumberToObject(astData, JsonParser::p_wdowDawnStart, stat.astData.wdowDawnStart);
	cJSON_AddNumberToObject(astData, JsonParser::p_wdowDawnStop, stat.astData.wdowDawnStop);
	cJSON_AddNumberToObject(astData, JsonParser::p_wdowDuskStart, stat.astData.wdowDuskStart);
	cJSON_AddNumberToObject(astData, JsonParser::p_wdowDuskStop, stat.astData.wdowDuskStop);
	cJSON_AddNumberToObject(astData, JsonParser::p_reductionStart, stat.astData.reductionStart);
	cJSON_AddNumberToObject(astData, JsonParser::p_reductionStop, stat.astData.reductionStop);
	cJSON_AddItemToObject(astcal, JsonParser::p_ast, astData);
	return astcal;
}


//------------------------------------------------------------------------------------
cJSON* getJsonFromAstCalBoot(const Blob::AstCalBootData_t& boot){
	cJSON* astcal = NULL;
	cJSON* item = NULL;
	if((astcal=cJSON_CreateObject()) == NULL){
		return NULL;
	}

	if((item = getJsonFromAstCalCfg(boot.cfg)) == NULL){
		goto __encodeBoot_Err;
	}
	cJSON_AddItemToObject(astcal, JsonParser::p_cfg, item);

	if((item = getJsonFromAstCalStat(boot.stat)) == NULL){
		goto __encodeBoot_Err;
	}
	cJSON_AddItemToObject(astcal, JsonParser::p_stat, item);
	return astcal;

__encodeBoot_Err:
	cJSON_Delete(astcal);
	return NULL;
}

//------------------------------------------------------------------------------------
uint32_t getAstCalCfgFromJson(Blob::AstCalCfgData_t &cfg, cJSON* json){
	cJSON* astcfg = NULL;
	cJSON* obj = NULL;
	cJSON* array = NULL;
	uint32_t keys = 0;

	if(json == NULL){
		return 0;
	}

	if((obj = cJSON_GetObjectItem(json,JsonParser::p_updFlags)) != NULL){
		cfg.updFlagMask = (Blob::AstCalUpdFlags)obj->valueint;
		keys |= Blob::AstCalKeyCfgUpd;
	}
	if((obj = cJSON_GetObjectItem(json,JsonParser::p_evtFlags)) != NULL){
		cfg.evtFlagMask = (Blob::AstCalEvtFlags)obj->valueint;
		keys |= Blob::AstCalKeyCfgEvt;
	}
	if((obj = cJSON_GetObjectItem(json, JsonParser::p_seasonCfg)) != NULL){
		char* seas_cfg = obj->valuestring;
		if(seas_cfg && strlen(seas_cfg) < Blob::LengthOfSeasonEnvText){
			strcpy(cfg.seasonCfg.envText, seas_cfg);
			keys |= Blob::AstCalKeyCfgSeason;
		}
	}

	if((astcfg = cJSON_GetObjectItem(json, JsonParser::p_ast)) != NULL){
		if((obj = cJSON_GetObjectItem(astcfg, JsonParser::p_latitude)) != NULL){
			cfg.astCfg.latitude = obj->valuedouble;
			keys |= Blob::AstCalKeyCfgLat;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_longitude)) != NULL){
			cfg.astCfg.longitude = obj->valuedouble;
			keys |= Blob::AstCalKeyCfgLon;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_wdowDawnStart)) != NULL){
			cfg.astCfg.wdowDawnStart = obj->valueint;
			keys |= Blob::AstCalKeyCfgWdaSta;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_wdowDawnStop)) != NULL){
			cfg.astCfg.wdowDawnStop = obj->valueint;
			keys |= Blob::AstCalKeyCfgWdaStp;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_wdowDuskStart)) != NULL){
			cfg.astCfg.wdowDuskStart = obj->valueint;
			keys |= Blob::AstCalKeyCfgWduSta;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_wdowDuskStop)) != NULL){
			cfg.astCfg.wdowDuskStop = obj->valueint;
			keys |= Blob::AstCalKeyCfgWduStp;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_reductionStart)) != NULL){
			cfg.astCfg.reductionStart = obj->valueint;
			keys |= Blob::AstCalKeyCfgRedSta;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_reductionStop)) != NULL){
			cfg.astCfg.reductionStop = obj->valueint;
			keys |= Blob::AstCalKeyCfgRedStp;
		}
	}

	if((array = cJSON_GetObjectItem(json, JsonParser::p_periods)) != NULL){
		if(cJSON_GetArraySize(array) <= Blob::AstCalMaxPeriodCount){
			int items = 0;
			for(int i=0;i<cJSON_GetArraySize(array);i++){
				cJSON* value = NULL;
				if((obj = cJSON_GetArrayItem(array, i)) != NULL){
					int params = 0;
					if((value = cJSON_GetObjectItem(obj, JsonParser::p_since)) != NULL){
						cfg.periods[i].since = (time_t)value->valuedouble;
						params++;
					}
					if((value = cJSON_GetObjectItem(obj, JsonParser::p_until)) != NULL){
						cfg.periods[i].until = (time_t)value->valuedouble;
						params++;
					}
					if((value = cJSON_GetObjectItem(obj, JsonParser::p_enabled)) != NULL){
						cfg.periods[i].enabled = (bool)value->valueint;
						params++;
					}
					if(params == 3)
						items++;
				}
			}
			if (items==cJSON_GetArraySize(array)){
				keys |= Blob::AstCalKeyCfgPeriods;
			}
		}
	}
	if((obj = cJSON_GetObjectItem(json,JsonParser::p_verbosity)) != NULL){
		cfg.verbosity = obj->valueint;
		keys |= Blob::AstCalKeyCfgVerbosity;
	}

	return keys;
}


//------------------------------------------------------------------------------------
uint32_t getAstCalStatFromJson(Blob::AstCalStatData_t &stat, cJSON* json){
	cJSON* astData = NULL;
	cJSON* obj = NULL;
	uint32_t keys = 0;

	if(json == NULL){
		return 0;
	}

	if((obj = cJSON_GetObjectItem(json,JsonParser::p_flags)) != NULL){
		stat.flags = obj->valueint;
		keys |= Blob::AstCalKeyAny;
	}
	if((obj = cJSON_GetObjectItem(json,JsonParser::p_period)) != NULL){
		stat.period = obj->valueint;
		keys |= Blob::AstCalKeyAny;
	}
	if((obj = cJSON_GetObjectItem(json, JsonParser::p_now)) != NULL){
		stat.now = (time_t)obj->valuedouble;
		keys |= Blob::AstCalKeyAny;
	}

	if((astData = cJSON_GetObjectItem(json, JsonParser::p_ast)) != NULL){
		if((obj = cJSON_GetObjectItem(astData, JsonParser::p_latitude)) != NULL){
			stat.astData.latitude = obj->valueint;
			keys |= Blob::AstCalKeyAny;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_longitude)) != NULL){
			stat.astData.longitude = obj->valueint;
			keys |= Blob::AstCalKeyAny;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_wdowDawnStart)) != NULL){
			stat.astData.wdowDawnStart = obj->valueint;
			keys |= Blob::AstCalKeyAny;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_wdowDawnStop)) != NULL){
			stat.astData.wdowDawnStop = obj->valueint;
			keys |= Blob::AstCalKeyAny;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_wdowDuskStart)) != NULL){
			stat.astData.wdowDuskStart = obj->valueint;
			keys |= Blob::AstCalKeyAny;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_wdowDuskStop)) != NULL){
			stat.astData.wdowDuskStop = obj->valueint;
			keys |= Blob::AstCalKeyAny;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_reductionStart)) != NULL){
			stat.astData.reductionStart = obj->valueint;
			keys |= Blob::AstCalKeyAny;
		}
		if((obj = cJSON_GetObjectItem(json, JsonParser::p_reductionStop)) != NULL){
			stat.astData.reductionStop = obj->valueint;
			keys |= Blob::AstCalKeyAny;
		}
	}
	return keys;
}


//------------------------------------------------------------------------------------
uint32_t getAstCalBootFromJson(Blob::AstCalBootData_t &obj, cJSON* json){
	cJSON* cfg = NULL;
	cJSON* stat = NULL;
	uint32_t keys = 0;

	if(json == NULL){
		return 0;
	}
	if((cfg = cJSON_GetObjectItem(json, JsonParser::p_cfg)) != NULL){
		keys |= getAstCalCfgFromJson(obj.cfg, cfg);
	}
	if((stat = cJSON_GetObjectItem(json, JsonParser::p_stat)) != NULL){
		keys |= getAstCalStatFromJson(obj.stat, stat);
	}
	return keys;
}


}	// end namespace JSON
