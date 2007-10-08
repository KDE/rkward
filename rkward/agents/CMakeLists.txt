INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(agents_STAT_SRCS
   rksaveagent.cpp
   rkloadagent.cpp
   showedittextfileagent.cpp
   rkquitagent.cpp
   rkeditobjectagent.cpp
   )

QT4_AUTOMOC(${agents_STAT_SRCS})

ADD_LIBRARY(agents STATIC ${agents_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libagents.a
#noinst_HEADERS = rksaveagent.h rkloadagent.h showedittextfileagent.h rkquitagent.h rkeditobjectagent.h
#libagents_a_SOURCES = rksaveagent.cpp rkloadagent.cpp showedittextfileagent.cpp rkquitagent.cpp  rkeditobjectagent.cpp
