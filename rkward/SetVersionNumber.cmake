# the IF condition enables manual setting of the version number string
# which allows for different versioning needed for MacPorts
IF(NOT RKVERSION_NUMBER)
	# set version number
	INCLUDE(${CMAKE_SOURCE_DIR}/VERSION.cmake)
ENDIF(NOT RKVERSION_NUMBER)

# replace placeholders with version number in several files
CONFIGURE_FILE(
	version.h.in
	${CMAKE_SOURCE_DIR}/rkward/version.h
	@ONLY)
CONFIGURE_FILE(
	ver.R.in
	${CMAKE_SOURCE_DIR}/rkward/rbackend/rpackages/rkward/R/ver.R
	@ONLY)
CONFIGURE_FILE(
	resource.ver.in
	${CMAKE_SOURCE_DIR}/rkward/resource.ver
	@ONLY)
 
