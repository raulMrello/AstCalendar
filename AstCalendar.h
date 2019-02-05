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


    /** Interfaz para postear un mensaje de la m�quina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     *  @return Resultado
     */
    virtual osStatus putMessage(State::Msg *msg);


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
	 * Codifica la configuraci�n actual en un objeto JSON astcal = {...}
	 * @param cfg Configuraci�n
	 * @return Objeto JSON o NULL en caso de error
	 */
	static cJSON* encodeCfg(const Blob::AstCalCfgData_t& cfg);

	/**
	 * Codifica el estado actual en un objeto JSON astcal = {...}
	 * @param stat Estado
	 * @return Objeto JSON o NULL en caso de error
	 */
	static cJSON* encodeStat(const Blob::AstCalStatData_t& stat);

	/**
	 * Codifica el estado de arranque en un objeto JSON astcal = {...}
	 * @param boot Estado de arranque
	 * @return Objeto JSON o NULL en caso de error
	 */
	static cJSON* encodeBoot(const Blob::AstCalBootData_t& boot);

	/**
	 * Decodifica una operaci�n SetRequest en la que se adjunta la nueva configuraci�n a aplicar
	 * @param req Recibe el objeto decodificado
	 * @param json_data Objeto JSON recibido
	 * @return True si la decodificaci�n es correcta
	 */
	static bool decodeSetRequest(Blob::SetRequest_t<Blob::AstCalCfgData_t>&req, char* json_data);

	/**
	 * Codifica la configuraci�n actual en un objeto JSON solicitado previamente con un idtrans
	 * @param resp Respuesta con la configuraci�n actual
	 * @return Objeto JSON
	 */
	static cJSON* encodeCfgResponse(const Blob::Response_t<Blob::AstCalCfgData_t> &resp);


  private:

    /** M�ximo n�mero de mensajes alojables en la cola asociada a la m�quina de estados */
    static const uint32_t MaxQueueMessages = 16;

    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	RecvCfgSet 	 = (State::EV_RESERVED_USER << 0),  /// Flag activado al recibir mensaje en "set/cfg"
    	RecvCfgGet	 = (State::EV_RESERVED_USER << 1),  /// Flag activado al recibir mensaje en "get/cfg"
    	RecvBootGet	  = (State::EV_RESERVED_USER << 2),  /// Flag activado al recibir mensaje en "get/boot"
    };


    /** Cola de mensajes de la m�quina de estados */
    Queue<State::Msg, MaxQueueMessages> _queue;

    /** Datos de configuraci�n y estado */
    Blob::AstCalBootData_t _astdata;

    /** Timer de simulaci�n de eventos */
    RtosTimer* _sim_tmr;

    /** Contador de segundos del simulador de eventos */
    uint32_t _sim_counter;

    /** Interfaz RealTimeClock */
    RealTimeClock* _rtc;

    /** Hora actual en formato tm */
    tm _now;

    /** Flag de control para el soporte de objetos json */
    bool _json_supported;


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent();


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
	void _updateConfig(const Blob::AstCalCfgData_t& cfg, uint32_t keys, Blob::ErrorData_t& err);


	/**
	 * Codifica la configuraci�n actual en un objeto JSON astcal = {...}
	 * @return Objeto JSON o NULL en caso de error
	 */
	cJSON* _encodeCfg(){
		return encodeCfg(_astdata.cfg);
	}

	/**
	 * Codifica el estado actual en un objeto JSON astcal = {...}
	 * @return Objeto JSON o NULL en caso de error
	 */
	cJSON* _encodeStat(){
		return encodeStat(_astdata.stat);
	}

	/**
	 * Codifica la informaci�n de arranque en un objeto JSON astcal = {...}
	 * @return Objeto JSON o NULL en caso de error
	 */
	cJSON* _encodeBoot(){
		return encodeBoot(_astdata);
	}

};
     
#endif /*__AstCalendar__H */

/**** END OF FILE ****/


