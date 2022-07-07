#ifndef UTIL_URL_H
#define UTIL_URL_H

#include "config.h"
#include "ustring.h"

class MNode;

bool  checkURL (ustring& url, upair* scheme, upair* delim, upair* user, upair* pass, upair* host, upair* port, upair* rest);
bool  checkURLSafe (ustring& url);
bool  checkScheme (ustring& proto);
#ifdef QUERYENCODEALT
ustring  urlencode (uiterator b, uiterator e);
ustring  urlencode_path (uiterator b, uiterator e);
#endif
ustring  buildQuery (MNode* query);

#endif /* UTIL_URL_H */
