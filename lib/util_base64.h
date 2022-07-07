#ifndef UTIL_BASE64_H
#define UTIL_BASE64_H

#include "ustring.h"

ustring  base64Encode (uiterator b, uiterator e);
ustring  base64Decode (uiterator b, uiterator e);
ustring  base64URLEncode (uiterator b, uiterator e);
ustring  base64URLDecode (uiterator b, uiterator e);

#endif /* UTIL_BASE64_H */
