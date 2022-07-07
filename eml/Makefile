.include "../Makefile.conf"
.include "../Makefile.src"

SRCS += app.cc
SRCS += expr.cc
SRCS += form.cc
SRCS += form_utf8.cc
SRCS += formfile.cc
SRCS += ftable.cc
SRCS += http.cc
SRCS += http-iconv.cc
SRCS += mail.cc
SRCS += main.cc
SRCS += ml.cc
SRCS += mlenv.cc
SRCS += ${MSRCS}
SRCS += motor.cc
SRCS += motorenv.cc
SRCS += motoroutput.cc
SRCS += motoroutput-iconv.cc
SRCS += motorvar.cc
SRCS += sigsafe.cc
SRCS += utf8.cc
SRCS += utf16.cc
SRCS += util_apache.cc
SRCS += util_base64.cc
SRCS += util_check.cc
SRCS += util_const.cc
SRCS += util_file.cc
SRCS += util_inet.cc
SRCS += util_mimetype.cc
SRCS += util_proc.cc
SRCS += util_random.cc
SRCS += util_string.cc
SRCS += util_tcp.cc
SRCS += util_time.cc
SRCS += util_url.cc
SRCS += util_wsearch.cc
SRCS += wikiattrib.cc
SRCS += wikimotor.cc
SRCS += wikiformat.cc
SRCS += wikicmd.cc
SRCS += wikienv.cc
SRCS += wikiline.cc
SRCS += wikitable.cc
.ifdef UTF8JP
SRCS += form_utf8-jp.cc
SRCS += iso2022jp.cc
SRCS += motoroutput-jp.cc
SRCS += utf8-jp.cc
SRCS += japanese-hankaku.c
.endif

.PATH: ../eml ../lib ../modules ../wiki ../ext

.ifdef DEBUG
CFLAGS = -g -O0
CFLAGS += -DDEBUG
.ifdef DEBUG2
CFLAGS += -DDEBUG2
.endif
.ifdef HEAPDEBUG
CFLAGS += -DHEAPDEBUG
.ifdef HEAPDEBUG_VERBOSE
CFLAGS += -DHEAPDEBUG_VERBOSE
.endif
.endif
.else
CFLAGS += -O2
.endif
#CFLAGS += -DGCC2
.if defined (BSD)
CFLAGS += -DHAVE_OPENLOCK
.endif
.ifdef Linux
CFLAGS += -DLinux -DNOT_HAVE_D_NAMLEN
.endif
.ifdef UTF8JP
CFLAGS += -DUTF8JP
.endif
.ifdef TARGET_MLDUMP
CFLAGS += -DTARGET_MLDUMP
.endif
.ifdef BDB5
CFLAGS += -DBDB5
.endif
.ifdef BDB5SQLITE
CFLAGS += -DBDB5SQLITE
.endif
.ifdef DEBUG
CFLAGS += -I../eml_debug
.endif
CFLAGS += -I. -I.. -I../eml -I../lib -I../wiki -I../modules -I../ext
CFLAGS += -I/usr/local/include

.ifdef HEAPDEBUG
LDADD += heapdebug.o
.endif
.ifdef UTF8JP
LDADD += ../ext/libcdbl.a
.endif
LDADD += -L/usr/local/lib
LDADD += ${MLDADD}
LDADD += -lboost_regex
LDADD += -lcrypto -lssl
LDADD += -liconv
LDADD += -lcrypt
LDADD += -lm
#LDADD += -lstdc++
LDADD += ${MLDADD2}
LDADD += -pthread
.ifndef PROG_CXX
PROG_CXX = eml
.endif
#NO_MAN = 1
MK_MAN=no
CLEANFILES += ftable.cc ml-id.h wikitable.cc heapdebug.o

${PROG_CXX}: ml-id.h ftable.cc wikitable.cc
.ifdef HEAPDEBUG
${PROG_CXX}: heapdebug.o
.endif

dist: ${PROG_CXX}
	rsync -avH ${PROG_CXX} ../dist/

.PHONY: dist

beforedepend: ml-id.h ftable.cc wikitable.cc

ftable.cc: ${MSRCS} ../Makefile.src
	../bin/MKTABLE.pl -o ftable.cc ${CFLAGS} ${.ALLSRC:M*.cc}

wikitable.cc: wikicmd.cc wikiline.cc
	../bin/MKWTABLE.pl -o wikitable.cc ${.ALLSRC:M*.cc}

ml-id.h: ${MSRCS:M*.cc:R:S/$/.h/g} ../Makefile.src
	../bin/MKID.pl -o ml-id.h ${.ALLSRC:M*.h}

.include <bsd.prog.mk>
