/*
 * test_AstCalendar.cpp
 *
 *	Test unitario para el m�dulo AstCalendar
 */


//------------------------------------------------------------------------------------
//-- TEST HEADERS --------------------------------------------------------------------
//------------------------------------------------------------------------------------

#include "mbed.h"
#include "AppConfig.h"
#include "mbed_api_userial.h"
#include "FSManager.h"
#include "MQLib.h"
#include "AstCalendar.h"
#include "unity.h"
#include "Heap.h"
#include "cJSON.h"

//------------------------------------------------------------------------------------
//-- REQUIRED HEADERS & COMPONENTS FOR TESTING ---------------------------------------
//------------------------------------------------------------------------------------

/** variables requeridas para realizar el test */
static FSManager* fs=NULL;
static MQ::PublishCallback s_published_cb;
static MQ::SubscribeCallback s_subscribed_cb;
static void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);
static void publishedCb(const char* topic, int32_t result);
static void executePrerequisites();
static bool s_test_done = false;


//------------------------------------------------------------------------------------
//-- SPECIFIC COMPONENTS FOR TESTING -------------------------------------------------
//------------------------------------------------------------------------------------

static AstCalendar* astcal=NULL;
static calendar_manager calendar = {0};
static const char* _MODULE_ = "[TEST_AstCal]...";
#define _EXPR_	(true)



//------------------------------------------------------------------------------------
//-- TEST CASES ----------------------------------------------------------------------
//------------------------------------------------------------------------------------

//---------------------------------------------------------------------------
/**
 * @brief Se verifica la creaci�n del objeto y la suscripci�n a topics
 * MQLib
 */
TEST_CASE("INIT..................................", "[AstCalendar]") {


	//firmwareStart(false);
	fs = FSManager::getStaticInstance();
	TEST_ASSERT_NOT_NULL(fs);

	s_subscribed_cb = callback(&subscriptionCb);
	TEST_ASSERT_EQUAL(MQ::MQClient::subscribe("stat/+/astcal", &s_subscribed_cb), MQ::SUCCESS);

	// crea el objeto
	TEST_ASSERT_NULL(astcal);
    // Crea el gestor del calendario astron�mico
    //  - Un �nico rel� en GPIO_NUM_16, activo a nivel alto y con detecci�n ZC en GPIO_NUM_36 en ambos flancos
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando AstCalendar... ");
	astcal = new AstCalendar(fs, true);
	MBED_ASSERT(astcal);
	astcal->setJSONSupport(false);
	// establece topics base pub-sub
    astcal->setPublicationBase("astcal");
    astcal->setSubscriptionBase("astcal");
    // espera a que arranque
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Esperando a AstCalendar");
    while(!astcal->ready()){
		Thread::wait(10);
	}
	TEST_ASSERT_TRUE(astcal->ready());
	Thread::wait(10000);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "AstCalendar OK!");
}


//---------------------------------------------------------------------------
/**
 * @brief Se verifican las publicaciones y suscripciones JSON, para ello el m�dulo
 * AstCalendar debe ser compilado con la opci�n ASTCAL_ENABLE_JSON_SUPPORT=1 o se debe
 * activar dicha capacidad mediante AstCalendar::setJSONSupport(true)
 */
TEST_CASE("Get Boot Stream.......................", "[AstCalendar]"){

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
	Thread::wait(1000);
	TEST_ASSERT_TRUE(s_test_done);
}


//---------------------------------------------------------------------------
/**
 * @brief Se verifican las publicaciones y suscripciones JSON, para ello el m�dulo
 * AstCalendar debe ser compilado con la opci�n ASTCAL_ENABLE_JSON_SUPPORT=1 o se debe
 * activar dicha capacidad mediante AstCalendar::setJSONSupport(true)
 */
TEST_CASE("Get Config Data.......................", "[AstCalendar]"){
	char* msg;
	MQ::ErrorResult res;
	double count = 0;
	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// solicita la configuraci�n mediante un GetRequest
	Blob::GetRequest_t* greq = new Blob::GetRequest_t(1);
	TEST_ASSERT_NOT_NULL(greq);
	cJSON* jreq = JsonParser::getJsonFromObj(*greq);
	TEST_ASSERT_NOT_NULL(jreq);
	msg = cJSON_PrintUnformatted(jreq);
	TEST_ASSERT_NOT_NULL(msg);
	cJSON_Delete(jreq);
	delete(greq);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Request:\r\nto:get/cfg/astcal\r\nmsg:%s", msg);

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
	Thread::wait(1000);
	TEST_ASSERT_TRUE(s_test_done);
}


