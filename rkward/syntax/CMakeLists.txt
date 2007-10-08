INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES   r.xml rkward.xml DESTINATION /share/apps/katepart/syntax/ )



#original Makefile.am contents follow:

#EXTRA_DIST = r-syntax-test.R
#
#katesyntaxdir = $(kde_datadir)/katepart/syntax/
#katesyntax_DATA =  r.xml rkward.xml
