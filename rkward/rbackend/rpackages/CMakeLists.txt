MACRO(createRpackage name)
	FILE(GLOB_RECURSE rkwardfiles${name} LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*")
	IF(UNIX AND (${CMAKE_SYSTEM_NAME} STREQUAL "Linux"))
		# For reproducible builds, avoid getting user name/id into the tar. Works with GNU tar, only, unfortunately.
		ADD_CUSTOM_COMMAND(OUTPUT ${name}.tgz
			COMMAND tar "cfz" "${CMAKE_CURRENT_BINARY_DIR}/${name}.tgz" "--owner=0" "--group=0" "--numeric-owner" "${name}"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			DEPENDS ${rkwardfiles${name}})
	ELSE(UNIX AND (${CMAKE_SYSTEM_NAME} STREQUAL "Linux"))
		ADD_CUSTOM_COMMAND(OUTPUT ${name}.tgz
			COMMAND ${CMAKE_COMMAND} -E tar "cfz" "${CMAKE_CURRENT_BINARY_DIR}/${name}.tgz" "${name}"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			DEPENDS ${rkwardfiles${name}})
	ENDIF(UNIX AND (${CMAKE_SYSTEM_NAME} STREQUAL "Linux"))
ENDMACRO()

createRpackage(rkward)
createRpackage(rkwardtests)
ADD_CUSTOM_TARGET(rpackages ALL DEPENDS rkward.tgz rkwardtests.tgz)

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/rkward.tgz" "${CMAKE_CURRENT_BINARY_DIR}/rkwardtests.tgz" DESTINATION ${DATA_INSTALL_DIR}/rkward/rpackages)
