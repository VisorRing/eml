.include "../Makefile.conf"

.ifdef BDB5SQLITE
.else
SUBDIR += sqlite
.endif
SUBDIR += cdblib

.include <bsd.subdir.mk>

dist:
