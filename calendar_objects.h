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


// Astronomico
/** Valor incorrecto para orto y ocaso */
#define Zone_MAX_SUNTIME_LIMIT  (uint16_t)1440
#define Zone_SET_SUNTIME_OOB 	0xF000

#define PI		(double)(3.14159265)
#define RAD		(double)(180/PI)
#define VP		(double)(8.22E-5)
#define M0		(double)(2.12344)
#define MN		(double)(1.72019E-2)
#define T0		(double)(2444000.5)
#define MR		(double)(0.04301)
#define FF		((double)13750.987)

#define	HUGE_VAL	(3.402823E+38F)	  /* FLT_MAX in 'float.h' */

#define	TWOOPI	 0.63661977236758134308
#define	P0S	    0.2078236841696101220215102e6
#define	P1S	    -0.765864156388469493709081e5
#define	P2S	    0.70641360814006880884734e4
#define	P3S	    -0.2378593245781215847583e3
#define	P4S	    0.28078274176220686085e1
#define	Q0S	    0.1323046665086493066402123e6
#define	Q1S	    0.5651686795316917682421e4
#define	Q2S	    0.1089998110371290528265e3

#define SQRT2   1.41421356237309504880
#define	SQ2P1	2.414213562373095048802e0
#define	SQ2M1	0.414213562373095048802e0
#define	PIO2	1.570796326794896619231e0
#define	PIO4	0.785398163397448309615e0
#define	P0	    0.445413400592906803197511e2
#define	PP1	    0.77477687719204208616481e2
#define	P2	    0.40969264832102256374186e2
#define	P3	    0.666057901700926265753e1
#define	P4	    0.1589740288482307048e0
#define	Q0	    0.445413400592906804445995e2
#define	Q1	    0.92324801072300974840693e2
#define	Q2	    0.62835930511032376833267e2
#define	Q3	    0.1550397755142198752523e2

#define	INVPI	1.27323954473516268      /*   ( 4 / PI )   */
#define	P0N	    -0.1306820264754825668269611177e+5
#define	P1N	    0.1055970901714953193602353981e+4
#define	P2N	    -0.1550685653483266376941705728e+2
#define	P3N	    0.3422554387241003435328470489e-1
#define	P4N	    0.3386638642677172096076369e-4
#define	Q0N	    -0.1663895238947119001851464661e+5
#define	Q1N	    0.4765751362916483698926655581e+4
#define	Q2N	    -0.1555033164031709966900124574e+3

#define INT_MAX		(32767)

#define	DOUBLE_FRACTION_SIZE		    52	/* without the hidden bit */
#define	DOUBLE_BIAS		        	    1023
#define	MIN_DOUBLE_BIASED_EXP 		    1
#define	MAX_DOUBLE_BIASED_EXP		    0x7fe
#define	DOUBLE_BIASED_INF_EXP 		    0x7ff
#define	DOUBLE_BIASED_NaN_EXP 		    0x7ff
#define	COPY_DOUBLE_SIGN( sign, hi )	( ((hi) & 0x7fffffff) | ((unsigned long)(sign) & 0x80000000) )
#define	GET_DOUBLE_SIGN( hi )		    ( ( (hi) & 0x80000000 ) ? 1 : 0)
#define	GET_DOUBLE_HI_MANTISSA( hi )	( (hi) & ( ( 1L << (DOUBLE_FRACTION_SIZE - 32) ) - 1 ) )
#define	GET_DOUBLE_LO_MANTISSA( lo )	(lo)
#define	GET_DOUBLE_EXPONENT( hi )	    ( ( (hi) >> (DOUBLE_FRACTION_SIZE - 32) ) & 0x7ff )
#define	STRIP_DOUBLE_EXPONENT( hi )	    ( (hi) & 0x800fffff )
#define	PUT_DOUBLE_EXPONENT( hi, exp )	( (hi) | ((unsigned long)exp << (DOUBLE_FRACTION_SIZE - 32) ) )
#define	DOUBLE_IS_ZERO( hi, lo )	    ( ((hi) & 0x7fffffff) == 0 && (lo) == 0 ) /* also for denormals */
#define BIAS	                        DOUBLE_BIAS

/** Codigos relativos a la fecha */
#define Rtc_DAY_MON		(uint8_t)0x40
#define Rtc_DAY_TUE		(uint8_t)0x20
#define Rtc_DAY_WED		(uint8_t)0x10
#define Rtc_DAY_THU		(uint8_t)0x08
#define Rtc_DAY_FRI		(uint8_t)0x04
#define Rtc_DAY_SAT		(uint8_t)0x02
#define Rtc_DAY_SUN		(uint8_t)0x01
#define Rtc_DAY_ALLWEEK (uint8_t)0x7F
#define Rtc_MON_JAN		(uint8_t)1
#define Rtc_MON_FEB		(uint8_t)2
#define Rtc_MON_MAR		(uint8_t)3
#define Rtc_MON_APR		(uint8_t)4
#define Rtc_MON_MAY		(uint8_t)5
#define Rtc_MON_JUN		(uint8_t)6
#define Rtc_MON_JUL		(uint8_t)7
#define Rtc_MON_AUG		(uint8_t)8
#define Rtc_MON_SEP		(uint8_t)9
#define Rtc_MON_OCT		(uint8_t)10
#define Rtc_MON_NOV		(uint8_t)11
#define Rtc_MON_DEC		(uint8_t)12

typedef union
{
	struct
	{
		unsigned long	lo;
		unsigned long	hi;
	}
	s;
	double	d;
}double2longs_t;

/** Macro de generaci�n de nombre de versi�n */
static inline const char* VERS_CALENDAR_NAME(){
	switch(VERS_CALENDAR_SELECTED){
		case VERS_CALENDAR_INTERNAL:	return VERS_CALENDAR_INTERNAL_NAME;
		default: 						return "";
	}
}

/////////////////////////////////////////////////////////////////

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

/** Tipo definido para objetos tipo CALENDAR_T */
typedef struct {
  	uint8_t hour; 
  	uint8_t minute;
  	uint8_t second;
  	uint8_t weekday;
  	uint8_t month;		//1-12
  	uint8_t date; 
  	uint8_t year;		//a partir del 2000, 23 = 2023
	uint8_t _NAN;
}CALENDAR_T;

// Estructura de un objeto coordenada en formato DMS
/*
	Ejemplo: 40.5372512,-3.6372245
	40.5372512 -> grados = 40, minutos = 32, segundos = 14, signo = 1
	-3.6372245 -> grados = -3, minutos = 38, segundos = 14, signo = -1
*/
typedef struct{
	int16_t	Grados;		//poner signo si es negativo
	int8_t	Minutos;
	int8_t	Segundos;
	int8_t  Signo; 		//1: positivo, -1: negativo
}COORD_T;



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
