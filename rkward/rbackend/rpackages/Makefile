all:
	-R CMD build rkward
install:
	-R CMD INSTALL rkward
check:
	-R CMD check rkward
distdir:
	@list=`find rkward -not -name '*~' -and -not -name 'CVS'`; \
	for file in $$list; do \
		echo $$file; \
		if test -d $$file; then \
			mkdir $(distdir)/$$file; \
		else \
			cp -dP $$file $(distdir)/$$file; \
		fi; \
	done; \
	cp -dP Makefile $(distdir)/Makefile
distclean:
clean: