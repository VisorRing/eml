#ifndef UTF8_JP_H
#define UTF8_JP_H

#include "ustring.h"

ustring  fixFromMS (const ustring& text);
ustring  fixToMS (const ustring& text);
ustring  fullWidthASCIItoASCII (const ustring& text);

#endif /* UTF8_JP_H */
