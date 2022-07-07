SRCDIR = sqlite-autoconf-3370200+

all:	../sqlite3.h ../libsqlite3.a

../sqlite3.h: ${SRCDIR}/sqlite3.h
	cp ${SRCDIR}/sqlite3.h ../

../libsqlite3.a: ${SRCDIR}/.libs/libsqlite3.a
	cp ${SRCDIR}/.libs/libsqlite3.a ../

${SRCDIR}/.libs/libsqlite3.a:
	(cd ${SRCDIR}; CFLAGS="-O2 -DSQLITE_OMIT_UTF16 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_DEPRECATED" ./configure; make)

.PHONY: clean cleandepend dist install

clean:
	-rm -f ../libsqlite3.a ../sqlite3.h
	-(cd ${SRCDIR}; make distclean)

depend: ../sqlite3.h

cleandepend:

dist:

install:
