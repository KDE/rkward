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

ADD_LIBRARY(settings STATIC ${settings_STAT_SRCS})

# NOTE: KNewStuff3 does not exist prior to KDE 4.4, and is buggy, at least up to KDE 4.5.x (does not properly list uninstalled files).
# Check again, when it becomes usable, and insert the proper version check, below.
#MACRO_ENSURE_VERSION("4.6.0" ${KDE_VERSION} RKWARD_USE_KNS3)
IF(RKWARD_USE_KNS3)
	ADD_DEFINITIONS(-DRKWARD_USE_KNS3)
	SET(KNS_LIBS ${KDE4_KNEWSTUFF3_LIBS})
ELSE(RKWARD_USE_KNS3)
	SET(KNS_LIBS ${KDE4_KNEWSTUFF2_LIBS})
ENDIF(RKWARD_USE_KNS3)

INSTALL(FILES rkward.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
