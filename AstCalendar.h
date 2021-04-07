/*
 * AstCalendar.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	AstCalendar es el mï¿½dulo encargado de gestionar la configuraciï¿½n del calendario astronï¿½mico y de generar
 *	los eventos correspondientes, conforme a dicha configuraciï¿½n
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
     * 	@param defdbg Flag para habilitar depuraciï¿½n por defecto
     */
    AstCalendar(FSManager* fs, bool defdbg = false);


    /** Destructor
     */
    virtual ~AstCalendar(){}


    /** Instala el interfaz RTC
     *  @param rtc Interfaz RTC asociado
     */
    void attachRealTimeClock(RealTimeClock* rtc) {_rtc = rtc; }


    /** Arranca el simulador de eventos basï¿½ndose en un RtosTimer
     *
     */
    void startSimulator();


    /** Detiene el simulador de eventos basï¿½ndose en un RtosTimer
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


  private:

    /** Mï¿½ximo nï¿½mero de mensajes alojables en la cola asociada a la mï¿½quina de estados */
    static const uint32_t MaxQueueMessages = 16;

    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	RecvCfgSet 	 = (State::EV_RESERVED_USER << 0),  /// Flag activado al recibir mensaje en "set/cfg"
    	RecvCfgGet	 = (State::EV_RESERVED_USER << 1),  /// Flag activado al recibir mensaje en "get/cfg"
    	RecvBootGet	  = (State::EV_RESERVED_USER << 2),  /// Flag activado al recibir mensaje en "get/boot"
      RecvRtcSet    = (State::EV_RESERVED_USER << 3)
    };

    /** Datos de configuraciï¿½n y estado */
    calendar_manager _astdata;

    /** Timer de simulaciï¿½n de eventos */
    RtosTimer* _sim_tmr;

    /** Contador de segundos del simulador de eventos */
    uint32_t _sim_counter;

    /** Interfaz RealTimeClock */
    RealTimeClock* _rtc;

    /** Hora actual en formato tm */
    tm _now;

    /** Segundos desde el último apagado */
    time_t _pw_fail;

    /** Flag de control para el soporte de objetos json */
    bool _json_supported;

    /** Variables para controlar la actualizaciï¿½n horaria via NTP */
    static const int NtpDifSecUpdate = 30;
    bool _ntp_enabled;
    time_t _last_rtc_time;


 	/** Interfaz para manejar los eventos en la mï¿½quina de estados por defecto
      *  @param se Evento a manejar
      *  @return State::StateResult Resultado del manejo del evento
      */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se);


 	/** Callback invocada al recibir una actualizaciï¿½n de un topic local al que estï¿½ suscrito
      *  @param topic Identificador del topic
      *  @param msg Mensaje recibido
      *  @param msg_len Tamaï¿½o del mensaje
      */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);


 	/** Callback invocada al finalizar una publicaciï¿½n local
      *  @param topic Identificador del topic
      *  @param result Resultado de la publicaciï¿½n
      */
    virtual void publicationCb(const char* topic, int32_t result);


   	/** Chequea la integridad de los datos de configuraciï¿½n <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	virtual bool checkIntegrity();


   	/** Establece la configuraciï¿½n por defecto grabï¿½ndola en memoria NV
	 */
	virtual void setDefaultConfig();


   	/** Recupera la configuraciï¿½n de memoria NV
	 */
	virtual void restoreConfig();


   	/** Graba la configuraciï¿½n en memoria NV
	 */
	virtual void saveConfig();


	/** Graba un parï¿½metro en la memoria NV
	 * 	@param param_id Identificador del parï¿½metro
	 * 	@param data Datos asociados
	 * 	@param size Tamaï¿½o de los datos
	 * 	@param type Tipo de los datos
	 * 	@return True: ï¿½xito, False: no se pudo recuperar
	 */
	virtual bool saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::saveParameter(param_id, data, size, type);
	}


	/** Recupera un parï¿½metro de la memoria NV
	 * 	@param param_id Identificador del parï¿½metro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tamaï¿½o de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: ï¿½xito, False: no se pudo recuperar
	 */
	virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::restoreParameter(param_id, data, size, type);
	}


	/** Ejecuta el simulador de eventos
	 *
	 */
	void eventSimulatorCb();


	/** Actualiza la configuraciï¿½n
	 *
	 * @param cfg Nueva configuraciï¿½n a aplicar
	 * @param keys Flags de parï¿½metros actualizados
	 * @param err Recibe los errores generados durante la actualizaciï¿½n
	 */
	void _updateConfig(const calendar_manager& data, Blob::ErrorData_t& err);

    /**
     * Callback invocada por lwip/sntp cada vez que reciba una actualizaciï¿½n horaria
     */
    void _ntpUpdateCb();

    void setRtcTime(time_t tnow);

};
     
#endif /*__AstCalendar__H */

/**** END OF FILE ****/


