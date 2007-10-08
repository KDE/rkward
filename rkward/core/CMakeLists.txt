INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(core_STAT_SRCS
   rkvariable.cpp
   robjectlist.cpp
   robject.cpp
   rcontainerobject.cpp
   rkmodificationtracker.cpp
   rfunctionobject.cpp
   renvironmentobject.cpp
   )

KDE3_AUTOMOC(${core_STAT_SRCS})

ADD_LIBRARY(core STATIC ${core_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libcore.a
#libcore_a_SOURCES = rkvariable.cpp robjectlist.cpp robject.cpp rcontainerobject.cpp rkmodificationtracker.cpp \
#	rfunctionobject.cpp renvironmentobject.cpp
#noinst_HEADERS = rkvariable.h robjectlist.h robject.h rcontainerobject.h rkmodificationtracker.h \
#	rfunctionobject.h renvironmentobject.h
