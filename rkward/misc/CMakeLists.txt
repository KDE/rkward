INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(misc_STAT_SRCS
   rkspinbox.cpp
   getfilenamewidget.cpp
   rkobjectlistview.cpp
   xmlhelper.cpp
   multistringselector.cpp
   rkcommonfunctions.cpp
   rkprogresscontrol.cpp
   rksaveobjectchooser.cpp
   rkdummypart.cpp
   rkstandardicons.cpp
   rkstandardactions.cpp
   rkxmlguisyncer.cpp
   )

QT4_AUTOMOC(${misc_STAT_SRCS})

ADD_LIBRARY(misc STATIC ${misc_STAT_SRCS})


########### install files ###############

INSTALL(FILES
	rkdummypart.rc
	DESTINATION ${DATA_INSTALL_DIR}/rkward)
