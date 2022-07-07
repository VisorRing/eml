#ifndef UTF16_H
#define UTF16_H

#include "ustring.h"

ustring  utf8to16 (const ustring& src);
std::wstring  utow (const ustring& src);
ustring  wtou (const std::wstring& src);

#endif /* UTF16_H */
