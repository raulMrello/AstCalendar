/*
 * metering_objects.h
 *
 *  Created on: Mar 2019
 *      Author: raulMrello
 *
 *	Objetos JSON relativos al módulo metering
 */
 
#ifndef CALENDAR_OBJECTS_
#define CALENDAR_OBJECTS_

#include <cstdint>
#include <type_traits>
#include "common_objects.h"
#include "cJSON.h"


/** UIDs */
#define UID_CALENDAR_MANAGER(vers)				(uint32_t)(0x0000000B | ((uint32_t)vers << 20))
#define UID_CALENDAR_CLOCK(vers)				(uint32_t)(0x0000000C | ((uint32_t)vers << 20))
#define UID_CALENDAR_PERIOD(vers)				(uint32_t)(0x0000000D | ((uint32_t)vers << 20))
#define UID_CALENDAR_GEOLOC(vers)				(uint32_t)(0x0000000E | ((uint32_t)vers << 20))

/** Versiones */
#define VERS_CALENDAR_DEFAULT		0
#define VERS_CALENDAR_NAME_0		""

#define VERS_CALENDAR_CURRENT		VERS_CALENDAR_DEFAULT



static inline const char* VERS_CALENDAR_NAME(int vers){
	switch(vers){
		case VERS_CALENDAR_DEFAULT:		return VERS_CALENDAR_NAME_0;
		default: 						return "";
	}
}


/** Flags para la variable calendar:manager/cfg.updFlags */
enum calendar_manager_cfg_updFlags {
	CalendarManagerCfgUpdNotif 		= (1 << 0),	/// Habilita notificación de cambios en cualquier parámetro de la configuración
};


/** Flags para la variable calendar:clock/stat.flags */
enum calendar_clock_stat_flags{
	 CalendarClockNoEvents		= 0,		//!< No hay eventos
	 CalendarClockYearEvt  		= (1 << 0),	//!< Evento al cambiar de año
	 CalendarClockIVEvt 		= (1 << 1),	//!< Evento al cambiar de invierno a verano
	 CalendarClockVIEvt 		= (1 << 2),	//!< Evento al cambiar de verano a invierno
	 CalendarClockMonthEvt  	= (1 << 3),	//!< Evento al cambiar de mes
	 CalendarClockWeekEvt 		= (1 << 4),	//!< Evento al cambiar de semana
	 CalendarClockDayEvt 		= (1 << 5),	//!< Evento al cambiar de día
	 CalendarClockMiddayEvt 	= (1 << 6),	//!< Evento al pasar por el medio día
	 CalendarClockPreDuskEvt 	= (1 << 7),	//!< Evento al pasar por el inicio de la ventana temporal de ocaso
	 CalendarClockDuskEvt 		= (1 << 8),	//!< Evento al ocaso
	 CalendarClockPostDuskEvt 	= (1 << 9),	//!< Evento al pasar por el final de la ventana temporal de ocaso
	 CalendarClockReducStartEvt	= (1 << 10),//!< Evento al pasar por la hora de inicio de reducción de flujo luminoso
	 CalendarClockReducStopEvt	= (1 << 11),//!< Evento al pasar por la hora de finalización de reducción de flujo luminoso
	 CalendarClockPreDawnEvt 	= (1 << 12),//!< Evento al pasar por el inicio de la ventana temporal de orto
	 CalendarClockDawnEvt 		= (1 << 13),//!< Evento al pasar por el orto
	 CalendarClockPostDawnEvt 	= (1 << 14),//!< Evento al pasar por el final de la ventana temporal de orto
	 CalendarClockHourEvt 		= (1 << 15),//!< Evento al cambiar de hora
	 CalendarClockMinEvt 		= (1 << 16),//!< Evento al cambiar de minuto
	 CalendarClockSecEvt 		= (1 << 17),//!< Evento al cambiar de segundo
	 CalendarClockDawnDuskUpdEvt= (1 << 18),//!< Evento al actualizar las horas de orto y ocaso
	 CalendarClockPeriodEvt 	= (1 << 19),//!< Evento al cambiar de periodo
	 CalendarClockEvtINVALID	= (1 << 31),//!< Indica un evento inválido
};

