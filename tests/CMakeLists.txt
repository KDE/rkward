ADD_CUSTOM_TARGET (plugintests
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../rkward/rkward --evaluate "source\\(\\\"all_tests.R\\\"\\)"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# This will hopefully run *after* everything is installed
INSTALL(CODE "MESSAGE(STATUS \"\n--------------------\nInstallation appears to be complete (unfortunately it is hard to tell in CMake).\nConsider running\n    make plugintests\nfor some automated functionality tests.\nIn case of errors, consult the file 'INSTALL'.\n--------------------\")")