//---------------------------------------------------------------------------
/**
 * @brief Se verifican las publicaciones y suscripciones JSON, para ello el m�dulo
 * AstCalendar debe ser compilado con la opci�n ASTCAL_ENABLE_JSON_SUPPORT=1 o se debe
 * activar dicha capacidad mediante AstCalendar::setJSONSupport(true)
 */
TEST_CASE("Set Config Data [partial].............", "[AstCalendar]"){
	MQ::ErrorResult res;
	double count = 0;

	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// actualiza la configuraci�n mediante un SetRequest
	char* msg = "{\"idTrans\":2,\"data\":{\"cfg\":{\"verbosity\":4},\"clock\":{\"cfg\":{\"periods\":[{\"uid\":13,\"since\":10,\"until\":20,\"enabled\":1}],\"geoloc\":{\"coords\":[40.2,-3.2]}}}}}";
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Request:\r\nto:set/cfg/astcal\r\nmsg:%s", msg);
	res = MQ::MQClient::publish("set/cfg/astcal", msg, strlen(msg)+1, &s_published_cb);
	TEST_ASSERT_EQUAL(res, MQ::SUCCESS);

	// wait for response at least 10 seconds, yielding this thread
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Wait 10secs to get a response...");
	count = 0;
	do{
		Thread::wait(100);
		count += 0.1;
	}while(!s_test_done && count < 10);
	Thread::wait(1000);
	TEST_ASSERT_TRUE(s_test_done);
}


//---------------------------------------------------------------------------
/**
 * @brief Se verifican las publicaciones y suscripciones JSON, para ello el m�dulo
 * AstCalendar debe ser compilado con la opci�n ASTCAL_ENABLE_JSON_SUPPORT=1 o se debe
 * activar dicha capacidad mediante AstCalendar::setJSONSupport(true)
 */
TEST_CASE("Set Config Data [full]................", "[AstCalendar]"){
	MQ::ErrorResult res;
	double count = 0;

	// -----------------------------------------------
	// borra flag de resultado
	s_test_done = false;

	// actualiza la configuraci�n mediante un SetRequest
	char* msg = "{\"idTrans\":2,\"data\":{\"cfg\":{\"updFlags\":1,\"evtFlags\":0,\"verbosity\":4},\"clock\":{\"cfg\":{\"periods\":[{\"since\":11,\"until\":21,\"enabled\":0},{\"since\":12,\"until\":22,\"enabled\":1},{\"since\":13,\"until\":23,\"enabled\":0},{\"since\":14,\"until\":24,\"enabled\":1},{\"since\":15,\"until\":25,\"enabled\":0},{\"since\":16,\"until\":26,\"enabled\":1},{\"since\":17,\"until\":27,\"enabled\":0},{\"since\":18,\"until\":28,\"enabled\":1}],\"geoloc\":{\"timezone\":\"GMT-1GMT-2,M3.5.0/2,M10.5.0\",\"coords\":[40.25,-3.25],\"astCorr\":[[1,11],[-2,-22],[3,33],[-4,-44],[5,55],[-6,-66],[7,77],[-8,-88]]}}}}}";
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Request:\r\nto:set/cfg/astcal\r\nmsg:%s", msg);
	res = MQ::MQClient::publish("set/cfg/astcal", msg, strlen(msg)+1, &s_published_cb);
	TEST_ASSERT_EQUAL(res, MQ::SUCCESS);

	// wait for response at least 10 seconds, yielding this thread
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Wait 10secs to get a response...");
	count = 0;
	do{
		Thread::wait(100);
		count += 0.1;
	}while(!s_test_done && count < 10);
	Thread::wait(1000);
	TEST_ASSERT_TRUE(s_test_done);
}


