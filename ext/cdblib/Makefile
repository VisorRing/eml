#SRCS += alloc.c
SRCS += buffer.c
SRCS += buffer_0.c
SRCS += buffer_1.c
SRCS += buffer_1s.c
SRCS += buffer_2.c
SRCS += buffer_copy.c
SRCS += buffer_get.c
SRCS += buffer_put.c
SRCS += byte_copy.c
SRCS += byte_cr.c
SRCS += byte_diff.c
SRCS += cdb.c
SRCS += cdb_mem.c
SRCS += cdb_hash.c
SRCS += open_read.c
SRCS += open_trunc.c
SRCS += seek_cur.c
SRCS += seek_set.c
SRCS += str_len.c
SRCS += strerr_die.c
SRCS += strerr_sys.c
SRCS += uint32_pack.c
SRCS += uint32_unpack.c

CFLAGS = -g
CFLAGS += -I../cdblib -I../cdb-0.75 -I. -I..

LIB = cdbl

#NOPIC=
#NOPROFILE=
#NO_PIC=
#NO_PROFILE=
MK_PROFILE=no

CLEANFILES += .depend ../cdb.h ../libcdbl.a

.PATH:../cdb-0.75

all: ../cdb.h ../libcdbl.a

../cdb.h:	cdb.h
	cp cdb.h ..

../libcdbl.a: libcdbl.a
	cp libcdbl.a ..

beforedepend: ../cdb.h

dist:

.include <bsd.lib.mk>
