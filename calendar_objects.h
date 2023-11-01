/*
 * metering_objects.h
 *
 *  Created on: Mar 2019
 *      Author: raulMrello
 *
 *	Objetos JSON relativos al m�dulo metering
 */
 
#ifndef CALENDAR_OBJECTS_
#define CALENDAR_OBJECTS_

#include <cstdint>
#include <type_traits>
#include "common_objects.h"
#include "cJSON.h"

/** Versiones soportadas */
#define VERS_CALENDAR_INTERNAL			0
#define VERS_CALENDAR_INTERNAL_NAME	(const char*)""


/** Selecci�n de la versi�n utilizada 	*/
/** DEFINIR SEG�N APLICACI�N 			*/
#define VERS_CALENDAR_SELECTED		VERS_CALENDAR_INTERNAL /*others...*/


/** Macro de generaci�n de UIDs*/
#define UID_CALENDAR_MANAGER		(uint32_t)(0x00000003 | ((uint32_t)VERS_CALENDAR_SELECTED << 20))
#define UID_CALENDAR_CLOCK			(uint32_t)(0x00000004 | ((uint32_t)VERS_CALENDAR_SELECTED << 20))

/** Macro de generaci�n de nombre de versi�n */
static inline const char* VERS_CALENDAR_NAME(){
	switch(VERS_CALENDAR_SELECTED){
		case VERS_CALENDAR_INTERNAL:	return VERS_CALENDAR_INTERNAL_NAME;
		default: 						return "";
	}
}

/** Flags para la variable calendar:manager/cfg.updFlags */
enum calendar_manager_cfg_updFlags {
	CalendarManagerCfgUpdNotif 		= (1 << 0),	/// Habilita notificaci�n de cambios en cualquier par�metro de la configuraci�n
};


/** Flags para la variable calendar:clock/stat.flags */
enum calendar_clock_stat_flags{
	 CalendarClockNoEvents		= 0,		//!< No hay eventos
	 CalendarClockYearEvt  		= (1 << 0),	//!< Evento al cambiar de a�o
	 CalendarClockIVEvt 		= (1 << 1),	//!< Evento al cambiar de invierno a verano
	 CalendarClockVIEvt 		= (1 << 2),	//!< Evento al cambiar de verano a invierno
	 CalendarClockMonthEvt  	= (1 << 3),	//!< Evento al cambiar de mes
	 CalendarClockWeekEvt 		= (1 << 4),	//!< Evento al cambiar de semana
	 CalendarClockDayEvt 		= (1 << 5),	//!< Evento al cambiar de d�a
	 CalendarClockMiddayEvt 	= (1 << 6),	//!< Evento al pasar por el medio d�a
	 CalendarClockPreDuskEvt 	= (1 << 7),	//!< Evento al pasar por el inicio de la ventana temporal de ocaso
	 CalendarClockDuskEvt 		= (1 << 8),	//!< Evento al ocaso
	 CalendarClockPostDuskEvt 	= (1 << 9),	//!< Evento al pasar por el final de la ventana temporal de ocaso
	 CalendarClockReducStartEvt	= (1 << 10),//!< Evento al pasar por la hora de inicio de reducci�n de flujo luminoso
	 CalendarClockReducStopEvt	= (1 << 11),//!< Evento al pasar por la hora de finalizaci�n de reducci�n de flujo luminoso
	 CalendarClockPreDawnEvt 	= (1 << 12),//!< Evento al pasar por el inicio de la ventana temporal de orto
	 CalendarClockDawnEvt 		= (1 << 13),//!< Evento al pasar por el orto
	 CalendarClockPostDawnEvt 	= (1 << 14),//!< Evento al pasar por el final de la ventana temporal de orto
	 CalendarClockHourEvt 		= (1 << 15),//!< Evento al cambiar de hora
	 CalendarClockMinEvt 		= (1 << 16),//!< Evento al cambiar de minuto
	 CalendarClockSecEvt 		= (1 << 17),//!< Evento al cambiar de segundo
	 CalendarClockDawnDuskUpdEvt= (1 << 18),//!< Evento al actualizar las horas de orto y ocaso
	 CalendarClockPeriodEvt 	= (1 << 19),//!< Evento al cambiar de periodo
	 CalendarClockNTPEvt 		= (1 << 20),//!< Evento al actualizar la hora via NTP
	 CalendarClockEvtINVALID	= (1 << 31),//!< Indica un evento inv�lido
};

char* calendarClockStatFlags2String(calendar_clock_stat_flags flags);

/** Flags para la variable calendar:manager/cfg.evtFlags */
typedef calendar_clock_stat_flags calendar_manager_cfg_evtFlags;


/** M�ximo n�mero de periodos permitidos para el array calendar:clock/cfg.periods[] */
static const uint8_t CalendarClockCfgMaxNumPeriods = 8;

/** Tama�o m�ximo del texto asociado a la variable calendar:geoloc/timezone */
static const uint8_t CalendarGeolocTimezoneLength = 64;

/** L�mites min-max de los par�metros astron�micos */
static const double CalendarGeolocLatitudeMin = -90;
static const double CalendarGeolocLatitudeMax =  90;
static const double CalendarGeolocLongitudeMin = -180;
static const double CalendarGeolocLongitudeMax =  180;