TEST_CASE("Orto  y ocaso ........................", "[AstCalendar]"){
	uint16_t sunrise = 0;
	uint16_t sunset = 0;
	uint8_t isAllDay = 0;
	uint8_t isAllNight = 0;

	int16_t corrSunrise = 0;
	int16_t corrSunset = 0;

	CALENDAR_T cal;
	cal.hour = 11;
	cal.minute = 0;
	cal.second = 0;
	cal.weekday = 0;
	cal.month = 10; //1-12
	cal.date = 27;
	cal.year = 23; //a partir del 2000
	cal._NAN = 0;

	COORD_T lat;
	// 40.528388,-3.6567716 Alcobendas
	lat.Grados = 40;
	lat.Minutos = 32;
	lat.Segundos = 14;
	lat.Signo = 1;
	
	COORD_T lng;
	lng.Grados = -3;
	lng.Minutos = 38;
	lng.Segundos = 14;
	lng.Signo = -1;
	int16_t gmt = 120; // en octubre pasamos a 60

	astcal->Zone_CalculateSuntimes(&cal, gmt, &lat, &lng, corrSunrise, corrSunset, &sunrise, &sunset,&isAllDay, &isAllNight);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "GMT2 - 27-10-2023 Sunrise: %d(%d:%d), Sunset: %d(%d:%d)", sunrise, sunrise/60, sunrise%60, sunset, sunset/60, sunset%60);

	// Probamos el calculo a 1 de noviembre
	cal.month = 11;
	cal.date = 1;
	gmt = 60;
	astcal->Zone_CalculateSuntimes(&cal, gmt, &lat, &lng, corrSunrise, corrSunset, &sunrise, &sunset,&isAllDay, &isAllNight);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "GMT1 - 01-11-2023 Sunrise: %d(%d:%d), Sunset: %d(%d:%d)", sunrise, sunrise/60, sunrise%60, sunset, sunset/60, sunset%60);
	
}
int diffff(struct tm* utc_time , struct tm* local_time ){
	DEBUG_TRACE_E(_EXPR_, _MODULE_, "Diferencia de horas");
	
	bool sameDay = true;
	int gmtDiff = 0;
	int utcMins = 0;
	int localMins = 0;
	int diff_hours = 0;
	
	utcMins = utc_time->tm_hour*60 + utc_time->tm_min;
	localMins = local_time->tm_hour*60 + local_time->tm_min;
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC[dia: %d]: %d:%d -> %d", utc_time->tm_mday, utc_time->tm_hour, utc_time->tm_min, utcMins);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "LCL[dia: %d]: %d:%d -> %d", local_time->tm_mday, local_time->tm_hour, local_time->tm_min, localMins);
	
	if(local_time->tm_mday != utc_time->tm_mday){
		sameDay = false;
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Diferente dia");
		
		if(localMins > utcMins)
			diff_hours = (1440+utcMins) - localMins;
		else
			diff_hours = (1440+localMins) - utcMins;
	}
	else{
		diff_hours = abs(localMins - utcMins);
	}

	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Diferencia: %d", diff_hours);

	return diff_hours;
}


TEST_CASE("Cambios de hora mismo dia ........................", "[AstCalendar]"){
	char timezone[] = "GMT-1GMT-2,M3.5.0/2,M10.5.0";
	tm _now;
	setenv("TZ", timezone, 1);
	tzset() ;
	
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Establece zona horaria '%s'", timezone);
	_now.tm_sec  = 0;
	_now.tm_min  = 0;
	_now.tm_hour = 8;
	_now.tm_mday = 27;
	_now.tm_wday = 5;
	_now.tm_mon  = 9;
	_now.tm_year = 2023 - 1900;
	_now.tm_yday = 300;
	_now.tm_isdst = 1;
	std::time_t utc = cpp_utils::timegm(&_now);
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC read tm_utc: %d", (int)utc);
	

	time_t tnow;
	timeval tv;
	tv.tv_sec = utc;
	tv.tv_usec = 0;

	settimeofday (&tv, NULL);

	tnow = time(NULL);
	localtime_r(&tnow, &_now);
	time_t t = time(NULL);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema");
	tm _gmtime = *gmtime(&t);
	tm _localtime = *localtime(&t);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC:   %s", asctime(&_gmtime));
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "local: %s", asctime(&_localtime));
	int diferencia = diffff(&_gmtime, &_localtime);

	DEBUG_TRACE_I(_EXPR_ ,_MODULE_, "Diferencia en minutos: %d",diferencia);

}

