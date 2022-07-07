.include "Makefile.conf"

SUBDIR += ext
SUBDIR += emldump
SUBDIR += eml_debug
SUBDIR += eml
#SUBDIR += cgi_debug
#SUBDIR += cgi

install: dist
	rsync -avH dist/* ${HTCMDDIR}/

.include <bsd.subdir.mk>

.PHONY: dist

dist: _SUBDIR
