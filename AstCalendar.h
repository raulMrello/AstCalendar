/*
 * AstCalendar.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	AstCalendar es el m�dulo encargado de gestionar la configuraci�n del calendario astron�mico y de generar
 *	los eventos correspondientes, conforme a dicha configuraci�n
 */
 
#ifndef __AstCalendar__H
#define __AstCalendar__H

#include "mbed.h"
#ifdef CONFIG_ASTCALENDAR_INACTIVE
#include "InactiveModule.h"
#else
#include "ActiveModule.h"
#endif
#include "AstCalendarBlob.h"
#include "RealTimeClock.h"
#include "JsonParserBlob.h"
#include "calendar_objects.h"
#include "sdkconfig.h"


/** Flag para habilitar el soporte de objetos JSON en las suscripciones a MQLib
 *  Por defecto DESACTIVADO
 */
#define ASTCAL_ENABLE_JSON_SUPPORT		0

const State::Event_type astcal_eventType = State::Event_type::EV_RESERVED_USER_UI64;
   
class AstCalendar : public
#ifdef CONFIG_ASTCALENDAR_INACTIVE
  InactiveModule
#else
  ActiveModule
#endif
{
  public:
              
    /** Constructor por defecto
     * 	@param fs Objeto FSManager para operaciones de backup
     * 	@param defdbg Flag para habilitar depuraci�n por defecto
     */
  AstCalendar(FSManager* fs, bool defdbg = false);


    /** Destructor
     */
    virtual ~AstCalendar(){}


    /** Instala el interfaz RTC
     *  @param rtc Interfaz RTC asociado
     */
    void attachRealTimeClock(RealTimeClock* rtc) {_rtc = rtc; }


    /** Arranca el simulador de eventos bas�ndose en un RtosTimer
     *
     */
    void startSimulator();


    /** Detiene el simulador de eventos bas�ndose en un RtosTimer
     *
     */
    void stopSimulator();


    /**
     * Activa y/o desactiva el soporte JSON
     * @param flag
     */
    void setJSONSupport(bool flag){
    	_json_supported = flag;
    }


    /**
     * Obtiene el estado del soporte JSON
     * @return
     */
    bool isJSONSupported(){
    	return _json_supported;
    }


    /**
     * Activa servicio NTPClient
     */
    void enableNTPClient();

    /**
	 * Segundos transcurridos desde el ultimo reset
	 */
    time_t GetSecondFromReset();
    /**
     * Calcula la hora de orto y ocaso dada una fecha y una localizacion
     * @param cal Referencia al calendario
     * @param gmt GMT aplicable al calculo en minutos
     * @param lat Referencia a la latitud
     * @param lng Referencia a la longitud
     * @param corrSunrise Correccion aplicable al resultado de orto
     * @param corrSunset Correccion aplicable al resultado de ocaso
     * @param sunrise Resultado de orto
     * @param sunset Resultado de ocaso
     * @param isAllDay Flag que se activa si resulta ser un dia sin ocaso
     * @param isAllNight Flag que se activa si resulta ser un dia sin orto
     * @return codigo de error <= 0
     */
    int8_t zoneCalculateSuntimes(CALENDAR_T *cal, int16_t gmt, COORD_T *lat, COORD_T *lng, int16_t corrSunrise, int16_t corrSunset, uint16_t * sunrise, uint16_t *sunset, uint8_t *isAllDay, uint8_t *isAllNight);

    /**
     * @brief Calculo de la desviacion respecto GMT en minutos
     * 
     * @param utc_time 
     * @param local_time 
     * @return int desviacion en minutos
     */
    int gmtDesviation(struct tm* utc_time , struct tm* local_time);

    void duskDawnCalc();
  private:

    /** M�ximo n�mero de mensajes alojables en la cola asociada a la m�quina de estados */
    static const uint32_t MaxQueueMessages = 16;

    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags : uint64_t{
    	RecvCfgSet 	 = ((uint64_t)astcal_eventType << 0),  /// Flag activado al recibir mensaje en "set/cfg"
    	RecvCfgGet	 = ((uint64_t)astcal_eventType << 1),  /// Flag activado al recibir mensaje en "get/cfg"
    	RecvBootGet	  = ((uint64_t)astcal_eventType << 2),  /// Flag activado al recibir mensaje en "get/boot"
		  RecvRtcSet    = ((uint64_t)astcal_eventType << 3),
		  RcvSetDefault   = ((uint64_t)astcal_eventType << 4),
      RcvOrtoGet = ((uint64_t)astcal_eventType << 5),
      RcvOcasoGet = ((uint64_t)astcal_eventType << 6)
    };

    /** Datos de configuraci�n y estado */
    calendar_manager _astdata;

    /** Timer de simulaci�n de eventos */
    RtosTimer* _sim_tmr;

    /** Contador de segundos del simulador de eventos */
    uint32_t _sim_counter;

    /** Interfaz RealTimeClock */
    RealTimeClock* _rtc;

    /** Hora actual en formato tm */
    tm _now;

    /** Segundos desde el �ltimo apagado */
    time_t _pw_fail;

    /** Flag de control para el soporte de objetos json */
    bool _json_supported;

    /** Variables para controlar la actualizaci�n horaria via NTP */
    static const int NtpDifSecUpdate = 30;
    bool _ntp_enabled;
    time_t _last_rtc_time;
    int _curr_dst;
    int _curr_sun;


 	/** Interfaz para manejar los eventos en la m�quina de estados por defecto
      *  @param se Evento a manejar
      *  @return State::StateResult Resultado del manejo del evento
      */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se);


 	/** Callback invocada al recibir una actualizaci�n de un topic local al que est� suscrito
      *  @param topic Identificador del topic
      *  @param msg Mensaje recibido
      *  @param msg_len Tama�o del mensaje
      */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);


 	/** Callback invocada al finalizar una publicaci�n local
      *  @param topic Identificador del topic
      *  @param result Resultado de la publicaci�n
      */
    virtual void publicationCb(const char* topic, int32_t result);


   	/** Chequea la integridad de los datos de configuraci�n <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	virtual bool checkIntegrity();


   	/** Establece la configuraci�n por defecto grab�ndola en memoria NV
	 */
	virtual void setDefaultConfig();


   	/** Recupera la configuraci�n de memoria NV
	 */
	virtual void restoreConfig();


   	/** Graba la configuraci�n en memoria NV
	 */
	virtual void saveConfig();

  /**
   * @brief Elimina de NVS claves antiguas
   * 
   */
  void cleanUp();


	/** Graba un par�metro en la memoria NV
	 * 	@param param_id Identificador del par�metro
	 * 	@param data Datos asociados
	 * 	@param size Tama�o de los datos
	 * 	@param type Tipo de los datos
	 * 	@return True: �xito, False: no se pudo recuperar
	 */
  virtual bool saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
