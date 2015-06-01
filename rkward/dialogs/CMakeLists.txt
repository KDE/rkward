INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )


########### next target ###############

SET(dialogs_STAT_SRCS
   startupdialog.cpp
   rkloadlibsdialog.cpp
   rkreadlinedialog.cpp
   rkimportdialog.cpp
   rkselectlistdialog.cpp
   rkrecoverdialog.cpp
   rkerrordialog.cpp
   )

ADD_LIBRARY(dialogs STATIC ${dialogs_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libdialogs.a
#noinst_HEADERS = startupdialog.h rkloadlibsdialog.h rkreadlinedialog.h rkimportdialog.h
#libdialogs_a_SOURCES = startupdialog.cpp rkloadlibsdialog.cpp rkreadlinedialog.cpp rkimportdialog.cpp
