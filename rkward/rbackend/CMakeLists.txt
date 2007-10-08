ADD_SUBDIRECTORY( rpackages ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

INCLUDE(FindR.cmake)
INCLUDE(DetectPthreads.cmake)

########### next target ###############

SET(rbackend_STAT_SRCS
   rembedinternal.cpp
   rinterface.cpp
   rthread.cpp
   rcommand.cpp
   rcommandreceiver.cpp
   rcommandstack.cpp
   rdata.cpp
   rkpthreadsupport.cpp
   rklocalesupport.cpp
   )

KDE3_AUTOMOC(${rbackend_STAT_SRCS})

ADD_LIBRARY(rbackend STATIC ${rbackend_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#
#AM_CPPFLAGS = -I$(R_INCLUDEDIR)
#
#noinst_LIBRARIES =  librbackend.a
#librbackend_a_SOURCES = rembedinternal.cpp rinterface.cpp rthread.cpp rcommand.cpp rcommandreceiver.cpp rcommandstack.cpp \
#rdata.cpp rkpthreadsupport.cpp rklocalesupport.cpp
#noinst_HEADERS = rembedinternal.h rinterface.h rthread.h rcommand.h rcommandreceiver.h rcommandstack.h \
#rdata.h rkpthreadsupport.h rklocalesupport.h
#SUBDIRS =  rpackages
#
#rembedinternal.o: rkstructuregetter.cpp