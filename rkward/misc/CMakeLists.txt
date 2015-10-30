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
   rkjobsequence.cpp
   rkspecialactions.cpp
   rkstandardicons.cpp
   rkstandardactions.cpp
   rkxmlguisyncer.cpp
   rkcommandhistory.cpp
   rktableview.cpp
   celleditor.cpp
   editlabelsdialog.cpp
   editformatdialog.cpp
   rkmessagecatalog.cpp
   rkdbusapi.cpp
   rkfindbar.cpp
   rkaccordiontable.cpp
   )

QT4_AUTOMOC(${misc_STAT_SRCS})
# Urghs. QT4_AUTOMOC scans only the .h-file for Q_OBJECT macros. Let's hope this will be better with KF5
QT4_GENERATE_MOC (rkaccordiontable.cpp ${CMAKE_CURRENT_BINARY_DIR}/rkaccordiontablemodel_moc.cpp)

ADD_LIBRARY(misc STATIC ${misc_STAT_SRCS})


########### install files ###############

INSTALL(FILES
	rkdummypart.rc
	DESTINATION ${DATA_INSTALL_DIR}/rkward)
