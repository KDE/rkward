INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )

SET(scriptbackends_STAT_SRCS
   scriptbackend.cpp
   simplebackend.cpp
   qtscriptbackend.cpp
   qtscripti18n.cpp
   rkcomponentscripting.cpp
   )

ADD_LIBRARY(scriptbackends STATIC ${scriptbackends_STAT_SRCS})
TARGET_LINK_LIBRARIES(scriptbackends Qt5::Script Qt5::Widgets Qt5::Xml KF5::WidgetsAddons KF5::I18n)

INSTALL(FILES common.js rkcomponentscripting.js DESTINATION ${DATA_INSTALL_DIR}/rkward/phpfiles )
