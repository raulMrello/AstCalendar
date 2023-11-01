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
#include "ActiveModule.h"
#include "AstCalendarBlob.h"
#include "RealTimeClock.h"
#include "JsonParserBlob.h"
#include "calendar_objects.h"


/** Flag para habilitar el soporte de objetos JSON en las suscripciones a MQLib
 *  Por defecto DESACTIVADO
 */
#define ASTCAL_ENABLE_JSON_SUPPORT		0


   
class AstCalendar : public ActiveModule {
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
    int8_t Zone_CalculateSuntimes(CALENDAR_T *cal, int16_t gmt, COORD_T *lat, COORD_T *lng, int16_t corrSunrise, int16_t corrSunset, uint16_t * sunrise, uint16_t *sunset, uint8_t *isAllDay, uint8_t *isAllNight);


  private:

    /** M�ximo n�mero de mensajes alojables en la cola asociada a la m�quina de estados */
    static const uint32_t MaxQueueMessages = 16;

    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	RecvCfgSet 	 = (State::EV_RESERVED_USER << 0),  /// Flag activado al recibir mensaje en "set/cfg"
    	RecvCfgGet	 = (State::EV_RESERVED_USER << 1),  /// Flag activado al recibir mensaje en "get/cfg"
    	RecvBootGet	  = (State::EV_RESERVED_USER << 2),  /// Flag activado al recibir mensaje en "get/boot"
		RecvRtcSet    = (State::EV_RESERVED_USER << 3),
		RcvSetDefault   = (State::EV_RESERVED_USER << 4)
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
		return ActiveModule::saveParameter(param_id, data, size, type);
	}


	/** Recupera un par�metro de la memoria NV
	 * 	@param param_id Identificador del par�metro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tama�o de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: �xito, False: no se pudo recuperar
	 */
	virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::restoreParameter(param_id, data, size, type);
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


