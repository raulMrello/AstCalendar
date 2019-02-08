/*
 * test_AstCalendar.cpp
 *
 *	Test unitario para el módulo AstCalendar
 */


//------------------------------------------------------------------------------------
//-- TEST HEADERS --------------------------------------------------------------------
//------------------------------------------------------------------------------------

#include "unity.h"
#include "AstCalendar.h"

//------------------------------------------------------------------------------------
//-- REQUIRED HEADERS & COMPONENTS FOR TESTING ---------------------------------------
//------------------------------------------------------------------------------------

#include "AppConfig.h"
#include "FSManager.h"
#include "cJSON.h"

/** variables requeridas para realizar el test */
static FSManager* fs=NULL;
static MQ::PublishCallback s_published_cb;
static void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);
static void publishedCb(const char* topic, int32_t result);
static void executePrerequisites();
static bool s_test_done = false;


//------------------------------------------------------------------------------------
//-- SPECIFIC COMPONENTS FOR TESTING -------------------------------------------------
//------------------------------------------------------------------------------------

static AstCalendar* astcal=NULL;
static const char* _MODULE_ = "[TEST_AstCal]...";
#define _EXPR_	(true)



//------------------------------------------------------------------------------------
//-- TEST CASES ----------------------------------------------------------------------
//------------------------------------------------------------------------------------

//---------------------------------------------------------------------------
/**
 * @brief Se verifica la creación del objeto y la suscripción a topics
 * MQLib
 */
TEST_CASE("Init & MQLib suscription..............", "[AstCalendar]") {

	// ejecuta requisitos previos
	executePrerequisites();

	// crea el objeto
	TEST_ASSERT_NULL(astcal);
    // Crea el gestor del calendario astronómico
    //  - Un único relé en GPIO_NUM_16, activo a nivel alto y con detección ZC en GPIO_NUM_36 en ambos flancos
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando AstCalendar... ");
	astcal = new AstCalendar(fs, true);
	MBED_ASSERT(astcal);
    astcal->setPublicationBase("astcal");
    astcal->setSubscriptionBase("astcal");
    while(!astcal->ready()){
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Esperando a AstCalendar");
		Thread::wait(1000);
	}
	TEST_ASSERT_TRUE(astcal->ready());
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "AstCalendar OK!");
}


//---------------------------------------------------------------------------
/**
 * @brief Se verifican las publicaciones y suscripciones JSON, para ello el módulo
 * AstCalendar debe ser compilado con la opción ASTCAL_ENABLE_JSON_SUPPORT=1 o se debe
 * activar dicha capacidad mediante AstCalendar::setJSONSupport(true)
 */
TEST_CASE("JSON support .........................", "[AstCalendar]"){

	// activa soporte JSON
	astcal->setJSONSupport(true);
	TEST_ASSERT_TRUE(astcal->isJSONSupported());

	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// solicita la trama de arranque
	char* msg = "{}";
	MQ::ErrorResult res = MQ::MQClient::publish("get/boot/astcal", msg, strlen(msg)+1, &s_published_cb);
	TEST_ASSERT_EQUAL(res, MQ::SUCCESS);

	// wait for response at least 10 seconds, yielding this thread
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Wait 10secs to get a response...");
	double count = 0;
	do{
		Thread::wait(100);
		count += 0.1;
	}while(!s_test_done && count < 10);
	TEST_ASSERT_TRUE(s_test_done);

	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// solicita la configuración mediante un GetRequest
	Blob::GetRequest_t* greq = new Blob::GetRequest_t(1);
	TEST_ASSERT_NOT_NULL(greq);
	cJSON* jreq = JsonParser::getJsonFromObj(*greq);
	TEST_ASSERT_NOT_NULL(jreq);
	msg = cJSON_Print(jreq);
	TEST_ASSERT_NOT_NULL(msg);
	cJSON_Delete(jreq);
	delete(greq);

	res = MQ::MQClient::publish("get/cfg/astcal", msg, strlen(msg)+1, &s_published_cb);
	TEST_ASSERT_EQUAL(res, MQ::SUCCESS);
	Heap::memFree(msg);

	// wait for response at least 10 seconds, yielding this thread
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Wait 10secs to get a response...");
	count = 0;
	do{
		Thread::wait(100);
		count += 0.1;
	}while(!s_test_done && count < 10);
	TEST_ASSERT_TRUE(s_test_done);

	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// actualiza la configuración mediante un SetRequest
	Blob::SetRequest_t<Blob::AstCalCfgData_t> req;
	req.idTrans = 2;
	req.data.updFlagMask = Blob::EnableAstCalCfgUpdNotif;
	req.data.evtFlagMask = Blob::AstCalMinEvt;
	req.data.astCfg.latitude = 40;
	req.data.astCfg.longitude = -3;
	req.data.astCfg.wdowDawnStart = -60;
	req.data.astCfg.wdowDawnStop = 60;
	req.data.astCfg.wdowDuskStart = -120;
	req.data.astCfg.wdowDuskStop = 120;
	req.data.astCfg.reductionStart = 100;
	req.data.astCfg.reductionStop = 200;
	strcpy(req.data.seasonCfg.envText, "GMT-1GMT-2,M3.5.0/2,M10.5.0");

	jreq = JsonParser::getJsonFromSetRequest(reqJsonParser::p_data);
	TEST_ASSERT_NOT_NULL(jreq);
	msg = cJSON_Print(jreq);
	TEST_ASSERT_NOT_NULL(msg);
	cJSON_Delete(jreq);

	res = MQ::MQClient::publish("set/cfg/astcal", msg, strlen(msg)+1, &s_published_cb);
	TEST_ASSERT_EQUAL(res, MQ::SUCCESS);
	Heap::memFree(msg);

	// wait for response at least 10 seconds, yielding this thread
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Wait 10secs to get a response...");
	count = 0;
	do{
		Thread::wait(100);
		count += 0.1;
	}while(!s_test_done && count < 10);
	TEST_ASSERT_TRUE(s_test_done);
}


