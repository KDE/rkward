INCLUDE(FindR.cmake)
INCLUDE(DetectPthreads.cmake)

ADD_SUBDIRECTORY( rpackages ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

########### next target ###############

SET(
	rbackend_STAT_SRCS
	rembedinternal.cpp
	rinterface.cpp
	rthread.cpp
	rcommand.cpp
	rcommandreceiver.cpp
	rcommandstack.cpp
	rdata.cpp
	rkpthreadsupport.cpp
	rksignalsupport.cpp
	rklocalesupport.cpp
)


QT4_AUTOMOC(${rbackend_STAT_SRCS})

ADD_LIBRARY(rbackend STATIC ${rbackend_STAT_SRCS})

SET_SOURCE_FILES_PROPERTIES(
	rembedinternal.cpp
	PROPERTIES COMPILE_FLAGS "-fno-strict-aliasing ${R_CPPFLAGS}"
)