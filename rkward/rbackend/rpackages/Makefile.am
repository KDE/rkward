SUBDIRS = rkward

all:
	-${R_HOMEDIR}/bin/R CMD build rkward
install:
	-mkdir -p tmp ; \
	OLDTMP=$$TMPDIR ; \
	export TMPDIR=`pwd`/tmp ; \
	${R_HOMEDIR}/bin/R CMD INSTALL -l $(DESTDIR)/$(R_LIBDIR) rkward ; \
	export TMPDIR=$$OLDTMP
uninstall:
	-${R_HOMEDIR}/bin/R CMD REMOVE -l $(DESTDIR)/$(R_LIBDIR) rkward
check:
	-${R_HOMEDIR}/bin/R CMD check rkward
clean \
distclean:
	-rm -f rkward_*.tar.gz
	-rm -rf tmp
