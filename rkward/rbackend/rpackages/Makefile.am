SUBDIRS = rkward

all:
	-${R_HOMEDIR}/bin/R CMD build rkward
install:
	-${R_HOMEDIR}/bin/R CMD INSTALL rkward
uninstall:
	-${R_HOMEDIR}/bin/R CMD REMOVE rkward
check:
	-${R_HOMEDIR}/bin/R CMD check rkward
clean \
distclean:
	-rm -f rkward_*.tar.gz