//---------------------------------------------------------------------------
/**
 * @brief Se verifican las publicaciones y suscripciones JSON, para ello el módulo
 * AstCalendar debe ser compilado con la opción ASTCAL_ENABLE_JSON_SUPPORT=1 o se debe
 * activar dicha capacidad mediante AstCalendar::setJSONSupport(true)
 */
TEST_CASE("Blob support .........................", "[AstCalendar]"){

	// activa soporte JSON
	astcal->setJSONSupport(false);
	TEST_ASSERT_FALSE(astcal->isJSONSupported());

	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// solicita la trama de arranque
	char* msg = "{}";
	MQ::ErrorResult res = MQ::MQClient::publish("get/boot/astcal", msg, strlen(msg)+1, &s_published_cb);
	TEST_ASSERT_EQUAL(res, MQ::SUCCESS);

	// wait for response at least 10 seconds, yielding this thread
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Wait 10secs to get a response...");
	double count = 0;
	do{
		Thread::wait(100);
		count += 0.1;
	}while(!s_test_done && count < 10);
	TEST_ASSERT_TRUE(s_test_done);

	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// solicita la configuración mediante un GetRequest
	Blob::GetRequest_t greq;
	greq.idTrans = 3;
	greq._error.code = Blob::ErrOK;
	greq._error.descr[0] = 0;

	res = MQ::MQClient::publish("get/cfg/astcal", &greq, sizeof(Blob::GetRequest_t), &s_published_cb);
	TEST_ASSERT_EQUAL(res, MQ::SUCCESS);


	// wait for response at least 10 seconds, yielding this thread
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Wait 10secs to get a response...");
	count = 0;
	do{
		Thread::wait(100);
		count += 0.1;
	}while(!s_test_done && count < 10);
	TEST_ASSERT_TRUE(s_test_done);

	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// actualiza la configuración mediante un SetRequest
	Blob::SetRequest_t<Blob::AstCalCfgData_t> sreq;
	sreq.idTrans = 4;
	sreq.keys = Blob::AstCalKeyCfgAll;
	sreq._error.code = Blob::ErrOK;
	sreq._error.descr[0] = 0;
	sreq.data.updFlagMask = Blob::EnableAstCalCfgUpdNotif;
	sreq.data.evtFlagMask = Blob::AstCalMinEvt;
	sreq.data.astCfg.latitude = 40;
	sreq.data.astCfg.longitude = -3;
	sreq.data.astCfg.wdowDawnStart = -60;
	sreq.data.astCfg.wdowDawnStop = 60;
	sreq.data.astCfg.wdowDuskStart = -120;
	sreq.data.astCfg.wdowDuskStop = 120;
	sreq.data.astCfg.reductionStart = 100;
	sreq.data.astCfg.reductionStop = 200;
	strcpy(sreq.data.seasonCfg.envText, "GMT-1GMT-2,M3.5.0/2,M10.5.0");

	res = MQ::MQClient::publish("set/cfg/astcal", &sreq, sizeof(Blob::SetRequest_t<Blob::AstCalCfgData_t>), &s_published_cb);
	TEST_ASSERT_EQUAL(res, MQ::SUCCESS);

	// wait for response at least 10 seconds, yielding this thread
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Wait 10secs to get a response...");
	count = 0;
	do{
		Thread::wait(100);
		count += 0.1;
	}while(!s_test_done && count < 10);
	TEST_ASSERT_TRUE(s_test_done);
}

