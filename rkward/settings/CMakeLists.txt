INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(settings_STAT_SRCS
   rksettings.cpp
   rksettingsmodule.cpp
   rksettingsmodulegeneral.cpp
   rksettingsmoduleoutput.cpp
   rksettingsmodulephp.cpp
   rksettingsmoduleplugins.cpp
   rksettingsmoduler.cpp
   rksettingsmodulewatch.cpp
   rksettingsmoduleobjectbrowser.cpp
   rksettingsmoduleconsole.cpp
   rksettingsmodulecommandeditor.cpp
   rksettingsmoduledebug.cpp
   )

QT4_AUTOMOC(${settings_STAT_SRCS})

ADD_LIBRARY(settings STATIC ${settings_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libsettings.a
#libsettings_a_SOURCES = rksettings.cpp rksettingsmodule.cpp \
#	rksettingsmodulegeneral.cpp rksettingsmoduleoutput.cpp rksettingsmodulephp.cpp \
#	rksettingsmoduleplugins.cpp rksettingsmoduler.cpp rksettingsmodulewatch.cpp \
#	rksettingsmoduleobjectbrowser.cpp rksettingsmoduleconsole.cpp
#noinst_HEADERS = rksettings.h rksettingsmodule.h rksettingsmodulegeneral.h \
#	rksettingsmoduleoutput.h rksettingsmodulephp.h rksettingsmoduleplugins.h rksettingsmoduler.h \
#	rksettingsmodulewatch.h rksettingsmoduleobjectbrowser.h rksettingsmoduleconsole.h
