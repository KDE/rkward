INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(settings_STAT_SRCS
   rksettings.cpp
   rksettingsmodule.cpp
   rksettingsmodulegeneral.cpp
   rksettingsmoduleoutput.cpp
   rksettingsmodulegraphics.cpp
   rksettingsmoduleplugins.cpp
   rksettingsmoduler.cpp
   rksettingsmodulewatch.cpp
   rksettingsmoduleobjectbrowser.cpp
   rksettingsmoduleconsole.cpp
   rksettingsmodulecommandeditor.cpp
   rksettingsmoduledebug.cpp
   )

QT4_AUTOMOC(${settings_STAT_SRCS})

IF (WIN32 OR USE_BINARY_PACKAGES)
	SET_PROPERTY (
		SOURCE rksettingsmoduler.cpp
		APPEND
		PROPERTY COMPILE_DEFINITIONS
		USE_BINARY_PACKAGES=1
	)
ENDIF (WIN32 OR USE_BINARY_PACKAGES)

ADD_LIBRARY(settings STATIC ${settings_STAT_SRCS})
