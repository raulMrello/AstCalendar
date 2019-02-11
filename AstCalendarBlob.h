/*
 * AstCalendarBlob.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	AstCalendarBlob es el componente del módulo AstCalendar en el que se definen los objetos y tipos relativos a
 *	los objetos BLOB de este módulo.
 *	Todos los tipos definidos en este componente están asociados al namespace "Blob", de forma que puedan ser
 *	accesibles mediante el uso de: "Blob::"  e importando este archivo de cabecera.
 */
 
#ifndef __AstCalendarBlob__H
#define __AstCalendarBlob__H

#include "Blob.h"
#include "mbed.h"
  

namespace Blob {


/** Flags para la configuración de notificaciones de un objeto AstCalendar cuando su configuración se ha
 *  modificado.
 */
 enum AstCalUpdFlags{
	 EnableAstCalCfgUpdNotif = (1 << 0),  	/// Flag activado para notificar cambios en la configuración en bloque del objeto
	 AstCalUpdFlagINVALID	 = (1 << 31)	/// Flag para indicar resultado inválido
 };


 /** Flags de evento que utiliza AstCalendar para notificar un cambio de estado
  */
 enum AstCalEvtFlags{
	 AstCalNoEvents			= 0,				//!< No hay eventos
	 AstCalYearEvt  		= (1 << 0),		//!< Evento al cambiar de año
	 AstCalIVEvt 			= (1 << 1),		//!< Evento al cambiar de invierno a verano
	 AstCalVIEvt 			= (1 << 2),		//!< Evento al cambiar de verano a invierno
	 AstCalMonthEvt  		= (1 << 3),		//!< Evento al cambiar de mes
	 AstCalWeekEvt 			= (1 << 4),		//!< Evento al cambiar de semana
	 AstCalDayEvt 			= (1 << 5),		//!< Evento al cambiar de día
	 AstCalMiddayEvt 		= (1 << 6),		//!< Evento al pasar por el medio día
	 AstCalPreDuskEvt 		= (1 << 7),		//!< Evento al pasar por el inicio de la ventana temporal de ocaso
	 AstCalDuskEvt 			= (1 << 8),		//!< Evento al ocaso
	 AstCalPostDuskEvt 		= (1 << 9),		//!< Evento al pasar por el final de la ventana temporal de ocaso
	 AstCalReducStartEvt	= (1 << 10),	//!< Evento al pasar por la hora de inicio de reducción de flujo luminoso
	 AstCalReducStopEvt		= (1 << 11),	//!< Evento al pasar por la hora de finalización de reducción de flujo luminoso
	 AstCalPreDawnEvt 		= (1 << 12),	//!< Evento al pasar por el inicio de la ventana temporal de orto
	 AstCalDawnEvt 			= (1 << 13),	//!< Evento al pasar por el orto
	 AstCalPostDawnEvt 		= (1 << 14),	//!< Evento al pasar por el final de la ventana temporal de orto
	 AstCalHourEvt 			= (1 << 15),	//!< Evento al cambiar de hora
	 AstCalMinEvt 			= (1 << 16),	//!< Evento al cambiar de minuto
	 AstCalSecEvt 			= (1 << 17),	//!< Evento al cambiar de segundo
	 AstCalDawnDuskUpdEvt 	= (1 << 18),	//!< Evento al actualizar las horas de orto y ocaso
	 AstCalPeriodEvt 		= (1 << 19),	//!< Evento al cambiar de periodo
	 AstCalEvtINVALID		= (1 << 31),	//!< Indica un evento inválido
  };


 /** Flags para identificar cada key-value de los objetos JSON que se han modificado en un SET remoto
  */
enum AstCalKeyNames{
	AstCalKeyNone 		= 0,
	AstCalKeyCfgUpd		= (1 << 0),
	AstCalKeyCfgEvt		= (1 << 1),
	AstCalKeyCfgLat		= (1 << 2),
	AstCalKeyCfgLon		= (1 << 3),
	AstCalKeyCfgWdaSta	= (1 << 4),
	AstCalKeyCfgWdaStp	= (1 << 5),
	AstCalKeyCfgWduSta	= (1 << 6),
	AstCalKeyCfgWduStp	= (1 << 7),
	AstCalKeyCfgRedSta	= (1 << 8),
	AstCalKeyCfgRedStp	= (1 << 9),
	AstCalKeyCfgSeason	= (1 << 10),
	AstCalKeyCfgPeriods = (1 << 11),
	//
	AstCalKeyAny		= (1 << 31),
	AstCalKeyCfgAll     = 0x7ff,
};


