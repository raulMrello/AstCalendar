/*
 * AstCalendar.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	AstCalendar es el módulo encargado de gestionar la configuración del calendario astronómico y de generar
 *	los eventos correspondientes, conforme a dicha configuración
 */
 
#ifndef __AstCalendar__H
#define __AstCalendar__H

#include "mbed.h"
#include "ActiveModule.h"
#include "AstCalendarBlob.h"
#include "RealTimeClock.h"

/** Flag para habilitar el soporte de objetos JSON en las suscripciones a MQLib */
#define ASTCAL_ENABLE_JSON_SUPPORT		1

#if ASTCAL_ENABLE_JSON_SUPPORT == 1
#include "cJSON.h"
#endif


   
class AstCalendar : public ActiveModule {
  public:
              
    /** Constructor por defecto
     * 	@param fs Objeto FSManager para operaciones de backup
     * 	@param defdbg Flag para habilitar depuración por defecto
     */
    AstCalendar(FSManager* fs, bool defdbg = false);


    /** Destructor
     */
    virtual ~AstCalendar(){}


    /** Instala el interfaz RTC
     *  @param rtc Interfaz RTC asociado
     */
    void attachRealTimeClock(RealTimeClock* rtc) {_rtc = rtc; }


    /** Arranca el simulador de eventos basándose en un RtosTimer
     *
     */
    void startSimulator();


    /** Detiene el simulador de eventos basándose en un RtosTimer
     *
     */
    void stopSimulator();


    /** Interfaz para postear un mensaje de la máquina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     *  @return Resultado
     */
    virtual osStatus putMessage(State::Msg *msg);


  private:

    /** Máximo número de mensajes alojables en la cola asociada a la máquina de estados */
    static const uint32_t MaxQueueMessages = 16;

    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	RecvCfgSet 	 = (State::EV_RESERVED_USER << 0),  /// Flag activado al recibir mensaje en "set/cfg"
    	RecvCfgGet	 = (State::EV_RESERVED_USER << 1),  /// Flag activado al recibir mensaje en "get/cfg"
    	RecvBootGet	  = (State::EV_RESERVED_USER << 2),  /// Flag activado al recibir mensaje en "get/boot"
    };


    /** Cola de mensajes de la máquina de estados */
    Queue<State::Msg, MaxQueueMessages> _queue;

    /** Datos de configuración y estado */
    Blob::AstCalBootData_t _astdata;

    /** Timer de simulación de eventos */
    RtosTimer* _sim_tmr;

    /** Contador de segundos del simulador de eventos */
    uint32_t _sim_counter;

    /** Interfaz RealTimeClock */
    RealTimeClock* _rtc;

    /** Hora actual en formato tm */
    tm _now;


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent();


 	/** Interfaz para manejar los eventos en la máquina de estados por defecto
      *  @param se Evento a manejar
      *  @return State::StateResult Resultado del manejo del evento
      */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se);


 	/** Callback invocada al recibir una actualización de un topic local al que está suscrito
      *  @param topic Identificador del topic
      *  @param msg Mensaje recibido
      *  @param msg_len Tamaño del mensaje
      */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);


 	/** Callback invocada al finalizar una publicación local
      *  @param topic Identificador del topic
      *  @param result Resultado de la publicación
      */
    virtual void publicationCb(const char* topic, int32_t result);


   	/** Chequea la integridad de los datos de configuración <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	virtual bool checkIntegrity();


   	/** Establece la configuración por defecto grabándola en memoria NV
	 */
	virtual void setDefaultConfig();


   	/** Recupera la configuración de memoria NV
	 */
	virtual void restoreConfig();


   	/** Graba la configuración en memoria NV
	 */
	virtual void saveConfig();


	/** Graba un parámetro en la memoria NV
	 * 	@param param_id Identificador del parámetro
	 * 	@param data Datos asociados
	 * 	@param size Tamaño de los datos
	 * 	@param type Tipo de los datos
	 * 	@return True: éxito, False: no se pudo recuperar
	 */
	virtual bool saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::saveParameter(param_id, data, size, type);
	}


	/** Recupera un parámetro de la memoria NV
	 * 	@param param_id Identificador del parámetro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tamaño de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: éxito, False: no se pudo recuperar
	 */
	virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::restoreParameter(param_id, data, size, type);
	}


	/** Ejecuta el simulador de eventos
	 *
	 */
	void eventSimulatorCb();


	/** Actualiza la configuración
	 *
	 * @param cfg Nueva configuración a aplicar
	 * @param err Recibe los errores generados durante la actualización
	 */
	void _updateConfig(const Blob::AstCalCfgData_t& cfg, Blob::ErrorData_t& err);


	#if ASTCAL_ENABLE_JSON_SUPPORT == 1
	void* _decodeSetRequest(char* topic, char* json_data, int* data_len);
	void* _decodeGetRequest(char* json_data, int* data_len);
	char* _encodeCfgResponse(uint32_t idtrans);
	char* _encodeStatEvent();
	#endif

};
     
#endif /*__AstCalendar__H */

/**** END OF FILE ****/


