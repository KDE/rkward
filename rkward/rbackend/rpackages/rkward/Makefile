all \
install \
uninstall \
check:

distdir:
	@list=`find . -not -name '*~' -and -not -wholename '*/.svn*'`; \
	for file in $$list; do \
		echo $$file; \
		if test -d $$file; then \
			mkdir $(distdir)/$$file; \
		else \
			cp -dP $$file $(distdir)/$$file; \
		fi; \
	done; \
	cp -dP Makefile $(distdir)/Makefile
clean \
distclean:
	rm -f src/dummylib.o; \
	rm -f src/rkward.so