#ifdef CONFIG_ASTCALENDAR_INACTIVE
    return InactiveModule::saveParameter(param_id, data, size, type);
#else
    return ActiveModule::saveParameter(param_id, data, size, type);
#endif
  }


	/** Recupera un par�metro de la memoria NV
	 * 	@param param_id Identificador del par�metro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tama�o de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: �xito, False: no se pudo recuperar
	 */
  virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
#ifdef CONFIG_ASTCALENDAR_INACTIVE
    return InactiveModule::restoreParameter(param_id, data, size, type);
#else
    return ActiveModule::restoreParameter(param_id, data, size, type);
#endif
  }


	/** Ejecuta el simulador de eventos
	 *
	 */
	void eventSimulatorCb();


	/** Actualiza la configuraci�n
	 *
	 * @param cfg Nueva configuraci�n a aplicar
	 * @param keys Flags de par�metros actualizados
	 * @param err Recibe los errores generados durante la actualizaci�n
	 */
	void _updateConfig(const calendar_manager& data, Blob::ErrorData_t& err);

    /**
     * Callback invocada por lwip/sntp cada vez que reciba una actualizaci�n horaria
     */
    void _ntpUpdateCb();

    void setRtcTime(time_t tnow);

    // actualiza la hora tras un cambio de configuración
    void _updateRtcFromCfg();

  unsigned char IsNaN(double hh, double mm, signed short int gmt);
  double own_abs(double x);

};
     
#endif /*__AstCalendar__H */

/**** END OF FILE ****/


