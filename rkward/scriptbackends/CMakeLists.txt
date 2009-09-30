INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(scriptbackends_STAT_SRCS
   phpbackend.cpp
   scriptbackend.cpp
   simplebackend.cpp
   qtscriptbackend.cpp
   )

QT4_AUTOMOC(${scriptbackends_STAT_SRCS})

ADD_LIBRARY(scriptbackends STATIC ${scriptbackends_STAT_SRCS})


########### install files ###############

INSTALL(FILES common.php php.ini common.js DESTINATION ${DATA_INSTALL_DIR}/rkward/phpfiles )



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libscriptbackends.a
#noinst_HEADERS =  phpbackend.h scriptbackend.h simplebackend.h
#libscriptbackends_a_SOURCES =  phpbackend.cpp scriptbackend.cpp simplebackend.cpp
#phpfilesdir = $(kde_datadir)/rkward/phpfiles
#phpfiles_DATA = common.php php.ini
#
