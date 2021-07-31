MACRO(createRpackage name)
	FILE(GLOB_RECURSE rkwardfiles${name} LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*")
	ADD_CUSTOM_COMMAND(OUTPUT ${name}.tgz
		COMMAND tar "cfz" "${CMAKE_CURRENT_BINARY_DIR}/${name}.tgz" "--owner=0" "--group=0" "--numeric-owner" "${name}"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${rkwardfiles${name}})
ENDMACRO()

createRpackage(rkward)
createRpackage(rkwardtests)
ADD_CUSTOM_TARGET(rpackages ALL DEPENDS rkward.tgz rkwardtests.tgz)

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/rkward.tgz" "${CMAKE_CURRENT_BINARY_DIR}/rkwardtests.tgz" DESTINATION ${DATA_INSTALL_DIR}/rkward/rpackages)
