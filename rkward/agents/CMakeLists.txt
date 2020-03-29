INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )


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

ADD_LIBRARY(agents STATIC ${agents_STAT_SRCS})
TARGET_LINK_LIBRARIES(agents Qt5::Widgets KF5::TextEditor)