/**Objeto calendar:geoloc */
struct calendar_geoloc{
	double 	 coords[2];
	char   	 timezone[CalendarGeolocTimezoneLength];
	uint8_t	 timezoneCode;
	time_t 	 astCorr[CalendarClockCfgMaxNumPeriods][2];
	uint8_t _numPeriods;
	uint32_t _keys;
};

/**Objeto calendar:geoloc_old */
struct calendar_geoloc_old{
	double 	 coords[2];
	char   	 timezone[CalendarGeolocTimezoneLength];
	time_t 	 astCorr[CalendarClockCfgMaxNumPeriods][2];
	uint8_t _numPeriods;
	uint32_t _keys;
};


/**Objeto calendar:period */
struct calendar_period{
	time_t   since;
	time_t   until;
	bool     enabled;
};


/**Objeto calendar:clock:cfg */
struct calendar_clock_cfg{
	calendar_period periods[CalendarClockCfgMaxNumPeriods];
	calendar_geoloc geoloc;
	uint8_t _numPeriods;
	uint32_t _keys;
};

/**Objeto calendar:clock:cfg_old */
struct calendar_clock_cfg_old{
	calendar_period periods[CalendarClockCfgMaxNumPeriods];
	calendar_geoloc_old geoloc;
	uint8_t _numPeriods;
	uint32_t _keys;
};


/**Objeto calendar:clock:stat */
struct calendar_clock_stat{
	uint32_t flags;
	int8_t   period;
	time_t	 localtime;
	time_t   dawn;
	time_t   dusk;
};


/**Objeto calendar:clock */
struct calendar_clock{
	uint32_t uid;
	calendar_clock_cfg  cfg;
	calendar_clock_stat stat;
	uint32_t _keys;
};


/**Objeto calendar:manager:cfg */
struct calendar_manager_cfg{
	uint32_t updFlags;
	uint32_t evtFlags;
	uint8_t  verbosity;
	uint32_t nvs_id;
	uint32_t _keys;
};


/**Objeto calendar:manager */
struct calendar_manager{
	uint32_t uid;
	calendar_manager_cfg cfg;
	calendar_clock clock;
	uint32_t _keys;
};



namespace JSON {

/**
 * Codifica el objeto en un JSON
 * @param obj Objeto
 * @return JSON resultante o NULL en caso de error
 */
cJSON* getJsonFromCalendarManager(const calendar_manager& obj, ObjDataSelection type);


/**
 * Codifica el objeto en un JSON
 * @param obj Objeto
 * @return JSON resultante o NULL en caso de error
 */
cJSON* getJsonFromCalendarClock(const calendar_clock& obj, ObjDataSelection type);


/**
 * Codifica el objeto en un JSON
 * @param obj Objeto
 * @return JSON resultante o NULL en caso de error
 */
cJSON* getJsonFromCalendarClockCfg(const calendar_clock_cfg& obj);


/**
 * Codifica el objeto en un JSON
 * @param obj Objeto
 * @return JSON resultante o NULL en caso de error
 */
cJSON* getJsonFromCalendarPeriod(const calendar_period& obj);


/**
 * Codifica el objeto en un JSON
 * @param obj Objeto
 * @return JSON resultante o NULL en caso de error
 */
cJSON* getJsonFromCalendarGeoloc(const calendar_geoloc& obj);


/**
 * Codifica el objeto en un JSON dependiendo del tipo de objeto
 * @param obj Objeto
 * @return JSON resultante o NULL en caso de error
 */
template <typename T>
cJSON* getJsonFromCalendar(const T& obj, ObjDataSelection type){
	if (std::is_same<T, calendar_manager>::value){
		return getJsonFromCalendarManager((const calendar_manager&)obj, type);
	}
	if (std::is_same<T, calendar_clock>::value){
		return getJsonFromCalendarClock((const calendar_clock&)obj, type);
	}
	return NULL;
}


/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Par�metros decodificados o 0 en caso de error
 */
uint32_t getCalendarManagerFromJson(calendar_manager &obj, cJSON* json);

/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Par�metros decodificados o 0 en caso de error
 */
uint32_t getCalendarClockFromJson(calendar_clock &obj, cJSON* json);

/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Par�metros decodificados o 0 en caso de error
 */
uint32_t getCalendarClockCfgFromJson(calendar_clock_cfg &obj, cJSON* json);

/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Par�metros decodificados o 0 en caso de error
 */
uint32_t getCalendarPeriodFromJson(calendar_period &obj, cJSON* json);

/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Par�metros decodificados o 0 en caso de error
 */
uint32_t getCalendarGeolocFromJson(calendar_geoloc &obj, cJSON* json);


template <typename T>
uint32_t getCalendarObjFromJson(T& obj, cJSON* json_obj){
	if (std::is_same<T, calendar_manager>::value){
		return JSON::getCalendarManagerFromJson((calendar_manager&)obj, json_obj);
	}
	if (std::is_same<T, calendar_clock_cfg>::value){
		return JSON::getCalendarClockCfgFromJson((calendar_clock_cfg&)obj, json_obj);
	}
	return 0;
}

}	// end namespace JSON

#endif
