INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(agents_STAT_SRCS
   rkdebughandler.cpp
   rkeditobjectagent.cpp
   rkloadagent.cpp
   rkprintagent.cpp
   rkquitagent.cpp
   rksaveagent.cpp
   showedittextfileagent.cpp
   )

QT4_AUTOMOC(${agents_STAT_SRCS})

ADD_LIBRARY(agents STATIC ${agents_STAT_SRCS})
