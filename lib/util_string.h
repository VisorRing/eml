#ifndef UTIL_STRING_H
#define UTIL_STRING_H

#include "ustring.h"
#include "util_regex.h"
#include <time.h>
#include "iconv_glue.h"
#include <iconv.h>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/lexical_cast.hpp>

class  UIConv {
 public:
    iconv_t  cd;

    UIConv (const char* in, const char* out);
    virtual  ~UIConv () {
	if (cd != ICONV_ERR) {
	    iconv_close (cd);
	    cd = ICONV_ERR;
	}
    };
    virtual ustring  cv (const ustring& text, bool flush = false);
};

ustring  c3 (const ustring& str);
ustring  to_ustring (int32_t v);
ustring  to_ustring (uint32_t v);
ustring  to_ustring (int64_t v);
ustring  to_ustring (uint64_t v);
ustring  to_ustring (long long int v);
ustring  to_ustring (unsigned long long int v);
ustring  to_ustring (double val);
int32_t  to_int32 (const ustring& v);
int64_t  to_int64 (const ustring& v);
uint64_t  to_uint64 (const ustring& v);
double  to_double (const ustring& v);
ustring  percentHEX (int c);
ustring  urldecode_nonul (const ustring& str);
ustring  omitCtrl (const ustring& str);
ustring  omitCtrlX (const ustring& str);
ustring  omitNul (const ustring& str);
ustring  omitNL (const ustring& str);
ustring  omitNonAscii (const ustring& str);
ustring  omitNonAsciiWord (const ustring& str);
ustring  percentEncode (uiterator b, uiterator e);
inline ustring  percentEncode (const ustring& str) {
    return percentEncode (str.begin (), str.end ());
}
ustring  percentEncode_path (uiterator b, uiterator e);
inline ustring  percentEncode_path (const ustring& str) {
    return percentEncode_path (str.begin (), str.end ());
}
ustring  percentDecode (const ustring& str);
ustring  cookieencode (const ustring& text);
ustring  cookiedecode (const ustring& text);
ustring  clipColon (const ustring& text);
ustring  dirPart (const ustring& path);
ustring  filePart_osSafe (const ustring& path);
void  split (uiterator b, uiterator e, uregex& re, std::vector<ustring>& ans);
void  split (uiterator b, uiterator e, int ch, std::vector<ustring>& ans);
void  splitE (uiterator b, uiterator e, uregex& re, std::vector<ustring>& ans);
void  splitE (uiterator b, uiterator e, int ch, std::vector<ustring>& ans);
bool  splitChar (uiterator b, uiterator e, uiterator::value_type ch, uiterator& m1);
ustring  filenameEncode (const ustring& text);
ustring  filenameDecode (const ustring& text);
ustring  escape_re (const ustring& text);
ustring  slashEncode (const ustring& text);
ustring  slashDecode (const ustring& text);
unsigned long  strtoul (const ustring& str);
unsigned long  strtoul (const uiterator& b);
double  strtod (const ustring& str);
long  strtol (const ustring& str);
bool  passMatch (const ustring& pass, const ustring& cpass);
typedef enum {
    FORMAT_MD5,
    FORMAT_BF,
    FORMAT_SHA256,
    FORMAT_SHA512,
}  passCryptFormat;
ustring  passCrypt (const ustring& pass, passCryptFormat format);
size_t  strLength (const ustring& src);
void  substring (const ustring& src, size_t idx, size_t len, int flen, ustring& ans);
ustring  jsEncode (const ustring& str);
bool  matchSkip (uiterator& b, uiterator e, const char* t, size_t s);
bool  matchHead (uiterator& b, uiterator e, const char* t, size_t s);
bool  matchHead (const ustring& str, const char* t, size_t s);
bool  matchHead (const ustring& str, const ustring& head);
bool  match (uiterator b, uiterator e, const char* t, size_t s);
bool  match (const ustring& str, const char* t, size_t s);
bool  match (uiterator b, uiterator e, const ustring& str);
bool  match (const ustring& str, const char* t, size_t s, const char* t2, size_t s2);
ustring  clipWhite (uiterator b, uiterator e);
ustring  clipWhite (const ustring& str);
ustring  getenvString (const char* key);
ustring  zeroPad (int n, const ustring& src);
ustring  padEmpty (const ustring& name);
uint64_t  hextoul (uiterator b, uiterator e);
double  hextod (uiterator b, uiterator e, int base = 16);
ustring  tohex (uint64_t e, int pad = 0, int base = 16, bool upcase = false);
ustring  toCRLF (const ustring& str);
void  skipChar (uiterator& b, uiterator e, int ch);
inline void  skipSpace (uiterator& b, uiterator e) {skipChar (b, e, ' ');}
void  skipNextToChar (uiterator& b, uiterator e, int ch);
ustring  toLower (uiterator b, uiterator e);

ustring  formatDateString (const ustring& format, struct tm& tmv);
ustring  toLower (const ustring& str);
ustring  toUpper (const ustring& str);
ustring  hexEncode (const ustring& data, bool upcase = false);
ustring  hexDecode (const ustring& data);
int  octchar (uiterator b);
ustring  octchar (int c);

bool  findNL (uiterator& b, uiterator e, uiterator& u);
bool  findNLb (uiterator& b, uiterator e);
bool  findChar (uiterator& b, uiterator e, int ch);
bool  findChars (uiterator& b, uiterator e, const ustring& pattern);
bool  findCharFn (uiterator& b, uiterator e, bool (*fn)(int));
bool  findSepColon (uiterator& b, uiterator e, uiterator& u);
bool  matchHeadFn (uiterator& b, uiterator e, bool (*fn)(int));
bool  matchWordTbl (uiterator b, uiterator e, char* tbl);
bool  matchWordFn (uiterator b, uiterator e, bool (*fn)(int));

#endif /* UTIL_STRING_H */
