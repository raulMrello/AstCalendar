idf_component_register(	SRCS "AstCalendar.cpp" 
							"AstCalendar_NVStore.cpp"
							"AstCalendar_StateMachine.cpp"
							"AstCalendar_Subscriptions.cpp"
							"calendar_objects.cpp"
						INCLUDE_DIRS "."
						REQUIRES Blob ActiveModule mbed_api_esp32 RealTimeClock mbed_api_userial cpp_utils)