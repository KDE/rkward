INCLUDES = $(all_includes)
METASOURCES = AUTO

AM_CPPFLAGS = -I$(R_INCLUDEDIR)

noinst_LIBRARIES =  librbackend.a
librbackend_a_SOURCES = rembedinternal.cpp rinterface.cpp rthread.cpp rcommand.cpp rcommandreceiver.cpp rcommandstack.cpp \
rkwindowcatcher.cpp rdata.cpp
noinst_HEADERS = rembedinternal.h rinterface.h rthread.h rcommand.h rcommandreceiver.h rcommandstack.h \
rkwindowcatcher.h rdata.h
SUBDIRS =  rpackages
