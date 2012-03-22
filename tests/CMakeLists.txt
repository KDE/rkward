ADD_CUSTOM_TARGET (plugintests
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../rkward/rkward --evaluate "source\\(\\\"all_tests.R\\\"\\)"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# This will hopefully run *after* everything is installed
INSTALL(CODE "MESSAGE(STATUS \"\n--------------------\nInstallation appears to be complete (unfortunately it is hard to tell in CMake).\nIn case of errors, please make sure to consult the file 'INSTALL'.\n--------------------\")")
