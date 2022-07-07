#ifndef UTF8_H
#define UTF8_H

#include "ustring.h"
#include "util_regex.h"

#define  UTF8_SPACE		"\x20"
#define  UTF8_NBSPACE		"\xc2\xa0"
#define  UTF8_ZWSPACE		"\xe2\x80\x8b"
#define  UTF8_WJOINERSPACE	"\xe2\x81\xa0"
#define  UTF8_IDEOSPACE		"\xe3\x80\x80"
#define  UTF8_ZWNBSPACE		"\xef\xbb\xbf"

class  CDBMem;
ustring  translateChar (const ustring& str, CDBMem* cdb);
ustring  fixUTF8 (const ustring& str);
void  nextChar (uiterator& it, const uiterator& end);
void  nextChar (uiterator& it, const uiterator& end, ustring& target);
void  lastChar (const ustring& text, uiterator& ans);
ustring  ellipsis (const ustring& text, int limit);
ustring  logText (const ustring& text);
void  clipEnd (ustring& val, uregex& re1, uregex& re2);
void  clipWhiteEnd (ustring& val);
void  clipNLEnd (ustring& val);

#endif /* UTF8_H */
