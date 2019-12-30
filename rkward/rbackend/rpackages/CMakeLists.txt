MACRO(createRpackage name)
	FILE(GLOB_RECURSE rkwardfiles${name} LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*")
	MESSAGE(STATUS ${rkwardfiles${name}})
	ADD_CUSTOM_COMMAND(OUTPUT ${name}.tgz
		COMMAND ${CMAKE_COMMAND} -E tar "cfz" "${CMAKE_CURRENT_BINARY_DIR}/${name}.tgz" "${name}"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${rkwardfiles${name}})
ENDMACRO()

createRpackage(rkward)
createRpackage(rkwardtests)
ADD_CUSTOM_TARGET(rpackages ALL DEPENDS rkward.tgz rkwardtests.tgz)

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/rkward.tgz" "${CMAKE_CURRENT_BINARY_DIR}/rkwardtests.tgz" DESTINATION ${DATA_INSTALL_DIR}/rkward/rpackages)
