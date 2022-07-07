#ifndef UTIL_MIMETYPE_H
#define UTIL_MIMETYPE_H

#include "ustring.h"

ustring  getExt (const ustring& name);
ustring  mimetype (const ustring& ext, bool f = true);

#endif /* UTIL_MIMETYPE_H */
