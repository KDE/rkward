ADD_SUBDIRECTORY( rkward ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = rkward
#
#all:
#	-${R_HOMEDIR}/bin/R CMD build rkward
#install:
#	-mkdir -p tmp ; \
#	OLDTMP=$$TMPDIR ; \
#	export TMPDIR=`pwd`/tmp ; \
#	mkdir -p $(DESTDIR)/$(R_LIBDIR) ; \
#	${R_HOMEDIR}/bin/R CMD INSTALL -l $(DESTDIR)/$(R_LIBDIR) rkward ; \
#	export TMPDIR=$$OLDTMP
#uninstall:
#	-${R_HOMEDIR}/bin/R CMD REMOVE -l $(DESTDIR)/$(R_LIBDIR) rkward
#check:
#	-${R_HOMEDIR}/bin/R CMD check rkward
#clean \
#distclean:
#	-rm -f rkward_*.tar.gz
#	-rm -rf tmp
