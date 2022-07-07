#ifndef UTIL_CHECK_H
#define UTIL_CHECK_H

#include "ustring.h"
#include "util_regex.h"

#define  UA_Windows	0x00000001
#define  UA_Mac		0x00000002
#define  UA_iPhone	0x00000004
#define  UA_IE		0x00000100
#define  UA_Mozilla	0x00000200
#define  UA_Safari	0x00000400
#define  UA_Opera	0x00000800
#define  UA_NetFront	0x00001000

bool  checkRe (const ustring& name, const uregex& re);
bool  checkRe (uiterator b, uiterator e, const uregex& re);
bool  matchName (const ustring& name);
bool  matchFilename (const ustring& name);
bool  matchResourceName (const ustring& name);
bool  matchAbsoluteResourceName (const ustring& name);
bool  matchASCII (uiterator b, uiterator e);
bool  matchIP (const ustring& name);
bool  matchDomain_dot (const ustring& name);
bool  matchHostname (const ustring& name);
bool  matchMimeType (const ustring& name);
bool  checkMailAddr (const ustring& name);
bool  matchAlNum (uiterator b, uiterator e);
bool  matchNum (uiterator b, uiterator e);			// [0-9]+
inline bool  matchNum (const ustring& text) {
    return matchNum (text.begin (), text.end ());
}
bool  matchWidth (uiterator b, uiterator e);
inline bool  matchWidth (const ustring& text) {
    return matchWidth (text.begin (), text.end ());
}
bool  checkColor (uiterator b, uiterator e);
inline bool  checkColor (const ustring& text) {
    return checkColor (text.begin (), text.end ());
}
bool  matchWikiID (uiterator b, uiterator e);
inline bool  matchWikiID (const ustring& text) {
    return matchWikiID (text.begin (), text.end ());
}
bool  checkAry (const ustring& name);
bool  checkAry (const ustring& name, ustring& sym);
bool  isHTTPS ();
int  checkAgent ();

#endif /* UTIL_CHECK_H */
