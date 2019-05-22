/*
 * test_calendar_objects.cpp
 *
 *	Test unitario para los objetos calendar
 */


//------------------------------------------------------------------------------------
//-- TEST HEADERS --------------------------------------------------------------------
//------------------------------------------------------------------------------------

#include "unity.h"
#include "JsonParserBlob.h"

//------------------------------------------------------------------------------------
//-- REQUIRED HEADERS & COMPONENTS FOR TESTING ---------------------------------------
//------------------------------------------------------------------------------------

#include "AppConfig.h"

/** variables requeridas para realizar el test */
static void executePrerequisites();
static bool s_test_done = false;


//------------------------------------------------------------------------------------
//-- SPECIFIC COMPONENTS FOR TESTING -------------------------------------------------
//------------------------------------------------------------------------------------

static const char* _MODULE_ = "[TEST_calendar].";
#define _EXPR_	(true)

static calendar_manager obj;
static calendar_manager obj_back;
//------------------------------------------------------------------------------------
//-- TEST CASES ----------------------------------------------------------------------
//------------------------------------------------------------------------------------

//---------------------------------------------------------------------------
/** Se verifica la codificación JSON del objeto completo
 *
 */
TEST_CASE("Test Obj->Json->Obj from Whole Obj........................", "[calendar_objs]") {

	// ejecuta requisitos previos
	executePrerequisites();

	obj_back = obj;
	TEST_ASSERT_EQUAL(memcmp(&obj, &obj_back, sizeof(calendar_manager)), 0);

	// Conversión Obj -> JSON
	cJSON* json = JsonParser::getJsonFromObj(obj, ObjSelectAll);
	TEST_ASSERT_NOT_NULL(json);
	char* json_msg = cJSON_PrintUnformatted(json);
	TEST_ASSERT_NOT_NULL(json_msg);
	cJSON_Delete(json);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "%s", json_msg);

	// Conversión JSON -> Obj
	JsonParser::getObjFromJson(obj_back, json_msg);
	json = JsonParser::getJsonFromObj(obj_back, ObjSelectAll);
	TEST_ASSERT_NOT_NULL(json);
	char* json_msg_2 = cJSON_PrintUnformatted(json);
	TEST_ASSERT_NOT_NULL(json_msg_2);
	cJSON_Delete(json);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Obj_back = %s", json_msg_2);
	int result = strcmp(json_msg, json_msg_2);
	free(json_msg_2);
	free(json_msg);

	// Evaluación
	TEST_ASSERT_EQUAL(0, result);
}


//---------------------------------------------------------------------------
/** Se verifica la codificación JSON del estado del objeto
 *
 */
TEST_CASE("Test Obj->Json->Obj from State Obj........................", "[calendar_objs]") {

	// ejecuta requisitos previos
	executePrerequisites();

	obj_back = obj;
	TEST_ASSERT_EQUAL(memcmp(&obj, &obj_back, sizeof(calendar_manager)), 0);

	// Conversión Obj -> JSON
	cJSON* json = JsonParser::getJsonFromObj(obj, ObjSelectState);
	TEST_ASSERT_NOT_NULL(json);
	char* json_msg = cJSON_PrintUnformatted(json);
	TEST_ASSERT_NOT_NULL(json_msg);
	cJSON_Delete(json);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "%s", json_msg);

	// Conversión JSON -> Obj
	JsonParser::getObjFromJson(obj_back, json_msg);
	json = JsonParser::getJsonFromObj(obj_back, ObjSelectState);
	TEST_ASSERT_NOT_NULL(json);
	char* json_msg_2 = cJSON_PrintUnformatted(json);
	TEST_ASSERT_NOT_NULL(json_msg_2);
	cJSON_Delete(json);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Obj_back = %s", json_msg_2);
	int result = strcmp(json_msg, json_msg_2);
	free(json_msg_2);
	free(json_msg);

	// Evaluación
	TEST_ASSERT_EQUAL(0, result);
}


//---------------------------------------------------------------------------
/** Se verifica la codificación JSON de la configuración del objeto
 *
 */
TEST_CASE("Test Obj->Json->Obj from Config Obj.......................", "[calendar_objs]") {

	// ejecuta requisitos previos
	executePrerequisites();

	obj_back = obj;
	TEST_ASSERT_EQUAL(memcmp(&obj, &obj_back, sizeof(calendar_manager)), 0);

	// Conversión Obj -> JSON
	cJSON* json = JsonParser::getJsonFromObj(obj, ObjSelectCfg);
	TEST_ASSERT_NOT_NULL(json);
	char* json_msg = cJSON_PrintUnformatted(json);
	TEST_ASSERT_NOT_NULL(json_msg);
	cJSON_Delete(json);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "%s", json_msg);

	// Conversión JSON -> Obj
	JsonParser::getObjFromJson(obj_back, json_msg);
	json = JsonParser::getJsonFromObj(obj_back, ObjSelectCfg);
	TEST_ASSERT_NOT_NULL(json);
	char* json_msg_2 = cJSON_PrintUnformatted(json);
	TEST_ASSERT_NOT_NULL(json_msg_2);
	cJSON_Delete(json);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Obj_back = %s", json_msg_2);
	int result = strcmp(json_msg, json_msg_2);
	free(json_msg_2);
	free(json_msg);

	// Evaluación
	TEST_ASSERT_EQUAL(0, result);
}





//------------------------------------------------------------------------------------
//-- PREREQUISITES -------------------------------------------------------------------
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
static void executePrerequisites(){
	/** Prerequisites execution control flag */
	static bool s_executed_prerequisites = false;
	if(!s_executed_prerequisites){
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Ready for test!");
		obj = {0};
		// marca flag de prerequisitos completado
		s_executed_prerequisites = true;
	}
}