 /** Estructura de datos para la configuración astronómica
  * 	Se forma por diferentes timestamp expresados en minuto/dia (max: 24*60)
  * 	@var latitude Latitud decimal (-90, 90)
  * 	@var longitude Longitud decimal, (-180, 180)
  * 	@var wdowDawnStart Hora de inicio de la ventana de orto en minutos
  * 	@var wdowDawnStop Hora de finalización de la ventana de orto en minutos
  * 	@var wdowDuskStart Hora de inicio de la ventana de ocaso en minutos
  * 	@var wdowDuskStop Hora de finalización de la ventana de ocaso en minutos
  * 	@var reductionStart Hora de inicio de la reducción de flujo luminoso nocturno en minutos
  * 	@var reductionStop Hora de finalización de la reducción de flujo luminoso nocturno en minuto
  */
struct __packed AstCalAstData_t{
 	double latitude;
 	double longitude;
 	int16_t wdowDawnStart;
 	int16_t wdowDawnStop;
 	int16_t wdowDuskStart;
 	int16_t wdowDuskStop;
 	uint16_t reductionStart;
 	uint16_t reductionStop;
 };

/** Límites min-max de los parámetros astronómicos */
static const double AstCalMinLatitude = -90;
static const double AstCalMaxLatitude =  90;
static const double AstCalMinLongitude = -180;
static const double AstCalMaxLongitude =  180;
static const int16_t AstCalMinAstWdow = -1439;
static const int16_t AstCalMaxAstWdow =  1439;
static const uint16_t AstCalMaxAstRedct=  1439;


/** Tamaño por defecto de la cadena de texto asociada a la zona horaria
 */
static const uint8_t LengthOfSeasonEnvText = 64;


/** Tamaño máximo de los parámetros textuales */
static const uint16_t AstCalTextParamLength = 32;


/** Estructura de datos para la configuración de los cambios de estación.
 *  Se compone de una cadena de texto que indica la zona horaria del tipo "GMT±KGMT±L,Mm.s.w/h, Mm.s.w/h“
 * 	@var envText Indicación de zona horaria
 */
struct __packed AstCalSeason_t {
  	char envText[LengthOfSeasonEnvText];
};


/** Estructura de datos para establecer un periodo temporal
 * 	@var since Fecha de inicio (inclusiva)
 * 	@var until Fecha de finalización (exclusiva)
 * 	@var enabled Flag de activación
 */
struct __packed AstCalPeriod_t{
	time_t since;
	time_t until;
	bool enabled;
};


/** Número máximo de periodos gestionables */
static const uint16_t AstCalMaxPeriodCount = 8;


/** Estructura de datos para la configuración en bloque del objeto AstCalendar.
 * 	Se forma por las distintas estructuras de datos de configuración
 * 	@var updFlags Flags de configuración de notificación de cambios de configuración
 * 	@var evtFlags Flags de configuración de notificación de eventos
 * 	@var astCfg Parámetros astronómicos
 * 	@var seasonCfg Parámetros relativos a la zona y cambio horario
 */
struct __packed AstCalCfgData_t{
	AstCalUpdFlags updFlagMask;
	AstCalEvtFlags evtFlagMask;
	AstCalAstData_t astCfg;
	AstCalSeason_t seasonCfg;
	AstCalPeriod_t periods[AstCalMaxPeriodCount];
};


/** Estructura de datos asociado a las notificaciones de evento
 */
struct __packed AstCalStatData_t{
	AstCalEvtFlags flags;		// Flags de eventos
	uint32_t period;			// periodo de ejecución actual
	time_t now;					// hora actual
	AstCalAstData_t astData;	// Horas de inicio y finalización de hitos astronómicos
};


/** Estructura de datos asociado al boot
 */
struct __packed AstCalBootData_t{
	AstCalCfgData_t cfg;
	AstCalStatData_t stat;
};

}	// end namespace Blob


namespace JSON {

/**
 * Codifica la configuración actual en un objeto JSON
 * @param cfg Configuración
 * @return Objeto JSON o NULL en caso de error
 */
cJSON* getJsonFromAstCalCfg(const Blob::AstCalCfgData_t& cfg);

/**
 * Codifica el estado actual en un objeto JSON
 * @param stat Estado
 * @return Objeto JSON o NULL en caso de error
 */
cJSON* getJsonFromAstCalStat(const Blob::AstCalStatData_t& stat);

/**
 * Codifica el estado de arranque en un objeto JSON
 * @param boot Estado de arranque
 * @return Objeto JSON o NULL en caso de error
 */
cJSON* getJsonFromAstCalBoot(const Blob::AstCalBootData_t& boot);



/**
 * Decodifica el mensaje JSON en un objeto de configuración
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Parámetros decodificados o 0 en caso de error
 */
uint32_t getAstCalCfgFromJson(Blob::AstCalCfgData_t &obj, cJSON* json);


/**
 * Decodifica el mensaje JSON en un objeto de estado
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Parámetros decodificados o 0 en caso de error
 */
uint32_t getAstCalStatFromJson(Blob::AstCalStatData_t &obj, cJSON* json);


/**
 * Decodifica el mensaje JSON en un objeto de arranque
 * @param obj Recibe el objeto decodificado
 * @param json Objeto JSON a decodificar
 * @return keys Parámetros decodificados o 0 en caso de error
 */
uint32_t getAstCalBootFromJson(Blob::AstCalBootData_t &obj, cJSON* json);


}	// end namespace JSON



#endif
