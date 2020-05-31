INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )

SET(dialogs_STAT_SRCS
   startupdialog.cpp
   rkloadlibsdialog.cpp
   rkreadlinedialog.cpp
   rkimportdialog.cpp
   rkselectlistdialog.cpp
   rkrecoverdialog.cpp
   rkerrordialog.cpp
   rksetupwizard.cpp
   )

ADD_LIBRARY(dialogs STATIC ${dialogs_STAT_SRCS})
TARGET_LINK_LIBRARIES(dialogs Qt5::Widgets KF5::Parts KF5::ConfigWidgets KF5::TextEditor)