TEST_CASE("Cambios de hora diferente dia ........................", "[AstCalendar]"){
	char timezone[] = "GMT-1GMT-2,M3.5.0/2,M10.5.0";
	tm _now;
	setenv("TZ", timezone, 1);
	tzset() ;
	
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Establece zona horaria '%s'", timezone);
	_now.tm_sec  = 0;
	_now.tm_min  = 0;
	_now.tm_hour = 23;
	_now.tm_mday = 27;
	_now.tm_wday = 5;
	_now.tm_mon  = 9;
	_now.tm_year = 2023 - 1900;
	_now.tm_yday = 300;
	_now.tm_isdst = 1;
	std::time_t utc = cpp_utils::timegm(&_now);
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC read tm_utc: %d", (int)utc);
	

	time_t tnow;
	timeval tv;
	tv.tv_sec = utc;
	tv.tv_usec = 0;

	settimeofday (&tv, NULL);

	tnow = time(NULL);
	localtime_r(&tnow, &_now);
	time_t t = time(NULL);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema");
	tm _gmtime = *gmtime(&t);
	tm _localtime = *localtime(&t);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC:   %s", asctime(&_gmtime));
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "local: %s", asctime(&_localtime));
	int diferencia = diffff(&_gmtime, &_localtime);

	DEBUG_TRACE_I(_EXPR_ ,_MODULE_, "Diferencia en minutos: %d",diferencia);

}

TEST_CASE("Cambios de hora diferente dia 2 ........................", "[AstCalendar]"){
	char timezone[] = "GMT-1GMT-2,M3.5.0/2,M10.5.0";
	tm _now;
	setenv("TZ", timezone, 1);
	tzset() ;
	
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Establece zona horaria '%s'", timezone);
	_now.tm_sec  = 0;
	_now.tm_min  = 0;
	_now.tm_hour = 22;
	_now.tm_mday = 27;
	_now.tm_wday = 5;
	_now.tm_mon  = 9;
	_now.tm_year = 2023 - 1900;
	_now.tm_yday = 300;
	_now.tm_isdst = 1;
	std::time_t utc = cpp_utils::timegm(&_now);
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC read tm_utc: %d", (int)utc);
	

	time_t tnow;
	timeval tv;
	tv.tv_sec = utc;
	tv.tv_usec = 0;

	settimeofday (&tv, NULL);

	tnow = time(NULL);
	localtime_r(&tnow, &_now);
	time_t t = time(NULL);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema");
	tm _gmtime = *gmtime(&t);
	tm _localtime = *localtime(&t);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC:   %s", asctime(&_gmtime));
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "local: %s", asctime(&_localtime));
	int diferencia = diffff(&_gmtime, &_localtime);

	DEBUG_TRACE_I(_EXPR_ ,_MODULE_, "Diferencia en minutos: %d",diferencia);

}

TEST_CASE("Cambios de hora  ........................", "[AstCalendar]"){
	char timezone[] = "GMT-1GMT-2,M3.5.0/2,M10.5.0";
	tm _now;
	setenv("TZ", timezone, 1);
	tzset() ;
	
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Establece zona horaria '%s'", timezone);
	_now.tm_sec  = 0;
	_now.tm_min  = 0;
	_now.tm_hour = 01;
	_now.tm_mday = 28;
	_now.tm_wday = 5;
	_now.tm_mon  = 9;
	_now.tm_year = 2023 - 1900;
	_now.tm_yday = 300;
	_now.tm_isdst = 1;
	std::time_t utc = cpp_utils::timegm(&_now);
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "RTC read tm_utc: %d", (int)utc);
	

	time_t tnow;
	timeval tv;
	tv.tv_sec = utc;
	tv.tv_usec = 0;

	settimeofday (&tv, NULL);

	tnow = time(NULL);
	localtime_r(&tnow, &_now);
	time_t t = time(NULL);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Hora del sistema");
	tm _gmtime = *gmtime(&t);
	tm _localtime = *localtime(&t);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "UTC:   %s", asctime(&_gmtime));
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "local: %s", asctime(&_localtime));
	int diferencia = diffff(&_gmtime, &_localtime);

	DEBUG_TRACE_I(_EXPR_ ,_MODULE_, "Diferencia en minutos: %d",diferencia);

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
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Recibido topic %s con mensaje:", topic);
	cJSON* obj = cJSON_Parse(msg);
	// Print JSON object
	if(obj){
		cJSON_Delete(obj);
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "%s", (char*)msg);
	}
	// Decode depending on the topic
	else{
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error procesando mensaje en topic %s", topic);
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

		// espera a que est� disponible
		while(!MQ::MQBroker::ready()){
			Thread::wait(100);
		}
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "MQLib OK!");

		// registra un manejador de publicaciones com�n
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


