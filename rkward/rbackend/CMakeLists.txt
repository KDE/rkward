INCLUDE(FindR.cmake)
INCLUDE(DetectPthreads.cmake)

ADD_SUBDIRECTORY( rpackages ) 

INCLUDE_DIRECTORIES( ${R_INCLUDEDIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

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
TARGET_LINK_LIBRARIES(rbackend ${R_USED_LIBS})
LINK_DIRECTORIES(${R_SHAREDLIBDIR})
