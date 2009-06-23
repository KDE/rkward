ADD_CUSTOM_TARGET (plugintests
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../rkward/rkward --evaluate 'source (\"import_export_plugins.R\")'
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})