//------------------------------------------------------------------------------------
//-- PREREQUISITES -------------------------------------------------------------------
//------------------------------------------------------------------------------------


/** Prerequisites execution control flag */
static bool s_executed_prerequisites = false;


//------------------------------------------------------------------------------------
static void publishedCb(const char* topic, int32_t result){

}


//------------------------------------------------------------------------------------
static void subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Recibido topic %s con mensaje:", topic);
	cJSON* obj = cJSON_Parse(msg);
	// Print JSON object
	if(obj){
		cJSON_Delete(obj);
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "%s", (char*)msg);
	}
	// Decode depending on the topic
	else{
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Formando objeto JSON a partir de objeto Blob...");
		if(MQ::MQClient::isTokenRoot(topic, "stat/cfg")){
			if(msg_len == sizeof(Blob::Response_t<Blob::AstCalCfgData_t>)){
				cJSON* obj = JsonParser::getJsonFromResponse(*((Blob::Response_t<Blob::AstCalCfgData_t>*)msg));
				if(obj){
					char* sobj = cJSON_Print(obj);
					cJSON_Delete(obj);
					DEBUG_TRACE_I(_EXPR_, _MODULE_, "%s", sobj);
					Heap::memFree(sobj);
				}
			}
			else if(msg_len == sizeof(Blob::AstCalCfgData_t)){
				cJSON* obj = AstCalendar::encodeCfg(*((Blob::AstCalCfgData_t*)msg));
				if(obj){
					char* sobj = cJSON_Print(obj);
					cJSON_Delete(obj);
					DEBUG_TRACE_I(_EXPR_, _MODULE_, "%s", sobj);
					Heap::memFree(sobj);
				}
			}

		}
		else if(MQ::MQClient::isTokenRoot(topic, "stat/value")){
			if(msg_len == sizeof(Blob::AstCalStatData_t)){
				cJSON* obj = JsonParser::getJsonFromObj(*((Blob::AstCalStatData_t*)msg));
				if(obj){
					char* sobj = cJSON_Print(obj);
					cJSON_Delete(obj);
					DEBUG_TRACE_I(_EXPR_, _MODULE_, "%s", sobj);
					Heap::memFree(sobj);
				}
			}
		}
		else if(MQ::MQClient::isTokenRoot(topic, "stat/boot")){
			if(msg_len == sizeof(Blob::AstCalBootData_t)){
				Blob::AstCalBootData_t* boot = (Blob::AstCalBootData_t*)msg;
				cJSON* obj = JsonParser::getJsonFromObj(*((Blob::AstCalBootData_t*)msg));
				if(obj){
					char* sobj = cJSON_Print(obj);
					cJSON_Delete(obj);
					DEBUG_TRACE_I(_EXPR_, _MODULE_, "%s", sobj);
					Heap::memFree(sobj);
				}
			}
		}
		else{
			DEBUG_TRACE_I(_EXPR_, _MODULE_, "Error procesando mensaje en topic %s", topic);
			s_test_done = false;
			return;
		}
	}
	s_test_done = true;
}


//------------------------------------------------------------------------------------
static void executePrerequisites(){
	if(!s_executed_prerequisites){

		// inicia mqlib
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Init MQLib...");
		MQ::ErrorResult res = MQ::MQBroker::start(64);
		TEST_ASSERT_EQUAL(res, MQ::SUCCESS);

		// espera a que esté disponible
		while(!MQ::MQBroker::ready()){
			Thread::wait(100);
		}
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "MQLib OK!");

		// registra un manejador de publicaciones común
		s_published_cb = callback(&publishedCb);

		// se suscribe a todas las notificaciones de estado: stat/#
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Suscription to stat/#");
		res = MQ::MQClient::subscribe("stat/#", new MQ::SubscribeCallback(&subscriptionCb));
		TEST_ASSERT_EQUAL(res, MQ::SUCCESS);

		// inicia el subsistema NV
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Init FSManager... ");

		fs = new FSManager("fs");
		TEST_ASSERT_NOT_NULL(fs);
		while(!fs->ready()){
			Thread::wait(100);
		}
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "FSManager OK!");

		// marca flag de prerequisitos completado
		s_executed_prerequisites = true;
	}
}