/** Flags para la variable calendar:manager/cfg.evtFlags */
typedef calendar_clock_stat_flags calendar_manager_cfg_evtFlags;


/** Máximo número de periodos permitidos para el array calendar:clock/cfg.periods[] */
static const uint8_t CalendarClockCfgMaxNumPeriods = 8;

/** Tamaño máximo del texto asociado a la variable calendar:geoloc/timezone */
static const uint8_t CalendarGeolocTimezoneLength = 64;

/** Límites min-max de los parámetros astronómicos */
static const double CalendarGeolocLatitudeMin = -90;
static const double CalendarGeolocLatitudeMax =  90;
static const double CalendarGeolocLongitudeMin = -180;
static const double CalendarGeolocLongitudeMax =  180;


/**Objeto calendar:geoloc */
struct calendar_geoloc{
	uint32_t uid;
	double 	 coords[2];
	char   	 timezone[CalendarGeolocTimezoneLength];
	time_t 	 astCorr[CalendarClockCfgMaxNumPeriods][2];
	uint8_t _numPeriods;
	uint32_t _keys;
	void setVersion(uint32_t vers){
		uid = UID_CALENDAR_GEOLOC(vers);
	}
};


/**Objeto calendar:period */
struct calendar_period{
	uint32_t uid;
	time_t   since;
	time_t   until;
	bool     enabled;
	void setVersion(uint32_t vers){
		uid = UID_CALENDAR_PERIOD(vers);
	}
};


/**Objeto calendar:clock:cfg */
struct calendar_clock_cfg{
	calendar_period periods[CalendarClockCfgMaxNumPeriods];
	calendar_geoloc geoloc;
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
	void setVersion(uint32_t vers){
		uid = UID_CALENDAR_CLOCK(vers);
		cfg.geoloc.setVersion(vers);
		for(int i=0; i<CalendarClockCfgMaxNumPeriods; i++){
			cfg.periods[i].setVersion(vers);
		}
	}
};


/**Objeto calendar:manager:cfg */
struct calendar_manager_cfg{
	uint32_t updFlags;
	uint32_t evtFlags;
	uint8_t  verbosity;
	uint32_t _keys;
};


/**Objeto calendar:manager */
struct calendar_manager{
	uint32_t uid;
	calendar_manager_cfg cfg;
	calendar_clock clock;
	uint32_t _keys;
	void setVersion(uint32_t vers){
		uid = UID_CALENDAR_MANAGER(vers);
		clock.setVersion(vers);
	}
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
 * @return keys Parámetros decodificados o 0 en caso de error
 */
uint32_t getCalendarManagerFromJson(calendar_manager &obj, cJSON* json);

/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Parámetros decodificados o 0 en caso de error
 */
uint32_t getCalendarClockFromJson(calendar_clock &obj, cJSON* json);

/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Parámetros decodificados o 0 en caso de error
 */
uint32_t getCalendarClockCfgFromJson(calendar_clock_cfg &obj, cJSON* json);

/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Parámetros decodificados o 0 en caso de error
 */
uint32_t getCalendarPeriodFromJson(calendar_period &obj, cJSON* json);

/**
 * Decodifica el mensaje JSON en un objeto
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Parámetros decodificados o 0 en caso de error
 */
uint32_t getCalendarGeolocFromJson(calendar_geoloc &obj, cJSON* json);


template <typename T>
uint32_t getCalendarObjFromJson(T& obj, cJSON* json_obj){
	if (std::is_same<T, calendar_manager>::value){
		return JSON::getCalendarManagerFromJson((calendar_manager&)obj, json_obj);
	}
	if (std::is_same<T, calendar_clock>::value){
		return JSON::getCalendarClockFromJson((calendar_clock&)obj, json_obj);
	}
	return 0;
}

}	// end namespace JSON

#endif
