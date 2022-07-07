#ifndef USTRING_H
#define USTRING_H

#include <string>
#include <utility>
#include <unistd.h>

inline char*  char_type (u_char* v) {return (char*)v;}
inline char*  char_type (void* v) {return (char*)v;}
inline char*  char_type (char* v) {return v;}
inline const char*  char_type (const u_char* v) {return (const char*)v;}
//inline u_char*  uchar_type (char* v) {return (u_char*)v;}
inline const u_char*  uchar_type (const char* v) {return (const u_char*)v;}
inline const u_char*  uchar_type (const void* v) {return (const u_char*)v;}
inline char*  noconst_char (const char* v) {return (char*)v;}
#define UCharConst(s)		(uchar_type(s)), (sizeof (s) - 1)
#define CharConst(s)		(s), (sizeof (s) - 1)
#define comapreHead(s)		compare(0, sizeof(s) - 1, (s), sizeof(s) - 1)

typedef std::basic_string<char>		ustring;
typedef ustring::const_iterator		uiterator;
typedef std::pair<ustring::const_iterator, ustring::const_iterator>  upair;

inline int  match (upair& p, const u_char* s, ustring::size_type len) {
    ustring::size_type  n = p.second - p.first;
    return (n == len && memcmp (p.first.base (), s, n) == 0);
}

inline uiterator  find (uiterator b, uiterator e, ustring::value_type ch) {
    for (; b != e; b ++) {
	if (*b == ch)
	    return b;
    }
    return b;
}

#endif /* USTRING_H */
