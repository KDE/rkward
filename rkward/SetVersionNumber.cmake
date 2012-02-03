# set version number
INCLUDE(${CMAKE_SOURCE_DIR}/VERSION.cmake)

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
 
