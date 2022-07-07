#ifndef SPAIR_H
#define SPAIR_H

#include "ustring.h"
#include <string>
#include <iterator>
#include <utility>
#include <string.h>

#define  CharConst(s)		(s), (sizeof (s) - 1)

typedef std::pair<std::string::const_iterator, std::string::const_iterator>  spair;

inline int  match (spair& p, char* s, int len) {
    int  n = p.second - p.first;
    return (n == len && memcmp (p.first.base (), s, n) == 0);
}

#endif /* SPAIR_H */
